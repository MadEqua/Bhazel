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

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) PassConstantBufferData {
        glm::mat4 viewMatrix; //World to camera space
        glm::mat4 projectionMatrix; //Camera to clip space
        glm::mat4 viewProjectionMatrix; //World to clip space
        glm::vec4 cameraPosition; //mat4 to simplify alignments
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) SceneConstantBufferData {
        glm::mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE]; //World to light clip space
        glm::vec4 dirLightDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
        glm::vec4 dirLightColors[MAX_DIR_LIGHTS_PER_SCENE]; //vec4 to simplify alignments
        glm::vec2 dirLightCountAndRadianceMapMips;
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) EntityConstantBufferData {
        glm::mat4 modelMatrix; //Model to world space
        glm::mat4 normalMatrix; //Model to world space, appropriate to transform vectors, mat4 to simplify alignments
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) MaterialConstantBufferData {
        glm::vec4 normalMetallicRoughnessAndAO;
        glm::vec4 heightAndUvScale;
    };

    constexpr uint32 PASS_CONSTANT_BUFFER_SIZE = sizeof(PassConstantBufferData) * MAX_PASSES_PER_FRAME;
    constexpr uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData);
    constexpr uint32 ENTITY_CONSTANT_BUFFER_SIZE = sizeof(EntityConstantBufferData) * MAX_ENTITIES_PER_SCENE;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_SIZE = sizeof(MaterialConstantBufferData) * MAX_MATERIALS_PER_SCENE;

    constexpr uint32 PASS_CONSTANT_BUFFER_OFFSET = 0;
    constexpr uint32 SCENE_CONSTANT_BUFFER_OFFSET = PASS_CONSTANT_BUFFER_SIZE;
    constexpr uint32 ENTITY_CONSTANT_BUFFER_OFFSET = SCENE_CONSTANT_BUFFER_OFFSET + SCENE_CONSTANT_BUFFER_SIZE;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_OFFSET = ENTITY_CONSTANT_BUFFER_OFFSET + ENTITY_CONSTANT_BUFFER_SIZE;

    constexpr uint32 SHADOW_MAP_SIZE = 1024;

    static DataLayout vertexDataLayout = {
        { DataType::Float32, DataElements::Vec3, "POSITION" },
        { DataType::Float32, DataElements::Vec3, "NORMAL" },
        { DataType::Float32, DataElements::Vec3, "TANGENT" },
        { DataType::Float32, DataElements::Vec3, "BITANGENT" },
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
        BufferPtr passConstantBufferPtr;
        BufferPtr sceneConstantBufferPtr;
        BufferPtr entityConstantBufferPtr;
        BufferPtr materialConstantBufferPtr;

        Ref<DescriptorSetLayout> globalDescriptorSetLayout;
        Ref<DescriptorSetLayout> passDescriptorSetLayout;
        Ref<DescriptorSetLayout> sceneDescriptorSetLayout;
        Ref<DescriptorSetLayout> entityDescriptorSetLayout;
        Ref<DescriptorSetLayout> materialDescriptorSetLayout;

        Ref<DescriptorSet> globalDescriptorSet;
        Ref<DescriptorSet> passDescriptorSet;
        Ref<DescriptorSet> entityDescriptorSet;

        Ref<Sampler> defaultSampler;
        Ref<Sampler> brdfLookupSampler;
        Ref<Sampler> shadowSampler;

        Ref<PipelineState> defaultPipelineState;
        Ref<PipelineState> skyBoxPipelineState;
        Ref<PipelineState> depthPassPipelineState;

        Ref<TextureView> brdfLookupTexture;

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
            PASS_CONSTANT_BUFFER_SIZE + SCENE_CONSTANT_BUFFER_SIZE + ENTITY_CONSTANT_BUFFER_SIZE + MATERIAL_CONSTANT_BUFFER_SIZE,
            MemoryType::CpuToGpu);

        rendererData.passConstantBufferPtr = rendererData.constantBuffer->map(0);
        rendererData.sceneConstantBufferPtr = rendererData.passConstantBufferPtr + SCENE_CONSTANT_BUFFER_OFFSET;
        rendererData.entityConstantBufferPtr = rendererData.passConstantBufferPtr + ENTITY_CONSTANT_BUFFER_OFFSET;
        rendererData.materialConstantBufferPtr = rendererData.passConstantBufferPtr + MATERIAL_CONSTANT_BUFFER_OFFSET;

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.globalDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder2;
        descriptorSetLayoutBuilder2.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Vertex), 1);
        rendererData.passDescriptorSetLayout = descriptorSetLayoutBuilder2.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder3;
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::All), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), MAX_DIR_LIGHTS_PER_SCENE);
        rendererData.sceneDescriptorSetLayout = descriptorSetLayoutBuilder3.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder4;
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Vertex), 1);
        rendererData.entityDescriptorSetLayout = descriptorSetLayoutBuilder4.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder5;
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Fragment), 1);

        //Albedo, Normal, Metallic, Roughness, Height and AO.
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.materialDescriptorSetLayout = descriptorSetLayoutBuilder5.build();

        //DefaultPipelineState
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("DefaultRenderer");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/DefaultVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/DefaultFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.passDescriptorSetLayout,
                                                   rendererData.sceneDescriptorSetLayout, rendererData.entityDescriptorSetLayout,
                                                   rendererData.materialDescriptorSetLayout };

        DepthStencilState depthStencilState;
        depthStencilState.enableDepthTest = true;
        depthStencilState.enableDepthWrite = true;
        depthStencilState.depthCompareFunction = CompareFunction::Less;
        pipelineStateData.depthStencilState = depthStencilState;

        RasterizerState rasterizerState;
        rasterizerState.cullMode = CullMode::Back;
        rasterizerState.frontFaceCounterClockwise = true;
        pipelineStateData.rasterizerState = rasterizerState;

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };
        pipelineStateData.blendingState = blendingState;

        const auto WINDOW_DIMS_INT = Application::getInstance().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::getInstance().getWindow().getDimensionsFloat();

        pipelineStateData.dataLayout = vertexDataLayout;
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };
        rendererData.defaultPipelineState = PipelineState::create(pipelineStateData);

        //SkyBoxPipelineState
        shaderBuilder.setName("SkyBox");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/SkyBoxVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/SkyBoxFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();
        rendererData.skyBoxPipelineState = PipelineState::create(pipelineStateData);

        //DepthPassPipelineState
        AttachmentDescription depthStencilAttachmentDesc;
        depthStencilAttachmentDesc.format = TextureFormatEnum::D32_SFLOAT;
        depthStencilAttachmentDesc.samples = 1;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
        depthStencilAttachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorStencil = StoreOperation::DontCare;
        depthStencilAttachmentDesc.initialLayout = TextureLayout::Undefined;
        depthStencilAttachmentDesc.finalLayout = TextureLayout::ShaderReadOnlyOptimal; //TODO: This is not optimal for a depth texture, but works. We should pick the optimal layout and then work with layout transitions.
        depthStencilAttachmentDesc.clearValues.floating.x = 1.0f;
        depthStencilAttachmentDesc.clearValues.integer.y = 0;
        rendererData.depthRenderPass = RenderPass::create({ depthStencilAttachmentDesc });
        pipelineStateData.renderPass = rendererData.depthRenderPass;

        pipelineStateData.rasterizerState.enableDepthBias = true;
        //pipelineStateData.rasterizerState.depthBiasConstantFactor = 0.0f;
        //pipelineStateData.rasterizerState.depthBiasSlopeFactor = 0.0f;
        pipelineStateData.dynamicStates = { DynamicState::DepthBias };

        pipelineStateData.blendingState = {};

        pipelineStateData.viewports = { { 0.0f, 0.0f, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } };
        pipelineStateData.scissorRects = { { 0u, 0u, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } };

        shaderBuilder.setName("ShadowPass");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/ShadowPassVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/ShadowPassFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        rendererData.depthPassPipelineState = PipelineState::create(pipelineStateData);

        Sampler::Builder samplerBuilder;
        rendererData.defaultSampler = samplerBuilder.build();

        Sampler::Builder samplerBuilder2;
        samplerBuilder2.setAddressModeAll(AddressMode::ClampToBorder);
        samplerBuilder2.enableCompare(CompareFunction::Less);
        rendererData.shadowSampler = samplerBuilder2.build();

        Sampler::Builder samplerBuilder3;
        samplerBuilder3.setAddressModeAll(AddressMode::ClampToEdge);
        rendererData.brdfLookupSampler = samplerBuilder3.build();

        auto brdfLookupTexRef = Texture2D::create(reinterpret_cast<const byte*>(brdfLut), brdfLutSize, brdfLutSize, TextureFormatEnum::R16G16_SFLOAT, MipmapData::Options::DoNothing);
        rendererData.brdfLookupTexture = TextureView::create(brdfLookupTexRef);

        rendererData.globalDescriptorSet = DescriptorSet::create(rendererData.globalDescriptorSetLayout);
        rendererData.globalDescriptorSet->setCombinedTextureSampler(rendererData.brdfLookupTexture, rendererData.brdfLookupSampler, 0);

        rendererData.passDescriptorSet = DescriptorSet::create(rendererData.passDescriptorSetLayout);
        rendererData.passDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, PASS_CONSTANT_BUFFER_OFFSET, sizeof(PassConstantBufferData));

        rendererData.entityDescriptorSet = DescriptorSet::create(rendererData.entityDescriptorSetLayout);
        rendererData.entityDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, ENTITY_CONSTANT_BUFFER_OFFSET, sizeof(EntityConstantBufferData));
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.constantBuffer.reset();

        rendererData.globalDescriptorSetLayout.reset();
        rendererData.passDescriptorSetLayout.reset();
        rendererData.sceneDescriptorSetLayout.reset();
        rendererData.entityDescriptorSetLayout.reset();
        rendererData.materialDescriptorSetLayout.reset();

        rendererData.globalDescriptorSet.reset();
        rendererData.passDescriptorSet.reset();
        rendererData.entityDescriptorSet.reset();

        rendererData.defaultPipelineState.reset();
        rendererData.skyBoxPipelineState.reset();
        rendererData.depthPassPipelineState.reset();

        rendererData.defaultSampler.reset();
        rendererData.brdfLookupSampler.reset();
        rendererData.shadowSampler.reset();

        rendererData.materialOffsetMap.clear();

        rendererData.brdfLookupTexture.reset();

        rendererData.depthRenderPass.reset();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&rendererData.stats, 0, sizeof(RendererStats));
 
        fillConstants(scene);

        rendererData.commandBufferId = Graphics::beginCommandBuffer();

        //Bind stuff that will not change, no matter the PipelineState (All of them have the same layout so the bindings will not be lost with changes).
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.globalDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_GLOBAL_DESCRIPTOR_SET_IDX, 0, 0);

        Graphics::bindDescriptorSet(rendererData.commandBufferId, scene.getDescriptorSet(),
            rendererData.defaultPipelineState, RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

        depthPass(scene);
        colorPass(scene);

        Graphics::endCommandBuffer(rendererData.commandBufferId);
    }

    void Renderer::depthPass(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.depthPassPipelineState);
        Graphics::setDepthBias(rendererData.commandBufferId, rendererData.depthBiasData.x, rendererData.depthBiasData.y, rendererData.depthBiasData.z);

        uint32 passIndex = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            uint32 passOffset = passIndex * sizeof(PassConstantBufferData);
            Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.passDescriptorSet,
                rendererData.depthPassPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, &passOffset, 1);

            Graphics::beginRenderPass(rendererData.commandBufferId, dirLight.shadowMapFramebuffer);

            uint32 entityIndex = 0;
            for (const auto &entity : scene.getEntities()) {
                if (entity.castShadow) {
                    drawEntity(entity, entityIndex);
                }
                entityIndex++;
            }

            Graphics::endRenderPass(rendererData.commandBufferId);
            passIndex++;
        }
    }

    void Renderer::colorPass(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        uint32 colorPassOffset = PASS_CONSTANT_BUFFER_SIZE - sizeof(PassConstantBufferData); //Color pass is the last (after the Depth passes).

        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.passDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, &colorPassOffset, 1);

        Graphics::beginRenderPass(rendererData.commandBufferId);

        if (scene.hasSkyBox()) {
            Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.skyBoxPipelineState);
            drawMesh(scene.getSkyBox().mesh, Material());
        }

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.defaultPipelineState);
        uint32 entityIndex = 0;
        for (const auto &entity : scene.getEntities()) {
            drawEntity(entity, entityIndex++);
        }

        Graphics::endRenderPass(rendererData.commandBufferId);
    }

    void Renderer::drawEntity(const Entity &entity, uint32 index) {
        BZ_PROFILE_FUNCTION();

        uint32 entityOffset = index * sizeof(EntityConstantBufferData);
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.entityDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_ENTITY_DESCRIPTOR_SET_IDX, &entityOffset, 1);
        drawMesh(entity.mesh, entity.overrideMaterial);
    }

    void Renderer::drawMesh(const Mesh &mesh, const Material &overrideMaterial) {
        BZ_PROFILE_FUNCTION();

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer(), 0);
        
        if (mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer(), 0);

        for (const auto &submesh : mesh.getSubmeshes()) {
            const Material &materialToUse = overrideMaterial.isValid() ? overrideMaterial : submesh.material;

            uint32 materialOffset = rendererData.materialOffsetMap[materialToUse];
            Graphics::bindDescriptorSet(rendererData.commandBufferId, materialToUse.getDescriptorSet(),
                rendererData.defaultPipelineState, RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);

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
        glm::mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE];
        glm::mat4 lightProjectionMatrices[MAX_DIR_LIGHTS_PER_SCENE];

        const PerspectiveCamera &camera = static_cast<const PerspectiveCamera&>(scene.getCamera());
        const PerspectiveCamera::Parameters &cameraParams = camera.getParameters();

        //Compute Sphere that enconpasses the frustum, in camera space.
        //The sphere is useful to have the shadow frustum to be always the same size regardless of the camera orientation, leading to no shadow flickering.
        float t = glm::tan(glm::radians(cameraParams.aspectRatio * cameraParams.fovy * 0.5f));
        glm::vec3 n(t * cameraParams.near, 0.0f, -cameraParams.near);
        glm::vec3 f(t * cameraParams.far, 0.0f, -cameraParams.far);

        //Solve the equation |f-center|=|n-center|, knowing that the sphere center is (0, 0, centerZ).
        float centerZ = ((f.x * f.x + f.z * f.z) - (n.x * n.x + n.z * n.z)) / (2.0f * f.z - 2.0f * n.z);
        float r = glm::distance(glm::vec3(0.0f, 0.0f, centerZ), n);

        glm::vec3 sphereCenterWorld = camera.getTransform().getLocalToParentMatrix() * glm::vec4(0.0f, 0.0f, centerZ, 1.0f);

        //Units of view space per shadow map texel (and world space, assuming no scaling between the two spaces).
        const float Q = (r * 2.0f) / SHADOW_MAP_SIZE;

        uint32 lightIndex = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {  
            Transform lightTransform = {};

            lightTransform.setTranslation(sphereCenterWorld - dirLight.getDirection() * r);
            lightTransform.lookAt(sphereCenterWorld);

            lightMatrices[lightIndex] = lightTransform.getParentToLocalMatrix();

            //Apply the quantization to translation to stabilize shadows when camera moves. We only the light camera in texel sized snaps.
            lightMatrices[lightIndex][3].x = glm::floor(lightMatrices[lightIndex][3].x / Q) * Q;
            lightMatrices[lightIndex][3].y = glm::floor(lightMatrices[lightIndex][3].y / Q) * Q;
            lightMatrices[lightIndex][3].z = glm::floor(lightMatrices[lightIndex][3].z / Q) * Q;

            lightProjectionMatrices[lightIndex] = Utils::ortho(-r, r, -r, r, 0.1f, r * 2.0f);
            lightIndex++;
        }

        fillPasses(scene, lightMatrices, lightProjectionMatrices);
        fillScene(scene, lightMatrices, lightProjectionMatrices);
        fillMaterials(scene);
        fillEntities(scene);
    }

    void Renderer::fillPasses(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices) {
        BZ_PROFILE_FUNCTION();

        //Depth Pass data
        uint32 passIndex = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            uint32 passOffset = passIndex * sizeof(PassConstantBufferData);
            PassConstantBufferData passConstantBufferData;
            passConstantBufferData.viewMatrix = lightMatrices[passIndex];
            passConstantBufferData.projectionMatrix = lightProjectionMatrices[passIndex];
            passConstantBufferData.viewProjectionMatrix = passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
            memcpy(rendererData.passConstantBufferPtr + passOffset, &passConstantBufferData, sizeof(glm::mat4) * 3);
            passIndex++;
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

    void Renderer::fillScene(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices) {
        BZ_PROFILE_FUNCTION();

        SceneConstantBufferData sceneConstantBufferData;
        int i = 0;
        for (const auto &dirLight : scene.getDirectionalLights()) {
            sceneConstantBufferData.lightMatrices[i] = lightProjectionMatrices[i] * lightMatrices[i];

            const auto &dir = dirLight.getDirection();
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].x = dir.x;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].y = dir.y;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].z = dir.z;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].w = dirLight.intensity;
            sceneConstantBufferData.dirLightColors[i].r = dirLight.color.r;
            sceneConstantBufferData.dirLightColors[i].g = dirLight.color.g;
            sceneConstantBufferData.dirLightColors[i].b = dirLight.color.b;
            i++;
        }
        sceneConstantBufferData.dirLightCountAndRadianceMapMips.x = static_cast<float>(i);
        sceneConstantBufferData.dirLightCountAndRadianceMapMips.y = scene.hasSkyBox()?scene.getSkyBox().radianceMapView->getTexture()->getMipLevels():0.0f;
        memcpy(rendererData.sceneConstantBufferPtr, &sceneConstantBufferData, sizeof(SceneConstantBufferData));
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
        auto shadowMapRef = Texture2D::createRenderTarget(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, rendererData.depthRenderPass->getDepthStencilAttachmentDescription()->format);
        return Framebuffer::create(rendererData.depthRenderPass, { TextureView::create(shadowMapRef) }, shadowMapRef->getDimensions());
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
}
