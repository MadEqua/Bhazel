#include "bzpch.h"

#include "Renderer2D.h"

#include "Graphics/Graphics.h"
#include "Graphics/DescriptorSet.h"
#include "Core/Application.h"


namespace BZ {

    constexpr uint32 MAX_RENDERER2D_TEXTURES = 8;
    constexpr uint32 MAX_RENDERER2D_OBJECTS = 100'000;

    static DataLayout dataLayout = {
        {DataType::Float32, DataElements::Vec3, "POSITION"},
        {DataType::Uint16, DataElements::Vec2, "TEXCOORD", true},
    };

    struct Vertex {
        float pos[3];
        uint16 texCoord[2];
    };

    constexpr uint16 MAX_TEX_COORD = 0xffff;
    static Vertex quadVertices[4] = {
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

    static uint16 quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

    static struct Renderer2DData {
        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;

        BufferPtr vertexBufferPtr;
        BufferPtr indexBufferPtr;

        Ref<PipelineState> pipelineState;
        Ref<Texture2D> whiteTexture;
        Ref<Sampler> sampler;
        Ref<DescriptorSetLayout> descriptorSetLayout;
        Ref<DescriptorSet> descriptorSet;

        Ref<TextureView> textureViews[MAX_RENDERER2D_TEXTURES];

        uint32 commandBufferId;
        uint32 nextObject;
        uint32 nextTexture;
    } data;


    static uint32 addTexture(const Ref<Texture2D>& texture) {
        for(uint32 i = 0; i < data.nextTexture; ++i) {
            if (data.textureViews[i]->getTexture() == texture) {
                return i;
            }
        }

        BZ_ASSERT_CORE(data.nextTexture < MAX_RENDERER2D_TEXTURES, "Reached the texture limit!");

        data.textureViews[data.nextTexture++] = TextureView::create(texture);
        return data.nextTexture - 1;
    }

    void Renderer2D::init() {
        BZ_PROFILE_FUNCTION();

        data.commandBufferId = -1;

        Shader::Builder shaderBuilder;
        shaderBuilder.setName("Renderer2D");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "shaders/bin/TextureVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "shaders/bin/TextureFrag.spv");

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = shaderBuilder.build();

        data.vertexBuffer = Buffer::create(BufferType::Vertex, 4 * sizeof(Vertex) * MAX_RENDERER2D_OBJECTS, MemoryType::CpuToGpu, dataLayout);
        data.indexBuffer = Buffer::create(BufferType::Index, 6 * sizeof(uint16) * MAX_RENDERER2D_OBJECTS, MemoryType::CpuToGpu);

        data.vertexBufferPtr = data.vertexBuffer->map(0);
        data.indexBufferPtr = data.indexBuffer->map(0);

        Sampler::Builder samplerBuilder;
        samplerBuilder.setAddressModeAll(AddressMode::ClampToEdge);
        data.sampler = samplerBuilder.build();

        byte whiteTextureData[] = {255, 255, 255, 255};
        data.whiteTexture = Texture2D::create(whiteTextureData, sizeof(whiteTextureData), 1, 1, TextureFormat::R8G8B8A8);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::Sampler, flagsToMask(ShaderStageFlags::Fragment), 1);
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::SampledTexture, flagsToMask(ShaderStageFlags::Fragment), 1);
        data.descriptorSetLayout = descriptorSetLayoutBuilder.build();

        data.descriptorSet = DescriptorSet::create(data.descriptorSetLayout);
        data.descriptorSet->setSampler(data.sampler, 0);

        //TODO: delete
        data.textureViews[0] = TextureView::create(Texture2D::create("textures/test.jpg", BZ::TextureFormat::R8G8B8A8_sRGB));
        data.descriptorSet->setSampledTexture(data.textureViews[0], 1);

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

        PushConstantDesc pushConstantDesc;
        pushConstantDesc.offset = 0;
        pushConstantDesc.size = sizeof(float) * 4;
        pushConstantDesc.shaderStageMask = flagsToMask(ShaderStageFlags::Fragment);

