#include "bzpch.h"

#include "Renderer.h"

#include "Graphics/Graphics.h"
#include "Core/Application.h"
#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"


namespace BZ {

    RendererStats Renderer::stats;

    //TODO
    //static DataLayout vertexLayout = {
    //    { DataType::Int16, DataElements::Vec3, "POSITION", true },
    //    { DataType::Int16, DataElements::Vec3, "NORMAL", true },
    //    { DataType::Int16, DataElements::Vec3, "TANGENT", true },
    //    { DataType::Uint16, DataElements::Vec2, "TEXCOORD", true },
    //};

    static DataLayout vertexLayout = {
        { DataType::Float32, DataElements::Vec3, "POSITION" },
        { DataType::Float32, DataElements::Vec3, "NORMAL" },
        { DataType::Float32, DataElements::Vec3, "TANGENT" },
        { DataType::Uint16, DataElements::Vec2, "TEXCOORD", true },
    };

    static DataLayout indexLayout = {
        {DataType::Uint32, DataElements::Scalar, ""},
    };

    struct Vertex {
        int16 pos[3];
        int16 normal[3];
        int16 tangent[3];
        uint16 texCoord[2];
    };

    constexpr int CUBE_VERTEX_COUNT = 36;
    
    constexpr int16 NEG_ONE_I = 0x8001;
    constexpr int16 ONE_I = 0x7fff - 1;

    constexpr uint16 ZERO = 0;
    constexpr uint16 ONE_UI = 0xffff - 1;

    Vertex cubeVertices[CUBE_VERTEX_COUNT] = {
        //Front
        { { NEG_ONE_I, NEG_ONE_I, ONE_I }, { ZERO, ZERO, ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { ONE_I, NEG_ONE_I, ONE_I }, { ZERO, ZERO, ONE_I }, { ONE_I, ZERO, ZERO }, { ONE_UI, ZERO } },
        { { ONE_I, ONE_I, ONE_I }, { ZERO, ZERO, ONE_I }, { ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, NEG_ONE_I, ONE_I }, { ZERO, ZERO, ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { ONE_I, ONE_I, ONE_I }, { ZERO, ZERO, ONE_I }, { ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, ONE_I, ONE_I }, { ZERO, ZERO, ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ONE_UI } },

        //Right
        { { ONE_I, NEG_ONE_I, ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_I }, { ZERO, ZERO } },
        { { ONE_I, NEG_ONE_I, NEG_ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_I }, { ONE_UI, ZERO } },
        { { ONE_I, ONE_I, NEG_ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_I }, { ONE_UI, ONE_UI } },
        { { ONE_I, NEG_ONE_I, ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_I }, { ZERO, ZERO } },
        { { ONE_I, ONE_I, NEG_ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_I }, { ONE_UI, ONE_UI } },
        { { ONE_I, ONE_I, ONE_I }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_I }, { ZERO, ONE_UI } },

        //Left
        { { NEG_ONE_I, NEG_ONE_I, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO, ONE_I }, { ZERO, ZERO } },
        { { NEG_ONE_I, NEG_ONE_I, ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO, ONE_I }, { ONE_UI, ZERO } },
        { { NEG_ONE_I, ONE_I, ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO, ONE_I }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, NEG_ONE_I, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO, ONE_I }, { ZERO, ZERO } },
        { { NEG_ONE_I, ONE_I, ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO, ONE_I }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, ONE_I, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO, ONE_I }, { ZERO, ONE_UI } },

