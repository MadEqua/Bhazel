#include "bzpch.h"

#include "Renderer2D.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Core/Application.h"
#include "Core/Utils.h"
#include "Renderer/ParticleSystem2D.h"
#include "Camera.h"


namespace BZ {

    Renderer2DStats Renderer2D::stats;

    constexpr uint32 MAX_RENDERER2D_SPRITES = 100'000;

    static DataLayout vertexLayout = {
        { DataType::Float32, DataElements::Vec2, "POSITION" },
        { DataType::Uint16, DataElements::Vec2, "TEXCOORD", true },
        { DataType::Uint32, DataElements::Scalar, "COLOR"},
    };

    static DataLayout indexLayout = {
        {DataType::Uint32, DataElements::Scalar, ""},
    };

    struct Vertex {
        float pos[2];
        uint16 texCoord[2];
        uint32 colorAndAlpha;
    };

    constexpr uint16 UINT16_MAX_VALUE = 0xffff;
    static Vertex quadVertices[4] = {
        {
            { -0.5f, -0.5f },
            { 0, 0 },
            0
        },
        {
            { 0.5f, -0.5f },
            { UINT16_MAX_VALUE, 0 },
            0
        },
        {
            { 0.5f, 0.5f },
            { UINT16_MAX_VALUE, UINT16_MAX_VALUE },
            0
        },
        {
            { -0.5f, 0.5f },
            { 0, UINT16_MAX_VALUE },
            0
        }
    };

    static uint32 quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

    struct InternalSprite {
        glm::vec2 position;
        glm::vec2 dimensions;
        float rotationDeg;
        glm::vec4 tintAndAlpha;
        uint64 textureHash;
        uint64 sortKey;
    };

    struct TexData {
        Ref<TextureView> textureView;
        Ref<DescriptorSet> descriptorSet;
    };

    static struct Renderer2DData {
        uint32 commandBufferId;

        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;

        BufferPtr vertexBufferPtr;
        BufferPtr indexBufferPtr;

        Ref<PipelineState> pipelineState;
        Ref<Texture2D> whiteTexture;
        Ref<Sampler> sampler;
        Ref<DescriptorSetLayout> descriptorSetLayout;

        std::unordered_map<uint64, TexData> texDataStorage;

        InternalSprite sprites[MAX_RENDERER2D_SPRITES + 1];
        uint32 nextSprite;
    } rendererData;


    static uint64 initTexture(const Ref<Texture2D>& texture) {
        uint64 hash = reinterpret_cast<uint64>(texture.get()); //TODO: something better
        if (rendererData.texDataStorage.find(hash) == rendererData.texDataStorage.end()) {
            auto texViewRef = TextureView::create(texture);
            auto descSetRef = DescriptorSet::create(rendererData.descriptorSetLayout);
            descSetRef->setSampler(rendererData.sampler, 0);
            descSetRef->setSampledTexture(texViewRef, 1);
            rendererData.texDataStorage.emplace(hash, TexData{ texViewRef, descSetRef });
        }
        return hash;
    }

    void Renderer2D::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer2D");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/TextureVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/TextureFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        rendererData.vertexBuffer = Buffer::create(BufferType::Vertex, 4 * sizeof(Vertex) * MAX_RENDERER2D_SPRITES, MemoryType::CpuToGpu, vertexLayout);
        rendererData.indexBuffer = Buffer::create(BufferType::Index, 6 * sizeof(uint32) * MAX_RENDERER2D_SPRITES, MemoryType::CpuToGpu, indexLayout);

        rendererData.vertexBufferPtr = rendererData.vertexBuffer->map(0);
        rendererData.indexBufferPtr = rendererData.indexBuffer->map(0);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        rendererData.sampler = samplerBuilder.build();

        byte whiteTextureData[] = {255, 255, 255, 255};
        rendererData.whiteTexture = Texture2D::create(whiteTextureData, sizeof(whiteTextureData), 1, 1, TextureFormat::R8G8B8A8, false);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::Sampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::SampledTexture, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.descriptorSetLayout = descriptorSetLayoutBuilder.build();

