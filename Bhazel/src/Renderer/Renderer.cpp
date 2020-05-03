#include "bzpch.h"

#include "Renderer.h"

#include "BRDFLookup.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"

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
        { DataType::Float32, DataElements::Vec3, "POSITION" },
        { DataType::Float32, DataElements::Vec3, "NORMAL" },
        { DataType::Float32, DataElements::Vec4, "TANGENT" },
        { DataType::Uint16, DataElements::Vec2, "TEXCOORD", true },
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
        uint32 commandBufferId;

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

        Ref<DescriptorSet> globalDescriptorSet;
        Ref<DescriptorSet> passDescriptorSet;
        Ref<DescriptorSet> passDescriptorSetForDepthPass;
        Ref<DescriptorSet> entityDescriptorSet;
        Ref<DescriptorSet> postProcessDescriptorSet;

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
        glm::vec3 depthBiasData = {1.0f, 0.0f, 2.5f};

        //Stats
        RendererStats stats;
        RendererStats visibleStats;

        uint32 statsRefreshPeriodMs = 250;
        uint32 statsRefreshTimeAcumMs;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        rendererData.constantBuffer = Buffer::create(BufferType::Constant, 
            SCENE_CONSTANT_BUFFER_SIZE + PASS_CONSTANT_BUFFER_SIZE + MATERIAL_CONSTANT_BUFFER_SIZE + ENTITY_CONSTANT_BUFFER_SIZE,
            MemoryType::CpuToGpu);

        rendererData.sceneConstantBufferPtr = rendererData.constantBuffer->map(0);
        rendererData.passConstantBufferPtr = rendererData.sceneConstantBufferPtr + PASS_CONSTANT_BUFFER_OFFSET;
        rendererData.materialConstantBufferPtr = rendererData.sceneConstantBufferPtr + MATERIAL_CONSTANT_BUFFER_OFFSET;
        rendererData.entityConstantBufferPtr = rendererData.sceneConstantBufferPtr + ENTITY_CONSTANT_BUFFER_OFFSET;

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        rendererData.globalDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        descriptorSetLayoutBuilder.reset();
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlag::All), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), MAX_DIR_LIGHTS_PER_SCENE);
        rendererData.sceneDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        descriptorSetLayoutBuilder.reset();
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlag::Vertex | ShaderStageFlag::Geometry), 1);
        rendererData.passDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        descriptorSetLayoutBuilder.reset();
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlag::Fragment), 1);
        //Albedo, Normal, Metallic, Roughness, Height and AO.
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        rendererData.materialDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        descriptorSetLayoutBuilder.reset();
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlag::Vertex | ShaderStageFlag::Geometry), 1);
        rendererData.entityDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        rendererData.entityDescriptorSet = DescriptorSet::create(rendererData.entityDescriptorSetLayout);
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

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlag::Vertex | ShaderStageFlag::Geometry), SHADOW_MAPPING_CASCADE_COUNT);
        rendererData.passDescriptorSetLayoutForDepthPass = descriptorSetLayoutBuilder.build();

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexDataLayout;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("DepthPass");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/DepthPassVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Geometry, "Bhazel/shaders/bin/DepthPassGeo.spv");
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                                   rendererData.passDescriptorSetLayoutForDepthPass, rendererData.materialDescriptorSetLayout,
                                                   rendererData.entityDescriptorSetLayout };
        pipelineStateData.viewports = { { 0.0f, 0.0f, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } };
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
        depthStencilState.depthCompareFunction = CompareFunction::Less;
        pipelineStateData.depthStencilState = depthStencilState;

        pipelineStateData.dynamicStates = { DynamicState::DepthBias };

        AttachmentDescription depthStencilAttachmentDesc;
        depthStencilAttachmentDesc.format = TextureFormatEnum::D32_SFLOAT;
        depthStencilAttachmentDesc.samples = 1;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
        depthStencilAttachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorStencil = StoreOperation::DontCare;
        depthStencilAttachmentDesc.initialLayout = TextureLayout::Undefined;
        depthStencilAttachmentDesc.finalLayout = TextureLayout::DepthStencilAttachmentOptimal;
        depthStencilAttachmentDesc.clearValues.floating.x = 1.0f;
        depthStencilAttachmentDesc.clearValues.integer.y = 0;

        SubPassDescription subPassDesc;
        subPassDesc.depthStencilAttachmentsRef = { 0, TextureLayout::DepthStencilAttachmentOptimal };

        rendererData.depthRenderPass = RenderPass::create({ depthStencilAttachmentDesc }, { subPassDesc });
        pipelineStateData.renderPass = rendererData.depthRenderPass;
        pipelineStateData.subPassIndex = 0;

        rendererData.depthPassPipelineState = PipelineState::create(pipelineStateData);

        rendererData.passDescriptorSetForDepthPass = DescriptorSet::create(rendererData.passDescriptorSetLayoutForDepthPass);
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
        samplerBuilder.setAddressModeAll(AddressMode::ClampToBorder);
        samplerBuilder.enableCompare(CompareFunction::Less);
        rendererData.shadowSampler = samplerBuilder.build();
    }

    void Renderer::initDefaultPassData() {
        BZ_PROFILE_FUNCTION();

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexDataLayout;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("DefaultRenderer");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/RendererVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/RendererFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                                   rendererData.passDescriptorSetLayout, rendererData.materialDescriptorSetLayout,
                                                   rendererData.entityDescriptorSetLayout };

        const auto WINDOW_DIMS_INT = Application::getInstance().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::getInstance().getWindow().getDimensionsFloat();
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };

        RasterizerState rasterizerState;
        rasterizerState.cullMode = CullMode::Back;
        rasterizerState.frontFaceCounterClockwise = true;
        pipelineStateData.rasterizerState = rasterizerState;

        DepthStencilState depthStencilState;
        depthStencilState.enableDepthTest = true;
        depthStencilState.enableDepthWrite = true;
        depthStencilState.depthCompareFunction = CompareFunction::Less;
        pipelineStateData.depthStencilState = depthStencilState;

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        pipelineStateData.renderPass = Application::getInstance().getGraphicsContext().getMainRenderPass();
        pipelineStateData.subPassIndex = 0;

        rendererData.defaultPipelineState = PipelineState::create(pipelineStateData);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        rendererData.brdfLookupSampler = samplerBuilder.build();

        auto brdfLookupTexRef = Texture2D::create(reinterpret_cast<const byte*>(brdfLut), brdfLutSize, brdfLutSize, TextureFormatEnum::R16G16_SFLOAT, MipmapData::Options::DoNothing);
        rendererData.brdfLookupTexture = TextureView::create(brdfLookupTexRef);

        rendererData.globalDescriptorSet = DescriptorSet::create(rendererData.globalDescriptorSetLayout);
        rendererData.globalDescriptorSet->setCombinedTextureSampler(rendererData.brdfLookupTexture, rendererData.brdfLookupSampler, 0);

        rendererData.passDescriptorSet = DescriptorSet::create(rendererData.passDescriptorSetLayout);
        rendererData.passDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, PASS_CONSTANT_BUFFER_OFFSET, sizeof(PassConstantBufferData));

        rendererData.dummyTextureArrayView = TextureView::create(brdfLookupTexRef, 0, 1);
    }

    void Renderer::initPostProcessPassData() {
        BZ_PROFILE_FUNCTION();

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = {};

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("PostProcess");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/FullScreenQuadVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/ToneMapFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlag::Fragment), 1);
        rendererData.postProcessPassDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        pipelineStateData.descriptorSetLayouts = { rendererData.postProcessPassDescriptorSetLayout };

        const auto WINDOW_DIMS_INT = Application::getInstance().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::getInstance().getWindow().getDimensionsFloat();
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        pipelineStateData.renderPass = Application::getInstance().getGraphicsContext().getSwapchainRenderPass();
        pipelineStateData.subPassIndex = 0;

        rendererData.postProcessPipelineState = PipelineState::create(pipelineStateData);

        rendererData.postProcessDescriptorSet = DescriptorSet::create(rendererData.postProcessPassDescriptorSetLayout);
        rendererData.postProcessDescriptorSet->setCombinedTextureSampler(Application::getInstance().getGraphicsContext().getColorTextureView(),
            rendererData.defaultSampler, 0);
    }

    void Renderer::initSkyBoxData() {
        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexDataLayout;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("SkyBox");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/SkyBoxVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/SkyBoxFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                                   rendererData.passDescriptorSetLayout, rendererData.materialDescriptorSetLayout,
                                                   rendererData.entityDescriptorSetLayout };

        const auto WINDOW_DIMS_INT = Application::getInstance().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::getInstance().getWindow().getDimensionsFloat();
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };

        RasterizerState rasterizerState;
        rasterizerState.cullMode = CullMode::Back;
        rasterizerState.frontFaceCounterClockwise = true;
        pipelineStateData.rasterizerState = rasterizerState;

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        pipelineStateData.renderPass = Application::getInstance().getGraphicsContext().getMainRenderPass();
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

        rendererData.globalDescriptorSet.reset();
        rendererData.passDescriptorSet.reset();
        rendererData.passDescriptorSetForDepthPass.reset();
        rendererData.entityDescriptorSet.reset();
        rendererData.postProcessDescriptorSet.reset();

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

        rendererData.commandBufferId = Graphics::beginCommandBuffer();

        //Bind stuff that will not change.
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.globalDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_GLOBAL_DESCRIPTOR_SET_IDX, 0, 0);
        
        Graphics::bindDescriptorSet(rendererData.commandBufferId, scene.getDescriptorSet(),
            rendererData.defaultPipelineState, RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

        depthPass(scene);
        colorPass(scene);
        postProcessPass();

        Graphics::endCommandBuffer(rendererData.commandBufferId);
    }

    void Renderer::depthPass(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.depthPassPipelineState);
        Graphics::setDepthBias(rendererData.commandBufferId, rendererData.depthBiasData.x, rendererData.depthBiasData.y, rendererData.depthBiasData.z);

        uint32 lightIdx = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            uint32 lightOffset = lightIdx * sizeof(PassConstantBufferData) * SHADOW_MAPPING_CASCADE_COUNT;
            uint32 lightOffsetArr[SHADOW_MAPPING_CASCADE_COUNT];
            for (uint32 i = 0; i < SHADOW_MAPPING_CASCADE_COUNT; ++i) {
                lightOffsetArr[i] = lightOffset;
            }
            Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.passDescriptorSetForDepthPass,
                rendererData.depthPassPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, lightOffsetArr, SHADOW_MAPPING_CASCADE_COUNT);

            Graphics::beginRenderPass(rendererData.commandBufferId, dirLight.shadowMapFramebuffer);
            drawEntities(scene, true);
            Graphics::endRenderPass(rendererData.commandBufferId);

            //Avoid starting the color pass before the depth pass is finished writing to the textures.
            Graphics::pipelineBarrierTexture(rendererData.commandBufferId, dirLight.shadowMapFramebuffer->getDepthStencilTextureView()->getTexture());

            lightIdx++;
        }
    }

    void Renderer::colorPass(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        uint32 colorPassOffset = PASS_CONSTANT_BUFFER_SIZE - sizeof(PassConstantBufferData); //Color pass is the last (after the Depth passes).
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.passDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, &colorPassOffset, 1);

        Graphics::beginRenderPass(rendererData.commandBufferId, Application::getInstance().getGraphicsContext().getMainFramebuffer());

        if (scene.hasSkyBox()) {
            Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.skyBoxPipelineState);
            drawMesh(scene.getSkyBox().mesh, Material(), false);
        }

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.defaultPipelineState);
        drawEntities(scene, false);
        Graphics::endRenderPass(rendererData.commandBufferId);
    }

    void Renderer::postProcessPass() {
        BZ_PROFILE_FUNCTION();

        Graphics::beginRenderPass(rendererData.commandBufferId, Application::getInstance().getGraphicsContext().getCurrentSwapchainFramebuffer());
        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.postProcessPipelineState);
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.postProcessDescriptorSet, rendererData.postProcessPipelineState, 0, nullptr, 0);
        Graphics::draw(rendererData.commandBufferId, 3, 1, 0, 0);
        Graphics::endRenderPass(rendererData.commandBufferId);
    }

    void Renderer::drawEntities(const Scene &scene, bool depthPass) {
        BZ_PROFILE_FUNCTION();

        uint32 entityIndex = 0;
        for (const auto &entity : scene.getEntities()) {
            if (!depthPass || entity.castShadow) {
                uint32 entityOffset = entityIndex * sizeof(EntityConstantBufferData);
                Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.entityDescriptorSet,
                    depthPass ? rendererData.depthPassPipelineState : rendererData.defaultPipelineState,
                    RENDERER_ENTITY_DESCRIPTOR_SET_IDX, &entityOffset, 1);

                drawMesh(entity.mesh, entity.overrideMaterial, depthPass);
                entityIndex++;
            }
        }
    }

    void Renderer::drawMesh(const Mesh &mesh, const Material &overrideMaterial, bool depthPass) {
        BZ_PROFILE_FUNCTION();

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer(), 0);
        
        if (mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer(), 0);

        for (const auto &submesh : mesh.getSubmeshes()) {
            if (!depthPass) {
                const Material &materialToUse = overrideMaterial.isValid() ? overrideMaterial : submesh.material;

                uint32 materialOffset = rendererData.materialOffsetMap[materialToUse];
                Graphics::bindDescriptorSet(rendererData.commandBufferId, materialToUse.getDescriptorSet(),
                    depthPass ? rendererData.depthPassPipelineState : rendererData.defaultPipelineState,
                    RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);
            }

            if (mesh.hasIndices())
                Graphics::drawIndexed(rendererData.commandBufferId, submesh.indexCount, 1, submesh.indexOffset, 0, 0);
            else
                Graphics::draw(rendererData.commandBufferId, submesh.vertexCount, 1, submesh.vertexOffset, 1);

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

    Ref<DescriptorSet> Renderer::createSceneDescriptorSet() {
        auto descriptorSet = DescriptorSet::create(rendererData.sceneDescriptorSetLayout);
        descriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, SCENE_CONSTANT_BUFFER_OFFSET, sizeof(SceneConstantBufferData));
        return descriptorSet;
    }

    Ref<DescriptorSet> Renderer::createMaterialDescriptorSet() {
        auto descriptorSet = DescriptorSet::create(rendererData.materialDescriptorSetLayout);
        descriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, MATERIAL_CONSTANT_BUFFER_OFFSET, sizeof(MaterialConstantBufferData));
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
