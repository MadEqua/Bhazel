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

        Ref<PipelineState> pipelineState;

        Ref<Mesh> cubeMesh;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        rendererData.defaultSampler = samplerBuilder.build();
        
        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        rendererData.materialDescriptorSetLayout = descriptorSetLayoutBuilder.build();
        
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer");
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
        rendererData.pipelineState = PipelineState::create(pipelineStateData);

        rendererData.cubeMesh = MakeRef<Mesh>(Mesh::createUnitCube());
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.cubeMesh.reset();
        rendererData.defaultSampler.reset();
        rendererData.materialDescriptorSetLayout.reset();

        rendererData.pipelineState.reset();
    }

    void Renderer::drawScene(const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        memset(&stats, 0, sizeof(stats));

        rendererData.commandBufferId = Graphics::beginCommandBuffer();

        const Camera &camera = scene.getCamera();
        Graphics::beginScene(rendererData.commandBufferId, rendererData.pipelineState, scene);

        for (const auto &entity : scene.getEntities()) {
            drawMesh(entity.mesh, entity.transform);
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
        drawMesh(*rendererData.cubeMesh, transform, material);
    }

    void Renderer::drawMesh(const Mesh &mesh, const Transform &transform) {
        BZ_PROFILE_FUNCTION();
        Renderer::drawMesh(mesh, transform, Material());
    }

    void Renderer::drawMesh(const Mesh &mesh, const Transform &transform, const Material &fallbackMaterial) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(mesh.isValid(), "Trying to draw a invalid/uninitialized Mesh!");
        BZ_ASSERT_CORE(mesh.getMaterial().isValid() || fallbackMaterial.isValid(), "Trying to draw a Mesh with no valid/initialized Material!");

        Graphics::beginObject(rendererData.commandBufferId, rendererData.pipelineState, transform.getLocalToParentMatrix(), transform.getNormalMatrix());

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer(), 0);

        if (mesh.hasIndices())
            Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer(), 0);

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.pipelineState);

        const Material &materialToUse = mesh.getMaterial().isValid() ? mesh.getMaterial() : fallbackMaterial;
        Graphics::bindDescriptorSet(rendererData.commandBufferId, materialToUse.getDescriptorSet(), rendererData.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);

        if (mesh.hasIndices())
            Graphics::drawIndexed(rendererData.commandBufferId, mesh.getIndexCount(), 1, 0, 0, 0);
        else
            Graphics::draw(rendererData.commandBufferId, mesh.getVertexCount(), 1, 0, 0);

        stats.drawCallCount++;
        stats.vertexCount += mesh.getVertexCount();
        stats.triangleCount += mesh.getIndexCount() / 3;
    }

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
