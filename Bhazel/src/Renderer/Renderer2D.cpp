#include "bzpch.h"

#include "Renderer2D.h"

#include "Graphics/Graphics.h"
#include "Core/Application.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    struct Renderer2DVertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    struct Renderer2DData {
        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;

        Ref<DescriptorSet> descriptorSet;
        Ref<PipelineState> pipelineState;

        Ref<Texture2D> texture;
        Ref<TextureView> textureView;
        Ref<Sampler> sampler;

        uint32 commandBufferId;
    };

    static Renderer2DData data;


    void Renderer2D::init() {
        data.commandBufferId = -1;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer2D");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "shaders/bin/TextureVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "shaders/bin/TextureFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        DataLayout dataLayout = {
            {DataType::Float32, DataElements::Vec3, "POSITION"}, //TODO: pack this better
            {DataType::Float32, DataElements::Vec2, "TEXCOORD"},
        };

        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.0f, 0.0f,

            0.5f, -0.5f, 0.0f,
            1.0f, 0.0f,

            0.5f, 0.5f, 0.0f,
            1.0f, 1.0f,

            -0.5f, 0.5f, 0.0f,
            0.0f, 1.0f,
        };

        uint16 indices[] = { 0, 1, 2, 2, 3, 0 };

        data.vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(vertices), MemoryType::GpuOnly, dataLayout);
        data.vertexBuffer->setData(vertices, sizeof(vertices), 0);
        data.indexBuffer = Buffer::create(BufferType::Index, sizeof(indices), MemoryType::GpuOnly);
        data.indexBuffer->setData(indices, sizeof(indices), 0);

        data.texture = Texture2D::create("textures/test.jpg", TextureFormat::R8G8B8A8_sRGB);
        data.textureView = TextureView::create(data.texture);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        data.sampler = samplerBuilder.build();

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::CombinedTextureSampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        Ref<DescriptorSetLayout> descriptorSetLayout = descriptorSetLayoutBuilder.build();

        data.descriptorSet = DescriptorSet::create(descriptorSetLayout);
        data.descriptorSet->setCombinedTextureSampler(data.textureView, data.sampler, 0);

        auto &windowDims = Application::getInstance().getWindow().getDimensions();

        pipelineStateData.dataLayout = dataLayout;
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(windowDims.x), static_cast<float>(windowDims.y) } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(windowDims.x), static_cast<uint32>(windowDims.y) } };
        pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };
        pipelineStateData.blendingState.attachmentBlendingStates = { {} };
        data.pipelineState = PipelineState::create(pipelineStateData);
    }

    void Renderer2D::destroy() {
        data.vertexBuffer.reset();
        data.indexBuffer.reset();

        data.descriptorSet.reset();
        data.pipelineState.reset();

        data.texture.reset();
        data.textureView.reset();
        data.sampler.reset();
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera) {
        BZ_ASSERT_CORE(data.commandBufferId == -1, "There's already an unended Scene!");

        data.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(data.commandBufferId, camera.getViewMatrix(), camera.getProjectionMatrix());

        Graphics::bindBuffer(data.commandBufferId, data.vertexBuffer, 0);
        Graphics::bindBuffer(data.commandBufferId, data.indexBuffer, 0);
        Graphics::bindPipelineState(data.commandBufferId, data.pipelineState);
        Graphics::bindDescriptorSet(data.commandBufferId, data.descriptorSet, data.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);
    }

    void Renderer2D::endScene() {
        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        Graphics::endCommandBuffer(data.commandBufferId);
        data.commandBufferId = -1;
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, const glm::vec3 &tint) {
        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        glm::mat4 modelMatrix = {};
        modelMatrix[0][0] = dimensions.x;
        modelMatrix[1][1] = dimensions.y;
        modelMatrix[2][2] = 1.0f;
        modelMatrix[3][3] = 1.0f;
        modelMatrix[3][0] = position.x;
        modelMatrix[3][1] = position.y;

        Graphics::beginObject(data.commandBufferId, modelMatrix, tint);
        Graphics::drawIndexed(data.commandBufferId, 6, 1, 0, 0, 0);
    }

    /*void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &dimensions, const glm::vec4 &tint) {
        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        BZ::Graphics::bindBuffer(commandBufferId, vertexBuffer, 0);
        BZ::Graphics::bindBuffer(commandBufferId, indexBuffer, 0);
        BZ::Graphics::bindPipelineState(commandBufferId, pipelineState);
        BZ::Graphics::bindDescriptorSet(commandBufferId, descriptorSet, pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);


        Graphics::beginObject(data.commandBufferId, modelMatrix);
        Graphics::drawIndexed(data.commandBufferId, 4, 1, 0, 0, 0);
    }*/
}