        const auto WINDOW_DIMS_INT = Application::getInstance().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::getInstance().getWindow().getDimensionsFloat();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingStateAttachment.enableBlending = true;
        blendingStateAttachment.srcColorBlendingFactor = BlendingFactor::SourceAlpha;
        blendingStateAttachment.dstColorBlendingFactor = BlendingFactor::OneMinusSourceAlpha;
        blendingStateAttachment.colorBlendingOperation = BlendingOperation::Add;
        blendingStateAttachment.srcAlphaBlendingFactor = BlendingFactor::SourceAlpha;
        blendingStateAttachment.dstAlphaBlendingFactor = BlendingFactor::OneMinusSourceAlpha;
        blendingStateAttachment.alphaBlendingOperation = BlendingOperation::Add;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        pipelineStateData.dataLayout = vertexLayout;

        //Push constants are used to pass tint and alpha values
        //PushConstantDesc pushConstantDesc;
        //pushConstantDesc.offset = 0;
        //pushConstantDesc.size = sizeof(glm::vec4);
        //pushConstantDesc.shaderStageMask = flagsToMask(ShaderStageFlags::Fragment);

        //pipelineStateData.pushConstantDescs = { pushConstantDesc };
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };
        pipelineStateData.descriptorSetLayouts = { rendererData.descriptorSetLayout };
        pipelineStateData.blendingState = blendingState;
        rendererData.pipelineState = PipelineState::create(pipelineStateData);
    }

    void Renderer2D::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.vertexBuffer->unmap();
        rendererData.indexBuffer->unmap();

        rendererData.vertexBuffer.reset();
        rendererData.indexBuffer.reset();

        rendererData.texDataStorage.clear();

        rendererData.pipelineState.reset();
        rendererData.sampler.reset();
        rendererData.whiteTexture.reset();
        rendererData.descriptorSetLayout.reset();
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId == -1, "There's already an unended Scene!");

        rendererData.nextSprite = 0;

        memset(&stats, 0, sizeof(stats));

        rendererData.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(rendererData.commandBufferId, rendererData.pipelineState, camera.getTransform().getTranslation(), camera.getViewMatrix(), camera.getProjectionMatrix());
    }

    void Renderer2D::endScene() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");

        if (rendererData.nextSprite > 0) {
            //Sort objects to minimize state changes when rendering. Sorted by Texture and then by tint.
            std::sort(rendererData.sprites, rendererData.sprites + rendererData.nextSprite, [](const InternalSprite &a, const InternalSprite &b) {
                return a.sortKey < b.sortKey;
            });

            Graphics::bindBuffer(rendererData.commandBufferId, rendererData.vertexBuffer, 0);
            Graphics::bindBuffer(rendererData.commandBufferId, rendererData.indexBuffer, 0);
            Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.pipelineState);

            uint32 spritesInBatch = 0;
            uint32 nextBatchOffset = 0;
            uint64 currentBoundTexHash = -1;
            //glm::vec4 currentActiveTint = glm::vec4(-1.0f);
            uint64 currentBatchTexHash = rendererData.sprites[0].textureHash;
            //glm::vec4 currentBatchTint = rendererData.sprites[0].tintAndAlpha;

            //Generate vertex and index buffers and record commands
            for (uint32 objIdx = 0; objIdx <= rendererData.nextSprite; ++objIdx) {
                const InternalSprite &spr = rendererData.sprites[objIdx];

                //We iterate past last object to finish the current batch on that case.
                bool isLastIteration = objIdx == rendererData.nextSprite;

                if (!isLastIteration) {
                    //Buffer generation
                    float c = glm::cos(glm::radians(spr.rotationDeg));
                    float s = glm::sin(glm::radians(spr.rotationDeg));

                    Vertex vertices[4];
                    uint32 indices[6];
                    uint32 packedColor = Utils::packColor(spr.tintAndAlpha);
                    for (int i = 0; i < 4; ++i) {
                        vertices[i].pos[0] = quadVertices[i].pos[0] * spr.dimensions.x * c + quadVertices[i].pos[1] * spr.dimensions.y * -s + spr.position.x;
                        vertices[i].pos[1] = quadVertices[i].pos[0] * spr.dimensions.x * s + quadVertices[i].pos[1] * spr.dimensions.y * c + spr.position.y;
                        vertices[i].texCoord[0] = quadVertices[i].texCoord[0];
                        vertices[i].texCoord[1] = quadVertices[i].texCoord[1];
                        vertices[i].colorAndAlpha = packedColor;
                    }

                    for (int i = 0; i < 6; ++i) {
                        indices[i] = quadIndices[i] + (objIdx * 4);
                    }

                    uint32 offset = objIdx * sizeof(vertices);
                    memcpy(rendererData.vertexBufferPtr + offset, &vertices, sizeof(vertices));

                    offset = objIdx * sizeof(indices);
                    memcpy(rendererData.indexBufferPtr + offset, &indices, sizeof(indices));
                }

                //Command recording
                bool texChanged = currentBatchTexHash != spr.textureHash;
                //bool tintChanged = currentBatchTint != spr.tintAndAlpha;

                //Batch finishes on these cases. Issue draw call.
                if (texChanged  || isLastIteration) {
                    if (currentBoundTexHash != currentBatchTexHash) {
                        const TexData& texData = rendererData.texDataStorage[currentBatchTexHash];
                        Graphics::bindDescriptorSet(rendererData.commandBufferId, texData.descriptorSet, rendererData.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);
                        currentBoundTexHash = currentBatchTexHash;
                        stats.descriptorSetBindCount++;
                    }

                    //if (currentActiveTint != currentBatchTint) {
                    //    Graphics::setPushConstants(rendererData.commandBufferId, rendererData.pipelineState, flagsToMask(ShaderStageFlags::Fragment), &currentBatchTint.x, sizeof(glm::vec4), 0);
                    //    currentActiveTint = currentBatchTint;
                    //    stats.tintPushCount++;
                    //}

                    Graphics::drawIndexed(rendererData.commandBufferId, spritesInBatch * 6, 1, nextBatchOffset * 6, 0, 0);
                    nextBatchOffset = objIdx;
                    spritesInBatch = 0;

                    currentBatchTexHash = spr.textureHash;
                    //currentBatchTint = spr.tintAndAlpha;

                    stats.drawCallCount++;
                }
                spritesInBatch++;
            }
        }

        Graphics::endCommandBuffer(rendererData.commandBufferId);
        rendererData.commandBufferId = -1;
    }

    void Renderer2D::drawSprite(const Sprite &sprite) {
        BZ_PROFILE_FUNCTION();

        drawQuad(sprite.position, sprite.dimensions, sprite.rotationDeg, sprite.texture, sprite.tintAndAlpha);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec4 &colorAndAlpha) {
        BZ_PROFILE_FUNCTION();

        drawQuad(position, dimensions, rotationDeg, rendererData.whiteTexture, colorAndAlpha);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec4 &tintAndAlpha) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");
        BZ_ASSERT_CORE(rendererData.nextSprite < MAX_RENDERER2D_SPRITES, "nextSprite exceeded MAX_RENDERER2D_SPRITES!");

        InternalSprite &spr = rendererData.sprites[rendererData.nextSprite++];
        spr.position = position;
        spr.dimensions = dimensions;
        spr.rotationDeg = rotationDeg;
        spr.textureHash = initTexture(texture);
        spr.tintAndAlpha = tintAndAlpha;

        //Sort by Texture and then by tint
        //std::hash<double> hasher;
        //size_t tintHash = Utils::hashCombine(Utils::hashCombine(Utils::hashCombine(hasher(tintAndAlpha.r), hasher(tintAndAlpha.g)), hasher(tintAndAlpha.b)), hasher(tintAndAlpha.a));
        //spr.sortKey = (spr.textureHash << 32) | (tintHash >> 32);
        spr.sortKey = spr.textureHash;

        stats.spriteCount++;
    }

    void Renderer2D::drawParticleSystem2D(const ParticleSystem2D & particleSystem) {
        for (const auto &emitter : particleSystem.getEmitters()) {
            for (const auto &particle : emitter.getActiveParticles()) {
                drawQuad(particle.position, particle.dimensions, particle.rotationDeg, emitter.texture, particle.tintAndAlpha);
            }
        }
    }
}
