#include "bzpch.h"

#include "Renderer.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"

#include "Core/Application.h"

#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"


namespace BZ {

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) PassConstantBufferData {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 viewProjectionMatrix;
        glm::vec4 cameraPosition; //mat4 to simplify alignments
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) SceneConstantBufferData {
        glm::vec4 dirLightDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
        glm::vec4 dirLightColors[MAX_DIR_LIGHTS_PER_SCENE]; //vec4 to simplify alignments
        glm::vec2 dirLightCountAndRadianceMapMips;
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) EntityConstantBufferData {
        glm::mat4 modelMatrix;
        glm::mat4 normalMatrix; //mat4 to simplify alignments
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) MaterialConstantBufferData {
        float parallaxOcclusionScale;
    };

    constexpr uint32 PASS_CONSTANT_BUFFER_SIZE = sizeof(PassConstantBufferData) * MAX_PASSES_PER_FRAME;
    constexpr uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData);
    constexpr uint32 ENTITY_CONSTANT_BUFFER_SIZE = sizeof(EntityConstantBufferData) * MAX_ENTITIES_PER_SCENE;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_SIZE = sizeof(MaterialConstantBufferData) * MAX_MATERIALS_PER_SCENE;

    constexpr uint32 PASS_CONSTANT_BUFFER_OFFSET = 0;
    constexpr uint32 SCENE_CONSTANT_BUFFER_OFFSET = PASS_CONSTANT_BUFFER_SIZE;
    constexpr uint32 ENTITY_CONSTANT_BUFFER_OFFSET = SCENE_CONSTANT_BUFFER_OFFSET + SCENE_CONSTANT_BUFFER_SIZE;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_OFFSET = ENTITY_CONSTANT_BUFFER_OFFSET + ENTITY_CONSTANT_BUFFER_SIZE;

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

    RendererStats Renderer::stats;

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

        Ref<PipelineState> defaultPipelineState;
        Ref<PipelineState> skyBoxPipelineState;
        Ref<PipelineState> depthPassPipelineState;

        Ref<TextureView> brdfLookupTexture;

        std::unordered_map<Material, uint32> materialOffsetMap;

        Ref<RenderPass> depthRenderPass;
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
        rendererData.sceneDescriptorSetLayout = descriptorSetLayoutBuilder3.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder4;
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Vertex), 1);
        rendererData.entityDescriptorSetLayout = descriptorSetLayoutBuilder4.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder5;
        descriptorSetLayoutBuilder5.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Fragment), 1);

        //Albedo, Normal, Metallic, Roughness and Height textures
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
        depthStencilState.depthTestFunction = TestFunction::Less;
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
        depthStencilAttachmentDesc.format = TextureFormat::D24S8;
        depthStencilAttachmentDesc.samples = 1;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
        depthStencilAttachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorStencil = StoreOperation::Store;
        depthStencilAttachmentDesc.initialLayout = TextureLayout::Undefined;
        depthStencilAttachmentDesc.finalLayout = TextureLayout::DepthStencilAttachmentOptimal;
        depthStencilAttachmentDesc.clearValues.floating.x = 1.0f;
        depthStencilAttachmentDesc.clearValues.integer.y = 0;
        rendererData.depthRenderPass = RenderPass::create({ depthStencilAttachmentDesc });
        pipelineStateData.renderPass = rendererData.depthRenderPass;

        pipelineStateData.blendingState = {};

        shaderBuilder.setName("ShadowPass");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/ShadowPassVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/ShadowPassFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        rendererData.depthPassPipelineState = PipelineState::create(pipelineStateData);

        Sampler::Builder samplerBuilder;
        rendererData.defaultSampler = samplerBuilder.build();

        //The ideal 2 Channels (RG) are not supported by stbi. 3 channels is badly supported by Vulkan implementations. So 4 channels...
        auto brdfTex = Texture2D::create("Bhazel/textures/ibl_brdf_lut.png", TextureFormat::R8G8B8A8, MipmapData::Options::DoNothing);
        rendererData.brdfLookupTexture = TextureView::create(brdfTex);

        rendererData.globalDescriptorSet = DescriptorSet::create(rendererData.globalDescriptorSetLayout);
        rendererData.globalDescriptorSet->setCombinedTextureSampler(rendererData.brdfLookupTexture, rendererData.defaultSampler, 0);

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
        rendererData.materialOffsetMap.clear();

        rendererData.brdfLookupTexture.reset();

        rendererData.depthRenderPass.reset();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&stats, 0, sizeof(stats));
 
        //Fill respective ConstantBuffer data.
        handleMaterials(scene);
        handleEntities(scene);

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
        const Camera &camera = scene.getCamera(); //TODO: get light camera
        PassConstantBufferData passConstantBufferData;
        passConstantBufferData.viewMatrix = camera.getViewMatrix();
        passConstantBufferData.projectionMatrix = camera.getProjectionMatrix();
        passConstantBufferData.viewProjectionMatrix = passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
        memcpy(rendererData.passConstantBufferPtr, &passConstantBufferData, sizeof(glm::mat4) * 3);

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.depthPassPipelineState);

        //Depth pass is pass #0
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.passDescriptorSet,
            rendererData.depthPassPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, 0, 0);

        for (auto &framebuffer : scene.getShadowMapFramebuffers()) {
            Graphics::beginRenderPass(rendererData.commandBufferId, framebuffer);

            uint32 entityIndex = 0;
            for (const auto &entity : scene.getEntities()) {
                drawEntity(entity, entityIndex++);
            }

            Graphics::endRenderPass(rendererData.commandBufferId);
        }
    }

    void Renderer::colorPass(const Scene &scene) {
        const Camera &camera = scene.getCamera();
        PassConstantBufferData passConstantBufferData;
        passConstantBufferData.viewMatrix = camera.getViewMatrix();
        passConstantBufferData.projectionMatrix = camera.getProjectionMatrix();
        passConstantBufferData.viewProjectionMatrix = passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
        const glm::vec3 &cameraPosition = camera.getTransform().getTranslation();
        passConstantBufferData.cameraPosition.x = cameraPosition.x;
        passConstantBufferData.cameraPosition.y = cameraPosition.y;
        passConstantBufferData.cameraPosition.z = cameraPosition.z;

        uint32 passOffset = sizeof(PassConstantBufferData); //Color pass is pass #1
        memcpy(rendererData.passConstantBufferPtr + passOffset, &passConstantBufferData, sizeof(PassConstantBufferData));

        SceneConstantBufferData sceneConstantBufferData;
        int i = 0;
        for (const auto &dirLight : scene.getDirectionalLights()) {
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].x = dirLight.direction.x;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].y = dirLight.direction.y;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].z = dirLight.direction.z;
            sceneConstantBufferData.dirLightDirectionsAndIntensities[i].w = dirLight.intensity;
            sceneConstantBufferData.dirLightColors[i].r = dirLight.color.r;
            sceneConstantBufferData.dirLightColors[i].g = dirLight.color.g;
            sceneConstantBufferData.dirLightColors[i].b = dirLight.color.b;
            i++;
        }
        sceneConstantBufferData.dirLightCountAndRadianceMapMips.x = static_cast<float>(i);
        sceneConstantBufferData.dirLightCountAndRadianceMapMips.y = scene.hasSkyBox() ? scene.getSkyBox().radianceMapView->getTexture()->getMipLevels() : 0.0f;
        memcpy(rendererData.sceneConstantBufferPtr, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        Graphics::beginRenderPass(rendererData.commandBufferId);

        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.passDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_PASS_DESCRIPTOR_SET_IDX, &passOffset, 1);

        if (scene.hasSkyBox()) {
            Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.skyBoxPipelineState);
            drawMesh(scene.getSkyBox().mesh, Transform());
        }

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.defaultPipelineState);
        uint32 entityIndex = 0;
        for (const auto &entity : scene.getEntities()) {
            drawEntity(entity, entityIndex++);
        }

        Graphics::endRenderPass(rendererData.commandBufferId);
    }

    void Renderer::drawEntity(const Entity &entity, uint32 index) {
        uint32 entityOffset = index * sizeof(EntityConstantBufferData);
        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.entityDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_ENTITY_DESCRIPTOR_SET_IDX, &entityOffset, 1);
        drawMesh(entity.mesh, entity.transform);
    }

    void Renderer::drawMesh(const Mesh &mesh, const Transform &transform) {
        BZ_PROFILE_FUNCTION();

        uint32 materialOffset = rendererData.materialOffsetMap[mesh.getMaterial()];
        Graphics::bindDescriptorSet(rendererData.commandBufferId, mesh.getMaterial().getDescriptorSet(),
            rendererData.defaultPipelineState, RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer(), 0);

        if (mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer(), 0);

        if (mesh.hasIndices())
            Graphics::drawIndexed(rendererData.commandBufferId, mesh.getIndexCount(), 1, 0, 0, 0);
        else
            Graphics::draw(rendererData.commandBufferId, mesh.getVertexCount(), 1, 0, 0);

        stats.drawCallCount++;
        stats.vertexCount += mesh.getVertexCount();
        stats.triangleCount += (mesh.hasIndices() ? mesh.getIndexCount() : mesh.getVertexCount()) / 3;
    }

    void Renderer::handleEntities(const Scene &scene) {
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
    void Renderer::handleMaterials(const Scene &scene) {
        rendererData.materialOffsetMap.clear();

        if (scene.hasSkyBox()) {
            handleMaterial(scene.getSkyBox().mesh.getMaterial());
        }

        for (const auto &entity : scene.getEntities()) {
            handleMaterial(entity.mesh.getMaterial());
        }

        uint32 materialCount = static_cast<uint32>(rendererData.materialOffsetMap.size());
        stats.materialCount = materialCount;
        BZ_ASSERT_CORE(materialCount <= MAX_MATERIALS_PER_SCENE, "Reached the max number of Materials!");
    }

    void Renderer::handleMaterial(const Material &material) {
        BZ_ASSERT_CORE(material.isValid(), "Trying to use an invalid/initialized Material!");

        const auto storedMaterialIt = rendererData.materialOffsetMap.find(material);
        uint32 materialOffset;

        //If it's the first time this Material is used on a Scene set the correspondent data.
        if (storedMaterialIt == rendererData.materialOffsetMap.end()) {
            MaterialConstantBufferData materialConstantBufferData;
            materialConstantBufferData.parallaxOcclusionScale = material.getParallaxOcclusionScale();

            materialOffset = static_cast<uint32>(rendererData.materialOffsetMap.size()) * sizeof(EntityConstantBufferData);
            memcpy(rendererData.materialConstantBufferPtr + materialOffset, &materialConstantBufferData, sizeof(MaterialConstantBufferData));

            rendererData.materialOffsetMap[material] = materialOffset;
        }
        else {
            materialOffset = storedMaterialIt->second;
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
        constexpr uint32 SIZE = 1024;

        //TODO: depth only format
        auto shadowMapRef = Texture2D::createRenderTarget(SIZE, SIZE, rendererData.depthRenderPass->getDepthStencilAttachmentDescription()->format.format);
        return Framebuffer::create(rendererData.depthRenderPass, { TextureView::create(shadowMapRef) }, glm::ivec3(SIZE, SIZE, 1));
    }

    const Ref<Sampler>& Renderer::getDefaultSampler() {
        return rendererData.defaultSampler;
    }
}
