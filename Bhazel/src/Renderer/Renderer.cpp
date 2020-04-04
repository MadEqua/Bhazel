#include "bzpch.h"

#include "Renderer.h"

#include "Graphics/Graphics.h"
#include "Core/Application.h"
#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"


namespace BZ {

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

        Ref<DescriptorSetLayout> materialDescriptorSetLayout;
        Ref<Sampler> defaultSampler;

        Ref<PipelineState> defaultPipelineState;
        Ref<PipelineState> skyBoxPipelineState;

        Ref<Mesh> cubeMesh;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        rendererData.defaultSampler = samplerBuilder.build();

        //DefaultPipelineState
        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        //Albedo, Normal, Metallic, Roughness and Height textures
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.materialDescriptorSetLayout = descriptorSetLayoutBuilder.build();
        
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("DefaultRenderer");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/DefaultVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/DefaultFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        pipelineStateData.descriptorSetLayouts = { rendererData.materialDescriptorSetLayout };

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

        rendererData.cubeMesh = MakeRef<Mesh>(Mesh::createUnitCube());
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.cubeMesh.reset();
        rendererData.defaultSampler.reset();
        rendererData.materialDescriptorSetLayout.reset();

        rendererData.defaultPipelineState.reset();
        rendererData.skyBoxPipelineState.reset();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&stats, 0, sizeof(stats));

        rendererData.commandBufferId = Graphics::beginCommandBuffer();

        const Camera &camera = scene.getCamera();
        Graphics::beginScene(rendererData.commandBufferId, rendererData.defaultPipelineState, scene);

        if (scene.hasSkyBox()) {
            drawMesh(rendererData.skyBoxPipelineState, scene.getSkyBox().mesh, Transform());
        }

        for (const auto &entity : scene.getEntities()) {
            drawMesh(rendererData.defaultPipelineState, entity.mesh, entity.transform);
        }

        Graphics::endCommandBuffer(rendererData.commandBufferId);
    }

    //void Renderer::beginScene(const Camera &camera) {
    //    BZ_PROFILE_FUNCTION();
    //
    //    BZ_ASSERT_CORE(rendererData.commandBufferId == -1, "There's already an unended Scene!");
    //
    //    memset(&stats, 0, sizeof(stats));
    //
    //    rendererData.commandBufferId = Graphics::beginCommandBuffer();
    //    Graphics::beginScene(rendererData.commandBufferId, rendererData.pipelineState, camera.getTransform().getTranslation(), camera.getViewMatrix(), camera.getProjectionMatrix());
    //}
    //
    //void Renderer::endScene() {
    //    BZ_PROFILE_FUNCTION();
    //
    //    BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");
    //
    //    Graphics::endCommandBuffer(rendererData.commandBufferId);
    //    rendererData.commandBufferId = -1;
    //}

    void Renderer::drawCube(const Transform &transform, const Material &material) {
        BZ_PROFILE_FUNCTION();
        drawMesh(rendererData.defaultPipelineState, *rendererData.cubeMesh, transform, material);
    }

    void Renderer::drawMesh(const Ref<PipelineState> &pipelineState, const Mesh &mesh, const Transform &transform) {
        BZ_PROFILE_FUNCTION();
        Renderer::drawMesh(pipelineState, mesh, transform, Material());
    }

    void Renderer::drawMesh(const Ref<PipelineState> &pipelineState, const Mesh &mesh, const Transform &transform, const Material &fallbackMaterial) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(mesh.isValid(), "Trying to draw a invalid/uninitialized Mesh!");
        BZ_ASSERT_CORE(mesh.getMaterial().isValid() || fallbackMaterial.isValid(), "Trying to draw a Mesh with no valid/initialized Material!");

        Graphics::beginObject(rendererData.commandBufferId, pipelineState, transform.getLocalToParentMatrix(), transform.getNormalMatrix());

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer(), 0);

        if (mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer(), 0);

        Graphics::bindPipelineState(rendererData.commandBufferId, pipelineState);

        const Material &materialToUse = mesh.getMaterial().isValid() ? mesh.getMaterial() : fallbackMaterial;
        Graphics::bindDescriptorSet(rendererData.commandBufferId, materialToUse.getDescriptorSet(), pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);

        if (mesh.hasIndices())
            Graphics::drawIndexed(rendererData.commandBufferId, mesh.getIndexCount(), 1, 0, 0, 0);
        else
            Graphics::draw(rendererData.commandBufferId, mesh.getVertexCount(), 1, 0, 0);

        stats.drawCallCount++;
        stats.vertexCount += mesh.getVertexCount();
        stats.triangleCount += (mesh.hasIndices() ? mesh.getIndexCount() : mesh.getVertexCount()) / 3;
    }

    /*void Renderer::drawSkyBox(const SkyBox &skyBox) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(skyBox.mesh.isValid(), "Trying to draw a invalid/uninitialized SkyBox Mesh!");

        Graphics::beginObject(rendererData.commandBufferId, rendererData.defaultPipelineState, glm::mat4(1.0f), glm::mat3(1.0f)));

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
   
    Ref<DescriptorSetLayout>& Renderer::getMaterialDescriptorSetLayout() {
        return rendererData.materialDescriptorSetLayout;
    }

    Ref<Sampler>& Renderer::getDefaultSampler() {
        return rendererData.defaultSampler;
    }

    DataLayout& Renderer::getVertexDataLayout() {
        return vertexDataLayout;
    }

    DataLayout& Renderer::getIndexDataLayout() {
        return indexDataLayout;
    }
}