        //Back
        { { ONE_I, NEG_ONE_I, NEG_ONE_I }, { ZERO, ZERO, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { NEG_ONE_I, NEG_ONE_I, NEG_ONE_I }, { ZERO, ZERO, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ONE_UI, ZERO } },
        { { NEG_ONE_I, ONE_I, NEG_ONE_I }, { ZERO, ZERO, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { ONE_I, NEG_ONE_I, NEG_ONE_I }, { ZERO, ZERO, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { NEG_ONE_I, ONE_I, NEG_ONE_I }, { ZERO, ZERO, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { ONE_I, ONE_I, NEG_ONE_I }, { ZERO, ZERO, NEG_ONE_I }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ONE_UI } },

        //Bottom
        { { NEG_ONE_I, NEG_ONE_I, NEG_ONE_I }, { ZERO, NEG_ONE_I, ZERO }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { ONE_I, NEG_ONE_I, NEG_ONE_I }, { ZERO, NEG_ONE_I, ZERO }, { NEG_ONE_I, ZERO, ZERO }, { ONE_UI, ZERO } },
        { { ONE_I, NEG_ONE_I, ONE_I }, { ZERO, NEG_ONE_I, ZERO }, { NEG_ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, NEG_ONE_I, NEG_ONE_I }, { ZERO, NEG_ONE_I, ZERO }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { ONE_I, NEG_ONE_I, ONE_I }, { ZERO, NEG_ONE_I, ZERO }, { NEG_ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, NEG_ONE_I, ONE_I }, { ZERO, NEG_ONE_I, ZERO }, { NEG_ONE_I, ZERO, ZERO }, { ZERO, ONE_UI } },

        //Top
        { { NEG_ONE_I, ONE_I, ONE_I }, { ZERO, ONE_I, ZERO }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { ONE_I, ONE_I, ONE_I }, { ZERO, ONE_I, ZERO }, { ONE_I, ZERO, ZERO }, { ONE_UI, ZERO } },
        { { ONE_I, ONE_I, NEG_ONE_I }, { ZERO, ONE_I, ZERO }, { ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, ONE_I, ONE_I }, { ZERO, ONE_I, ZERO }, { ONE_I, ZERO, ZERO }, { ZERO, ZERO } },
        { { ONE_I, ONE_I, NEG_ONE_I }, { ZERO, ONE_I, ZERO }, { ONE_I, ZERO, ZERO }, { ONE_UI, ONE_UI } },
        { { NEG_ONE_I, ONE_I, NEG_ONE_I }, { ZERO, ONE_I, ZERO }, { ONE_I, ZERO, ZERO }, { ZERO, ONE_UI } },
    };

    static struct RendererData {
        uint32 commandBufferId;

        Ref<Buffer> cubeVertexBuffer;

        Ref<DescriptorSetLayout> materialDescriptorSetLayout;
        Ref<Sampler> defaultSampler;

        Ref<PipelineState> pipelineState;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        rendererData.cubeVertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * CUBE_VERTEX_COUNT, MemoryType::GpuOnly, vertexLayout);
        rendererData.cubeVertexBuffer->setData(cubeVertices, sizeof(cubeVertices), 0);

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

        pipelineStateData.dataLayout = vertexLayout;
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };
        rendererData.pipelineState = PipelineState::create(pipelineStateData);
    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.cubeVertexBuffer.reset();
        rendererData.defaultSampler.reset();
        rendererData.materialDescriptorSetLayout.reset();

        rendererData.pipelineState.reset();
    }

    void Renderer::beginScene(const Camera &camera) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId == -1, "There's already an unended Scene!");

        memset(&stats, 0, sizeof(stats));

        rendererData.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(rendererData.commandBufferId, rendererData.pipelineState, camera.getTransform().getTranslation(), camera.getViewMatrix(), camera.getProjectionMatrix());
    }

    void Renderer::endScene() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");

        Graphics::endCommandBuffer(rendererData.commandBufferId);
        rendererData.commandBufferId = -1;
    }

    void Renderer::drawCube(const Transform &transform) {
        BZ_PROFILE_FUNCTION();

        Graphics::beginObject(rendererData.commandBufferId, rendererData.pipelineState, transform.getLocalToParentMatrix(), transform.getNormalMatrix());

        Graphics::bindBuffer(rendererData.commandBufferId, rendererData.cubeVertexBuffer, 0);
        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.pipelineState);
        //Graphics::bindDescriptorSet(rendererData.commandBufferId, rendererData.descriptorSet, rendererData.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);

        Graphics::draw(rendererData.commandBufferId, CUBE_VERTEX_COUNT, 1, 0, 0);
    }

    void Renderer::drawMesh(const Mesh &mesh, const Transform &transform) {
        BZ_PROFILE_FUNCTION();

        Graphics::beginObject(rendererData.commandBufferId, rendererData.pipelineState, transform.getLocalToParentMatrix(), transform.getNormalMatrix());

        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getVertexBuffer() , 0);
        Graphics::bindBuffer(rendererData.commandBufferId, mesh.getIndexBuffer() , 0);
        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.pipelineState);
        Graphics::bindDescriptorSet(rendererData.commandBufferId, mesh.getMaterial().getDescriptorSet(), rendererData.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);

        Graphics::drawIndexed(rendererData.commandBufferId, mesh.getIndexCount(), 1, 0, 0, 0);
    }

    Ref<DescriptorSetLayout>& Renderer::getMaterialDescriptorSetLayout() {
        return rendererData.materialDescriptorSetLayout;
    }

    Ref<Sampler>& Renderer::getDefaultSampler() {
        return rendererData.defaultSampler;
    }
}
