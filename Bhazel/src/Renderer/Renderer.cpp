#include "bzpch.h"

#include "Renderer.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Core/Application.h"


namespace BZ {

    RendererStats Renderer::stats;

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
        float pos[3];
        float normal[3];
        float tangent[3];
        uint16 texCoord[2];
    };

    constexpr int CUBE_VERTEX_COUNT = 8;
    constexpr int CUBE_INDEX_COUNT = 6 * 2 * 3;
    constexpr uint16 MAX_TEX_COORD = 0xffff;
    constexpr float v = 1.0f;
    Vertex cubeVertices[CUBE_VERTEX_COUNT] = {
        { { -v, -v, v }, {}, {}, { 0, 0 } },
    { { v, -v, v }, {}, {}, { MAX_TEX_COORD, 0 } },
    { { v, v, v }, {}, {}, { MAX_TEX_COORD, MAX_TEX_COORD } },
    { { -v, v, v }, {}, {}, { 0, MAX_TEX_COORD } },
    { { -v, -v, -v }, {}, {}, { MAX_TEX_COORD, 0 } },
    { { v, -v, -v }, {}, {}, { 0, 0 } },
    { { v, v, -v }, {}, {}, { 0, MAX_TEX_COORD } },
    { { -v, v, -v }, {}, {}, { MAX_TEX_COORD, MAX_TEX_COORD } },
    };

    uint32 cubeIndices[CUBE_INDEX_COUNT] = {
        0, 1, 2,
        0, 2, 3,
        1, 5, 6,
        1, 6, 2,
        4, 0, 3,
        4, 3, 7,
        5, 4, 6,
        5, 7, 6,
        4, 5, 1,
        4, 1, 0,
        3, 2, 6,
        3, 6, 7
    };

    static struct RendererData {
        uint32 commandBufferId;

        Ref<Buffer> cubeVertexBuffer;
        Ref<Buffer> cubeIndexBuffer;

        Ref<PipelineState> pipelineState;
    } rendererData;


    void Renderer::init() {
        BZ_PROFILE_FUNCTION();

        rendererData.commandBufferId = -1;

        rendererData.cubeVertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * CUBE_VERTEX_COUNT, MemoryType::GpuOnly, vertexLayout);
        rendererData.cubeIndexBuffer = Buffer::create(BufferType::Index, sizeof(uint32) * CUBE_INDEX_COUNT, MemoryType::GpuOnly, indexLayout);

        rendererData.cubeVertexBuffer->setData(cubeVertices, sizeof(cubeVertices), 0);
        rendererData.cubeIndexBuffer->setData(cubeIndices, sizeof(cubeIndices), 0);

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "Bhazel/shaders/bin/DefaultVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "Bhazel/shaders/bin/DefaultFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        const auto WINDOW_DIMS_INT = Application::getInstance().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::getInstance().getWindow().getDimensionsFloat();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        pipelineStateData.dataLayout = vertexLayout;

        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };
        //pipelineStateData.descriptorSetLayouts = { rendererData.descriptorSetLayout };
        pipelineStateData.blendingState = blendingState;
        rendererData.pipelineState = PipelineState::create(pipelineStateData);

    }

    void Renderer::destroy() {
        BZ_PROFILE_FUNCTION();

        rendererData.cubeVertexBuffer.reset();
        rendererData.cubeIndexBuffer.reset();

        rendererData.pipelineState.reset();
    }

    void Renderer::beginScene(const Camera &camera) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId == -1, "There's already an unended Scene!");

        memset(&stats, 0, sizeof(stats));

        rendererData.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(rendererData.commandBufferId, rendererData.pipelineState, camera.getViewMatrix(), camera.getProjectionMatrix());
    }

    void Renderer::endScene() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rendererData.commandBufferId != -1, "There's not a started Scene!");

        Graphics::endCommandBuffer(rendererData.commandBufferId);
        rendererData.commandBufferId = -1;
    }

    void Renderer::drawCube(const Transform &transform) {
        BZ_PROFILE_FUNCTION();

        Graphics::bindBuffer(rendererData.commandBufferId, rendererData.cubeVertexBuffer, 0);
        Graphics::bindBuffer(rendererData.commandBufferId, rendererData.cubeIndexBuffer, 0);

        Graphics::bindPipelineState(rendererData.commandBufferId, rendererData.pipelineState);

        //TODO: bind transform data
        //Graphics::bindDescriptorSet(rendererData.commandBufferId, 

        Graphics::drawIndexed(rendererData.commandBufferId, CUBE_INDEX_COUNT, 1, 0, 0, 0);
    }

    void Renderer::drawMesh(const Mesh &mesh) {
        BZ_PROFILE_FUNCTION();

        //TODO
    }
}
