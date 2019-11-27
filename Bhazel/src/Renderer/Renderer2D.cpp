#include "bzpch.h"

#include "Renderer2D.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Core/Application.h"
#include "Core/Utils.h"


namespace BZ {

    constexpr uint32 MAX_RENDERER2D_TEXTURES = 16;
    constexpr uint32 MAX_RENDERER2D_OBJECTS = 100'000;

    static DataLayout dataLayout = {
        {DataType::Float32, DataElements::Vec3, "POSITION"},
        {DataType::Uint16, DataElements::Vec2, "TEXCOORD", true},
    };

    struct Vertex {
        float pos[3];
        uint16 texCoord[2];
    };

    constexpr uint16 MAX_TEX_COORD = 0xffff;
    static Vertex quadVertices[4] = {
        {
            { -0.5f, -0.5f, 0.0f },
            { 0, 0 }
        },
        {
            { 0.5f, -0.5f, 0.0f },
            { MAX_TEX_COORD, 0 }
        },
        {
            { 0.5f, 0.5f, 0.0f },
            { MAX_TEX_COORD, MAX_TEX_COORD }
        },
        {
            { -0.5f, 0.5f, 0.0f },
            { 0, MAX_TEX_COORD }
        }
    };

    static uint16 quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

    struct Object {
        glm::vec2 position;
        glm::vec2 dimensions;
        float rotationDeg;
        glm::vec3 tint;
        uint32 textureIdx;
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
        Ref<DescriptorSet> descriptorSet;

        Texture* batchTextures[MAX_RENDERER2D_TEXTURES];
        uint32 nextBatchTexture;

        std::unordered_map<uint32, TexData> texDataStorage;

        Object objects[MAX_RENDERER2D_OBJECTS + 1];
        uint32 nextObject;
    } rendererData;


    static uint32 addBatchTexture(const Ref<Texture2D>& texture) {
        for(uint32 i = 0; i < rendererData.nextBatchTexture; ++i) {
            if (rendererData.batchTextures[i] == texture.get()) {
                return i;
            }
        }

        BZ_ASSERT_CORE(rendererData.nextBatchTexture < MAX_RENDERER2D_TEXTURES, "Reached the texture limit!");

        rendererData.batchTextures[rendererData.nextBatchTexture++] = texture.get();
        return rendererData.nextBatchTexture - 1;
    }

    static const TexData& getTexDataForTexture(const Texture &texture) {
        uint32 hash = reinterpret_cast<uint32>(&texture); //TODO: something better
        if (rendererData.texDataStorage.find(hash) == rendererData.texDataStorage.end()) {
            auto texViewRef = TextureView::create(texture);
            auto descSetRef = DescriptorSet::create(rendererData.descriptorSetLayout);
            descSetRef->setSampler(rendererData.sampler, 0);
            descSetRef->setSampledTexture(texViewRef, 1);
            rendererData.texDataStorage.emplace(hash, TexData{texViewRef, descSetRef});
        }

        return rendererData.texDataStorage[hash];
    }

    void Renderer2D::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer2D");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "shaders/bin/TextureVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "shaders/bin/TextureFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        rendererData.vertexBuffer = Buffer::create(BufferType::Vertex, 4 * sizeof(Vertex) * MAX_RENDERER2D_OBJECTS, MemoryType::CpuToGpu, dataLayout);
        rendererData.indexBuffer = Buffer::create(BufferType::Index, 6 * sizeof(uint16) * MAX_RENDERER2D_OBJECTS, MemoryType::CpuToGpu);

        rendererData.vertexBufferPtr = rendererData.vertexBuffer->map(0);
        rendererData.indexBufferPtr = rendererData.indexBuffer->map(0);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        rendererData.sampler = samplerBuilder.build();

        byte whiteTextureData[] = {255, 255, 255, 255};
        rendererData.whiteTexture = Texture2D::create(whiteTextureData, sizeof(whiteTextureData), 1, 1, TextureFormat::R8G8B8A8);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::Sampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::SampledTexture, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.descriptorSetLayout = descriptorSetLayoutBuilder.build();

        rendererData.descriptorSet = DescriptorSet::create(rendererData.descriptorSetLayout);
        rendererData.descriptorSet->setSampler(rendererData.sampler, 0);

        auto &windowDims = Application::getInstance().getWindow().getDimensions();

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

        pipelineStateData.dataLayout = dataLayout;

        //Push constants are used to pass tint values
        PushConstantDesc pushConstantDesc;
        pushConstantDesc.offset = 0;
        pushConstantDesc.size = sizeof(glm::vec3);
        pushConstantDesc.shaderStageMask = flagsToMask(ShaderStageFlags::Fragment);

