#include "bzpch.h"

#include "Renderer.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"

#include "Core/Application.h"

#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"


namespace BZ {

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) SceneConstantBufferData {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 viewProjectionMatrix;
        glm::vec4 cameraPositionAndDirLightCount;
        glm::vec4 dirLightsDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
        glm::vec4 dirLightsColors[MAX_DIR_LIGHTS_PER_SCENE]; //vec4 to simplify alignments
        float radianceMapMips;
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) EntityConstantBufferData {
        glm::mat4 modelMatrix;
        glm::mat4 normalMatrix; //mat4 to simplify alignments
    };

    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) MaterialConstantBufferData {
        float parallaxOcclusionScale;
    };

    constexpr uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData);
    constexpr uint32 ENTITY_CONSTANT_BUFFER_SIZE = sizeof(EntityConstantBufferData) * MAX_ENTITIES_PER_FRAME;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_SIZE = sizeof(MaterialConstantBufferData) * MAX_MATERIALS_PER_FRAME;

    constexpr uint32 SCENE_CONSTANT_BUFFER_OFFSET = 0;
    constexpr uint32 ENTITY_CONSTANT_BUFFER_OFFSET = SCENE_CONSTANT_BUFFER_SIZE;
    constexpr uint32 MATERIAL_CONSTANT_BUFFER_OFFSET = SCENE_CONSTANT_BUFFER_SIZE + ENTITY_CONSTANT_BUFFER_SIZE;

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
        BufferPtr sceneConstantBufferPtr;
        BufferPtr entityConstantBufferPtr;
        BufferPtr materialConstantBufferPtr;

        Ref<DescriptorSetLayout> globalDescriptorSetLayout;
        Ref<DescriptorSetLayout> sceneDescriptorSetLayout;
        Ref<DescriptorSetLayout> entityDescriptorSetLayout;
        Ref<DescriptorSetLayout> materialDescriptorSetLayout;

        Ref<DescriptorSet> globalDescriptorSet;
        Ref<DescriptorSet> entityDescriptorSet;

        Ref<Sampler> defaultSampler;

        Ref<PipelineState> defaultPipelineState;
        Ref<PipelineState> skyBoxPipelineState;

        Ref<TextureView> brdfLookupTexture;

        std::unordered_map<Material, uint32> materialOffsetMap;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        rendererData.constantBuffer = Buffer::create(BufferType::Constant, SCENE_CONSTANT_BUFFER_SIZE + ENTITY_CONSTANT_BUFFER_SIZE + MATERIAL_CONSTANT_BUFFER_SIZE, MemoryType::CpuToGpu);
        rendererData.sceneConstantBufferPtr = rendererData.constantBuffer->map(0);
        rendererData.entityConstantBufferPtr = rendererData.sceneConstantBufferPtr + ENTITY_CONSTANT_BUFFER_OFFSET;
        rendererData.materialConstantBufferPtr = rendererData.sceneConstantBufferPtr + MATERIAL_CONSTANT_BUFFER_OFFSET;

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.globalDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder2;
        descriptorSetLayoutBuilder2.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::All), 1);
        descriptorSetLayoutBuilder2.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder2.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.sceneDescriptorSetLayout = descriptorSetLayoutBuilder2.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder3;
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Vertex), 1);
        rendererData.entityDescriptorSetLayout = descriptorSetLayoutBuilder3.build();
        rendererData.entityDescriptorSet = DescriptorSet::create(rendererData.entityDescriptorSetLayout);
        rendererData.entityDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, ENTITY_CONSTANT_BUFFER_OFFSET, sizeof(EntityConstantBufferData));

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder4;
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Fragment), 1);

        //Albedo, Normal, Metallic, Roughness and Height textures
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder4.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.materialDescriptorSetLayout = descriptorSetLayoutBuilder4.build();

        //DefaultPipelineState
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("DefaultRenderer");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/DefaultVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/DefaultFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.descriptorSetLayouts = { rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout, 
                                                   rendererData.entityDescriptorSetLayout, rendererData.materialDescriptorSetLayout };

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

        Sampler::Builder builder;
        rendererData.defaultSampler = builder.build();

        //The ideal 2 Channels (RG) are not supported by stbi. 3 channels is badly supported by Vulkan implementations. So 4 channels...
        auto brdfTex = Texture2D::create("Bhazel/textures/ibl_brdf_lut.png", TextureFormat::R8G8B8A8, MipmapData::Options::DoNothing);
        rendererData.brdfLookupTexture = TextureView::create(brdfTex);

        rendererData.globalDescriptorSet = DescriptorSet::create(rendererData.globalDescriptorSetLayout);
        rendererData.globalDescriptorSet->setCombinedTextureSampler(rendererData.brdfLookupTexture, rendererData.defaultSampler, 0);
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.constantBuffer.reset();

        rendererData.globalDescriptorSetLayout.reset();
        rendererData.sceneDescriptorSetLayout.reset();
        rendererData.entityDescriptorSetLayout.reset();
        rendererData.materialDescriptorSetLayout.reset();

        rendererData.globalDescriptorSet.reset();
        rendererData.entityDescriptorSet.reset();

        rendererData.defaultPipelineState.reset();
        rendererData.skyBoxPipelineState.reset();

        rendererData.defaultSampler.reset();
        rendererData.materialOffsetMap.clear();

        rendererData.brdfLookupTexture.reset();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&stats, 0, sizeof(stats));
        rendererData.materialOffsetMap.clear();

        rendererData.commandBufferId = Graphics::beginCommandBuffer();

        const Camera &camera = scene.getCamera();
        SceneConstantBufferData sceneConstantBufferData;
        sceneConstantBufferData.viewMatrix = camera.getViewMatrix();
        sceneConstantBufferData.projectionMatrix = camera.getProjectionMatrix();
        sceneConstantBufferData.viewProjectionMatrix = sceneConstantBufferData.projectionMatrix * sceneConstantBufferData.viewMatrix;
        const glm::vec3 &cameraPosition = camera.getTransform().getTranslation();
        sceneConstantBufferData.cameraPositionAndDirLightCount.x = cameraPosition.x;
        sceneConstantBufferData.cameraPositionAndDirLightCount.y = cameraPosition.y;
        sceneConstantBufferData.cameraPositionAndDirLightCount.z = cameraPosition.z;

        int i = 0;
        for (const auto &dirLight : scene.getDirectionalLights()) {
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].x = dirLight.direction.x;
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].y = dirLight.direction.y;
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].z = dirLight.direction.z;
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].w = dirLight.intensity;
            sceneConstantBufferData.dirLightsColors[i].r = dirLight.color.r;
            sceneConstantBufferData.dirLightsColors[i].g = dirLight.color.g;
            sceneConstantBufferData.dirLightsColors[i].b = dirLight.color.b;
            i++;
        }
        sceneConstantBufferData.cameraPositionAndDirLightCount.w = static_cast<float>(i);
        sceneConstantBufferData.radianceMapMips = scene.hasSkyBox() ? scene.getSkyBox().radianceMapView->getTexture()->getMipLevels() : 0.0f;
        memcpy(rendererData.sceneConstantBufferPtr, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.globalDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_GLOBAL_DESCRIPTOR_SET_IDX, 0, 0);

        Graphics::bindDescriptorSet(rendererData.commandBufferId, scene.getDescriptorSet(), 
            rendererData.defaultPipelineState, RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

        if (scene.hasSkyBox()) {
            handleMaterial(scene.getSkyBox().mesh.getMaterial());
            drawMesh(rendererData.skyBoxPipelineState, scene.getSkyBox().mesh, Transform());
        }

        uint32 entityIndex = 0;
        for (const auto &entity : scene.getEntities()) {
            handleMaterial(entity.mesh.getMaterial());
            drawEntity(entity, entityIndex++);
        }
        stats.materialCount = static_cast<uint32>(rendererData.materialOffsetMap.size());

        Graphics::endCommandBuffer(rendererData.commandBufferId);
    }

    void Renderer::drawEntity(const Entity &entity, uint32 index) {
        EntityConstantBufferData entityConstantBufferData;
        entityConstantBufferData.modelMatrix = entity.transform.getLocalToParentMatrix();
        entityConstantBufferData.normalMatrix = entity.transform.getNormalMatrix();

        uint32 entityOffset = index * sizeof(EntityConstantBufferData);
        memcpy(rendererData.entityConstantBufferPtr + entityOffset, &entityConstantBufferData, sizeof(EntityConstantBufferData));

        Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.entityDescriptorSet,
            rendererData.defaultPipelineState, RENDERER_ENTITY_DESCRIPTOR_SET_IDX, &entityOffset, 1);

        drawMesh(rendererData.defaultPipelineState, entity.mesh, entity.transform);
    }

    void Renderer::drawMesh(const Ref<PipelineState> &pipelineState, const Mesh &mesh, const Transform &transform) {
        BZ_PROFILE_FUNCTION();

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer(), 0);

        if (mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer(), 0);

        Graphics::bindPipelineState(rendererData.commandBufferId, pipelineState);

        if (mesh.hasIndices())
            Graphics::drawIndexed(rendererData.commandBufferId, mesh.getIndexCount(), 1, 0, 0, 0);
        else
            Graphics::draw(rendererData.commandBufferId, mesh.getVertexCount(), 1, 0, 0);

        stats.drawCallCount++;
        stats.vertexCount += mesh.getVertexCount();
        stats.triangleCount += (mesh.hasIndices() ? mesh.getIndexCount() : mesh.getVertexCount()) / 3;
    }

    //TODO: There's no need to call this every frame, like it's being done now.
    void Renderer::handleMaterial(const Material &material) {
        BZ_ASSERT_CORE(material.isValid(), "Trying to use an invalid/initialized Material!");

        const auto storedMaterialIt = rendererData.materialOffsetMap.find(material);
        uint32 materialOffset;

        //If it's the first time this Material is used on a Scene set the correspondent data.
        if (storedMaterialIt == rendererData.materialOffsetMap.end()) {
            MaterialConstantBufferData materialConstantBufferData;
            materialConstantBufferData.parallaxOcclusionScale = material.getParallaxOcclusionScale();

            materialOffset = rendererData.materialOffsetMap.size() * sizeof(EntityConstantBufferData);
            memcpy(rendererData.materialConstantBufferPtr + materialOffset, &materialConstantBufferData, sizeof(MaterialConstantBufferData));

            rendererData.materialOffsetMap[material] = materialOffset;
        }
        else {
            materialOffset = storedMaterialIt->second;
        }

        Graphics::bindDescriptorSet(rendererData.commandBufferId, material.getDescriptorSet(),
            rendererData.defaultPipelineState, RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);
    }

    /*void Renderer::drawSkyBox(const SkyBox &skyBox) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(skyBox.mesh.isValid(), "Trying to draw a invalid/uninitialized SkyBox Mesh!");

        Graphics::beginEntity(rendererData.commandBufferId, rendererData.defaultPipelineState, glm::mat4(1.0f), glm::mat3(1.0f)));

        Graphics::bindBuffer(rendererData.commandBufferId, skyBox.mesh.getVertexBuffer(), 0);

        if (skyBox.mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, skyBox.mesh.getIndexBuffer(), 0);

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.skyBoxPipelineState);

        const Material &materialToUse = mesh.getMaterial().isValid()?mesh.getMaterial():fallbackMaterial;
        Graphics::bindDescriptorSet(rendererData.commandBufferId, materialToUse.getDescriptorSet(), rendererData.defaultPipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);

        if (mesh.hasIndices())
            Graphics::drawIndexed(rendererData.commandBufferId, mesh.getIndexCount(), 1, 0, 0, 0);
        else
            Graphics::draw(rendererData.commandBufferId, mesh.getVertexCount(), 1, 0, 0);

        stats.drawCallCount++;
        stats.vertexCount += mesh.getVertexCount();
        stats.triangleCount += (mesh.hasIndices()?mesh.getIndexCount():mesh.getVertexCount()) / 3;
    }*/

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

    Ref<Sampler> Renderer::getDefaultSampler() {
        return rendererData.defaultSampler;
    }
}
