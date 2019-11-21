#include "bzpch.h"

#include "Renderer2D.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Core/Application.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    struct TextureData {
        Ref<TextureView> textureView;
        Ref<DescriptorSet> descriptorSet;
    };

    struct Renderer2DData {
        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;

        Ref<PipelineState> pipelineState;
        Ref<Texture2D> whiteTexture;
        Ref<Sampler> sampler;
        Ref<DescriptorSetLayout> descriptorSetLayout;

        std::unordered_map<uint32, TextureData> textureDatas;

        uint32 commandBufferId;
    };

    static Renderer2DData data;


    void Renderer2D::init() {
        BZ_PROFILE_FUNCTION();

        data.commandBufferId = -1;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer2D");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "shaders/bin/TextureVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "shaders/bin/TextureFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        DataLayout dataLayout = {
            {DataType::Float32, DataElements::Vec3, "POSITION"},
            {DataType::Uint16, DataElements::Vec2, "TEXCOORD", true}
        };

        struct Vertex {
            float pos[3];
            uint16 texCoord[2];
        };

        const uint16 MAX_TEX_COORD = 0xffff;
        Vertex vertices[4] = {
            {
                { -0.5f, -0.5f, 0.0f },
                { 0, 0 }
            },
            {
                { 0.5f, -0.5f, 0.0f },
                { MAX_TEX_COORD, 0 }
            },
            {
                { 0.5f, 0.5f, 0.0f },
                { MAX_TEX_COORD, MAX_TEX_COORD }
            },
            {
                { -0.5f, 0.5f, 0.0f },
                { 0, MAX_TEX_COORD }
            }
        };

        uint16 indices[] = { 0, 1, 2, 2, 3, 0 };

        //TODO: use a single buffer
        data.vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(vertices), MemoryType::GpuOnly, dataLayout);
        data.vertexBuffer->setData(vertices, sizeof(vertices), 0);
        data.indexBuffer = Buffer::create(BufferType::Index, sizeof(indices), MemoryType::GpuOnly);
        data.indexBuffer->setData(indices, sizeof(indices), 0);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        data.sampler = samplerBuilder.build();

        byte whiteTextureData[] = {255, 255, 255, 255};
        data.whiteTexture = Texture2D::create(whiteTextureData, sizeof(whiteTextureData), 1, 1, TextureFormat::R8G8B8A8);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::Sampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::SampledTexture, flagsToMask(ShaderStageFlags::Fragment), 1);
        data.descriptorSetLayout = descriptorSetLayoutBuilder.build();

        auto &windowDims = Application::getInstance().getWindow().getDimensions();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingStateAttachment.enableBlending = true;
        blendingStateAttachment.srcColorBlendingFactor = BlendingFactor::SourceAlpha;
        blendingStateAttachment.dstColorBlendingFactor = BlendingFactor::OneMinusSourceAlpha;
        blendingStateAttachment.colorBlendingOperation = BlendingOperation::Add;
        blendingStateAttachment.srcAlphaBlendingFactor = BlendingFactor::SourceAlpha;
        blendingStateAttachment.dstAlphaBlendingFactor = BlendingFactor::OneMinusSourceAlpha;
        blendingStateAttachment.alphaBlendingOperation = BlendingOperation::Add;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        pipelineStateData.dataLayout = dataLayout;
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(windowDims.x), static_cast<float>(windowDims.y) } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(windowDims.x), static_cast<uint32>(windowDims.y) } };
        pipelineStateData.descriptorSetLayouts = { data.descriptorSetLayout };
        pipelineStateData.blendingState = blendingState;
        data.pipelineState = PipelineState::create(pipelineStateData);
    }

    void Renderer2D::destroy() {
        BZ_PROFILE_FUNCTION();

        data.vertexBuffer.reset();
        data.indexBuffer.reset();
        data.pipelineState.reset();
        data.sampler.reset();
        data.textureDatas.clear();
        data.whiteTexture.reset();
        data.descriptorSetLayout.reset();
    }

    const Ref<DescriptorSet>& Renderer2D::getDescriptorSetForTexture(const Ref<Texture2D> &texture) {
        uint32 hash = reinterpret_cast<uint32>(texture.get()); //TODO: something better
        if (data.textureDatas.find(hash) == data.textureDatas.end()) {
            TextureData texData;
            texData.textureView = TextureView::create(texture);
            texData.descriptorSet = DescriptorSet::create(data.descriptorSetLayout);
            texData.descriptorSet->setSampler(data.sampler, 0);
            texData.descriptorSet->setSampledTexture(texData.textureView, 1);
            data.textureDatas.emplace(hash, texData);
        }

        TextureData &texData = data.textureDatas[hash];
        return texData.descriptorSet;
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(data.commandBufferId == -1, "There's already an unended Scene!");

        data.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(data.commandBufferId, camera.getViewMatrix(), camera.getProjectionMatrix());

        Graphics::bindBuffer(data.commandBufferId, data.vertexBuffer, 0);
        Graphics::bindBuffer(data.commandBufferId, data.indexBuffer, 0);
        Graphics::bindPipelineState(data.commandBufferId, data.pipelineState);
    }

    void Renderer2D::endScene() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        Graphics::endCommandBuffer(data.commandBufferId);
        data.commandBufferId = -1;
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec3 &color) {
        BZ_PROFILE_FUNCTION();

        drawQuad(position, dimensions, rotationDeg, data.whiteTexture, color);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2  &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec3 &tint) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        float c = glm::cos(glm::radians(rotationDeg));
        float s = glm::sin(glm::radians(rotationDeg));

        glm::mat4 modelMatrix = {};
        modelMatrix[0][0] = dimensions.x * c;
        modelMatrix[0][1] = dimensions.x * s;
        modelMatrix[1][0] = dimensions.y * -s;
        modelMatrix[1][1] = dimensions.y * c;
        modelMatrix[2][2] = 1.0f;
        modelMatrix[3][3] = 1.0f;
        modelMatrix[3][0] = position.x;
        modelMatrix[3][1] = position.y;

        auto &descriptorRef = getDescriptorSetForTexture(texture);
        Graphics::bindDescriptorSet(data.commandBufferId, descriptorRef, data.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);

        Graphics::beginObject(data.commandBufferId, modelMatrix, tint);
        Graphics::drawIndexed(data.commandBufferId, 6, 1, 0, 0, 0);
    }
}
