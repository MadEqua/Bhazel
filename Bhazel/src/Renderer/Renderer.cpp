#include "bzpch.h"

#include "Renderer.h"

#include "BRDFLookup.h"

#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/CommandBuffer.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Utils.h"

#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"

#include <imgui.h>


namespace BZ {

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) SceneConstantBufferData {
        glm::mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT]; //World to light clip space
        glm::vec4 dirLightDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
        glm::vec4 dirLightColors[MAX_DIR_LIGHTS_PER_SCENE]; //vec4 to simplify alignments
        glm::vec4 cascadeSplits; //TODO: Hardcoded to 4.
        glm::vec2 dirLightCountAndRadianceMapMips;
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) PassConstantBufferData {
        glm::mat4 viewMatrix; //World to camera space
        glm::mat4 projectionMatrix; //Camera to clip space
        glm::mat4 viewProjectionMatrix; //World to clip space
        glm::vec4 cameraPosition; //mat4 to simplify alignments
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) MaterialConstantBufferData {
        glm::vec4 normalMetallicRoughnessAndAO;
        glm::vec4 heightAndUvScale;
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) EntityConstantBufferData {
        glm::mat4 modelMatrix; //Model to world space
        glm::mat4 normalMatrix; //Model to world space, appropriate to transform vectors, mat4 to simplify alignments
    };

    constexpr uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData);
    constexpr uint32 PASS_CONSTANT_BUFFER_SIZE = sizeof(PassConstantBufferData) * MAX_PASSES_PER_FRAME;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_SIZE = sizeof(MaterialConstantBufferData) * MAX_MATERIALS_PER_SCENE;
    constexpr uint32 ENTITY_CONSTANT_BUFFER_SIZE = sizeof(EntityConstantBufferData) * MAX_ENTITIES_PER_SCENE;

    constexpr uint32 SCENE_CONSTANT_BUFFER_OFFSET = 0;
    constexpr uint32 PASS_CONSTANT_BUFFER_OFFSET = SCENE_CONSTANT_BUFFER_SIZE;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_OFFSET = PASS_CONSTANT_BUFFER_OFFSET + PASS_CONSTANT_BUFFER_SIZE;
    constexpr uint32 ENTITY_CONSTANT_BUFFER_OFFSET = MATERIAL_CONSTANT_BUFFER_OFFSET + MATERIAL_CONSTANT_BUFFER_SIZE;

    constexpr uint32 SHADOW_MAP_SIZE = 1024;

    static DataLayout vertexDataLayout = {
        { DataType::Float32, DataElements::Vec3 },
        { DataType::Float32, DataElements::Vec3 },
        { DataType::Float32, DataElements::Vec4 },
        { DataType::Uint16, DataElements::Vec2, true },
    };

    static DataLayout indexDataLayout = {
        { DataType::Uint32, DataElements::Scalar, "" }
    };

    struct RendererStats {
        uint32 vertexCount;
        uint32 triangleCount;
        uint32 drawCallCount;
        uint32 materialCount;
    };

    static struct RendererData {
        CommandBuffer *commandBuffer;

        Ref<Buffer> constantBuffer;
        BufferPtr sceneConstantBufferPtr;
        BufferPtr passConstantBufferPtr;
        BufferPtr materialConstantBufferPtr;
        BufferPtr entityConstantBufferPtr;

        Ref<DescriptorSetLayout> globalDescriptorSetLayout;
        Ref<DescriptorSetLayout> sceneDescriptorSetLayout;
        Ref<DescriptorSetLayout> passDescriptorSetLayout;
        Ref<DescriptorSetLayout> passDescriptorSetLayoutForDepthPass;
        Ref<DescriptorSetLayout> materialDescriptorSetLayout;
        Ref<DescriptorSetLayout> entityDescriptorSetLayout;
        Ref<DescriptorSetLayout> postProcessPassDescriptorSetLayout;

        DescriptorSet *globalDescriptorSet;
        DescriptorSet *passDescriptorSet;
        DescriptorSet *passDescriptorSetForDepthPass;
        DescriptorSet *entityDescriptorSet;
        DescriptorSet *postProcessDescriptorSet;

        Ref<Sampler> defaultSampler;
        Ref<Sampler> brdfLookupSampler;
        Ref<Sampler> shadowSampler;

        Ref<PipelineState> defaultPipelineState;
        Ref<PipelineState> skyBoxPipelineState;
        Ref<PipelineState> depthPassPipelineState;
        Ref<PipelineState> postProcessPipelineState;

        Ref<TextureView> brdfLookupTexture;
        Ref<TextureView> dummyTextureArrayView;

        std::unordered_map<Material, uint32> materialOffsetMap;

        Ref<RenderPass> depthRenderPass;

        //ConstantFactor, clamp and slopeFactor
        glm::vec3 depthBiasData = { 1.0f, 0.0f, 2.5f };

        //Stats
        RendererStats stats;
        RendererStats visibleStats;

        uint32 statsRefreshPeriodMs = 250;
        uint32 statsRefreshTimeAcumMs;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBuffer = nullptr;

        rendererData.constantBuffer = Buffer::create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            SCENE_CONSTANT_BUFFER_SIZE + PASS_CONSTANT_BUFFER_SIZE + MATERIAL_CONSTANT_BUFFER_SIZE + ENTITY_CONSTANT_BUFFER_SIZE,
            MemoryType::CpuToGpu);

        rendererData.sceneConstantBufferPtr = rendererData.constantBuffer->map(0);
        rendererData.passConstantBufferPtr = rendererData.sceneConstantBufferPtr + PASS_CONSTANT_BUFFER_OFFSET;
        rendererData.materialConstantBufferPtr = rendererData.sceneConstantBufferPtr + MATERIAL_CONSTANT_BUFFER_OFFSET;
        rendererData.entityConstantBufferPtr = rendererData.sceneConstantBufferPtr + ENTITY_CONSTANT_BUFFER_OFFSET;

        rendererData.globalDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        rendererData.sceneDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_ALL, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_DIR_LIGHTS_PER_SCENE } });

        rendererData.passDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 1 } });

        rendererData.materialDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          //Albedo, Normal, Metallic, Roughness, Height and AO.
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        rendererData.entityDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 1 } });

        rendererData.entityDescriptorSet = &DescriptorSet::get(rendererData.entityDescriptorSetLayout);
        rendererData.entityDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, ENTITY_CONSTANT_BUFFER_OFFSET, sizeof(EntityConstantBufferData));

        Sampler::Builder samplerBuilder;
        rendererData.defaultSampler = samplerBuilder.build();

        initDepthPassData();
        initDefaultPassData();
        initPostProcessPassData();
        initSkyBoxData();
    }

    void Renderer::initDepthPassData() {
        BZ_PROFILE_FUNCTION();

        rendererData.passDescriptorSetLayoutForDepthPass =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, SHADOW_MAPPING_CASCADE_COUNT } });

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexDataLayout;

        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/DepthPassVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/DepthPassGeo.spv", VK_SHADER_STAGE_GEOMETRY_BIT } });

        pipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                                   rendererData.passDescriptorSetLayoutForDepthPass, rendererData.materialDescriptorSetLayout,
                                                   rendererData.entityDescriptorSetLayout };
        pipelineStateData.viewports = { { 0.0f, 0.0f, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } };

        //To disable clipping on z axis. Geometry behind "light camera" will still cast shadows.
        pipelineStateData.rasterizerState.enableDepthClamp = true;

        //To avoid shadow acne.
        pipelineStateData.rasterizerState.enableDepthBias = true;
        //pipelineStateData.rasterizerState.depthBiasConstantFactor = 0.0f;
        //pipelineStateData.rasterizerState.depthBiasSlopeFactor = 0.0f;

        DepthStencilState depthStencilState;
        depthStencilState.enableDepthTest = true;
        depthStencilState.enableDepthWrite = true;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        pipelineStateData.depthStencilState = depthStencilState;

        pipelineStateData.dynamicStates = { VK_DYNAMIC_STATE_DEPTH_BIAS };

        AttachmentDescription depthStencilAttachmentDesc;
        depthStencilAttachmentDesc.format = VK_FORMAT_D32_SFLOAT;
        depthStencilAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthStencilAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthStencilAttachmentDesc.clearValue.depthStencil.depth = 1.0f;
        depthStencilAttachmentDesc.clearValue.depthStencil.stencil = 0;

        SubPassDescription subPassDesc;
        subPassDesc.depthStencilAttachmentsRef = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        //SubPassDependency dependency1;
        //dependency1.srcSubPassIndex = -1;
        //dependency1.dstSubPassIndex = 0;
        //dependency1.srcStageMask = flagsToMask(PipelineStageFlag::FragmentShader);
        //dependency1.dstStageMask = flagsToMask(PipelineStageFlag::EarlyFragmentTests);
        //dependency1.srcAccessMask = 0;
        //dependency1.dstAccessMask = flagsToMask(AccessFlag::DepthStencilAttachmentWrite);
        //dependency1.dependencyFlags = flagsToMask(DependencyFlag::ByRegion);

        SubPassDependency dependency2;
        dependency2.srcSubPassIndex = 0;
        dependency2.dstSubPassIndex = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency2.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency2.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        rendererData.depthRenderPass = RenderPass::create({ depthStencilAttachmentDesc }, { subPassDesc }, { dependency2 });
        pipelineStateData.renderPass = rendererData.depthRenderPass;
        pipelineStateData.subPassIndex = 0;

        rendererData.depthPassPipelineState = PipelineState::create(pipelineStateData);

        rendererData.passDescriptorSetForDepthPass = &DescriptorSet::get(rendererData.passDescriptorSetLayoutForDepthPass);
        Ref<Buffer> constantBuffers[SHADOW_MAPPING_CASCADE_COUNT];
        uint32 offsets[SHADOW_MAPPING_CASCADE_COUNT];
        uint32 sizes[SHADOW_MAPPING_CASCADE_COUNT];
        for (uint32 i = 0; i < SHADOW_MAPPING_CASCADE_COUNT; ++i) {
            constantBuffers[i] = rendererData.constantBuffer;
            offsets[i] = PASS_CONSTANT_BUFFER_OFFSET + sizeof(PassConstantBufferData) * i;
            sizes[i] = sizeof(PassConstantBufferData);
        }
        rendererData.passDescriptorSetForDepthPass->setConstantBuffers(constantBuffers, SHADOW_MAPPING_CASCADE_COUNT, 0, 0, offsets, sizes);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        samplerBuilder.setBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
        samplerBuilder.enableCompare(VK_COMPARE_OP_LESS);
        rendererData.shadowSampler = samplerBuilder.build();
    }

    void Renderer::initDefaultPassData() {
        BZ_PROFILE_FUNCTION();

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexDataLayout;

        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/RendererVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/RendererFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });

        pipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                                   rendererData.passDescriptorSetLayout, rendererData.materialDescriptorSetLayout,
                                                   rendererData.entityDescriptorSetLayout };

        const auto WINDOW_DIMS_INT = Application::get().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::get().getWindow().getDimensionsFloat();
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };

        RasterizerState rasterizerState;
        rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerState.frontFaceCounterClockwise = true;
        pipelineStateData.rasterizerState = rasterizerState;

        DepthStencilState depthStencilState;
        depthStencilState.enableDepthTest = true;
        depthStencilState.enableDepthWrite = true;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        pipelineStateData.depthStencilState = depthStencilState;

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        pipelineStateData.renderPass = Application::get().getGraphicsContext().getMainRenderPass();
        pipelineStateData.subPassIndex = 0;

        rendererData.defaultPipelineState = PipelineState::create(pipelineStateData);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        rendererData.brdfLookupSampler = samplerBuilder.build();

        auto brdfLookupTexRef = Texture2D::create(reinterpret_cast<const byte*>(brdfLut), brdfLutSize, brdfLutSize, VK_FORMAT_R16G16_SFLOAT, MipmapData::Options::DoNothing);
        rendererData.brdfLookupTexture = TextureView::create(brdfLookupTexRef);

        rendererData.globalDescriptorSet = &DescriptorSet::get(rendererData.globalDescriptorSetLayout);
        rendererData.globalDescriptorSet->setCombinedTextureSampler(rendererData.brdfLookupTexture, rendererData.brdfLookupSampler, 0);

        rendererData.passDescriptorSet = &DescriptorSet::get(rendererData.passDescriptorSetLayout);
        rendererData.passDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, PASS_CONSTANT_BUFFER_OFFSET, sizeof(PassConstantBufferData));

        rendererData.dummyTextureArrayView = TextureView::create(brdfLookupTexRef, 0, 1);
    }

    void Renderer::initPostProcessPassData() {
        BZ_PROFILE_FUNCTION();

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = {};

        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenQuadVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/ToneMapFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });

        pipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        rendererData.postProcessPassDescriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1} });

        pipelineStateData.descriptorSetLayouts = { rendererData.postProcessPassDescriptorSetLayout };

        const auto WINDOW_DIMS_INT = Application::get().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::get().getWindow().getDimensionsFloat();
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        pipelineStateData.renderPass = Application::get().getGraphicsContext().getSwapchainRenderPass();
        pipelineStateData.subPassIndex = 0;

        rendererData.postProcessPipelineState = PipelineState::create(pipelineStateData);

        rendererData.postProcessDescriptorSet = &DescriptorSet::get(rendererData.postProcessPassDescriptorSetLayout);
        rendererData.postProcessDescriptorSet->setCombinedTextureSampler(Application::get().getGraphicsContext().getColorTextureView(),
            rendererData.defaultSampler, 0);
    }

    void Renderer::initSkyBoxData() {
        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexDataLayout;

        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/SkyBoxVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/SkyBoxFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });

        pipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                                   rendererData.passDescriptorSetLayout, rendererData.materialDescriptorSetLayout,
                                                   rendererData.entityDescriptorSetLayout };

        const auto WINDOW_DIMS_INT = Application::get().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::get().getWindow().getDimensionsFloat();
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };

        RasterizerState rasterizerState;
        rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerState.frontFaceCounterClockwise = true;
        pipelineStateData.rasterizerState = rasterizerState;

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        pipelineStateData.renderPass = Application::get().getGraphicsContext().getMainRenderPass();
        pipelineStateData.subPassIndex = 0;

        rendererData.skyBoxPipelineState = PipelineState::create(pipelineStateData);
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.constantBuffer.reset();

        rendererData.globalDescriptorSetLayout.reset();
        rendererData.sceneDescriptorSetLayout.reset();
        rendererData.passDescriptorSetLayout.reset();
        rendererData.passDescriptorSetLayoutForDepthPass.reset();
        rendererData.materialDescriptorSetLayout.reset();
        rendererData.entityDescriptorSetLayout.reset();
        rendererData.postProcessPassDescriptorSetLayout.reset();

        rendererData.defaultPipelineState.reset();
        rendererData.skyBoxPipelineState.reset();
        rendererData.depthPassPipelineState.reset();
        rendererData.postProcessPipelineState.reset();

        rendererData.defaultSampler.reset();
        rendererData.brdfLookupSampler.reset();
        rendererData.shadowSampler.reset();

        rendererData.materialOffsetMap.clear();

        rendererData.brdfLookupTexture.reset();
        rendererData.dummyTextureArrayView.reset();

        rendererData.depthRenderPass.reset();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&rendererData.stats, 0, sizeof(RendererStats));
 
        fillConstants(scene);

        rendererData.commandBuffer = &CommandBuffer::getAndBegin(QueueProperty::Graphics);

        //Bind stuff that will not change.
        rendererData.commandBuffer->bindDescriptorSet(*rendererData.globalDescriptorSet, rendererData.defaultPipelineState, RENDERER_GLOBAL_DESCRIPTOR_SET_IDX, 0, 0);
        rendererData.commandBuffer->bindDescriptorSet(scene.getDescriptorSet(), rendererData.defaultPipelineState, RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

        depthPass(scene);
        colorPass(scene);
        postProcessPass();

        rendererData.commandBuffer->endAndSubmit();
    }

    void Renderer::depthPass(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBuffer->bindPipelineState(rendererData.depthPassPipelineState);
        rendererData.commandBuffer->setDepthBias(rendererData.depthBiasData.x, rendererData.depthBiasData.y, rendererData.depthBiasData.z);

        uint32 lightIdx = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            uint32 lightOffset = lightIdx * sizeof(PassConstantBufferData) * SHADOW_MAPPING_CASCADE_COUNT;
            uint32 lightOffsetArr[SHADOW_MAPPING_CASCADE_COUNT];
            for (uint32 i = 0; i < SHADOW_MAPPING_CASCADE_COUNT; ++i) {
                lightOffsetArr[i] = lightOffset;
            }
            rendererData.commandBuffer->bindDescriptorSet(*rendererData.passDescriptorSetForDepthPass,
                rendererData.depthPassPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, lightOffsetArr, SHADOW_MAPPING_CASCADE_COUNT);

            rendererData.commandBuffer->beginRenderPass(dirLight.shadowMapFramebuffer, true);
            drawEntities(scene, true);
            rendererData.commandBuffer->endRenderPass();

            lightIdx++;
        }
    }

    void Renderer::colorPass(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        uint32 colorPassOffset = PASS_CONSTANT_BUFFER_SIZE - sizeof(PassConstantBufferData); //Color pass is the last (after the Depth passes).
        rendererData.commandBuffer->bindDescriptorSet(*rendererData.passDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, &colorPassOffset, 1);

        rendererData.commandBuffer->beginRenderPass(Application::get().getGraphicsContext().getMainFramebuffer(), false);

        if (scene.hasSkyBox()) {
            rendererData.commandBuffer->bindPipelineState(rendererData.skyBoxPipelineState);
            drawMesh(scene.getSkyBox().mesh, Material(), false);
        }

        rendererData.commandBuffer->bindPipelineState(rendererData.defaultPipelineState);
        drawEntities(scene, false);
        rendererData.commandBuffer->endRenderPass();
    }

    void Renderer::postProcessPass() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBuffer->beginRenderPass(Application::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer(), true);
        rendererData.commandBuffer->bindPipelineState(rendererData.postProcessPipelineState);
        rendererData.commandBuffer->bindDescriptorSet(*rendererData.postProcessDescriptorSet, rendererData.postProcessPipelineState, 0, nullptr, 0);
        rendererData.commandBuffer->draw(3, 1, 0, 0);
        rendererData.commandBuffer->endRenderPass();
    }

    void Renderer::drawEntities(const Scene &scene, bool depthPass) {
        BZ_PROFILE_FUNCTION();

        uint32 entityIndex = 0;
        for (const auto &entity : scene.getEntities()) {
            if (!depthPass || entity.castShadow) {
                uint32 entityOffset = entityIndex * sizeof(EntityConstantBufferData);
                rendererData.commandBuffer->bindDescriptorSet(*rendererData.entityDescriptorSet,
                    depthPass ? rendererData.depthPassPipelineState : rendererData.defaultPipelineState,
                    RENDERER_ENTITY_DESCRIPTOR_SET_IDX, &entityOffset, 1);

                drawMesh(entity.mesh, entity.overrideMaterial, depthPass);
                entityIndex++;
            }
        }
    }

    void Renderer::drawMesh(const Mesh &mesh, const Material &overrideMaterial, bool depthPass) {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBuffer->bindBuffer(mesh.getVertexBuffer(), 0);
        
        if (mesh.hasIndices())
            rendererData.commandBuffer->bindBuffer(mesh.getIndexBuffer(), 0);

        for (const auto &submesh : mesh.getSubmeshes()) {
            if (!depthPass) {
                const Material &materialToUse = overrideMaterial.isValid() ? overrideMaterial : submesh.material;

                uint32 materialOffset = rendererData.materialOffsetMap[materialToUse];
                rendererData.commandBuffer->bindDescriptorSet(materialToUse.getDescriptorSet(),
                    depthPass ? rendererData.depthPassPipelineState : rendererData.defaultPipelineState,
                    RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);
            }

            if (mesh.hasIndices())
                rendererData.commandBuffer->drawIndexed(submesh.indexCount, 1, submesh.indexOffset, 0, 0);
            else
                rendererData.commandBuffer->draw(submesh.vertexCount, 1, submesh.vertexOffset, 1);

            rendererData.stats.drawCallCount++;
        }

        rendererData.stats.vertexCount += mesh.getVertexCount();
        rendererData.stats.triangleCount += (mesh.hasIndices() ? mesh.getIndexCount() : mesh.getVertexCount()) / 3;
    }

    void Renderer::fillConstants(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        //Matrices related to the depth passes for the lights.
        glm::mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT];
        glm::mat4 lightProjectionMatrices[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT];

        const PerspectiveCamera &camera = static_cast<const PerspectiveCamera&>(scene.getCamera());
        const PerspectiveCamera::Parameters &cameraParams = camera.getParameters();

        float cascadeSplits[SHADOW_MAPPING_CASCADE_COUNT];
        computeCascadedShadowMappingSplits(cascadeSplits, SHADOW_MAPPING_CASCADE_COUNT, cameraParams.near, cameraParams.far);

        uint32 matrixIndex = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            
            for (uint32 cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
                const float cascadeNear = cascadeIdx == 0 ? cameraParams.near : cascadeSplits[cascadeIdx];
                const float cascadeFar = cascadeIdx == 0 ? cascadeSplits[0] : cascadeSplits[cascadeIdx + 1];

                //Compute Sphere that enconpasses the frustum, in camera space.
                //The sphere is useful to have the shadow frustum to be always the same size regardless of the camera orientation, leading to no shadow flickering.
                float t = glm::tan(glm::radians(cameraParams.aspectRatio * cameraParams.fovy * 0.5f));
                glm::vec3 n(t * cascadeNear, 0.0f, cascadeNear);
                glm::vec3 f(t * cascadeFar, 0.0f, cascadeFar);

                //Solve the equation |f-center|=|n-center|, knowing that the sphere center is (0, 0, centerZ).
                float centerZ = ((f.x * f.x + f.z * f.z) - (n.x * n.x + n.z * n.z)) / (2.0f * f.z - 2.0f * n.z);
                float r = glm::distance(glm::vec3(0.0f, 0.0f, centerZ), n);

                glm::vec3 sphereCenterWorld = camera.getTransform().getLocalToParentMatrix() * glm::vec4(0.0f, 0.0f, centerZ, 1.0f);

                //Units of view space per shadow map texel (and world space, assuming no scaling between the two spaces).
                const float Q = glm::ceil(r * 2.0f) / static_cast<float>(SHADOW_MAP_SIZE);

                glm::vec3 lightCamPos = sphereCenterWorld - dirLight.getDirection() * r;
                lightMatrices[matrixIndex] = glm::lookAtRH(lightCamPos, sphereCenterWorld, glm::vec3(0, 1, 0));

                //Apply the quantization to translation to stabilize shadows when camera moves. We only move the light camera in texel sized snaps.
                lightMatrices[matrixIndex][3].x = glm::floor(lightMatrices[matrixIndex][3].x / Q) * Q;
                lightMatrices[matrixIndex][3].y = glm::floor(lightMatrices[matrixIndex][3].y / Q) * Q;
                lightMatrices[matrixIndex][3].z = glm::floor(lightMatrices[matrixIndex][3].z / Q) * Q;

                lightProjectionMatrices[matrixIndex] = Utils::ortho(-r, r, -r, r, 0.1f, r * 2.0f);
                matrixIndex++;
            }
        }

        fillScene(scene, lightMatrices, lightProjectionMatrices, cascadeSplits);
        fillPasses(scene, lightMatrices, lightProjectionMatrices);
        fillMaterials(scene);
        fillEntities(scene);
    }

    void Renderer::fillScene(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices, const float cascadeSplits []) {
        BZ_PROFILE_FUNCTION();

        SceneConstantBufferData sceneConstantBufferData;
        int lightIdx = 0;
        int totalCascadeIdx = 0;
        for (const auto &dirLight : scene.getDirectionalLights()) {
            for (uint32 cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
                sceneConstantBufferData.lightMatrices[totalCascadeIdx] = lightProjectionMatrices[totalCascadeIdx] * lightMatrices[totalCascadeIdx];
                sceneConstantBufferData.cascadeSplits[cascadeIdx] = cascadeSplits[cascadeIdx];
                totalCascadeIdx++;
            }

            const auto &dir = dirLight.getDirection();
            sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].x = dir.x;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].y = dir.y;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].z = dir.z;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].w = dirLight.intensity;
            sceneConstantBufferData.dirLightColors[lightIdx].r = dirLight.color.r;
            sceneConstantBufferData.dirLightColors[lightIdx].g = dirLight.color.g;
            sceneConstantBufferData.dirLightColors[lightIdx].b = dirLight.color.b;
            lightIdx++;
        }
        sceneConstantBufferData.dirLightCountAndRadianceMapMips.x = static_cast<float>(lightIdx);
        sceneConstantBufferData.dirLightCountAndRadianceMapMips.y = scene.hasSkyBox()?scene.getSkyBox().radianceMapView->getTexture()->getMipLevels():0.0f;
        memcpy(rendererData.sceneConstantBufferPtr, &sceneConstantBufferData, sizeof(SceneConstantBufferData));
    }

    void Renderer::fillPasses(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices) {
        BZ_PROFILE_FUNCTION();

        //Depth Pass data
        uint32 passIndex = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            for (uint32 cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
                uint32 passOffset = passIndex * sizeof(PassConstantBufferData);
                PassConstantBufferData passConstantBufferData;
                passConstantBufferData.viewMatrix = lightMatrices[passIndex];
                passConstantBufferData.projectionMatrix = lightProjectionMatrices[passIndex];
                passConstantBufferData.viewProjectionMatrix = passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
                memcpy(rendererData.passConstantBufferPtr + passOffset, &passConstantBufferData, sizeof(PassConstantBufferData));
                passIndex++;
            }
        }

        //Color pass data
        PassConstantBufferData passConstantBufferData;
        passConstantBufferData.viewMatrix = scene.getCamera().getViewMatrix();
        passConstantBufferData.projectionMatrix = scene.getCamera().getProjectionMatrix();
        passConstantBufferData.viewProjectionMatrix = passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
        const glm::vec3 &cameraPosition = scene.getCamera().getTransform().getTranslation();
        passConstantBufferData.cameraPosition.x = cameraPosition.x;
        passConstantBufferData.cameraPosition.y = cameraPosition.y;
        passConstantBufferData.cameraPosition.z = cameraPosition.z;

        uint32 colorPassOffset = PASS_CONSTANT_BUFFER_SIZE - sizeof(PassConstantBufferData); //Color pass is the last (after the Depth passes).
        memcpy(rendererData.passConstantBufferPtr + colorPassOffset, &passConstantBufferData, sizeof(PassConstantBufferData));
    }

    //TODO: There's no need to call this every frame, like it's being done now.
    void Renderer::fillMaterials(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        rendererData.materialOffsetMap.clear();

        if (scene.hasSkyBox()) {
            fillMaterial(scene.getSkyBox().mesh.getSubMeshIdx(0).material);
        }

        for (const auto &entity : scene.getEntities()) {
            if (entity.overrideMaterial.isValid()) {
                fillMaterial(entity.overrideMaterial);
            }
            else {
                for (const auto &submesh : entity.mesh.getSubmeshes()) {
                    fillMaterial(submesh.material);
                }
            }
        }

        uint32 materialCount = static_cast<uint32>(rendererData.materialOffsetMap.size());
        rendererData.stats.materialCount = materialCount;
        BZ_ASSERT_CORE(materialCount <= MAX_MATERIALS_PER_SCENE, "Reached the max number of Materials!");
    }

    void Renderer::fillMaterial(const Material &material) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(material.isValid(), "Trying to use an invalid/uninitialized Material!");

        const auto storedMaterialIt = rendererData.materialOffsetMap.find(material);

        //If it's the first time this Material is used on a Scene set the correspondent data.
        if (storedMaterialIt == rendererData.materialOffsetMap.end()) {
            const auto &uvScale = material.getUvScale();
            MaterialConstantBufferData materialConstantBufferData;
            materialConstantBufferData.normalMetallicRoughnessAndAO.x = material.hasNormalTexture() ? 1.0f : 0.0f;
            materialConstantBufferData.normalMetallicRoughnessAndAO.y = material.hasMetallicTexture() ? -1.0f : material.getMetallic();
            materialConstantBufferData.normalMetallicRoughnessAndAO.z = material.hasRoughnessTexture() ? -1.0f : material.getRoughness();
            materialConstantBufferData.normalMetallicRoughnessAndAO.w = material.hasAOTexture() ? 1.0f : 0.0f;

            materialConstantBufferData.heightAndUvScale.x = material.hasHeightTexture() ? material.getParallaxOcclusionScale() : -1.0f;
            materialConstantBufferData.heightAndUvScale.y = uvScale.x;
            materialConstantBufferData.heightAndUvScale.z = uvScale.y;

            uint32 materialOffset = static_cast<uint32>(rendererData.materialOffsetMap.size()) * sizeof(EntityConstantBufferData);
            memcpy(rendererData.materialConstantBufferPtr + materialOffset, &materialConstantBufferData, sizeof(MaterialConstantBufferData));

            rendererData.materialOffsetMap[material] = materialOffset;
        }
    }

    void Renderer::fillEntities(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        uint32 entityIndex = 0;
        for (const auto &entity : scene.getEntities()) {
            EntityConstantBufferData entityConstantBufferData;
            entityConstantBufferData.modelMatrix = entity.transform.getLocalToParentMatrix();
            entityConstantBufferData.normalMatrix = entity.transform.getNormalMatrix();

            uint32 entityOffset = entityIndex * sizeof(EntityConstantBufferData);
            memcpy(rendererData.entityConstantBufferPtr + entityOffset, &entityConstantBufferData, sizeof(EntityConstantBufferData));
            entityIndex++;
        }
        BZ_ASSERT_CORE(entityIndex <= MAX_ENTITIES_PER_SCENE, "Reached the max number of Entities!");
    }

    void Renderer::onImGuiRender(const FrameStats &frameStats) {
        BZ_PROFILE_FUNCTION();

        if (ImGui::Begin("Renderer")) {
            ImGui::Text("Depth Bias:");
            ImGui::DragFloat("ConstantFactor", &rendererData.depthBiasData.x, 0.05f, 0.0f, 10.0f);
            ImGui::DragFloat("Clamp", &rendererData.depthBiasData.y, 0.05f, -10.0f, 10.0f);
            ImGui::DragFloat("SlopeFactor", &rendererData.depthBiasData.z, 0.05f, 0.0f, 100.0f);
            ImGui::Separator();

            rendererData.statsRefreshTimeAcumMs += frameStats.lastFrameTime.asMillisecondsUint32();
            if (rendererData.statsRefreshTimeAcumMs >= rendererData.statsRefreshPeriodMs) {
                rendererData.statsRefreshTimeAcumMs = 0;
                rendererData.visibleStats = rendererData.stats;
            }
            ImGui::Text("Stats:");
            ImGui::Text("Vertex Count: %d", rendererData.visibleStats.vertexCount);
            ImGui::Text("Triangle Count: %d", rendererData.visibleStats.triangleCount);
            ImGui::Text("Draw Call Count: %d", rendererData.visibleStats.drawCallCount);
            ImGui::Text("Material Count: %d", rendererData.visibleStats.materialCount);
            ImGui::Separator();

            ImGui::SliderInt("Refresh period ms", reinterpret_cast<int*>(&rendererData.statsRefreshPeriodMs), 0, 1000);
        }
        ImGui::End();
    }

    void Renderer::computeCascadedShadowMappingSplits(float out[], uint32 splits, float nearPlane, float farPlane) {
        for (uint32 i = 1; i <= splits; ++i) {
            float log = nearPlane * std::pow(farPlane / nearPlane, static_cast<float>(i) / splits);
            float uni = nearPlane + (farPlane - nearPlane) * (static_cast<float>(i) / splits);
            out[i - 1] = -glm::mix(log, uni, 0.4f); //Negated to be on a right-handed camera space
        }
    }

    const DataLayout& Renderer::getVertexDataLayout() {
        return vertexDataLayout;
    }

    const DataLayout& Renderer::getIndexDataLayout() {
        return indexDataLayout;
    }

    DescriptorSet& Renderer::createSceneDescriptorSet() {
        auto &descriptorSet = DescriptorSet::get(rendererData.sceneDescriptorSetLayout);
        descriptorSet.setConstantBuffer(rendererData.constantBuffer, 0, SCENE_CONSTANT_BUFFER_OFFSET, sizeof(SceneConstantBufferData));
        return descriptorSet;
    }

    DescriptorSet& Renderer::createMaterialDescriptorSet() {
        auto &descriptorSet = DescriptorSet::get(rendererData.materialDescriptorSetLayout);
        descriptorSet.setConstantBuffer(rendererData.constantBuffer, 0, MATERIAL_CONSTANT_BUFFER_OFFSET, sizeof(MaterialConstantBufferData));
        return descriptorSet;
    }

    Ref<Framebuffer> Renderer::createShadowMapFramebuffer() {
        auto shadowMapRef = Texture2D::createRenderTarget(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, SHADOW_MAPPING_CASCADE_COUNT, rendererData.depthRenderPass->getDepthStencilAttachmentDescription()->format);
        glm::ivec3 dimsAndLayers(shadowMapRef->getDimensions().x, shadowMapRef->getDimensions().y, shadowMapRef->getLayers());
        return Framebuffer::create(rendererData.depthRenderPass, { TextureView::create(shadowMapRef, 0, shadowMapRef->getLayers()) }, dimsAndLayers);
    }

    const Ref<Sampler>& Renderer::getDefaultSampler() {
        return rendererData.defaultSampler;
    }

    const Ref<Sampler>& Renderer::getShadowSampler() {
        return rendererData.shadowSampler;
    }

    const Ref<TextureView>& Renderer::getDummyTextureView() {
        //Since this texture is permanentely bound, might as well be the dummy texture.
        return rendererData.brdfLookupTexture;
    }

    const Ref<TextureView>& Renderer::getDummyTextureArrayView() {
        return rendererData.dummyTextureArrayView;
    }
}
