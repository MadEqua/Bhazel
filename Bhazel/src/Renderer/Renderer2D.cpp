#include "bzpch.h"

#include "Renderer2D.h"

#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/Shader.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"

#include "Core/Application.h"
#include "Core/Utils.h"

#include "Renderer/ParticleSystem2D.h"

#include "Camera.h"

#include <imgui.h>


namespace BZ {

    struct Renderer2DStats {
        uint32 spriteCount;
        uint32 drawCallCount;
        uint32 descriptorSetBindCount;
        //uint32 tintPushCount;
    };

    constexpr uint32 MAX_RENDERER2D_SPRITES = 100'000;

    static DataLayout vertexLayout = {
        { DataType::Float32, DataElements::Vec2 },
        { DataType::Uint16, DataElements::Vec2, true },
        { DataType::Uint32, DataElements::Scalar },
    };

    static DataLayout indexLayout = {
        {DataType::Uint32, DataElements::Scalar, ""},
    };

    struct VertexData {
        float pos[2];
        uint16 texCoord[2];
        uint32 colorAndAlpha;
    };

    constexpr uint16 UINT16_MAX_VALUE = 0xffff;
    static VertexData quadVertices[4] = {
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
        DescriptorSet *descriptorSet;
    };

    static struct Renderer2DData {
        const OrthographicCamera *camera;

        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;
        Ref<Buffer> constantBuffer;

        BufferPtr vertexBufferPtr;
        BufferPtr indexBufferPtr;
        BufferPtr constantBufferPtr;

        Ref<PipelineState> pipelineState;
        Ref<Texture2D> whiteTexture;
        Ref<Sampler> sampler;

        Ref<DescriptorSetLayout> constantsDescriptorSetLayout;
        Ref<DescriptorSetLayout> textureDescriptorSetLayout;
        DescriptorSet *constantsDescriptorSet;

        std::unordered_map<uint64, TexData> texDataStorage;

        InternalSprite sprites[MAX_RENDERER2D_SPRITES + 1];
        uint32 nextSprite;

        //Stats
        Renderer2DStats stats;
        Renderer2DStats visibleStats;

        uint32 statsRefreshPeriodMs = 250;
        uint32 statsRefreshTimeAcumMs;
    } rendererData;


    static uint64 initTexture(const Ref<Texture2D>& texture) {
        uint64 hash = reinterpret_cast<uint64>(texture.get()); //TODO: something better
        if (rendererData.texDataStorage.find(hash) == rendererData.texDataStorage.end()) {
            auto texViewRef = TextureView::create(texture, 0, 1);
            auto descSetRef = &DescriptorSet::get(rendererData.textureDescriptorSetLayout);
            descSetRef->setCombinedTextureSampler(texViewRef, rendererData.sampler, 0);
            rendererData.texDataStorage.emplace(hash, TexData{ texViewRef, descSetRef });
        }
        return hash;
    }

