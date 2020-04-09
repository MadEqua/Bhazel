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
        glm::vec4 cameraPosition; //vec4 to simplify alignments
        glm::vec4 dirLightsDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
        glm::vec4 dirLightsColors[MAX_DIR_LIGHTS_PER_SCENE]; //vec4 to simplify alignments
        uint32 dirLightsCount;
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

        Ref<DescriptorSetLayout> sceneDescriptorSetLayout;
        Ref<DescriptorSetLayout> entityDescriptorSetLayout;
        Ref<DescriptorSetLayout> materialDescriptorSetLayout;

        Ref<DescriptorSet> entityDescriptorSet;

        Ref<Sampler> defaultSampler;

        Ref<PipelineState> defaultPipelineState;
        Ref<PipelineState> skyBoxPipelineState;

        std::unordered_set<Material> materialSet;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        rendererData.constantBuffer = Buffer::create(BufferType::Constant, SCENE_CONSTANT_BUFFER_SIZE + ENTITY_CONSTANT_BUFFER_SIZE + MATERIAL_CONSTANT_BUFFER_SIZE, MemoryType::CpuToGpu);
        rendererData.sceneConstantBufferPtr = rendererData.constantBuffer->map(0);
        rendererData.entityConstantBufferPtr = rendererData.sceneConstantBufferPtr + ENTITY_CONSTANT_BUFFER_OFFSET;
        rendererData.materialConstantBufferPtr = rendererData.sceneConstantBufferPtr + MATERIAL_CONSTANT_BUFFER_OFFSET;

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::All), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.sceneDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder2;
        descriptorSetLayoutBuilder2.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Vertex), 1);
        rendererData.entityDescriptorSetLayout = descriptorSetLayoutBuilder2.build();
        rendererData.entityDescriptorSet = DescriptorSet::create(rendererData.entityDescriptorSetLayout);
        rendererData.entityDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, ENTITY_CONSTANT_BUFFER_OFFSET, sizeof(EntityConstantBufferData));

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder3;
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::Fragment), 1);

        //Albedo, Normal, Metallic, Roughness and Height textures
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder3.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.materialDescriptorSetLayout = descriptorSetLayoutBuilder3.build();

        //DefaultPipelineState   
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("DefaultRenderer");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/DefaultVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/DefaultFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.descriptorSetLayouts = { rendererData.sceneDescriptorSetLayout, rendererData.entityDescriptorSetLayout, rendererData.materialDescriptorSetLayout };

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
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.constantBuffer.reset();

        rendererData.sceneDescriptorSetLayout.reset();
        rendererData.entityDescriptorSetLayout.reset();
        rendererData.materialDescriptorSetLayout.reset();

        rendererData.entityDescriptorSet.reset();

        rendererData.defaultPipelineState.reset();
        rendererData.skyBoxPipelineState.reset();

        rendererData.defaultSampler.reset();
        rendererData.materialSet.clear();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&stats, 0, sizeof(stats));
        rendererData.materialSet.clear();

        rendererData.commandBufferId = Graphics::beginCommandBuffer();

        const Camera &camera = scene.getCamera();
        SceneConstantBufferData sceneConstantBufferData;
        sceneConstantBufferData.viewMatrix = camera.getViewMatrix();
        sceneConstantBufferData.projectionMatrix = camera.getProjectionMatrix();
        sceneConstantBufferData.viewProjectionMatrix = sceneConstantBufferData.projectionMatrix * sceneConstantBufferData.viewMatrix;
        const glm::vec3 &cameraPosition = camera.getTransform().getTranslation();
        sceneConstantBufferData.cameraPosition.x = cameraPosition.x;
        sceneConstantBufferData.cameraPosition.y = cameraPosition.y;
        sceneConstantBufferData.cameraPosition.z = cameraPosition.z;

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
        sceneConstantBufferData.dirLightsCount = i;

        memcpy(rendererData.sceneConstantBufferPtr, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        Graphics::bindDescriptorSet(rendererData.commandBufferId, scene.getDescriptorSet(), 
            rendererData.defaultPipelineState, RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

        if (scene.hasSkyBox()) {
            handleMaterial(scene.getSkyBox().mesh.getMaterial(), 0);
            drawMesh(rendererData.skyBoxPipelineState, scene.getSkyBox().mesh, Transform());
        }

        uint32 entityIndex = scene.hasSkyBox() ? 1 : 0;
        for (const auto &entity : scene.getEntities()) {
            handleMaterial(entity.mesh.getMaterial(), entityIndex);
            drawEntity(entity, entityIndex++);
        }

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
    void Renderer::handleMaterial(const Material &material, uint32 index) {
        BZ_ASSERT_CORE(material.isValid(), "Trying to use an invalid/initialized Material!");

        const auto storedMaterial = rendererData.materialSet.find(material);

        //If it's the first time this Material is used on a Scene set the data and bind the DescriptorSet.
        if (storedMaterial == rendererData.materialSet.end()) {
            stats.materialCount++;
            rendererData.materialSet.insert(material);

            MaterialConstantBufferData materialConstantBufferData;
            materialConstantBufferData.parallaxOcclusionScale = material.getParallaxOcclusionScale();

            uint32 materialOffset = index * sizeof(EntityConstantBufferData);
            memcpy(rendererData.materialConstantBufferPtr + materialOffset, &materialConstantBufferData, sizeof(MaterialConstantBufferData));

            Graphics::bindDescriptorSet(rendererData.commandBufferId, material.getDescriptorSet(),
                rendererData.defaultPipelineState, RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);
        }
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

    DataLayout& Renderer::getVertexDataLayout() {
        return vertexDataLayout;
    }

    DataLayout& Renderer::getIndexDataLayout() {
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