        pipelineStateData.pushConstantDescs = { pushConstantDesc };
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(windowDims.x), static_cast<float>(windowDims.y) } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(windowDims.x), static_cast<uint32>(windowDims.y) } };
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
        rendererData.descriptorSet.reset();
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId == -1, "There's already an unended Scene!");

        rendererData.nextBatchTexture = 0;
        rendererData.nextObject = 0;

        rendererData.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(rendererData.commandBufferId, rendererData.pipelineState, camera.getViewMatrix(), camera.getProjectionMatrix());
    }

    void Renderer2D::endScene() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");

        if (rendererData.nextObject > 0) {
            //Sort objects to minimize state changes when rendering. Sorted by Texture and then by tint.
            std::sort(rendererData.objects, rendererData.objects + rendererData.nextObject, [](const Object& a, const Object& b) {
                return a.sortKey < b.sortKey;
            });

            Graphics::bindBuffer(rendererData.commandBufferId, rendererData.vertexBuffer, 0);
            Graphics::bindBuffer(rendererData.commandBufferId, rendererData.indexBuffer, 0);
            Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.pipelineState);

            uint32 objectsToDraw = 0;
            uint32 nextBatchOffset = 0;
            uint32 currentBoundTexIndex = -1;
            glm::vec3 currentActiveTint = glm::vec3(-1.0f);
            uint32 currentBatchTexIndex = rendererData.objects[0].textureIdx;
            glm::vec3 currentBatchTint = rendererData.objects[0].tint;

            //Generate vertex and index buffers and record commands
            for (uint32 objIdx = 0; objIdx <= rendererData.nextObject; ++objIdx) {
                const Object &obj = rendererData.objects[objIdx];

                //We iterate past last object to finish the current batch on that case.
                bool isLastIteration = objIdx == rendererData.nextObject;

                if (!isLastIteration) {
                    //Buffer generation
                    float c = glm::cos(glm::radians(obj.rotationDeg));
                    float s = glm::sin(glm::radians(obj.rotationDeg));

                    Vertex vertices[4];
                    uint16 indices[6];
                    for (int i = 0; i < 4; ++i) {
                        vertices[i].pos[0] = quadVertices[i].pos[0] * obj.dimensions.x * c + quadVertices[i].pos[1] * obj.dimensions.y * -s + obj.position.x;
                        vertices[i].pos[1] = quadVertices[i].pos[0] * obj.dimensions.x * s + quadVertices[i].pos[1] * obj.dimensions.y * c + obj.position.y;
                        vertices[i].pos[2] = 0.0f;
                        vertices[i].texCoord[0] = quadVertices[i].texCoord[0];
                        vertices[i].texCoord[1] = quadVertices[i].texCoord[1];
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
                bool texChanged = currentBatchTexIndex != obj.textureIdx;
                bool tintChanged = currentBatchTint != obj.tint;
                if (texChanged || tintChanged || isLastIteration) {
                    if (currentBoundTexIndex != currentBatchTexIndex) {
                        const TexData& texData = getTexDataForTexture(*rendererData.batchTextures[currentBatchTexIndex]);
                        Graphics::bindDescriptorSet(rendererData.commandBufferId, texData.descriptorSet, rendererData.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);
                        currentBoundTexIndex = currentBatchTexIndex;
                    }

                    if (currentActiveTint != currentBatchTint) {
                        Graphics::setPushConstants(rendererData.commandBufferId, rendererData.pipelineState, flagsToMask(ShaderStageFlags::Fragment), &currentBatchTint.x, sizeof(glm::vec3), 0);
                        currentActiveTint = currentBatchTint;
                    }

                    Graphics::drawIndexed(rendererData.commandBufferId, objectsToDraw * 6, 1, nextBatchOffset * 6, 0, 0);
                    nextBatchOffset = objIdx;
                    objectsToDraw = 0;

                    currentBatchTexIndex = obj.textureIdx;
                    currentBatchTint = obj.tint;
                }
                objectsToDraw++;
            }
        }

        Graphics::endCommandBuffer(rendererData.commandBufferId);
        rendererData.commandBufferId = -1;
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec3 &color) {
        BZ_PROFILE_FUNCTION();

        drawQuad(position, dimensions, rotationDeg, rendererData.whiteTexture, color);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec3 &tint) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");
        BZ_ASSERT_CORE(rendererData.nextObject < MAX_RENDERER2D_OBJECTS, "nextObject exceeded MAX_RENDERER2D_OBJECTS!");

        Object &object = rendererData.objects[rendererData.nextObject++];
        object.position = position;
        object.dimensions = dimensions;
        object.rotationDeg = rotationDeg;
        object.textureIdx = addBatchTexture(texture);
        object.tint = tint;

        //Sort by Texture and then by tint
        std::hash<double> hasher;
        size_t tintHash = Utils::hashCombine(Utils::hashCombine(hasher(tint.r), hasher(tint.g)), hasher(tint.b));

        uint64 texIndex = object.textureIdx;
        object.sortKey = (texIndex << 60) | (tintHash >> 4);
    }
}