    void Renderer2D::init() {
        BZ_PROFILE_FUNCTION();

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/Renderer2DVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/Renderer2DFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });

        rendererData.vertexBuffer = Buffer::create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 4 * sizeof(VertexData) * MAX_RENDERER2D_SPRITES, MemoryType::CpuToGpu, vertexLayout);
        rendererData.indexBuffer = Buffer::create(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 6 * sizeof(uint32) * MAX_RENDERER2D_SPRITES, MemoryType::CpuToGpu, indexLayout);

        rendererData.vertexBufferPtr = rendererData.vertexBuffer->map(0);
        rendererData.indexBufferPtr = rendererData.indexBuffer->map(0);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        rendererData.sampler = samplerBuilder.build();

        byte whiteTextureData[] = {255, 255, 255, 255};
        rendererData.whiteTexture = Texture2D::create(whiteTextureData, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, MipmapData::Options::DoNothing);

        rendererData.constantsDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1 } });

        rendererData.textureDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        const auto WINDOW_DIMS_INT = Application::get().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::get().getWindow().getDimensionsFloat();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingStateAttachment.enableBlending = true;
        blendingStateAttachment.srcColorBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendingStateAttachment.dstColorBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendingStateAttachment.colorBlendingOperation = VK_BLEND_OP_ADD;
        blendingStateAttachment.srcAlphaBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendingStateAttachment.dstAlphaBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendingStateAttachment.alphaBlendingOperation = VK_BLEND_OP_ADD;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        pipelineStateData.dataLayout = vertexLayout;

        //Push constants are used to pass tint and alpha values
        //PushConstantDesc pushConstantDesc;
        //pushConstantDesc.offset = 0;
        //pushConstantDesc.size = sizeof(glm::vec4);
        //pushConstantDesc.shaderStageMask = flagsToMask(ShaderStageFlag::Fragment);

        //pipelineStateData.pushConstantDescs = { pushConstantDesc };

        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };
        pipelineStateData.descriptorSetLayouts = { rendererData.constantsDescriptorSetLayout, rendererData.textureDescriptorSetLayout };
        pipelineStateData.blendingState = blendingState;
        pipelineStateData.renderPass = Application::get().getGraphicsContext().getSwapchainDefaultRenderPass();
        pipelineStateData.subPassIndex = 0;

        rendererData.pipelineState = PipelineState::create(pipelineStateData);

        rendererData.constantBuffer = Buffer::create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, MIN_UNIFORM_BUFFER_OFFSET_ALIGN, MemoryType::CpuToGpu);
        rendererData.constantBufferPtr = rendererData.constantBuffer->map(0);

        rendererData.constantsDescriptorSet = &DescriptorSet::get(rendererData.constantsDescriptorSetLayout);
        rendererData.constantsDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, 0, sizeof(glm::mat4));
    }

    void Renderer2D::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.vertexBuffer.reset();
        rendererData.indexBuffer.reset();
        rendererData.constantBuffer.reset();

        rendererData.texDataStorage.clear();

        rendererData.pipelineState.reset();
        rendererData.sampler.reset();
        rendererData.whiteTexture.reset();

        rendererData.constantsDescriptorSetLayout.reset();
        rendererData.textureDescriptorSetLayout.reset();
    }

    void Renderer2D::begin(const OrthographicCamera &camera) {
        BZ_PROFILE_FUNCTION();

        rendererData.nextSprite = 0;
        rendererData.camera = &camera;

        memset(&rendererData.stats, 0, sizeof(Renderer2DStats));
    }

    void Renderer2D::end() {
        BZ_PROFILE_FUNCTION();

        if(rendererData.nextSprite > 0) {
            //Sort objects to minimize state changes when rendering. Sorted by Texture and then by tint.
            std::sort(rendererData.sprites, rendererData.sprites + rendererData.nextSprite, [](const InternalSprite &a, const InternalSprite &b) {
                return a.sortKey < b.sortKey;
            });
        }
    }

    void Renderer2D::renderSprite(const Sprite &sprite) {
        BZ_PROFILE_FUNCTION();

        renderQuad(sprite.position, sprite.dimensions, sprite.rotationDeg, sprite.texture, sprite.tintAndAlpha);
    }

    void Renderer2D::renderQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec4 &colorAndAlpha) {
        BZ_PROFILE_FUNCTION();

        renderQuad(position, dimensions, rotationDeg, rendererData.whiteTexture, colorAndAlpha);
    }

    void Renderer2D::renderQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec4 &tintAndAlpha) {
        BZ_PROFILE_FUNCTION();

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

        rendererData.stats.spriteCount++;
    }

    void Renderer2D::renderParticleSystem2D(const ParticleSystem2D & particleSystem) {
        BZ_PROFILE_FUNCTION();

        for (const auto &emitter : particleSystem.getEmitters()) {
            for (const auto &particle : emitter.getActiveParticles()) {
                renderQuad(particle.position, particle.dimensions, particle.rotationDeg, emitter.texture, particle.tintAndAlpha);
            }
        }
    }

    void Renderer2D::render(const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer)  {
        BZ_PROFILE_FUNCTION();

        if(rendererData.nextSprite > 0) {
            CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
            commandBuffer.beginRenderPass(swapchainRenderPass, swapchainFramebuffer);

            glm::mat4 viewProjMatrix = rendererData.camera->getProjectionMatrix() * rendererData.camera->getViewMatrix();
            memcpy(rendererData.constantBufferPtr, &viewProjMatrix[0][0], sizeof(glm::mat4));
            commandBuffer.bindDescriptorSet(*rendererData.constantsDescriptorSet, rendererData.pipelineState, 0, nullptr, 0);

            commandBuffer.bindBuffer(rendererData.vertexBuffer, 0);
            commandBuffer.bindBuffer(rendererData.indexBuffer, 0);
            commandBuffer.bindPipelineState(rendererData.pipelineState);

            uint32 spritesInBatch = 0;
            uint32 nextBatchOffset = 0;
            uint64 currentBoundTexHash = -1;
            //glm::vec4 currentActiveTint = glm::vec4(-1.0f);
            uint64 currentBatchTexHash = rendererData.sprites[0].textureHash;
            //glm::vec4 currentBatchTint = rendererData.sprites[0].tintAndAlpha;

            //Generate vertex and index buffers and record commands
            for(uint32 objIdx = 0; objIdx <= rendererData.nextSprite; ++objIdx) {
                const InternalSprite &spr = rendererData.sprites[objIdx];

                //We iterate past last object to finish the current batch on that case.
                bool isLastIteration = objIdx == rendererData.nextSprite;

                if(!isLastIteration) {
                    //Buffer generation
                    float c = glm::cos(glm::radians(spr.rotationDeg));
                    float s = glm::sin(glm::radians(spr.rotationDeg));

                    VertexData vertices[4];
                    uint32 indices[6];
                    uint32 packedColor = Utils::packColor(spr.tintAndAlpha);
                    for(int i = 0; i < 4; ++i) {
                        vertices[i].pos[0] = quadVertices[i].pos[0] * spr.dimensions.x * c + quadVertices[i].pos[1] * spr.dimensions.y * -s + spr.position.x;
                        vertices[i].pos[1] = quadVertices[i].pos[0] * spr.dimensions.x * s + quadVertices[i].pos[1] * spr.dimensions.y * c + spr.position.y;
                        vertices[i].texCoord[0] = quadVertices[i].texCoord[0];
                        vertices[i].texCoord[1] = quadVertices[i].texCoord[1];
                        vertices[i].colorAndAlpha = packedColor;
                    }

                    for(int i = 0; i < 6; ++i) {
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
                if(texChanged || isLastIteration) {
                    if(currentBoundTexHash != currentBatchTexHash) {
                        const TexData& texData = rendererData.texDataStorage[currentBatchTexHash];
                        commandBuffer.bindDescriptorSet(*texData.descriptorSet, rendererData.pipelineState, 1, nullptr, 0);
                        currentBoundTexHash = currentBatchTexHash;
                        rendererData.stats.descriptorSetBindCount++;
                    }

                    //if (currentActiveTint != currentBatchTint) {
                    //    Graphics::setPushConstants(rendererData.commandBufferId, rendererData.pipelineState, flagsToMask(ShaderStageFlag::Fragment), &currentBatchTint.x, sizeof(glm::vec4), 0);
                    //    currentActiveTint = currentBatchTint;
                    //    stats.tintPushCount++;
                    //}

                    commandBuffer.drawIndexed(spritesInBatch * 6, 1, nextBatchOffset * 6, 0, 0);
                    nextBatchOffset = objIdx;
                    spritesInBatch = 0;

                    currentBatchTexHash = spr.textureHash;
                    //currentBatchTint = spr.tintAndAlpha;

                    rendererData.stats.drawCallCount++;
                }
                spritesInBatch++;
            }

            commandBuffer.endRenderPass();
            commandBuffer.endAndSubmit();
        }
    }

    void Renderer2D::onImGuiRender(const FrameStats &frameStats) {
        BZ_PROFILE_FUNCTION();

        if (ImGui::Begin("Renderer2D")) {
            rendererData.statsRefreshTimeAcumMs += frameStats.lastFrameTime.asMillisecondsUint32();
            if (rendererData.statsRefreshTimeAcumMs >= rendererData.statsRefreshPeriodMs) {
                rendererData.statsRefreshTimeAcumMs = 0;
                rendererData.visibleStats = rendererData.stats;
            }
            ImGui::Text("Stats:");
            ImGui::Text("Sprite Count: %d", rendererData.visibleStats.spriteCount);
            ImGui::Text("Draw Call Count: %d", rendererData.visibleStats.drawCallCount);
            ImGui::Text("Descriptor Set Bind Count: %d", rendererData.visibleStats.descriptorSetBindCount);
            //ImGui::Text("Tint Push Count: %d", visibleFrameStats.tintPushCount);
            ImGui::Separator();

            ImGui::SliderInt("Refresh period ms", reinterpret_cast<int*>(&rendererData.statsRefreshPeriodMs), 0, 1000);
        }
        ImGui::End();
    }
}