        pipelineStateData.pushConstantDescs = { pushConstantDesc };
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
        data.whiteTexture.reset();
        data.descriptorSetLayout.reset();
        data.descriptorSet.reset();
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(data.commandBufferId == -1, "There's already an unended Scene!");

        data.nextTexture = 0;
        data.nextObject = 0;

        data.commandBufferId = Graphics::beginCommandBuffer();
        Graphics::beginScene(data.commandBufferId, data.pipelineState, camera.getViewMatrix(), camera.getProjectionMatrix());
    }

    void Renderer2D::endScene() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        Graphics::bindBuffer(data.commandBufferId, data.vertexBuffer, 0);
        Graphics::bindBuffer(data.commandBufferId, data.indexBuffer, 0);
        Graphics::bindDescriptorSet(data.commandBufferId, data.descriptorSet, data.pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);
        Graphics::bindPipelineState(data.commandBufferId, data.pipelineState);

        glm::vec3 tint(1.0f, 0.0f,  1.0f);
        Graphics::setPushConstants(data.commandBufferId, data.pipelineState, flagsToMask(ShaderStageFlags::Fragment), &tint.x, sizeof(tint), 0);

        Graphics::drawIndexed(data.commandBufferId, data.nextObject * 6, 1, 0, 0, 0);
        Graphics::endCommandBuffer(data.commandBufferId);
        data.commandBufferId = -1;
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec3 &color) {
        BZ_PROFILE_FUNCTION();

        drawQuad(position, dimensions, rotationDeg, data.whiteTexture, color);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec3 &tint) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(data.commandBufferId != -1, "There's not a started Scene!");

        float c = glm::cos(glm::radians(rotationDeg));
        float s = glm::sin(glm::radians(rotationDeg));

        /*glm::mat4 modelMatrix = {};
        modelMatrix[0][0] = dimensions.x * c;
        modelMatrix[0][1] = dimensions.x * s;
        modelMatrix[1][0] = dimensions.y * -s;
        modelMatrix[1][1] = dimensions.y * c;
        modelMatrix[2][2] = 1.0f;
        modelMatrix[3][3] = 1.0f;
        modelMatrix[3][0] = position.x;
        modelMatrix[3][1] = position.y;*/

        //uint32 texIdx = addTexture(texture);
 
        BZ_ASSERT_CORE(data.nextObject < MAX_RENDERER2D_OBJECTS, "nextObject exceeded MAX_RENDERER2D_OBJECTS!");

        //glm::vec2 centerPos; 
        //centerPos.x = glm::dot(glm::vec4(dimensions.x * c, dimensions.y * -s, 0.0f, 0.0f), glm::vec4(position, 0.0f, 1.0f));
        //centerPos.y = glm::dot(glm::vec4(dimensions.x * s, dimensions.y * c, 0.0f, 0.0f), glm::vec4(position, 0.0f, 1.0f));

        //float halfW = dimensions.x * 0.5f;
        //float halfH = dimensions.y * 0.5f;

        Vertex vertices[4];
        uint16 indices[6];
        for (int i = 0; i < 4; ++i) {
            vertices[i].pos[0] = quadVertices[i].pos[0] * dimensions.x * c + quadVertices[i].pos[1] * dimensions.y * -s + position.x;
            vertices[i].pos[1] = quadVertices[i].pos[0] * dimensions.x * s + quadVertices[i].pos[1] * dimensions.y * c + position.y;
            vertices[i].pos[2] = 0.0f;
            vertices[i].texCoord[0] = quadVertices[i].texCoord[0];
            vertices[i].texCoord[1] = quadVertices[i].texCoord[1];
        }

        for (int i = 0; i < 6; ++i) {
            indices[i] = quadIndices[i] + (data.nextObject * 4);
        }

        uint32 offset = data.nextObject * sizeof(vertices);
        memcpy(data.vertexBufferPtr + offset, &vertices, sizeof(vertices));

        offset = data.nextObject * sizeof(indices);
        memcpy(data.indexBufferPtr + offset, &indices, sizeof(indices));

        data.nextObject++;
    }
}
