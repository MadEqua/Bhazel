#include "bzpch.h"

#include "Renderer.h"

#include "BRDFLookup.h"

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

#include "Core/Engine.h"
#include "Core/Utils.h"
#include "Core/Window.h"

#include "Renderer/Components/Camera.h"
#include "Renderer/Components/Material.h"
#include "Renderer/Components/Mesh.h"
#include "Renderer/Components/Transform.h"
#include "Renderer/PostProcessor.h"
#include "Renderer/Scene.h"

#include <imgui.h>


namespace BZ {

constexpr uint32 RENDERER_GLOBAL_DESCRIPTOR_SET_IDX = 0;
constexpr uint32 RENDERER_SCENE_DESCRIPTOR_SET_IDX = 1;
constexpr uint32 RENDERER_PASS_DESCRIPTOR_SET_IDX = 2;
constexpr uint32 RENDERER_MATERIAL_DESCRIPTOR_SET_IDX = 3;
constexpr uint32 RENDERER_ENTITY_DESCRIPTOR_SET_IDX = 4;
constexpr uint32 APP_FIRST_DESCRIPTOR_SET_IDX = 5;

constexpr uint32 SHADOW_MAP_SIZE = 1024;
constexpr uint32 SHADOW_MAPPING_CASCADE_COUNT = 4;

constexpr uint32 MAX_PASSES_PER_FRAME =
    Renderer::MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT + 1; // Depth Passes + Color Pass

struct alignas(GraphicsContext::MIN_UNIFORM_BUFFER_OFFSET_ALIGN) SceneConstantBufferData {
    glm::mat4
        lightMatrices[Renderer::MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT]; // World to light clip space
    glm::vec4 dirLightDirectionsAndIntensities[Renderer::MAX_DIR_LIGHTS_PER_SCENE];
    glm::vec4 dirLightColors[Renderer::MAX_DIR_LIGHTS_PER_SCENE]; // vec4 to simplify alignments
    glm::vec4 cascadeSplits;                                      // TODO: Hardcoded to 4.
    float dirLightCount;
};

struct alignas(GraphicsContext::MIN_UNIFORM_BUFFER_OFFSET_ALIGN) PassConstantBufferData {
    glm::mat4 viewMatrix;           // World to camera space
    glm::mat4 projectionMatrix;     // Camera to clip space
    glm::mat4 viewProjectionMatrix; // World to clip space
    glm::vec4 cameraPosition;
};

struct alignas(GraphicsContext::MIN_UNIFORM_BUFFER_OFFSET_ALIGN) MaterialConstantBufferData {
    glm::vec4 normalMetallicRoughnessAndAO;
    glm::vec4 heightAndUvScale;
};

struct alignas(GraphicsContext::MIN_UNIFORM_BUFFER_OFFSET_ALIGN) EntityConstantBufferData {
    glm::mat4 modelMatrix;  // Model to world space
    glm::mat4 normalMatrix; // Model to world space, appropriate to transform vectors, mat4 to simplify alignments
};

constexpr uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData);
constexpr uint32 PASS_CONSTANT_BUFFER_SIZE = sizeof(PassConstantBufferData) * MAX_PASSES_PER_FRAME;
constexpr uint32 MATERIAL_CONSTANT_BUFFER_SIZE = sizeof(MaterialConstantBufferData) * Renderer::MAX_MATERIALS_PER_SCENE;
constexpr uint32 ENTITY_CONSTANT_BUFFER_SIZE = sizeof(EntityConstantBufferData) * Renderer::MAX_ENTITIES_PER_SCENE;
constexpr uint32 POST_PROCESS_CONSTANT_BUFFER_SIZE = sizeof(PostProcessConstantBufferData);

constexpr uint32 SCENE_CONSTANT_BUFFER_OFFSET = 0;
constexpr uint32 PASS_CONSTANT_BUFFER_OFFSET = SCENE_CONSTANT_BUFFER_SIZE;
constexpr uint32 MATERIAL_CONSTANT_BUFFER_OFFSET = PASS_CONSTANT_BUFFER_OFFSET + PASS_CONSTANT_BUFFER_SIZE;
constexpr uint32 ENTITY_CONSTANT_BUFFER_OFFSET = MATERIAL_CONSTANT_BUFFER_OFFSET + MATERIAL_CONSTANT_BUFFER_SIZE;
constexpr uint32 POST_PROCESS_CONSTANT_BUFFER_OFFSET = ENTITY_CONSTANT_BUFFER_OFFSET + ENTITY_CONSTANT_BUFFER_SIZE;

static DataLayout vertexDataLayout = {
    { DataType::Float32, DataElements::Vec3 },
    { DataType::Float32, DataElements::Vec3 },
    { DataType::Float32, DataElements::Vec4 },
    { DataType::Uint16, DataElements::Vec2, true },
};

static DataLayout indexDataLayout = { { DataType::Uint32, DataElements::Scalar, "" } };

struct RendererStats {
    uint32 vertexCount;
    uint32 triangleCount;
    uint32 drawCallCount;
    uint32 materialCount;
};

static struct RendererData {
    PostProcessor postProcessor;

    Ref<Buffer> constantBuffer;
    BufferPtr sceneConstantBufferPtr;
    BufferPtr passConstantBufferPtr;
    BufferPtr materialConstantBufferPtr;
    BufferPtr entityConstantBufferPtr;
    BufferPtr postProcessConstantBufferPtr;

    Ref<DescriptorSetLayout> globalDescriptorSetLayout;
    Ref<DescriptorSetLayout> sceneDescriptorSetLayout;
    Ref<DescriptorSetLayout> passDescriptorSetLayout;
    Ref<DescriptorSetLayout> passDescriptorSetLayoutForShadowPass;
    Ref<DescriptorSetLayout> materialDescriptorSetLayout;
    Ref<DescriptorSetLayout> entityDescriptorSetLayout;
    Ref<PipelineLayout> pipelineLayout;

    DescriptorSet *globalDescriptorSet;
    DescriptorSet *passDescriptorSet;
    DescriptorSet *passDescriptorSetForShadowPass;
    DescriptorSet *entityDescriptorSet;

    Ref<Sampler> defaultSampler;
    Ref<Sampler> defaultAnisotropicSampler;
    Ref<Sampler> brdfLookupSampler;
    Ref<Sampler> shadowSampler;

    Ref<PipelineLayout> shadowPassPipelineLayout;
    Ref<PipelineState> colorPassPipelineState;
    Ref<PipelineState> skyBoxPipelineState;
    Ref<PipelineState> shadowPassPipelineState;

    Ref<TextureView> brdfLookupTexture;

    std::unordered_map<Material, uint32> materialOffsetMap;

    Ref<RenderPass> shadowRenderPass;
    Ref<RenderPass> colorRenderPass;

    Ref<Framebuffer> colorFramebuffer;
    Ref<TextureView> colorTexView;
    Ref<TextureView> depthTexView;

    const Scene *sceneToRender;

    // ConstantFactor, clamp and slopeFactor
    glm::vec3 depthBiasData = { 1.0f, 0.0f, 2.5f };

    // Stats
    RendererStats stats;
    RendererStats visibleStats;

    uint32 statsRefreshPeriodMs = 250;
    uint32 statsRefreshTimeAcumMs;
} rendererData;


void Renderer::init() {
    BZ_PROFILE_FUNCTION();

    rendererData.sceneToRender = nullptr;

    rendererData.constantBuffer =
        Buffer::create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       SCENE_CONSTANT_BUFFER_SIZE + PASS_CONSTANT_BUFFER_SIZE + MATERIAL_CONSTANT_BUFFER_SIZE +
                           ENTITY_CONSTANT_BUFFER_SIZE + POST_PROCESS_CONSTANT_BUFFER_SIZE,
                       MemoryType::CpuToGpu);
    BZ_SET_BUFFER_DEBUG_NAME(rendererData.constantBuffer, "Renderer ConstantBuffer");

    rendererData.sceneConstantBufferPtr = rendererData.constantBuffer->map(0);
    rendererData.passConstantBufferPtr = rendererData.sceneConstantBufferPtr + PASS_CONSTANT_BUFFER_OFFSET;
    rendererData.materialConstantBufferPtr = rendererData.sceneConstantBufferPtr + MATERIAL_CONSTANT_BUFFER_OFFSET;
    rendererData.entityConstantBufferPtr = rendererData.sceneConstantBufferPtr + ENTITY_CONSTANT_BUFFER_OFFSET;
    rendererData.postProcessConstantBufferPtr =
        rendererData.sceneConstantBufferPtr + POST_PROCESS_CONSTANT_BUFFER_OFFSET;

    rendererData.globalDescriptorSetLayout =
        DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

    rendererData.sceneDescriptorSetLayout = DescriptorSetLayout::create(
        { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_ALL, 1 },
          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_DIR_LIGHTS_PER_SCENE } });

    rendererData.passDescriptorSetLayout =
        DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 1 } });

    rendererData.materialDescriptorSetLayout =
        DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                      // Albedo, Normal, Metallic, Roughness, Height and AO.
                                      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

    rendererData.entityDescriptorSetLayout =
        DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 1 } });

    rendererData.pipelineLayout =
        PipelineLayout::create({ rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                 rendererData.passDescriptorSetLayout, rendererData.materialDescriptorSetLayout,
                                 rendererData.entityDescriptorSetLayout });

    rendererData.entityDescriptorSet = &DescriptorSet::get(rendererData.entityDescriptorSetLayout);
    rendererData.entityDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, ENTITY_CONSTANT_BUFFER_OFFSET,
                                                        sizeof(EntityConstantBufferData));

    Sampler::Builder samplerBuilder;
    rendererData.defaultSampler = samplerBuilder.build();

    Sampler::Builder samplerBuilder2;
    samplerBuilder2.enableAnisotropy(16.0f);
    rendererData.defaultAnisotropicSampler = samplerBuilder2.build();

    initShadowPassData();
    initColorPassData();
    initSkyBoxData();

    rendererData.postProcessor.init(rendererData.colorTexView, rendererData.constantBuffer,
                                    POST_PROCESS_CONSTANT_BUFFER_OFFSET);
}

void Renderer::initShadowPassData() {
    BZ_PROFILE_FUNCTION();

    AttachmentDescription depthAttachmentDesc;
    depthAttachmentDesc.format = VK_FORMAT_D32_SFLOAT;
    depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthAttachmentDesc.clearValue.depthStencil.depth = 1.0f;

    SubPassDescription subPassDesc;
    subPassDesc.depthStencilAttachmentsRef = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    // TODO: this dependency is too strong. Will make it wait on the whole previous frame. Use an event to wait only on
    // color pass.
    SubPassDependency dependency1;
    dependency1.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
    dependency1.dstSubPassIndex = 0;
    dependency1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency1.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency1.srcAccessMask = 0;
    dependency1.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency1.dependencyFlags = 0;

    // Wait for the memcpyied constant data to be available before doing actual rendering.
    SubPassDependency dependency2;
    dependency2.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
    dependency2.dstSubPassIndex = 0;
    dependency2.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dependency2.dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    dependency2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dependency2.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    dependency2.dependencyFlags = 0;

    rendererData.shadowRenderPass =
        RenderPass::create({ depthAttachmentDesc }, { subPassDesc }, { dependency1, dependency2 });

    rendererData.passDescriptorSetLayoutForShadowPass = DescriptorSetLayout::create(
        { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
            SHADOW_MAPPING_CASCADE_COUNT } });

    rendererData.shadowPassPipelineLayout =
        PipelineLayout::create({ rendererData.globalDescriptorSetLayout, rendererData.sceneDescriptorSetLayout,
                                 rendererData.passDescriptorSetLayoutForShadowPass,
                                 rendererData.materialDescriptorSetLayout, rendererData.entityDescriptorSetLayout });
    DepthStencilState depthStencilState;
    depthStencilState.enableDepthTest = true;
    depthStencilState.enableDepthWrite = true;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

    RasterizerState rasterizerState;
    // To disable clipping on z axis. Geometry behind "light camera" will still cast shadows.
    rasterizerState.enableDepthClamp = true;
    // To avoid shadow acne.
    rasterizerState.enableDepthBias = true;
    // rasterizerState.depthBiasConstantFactor = 0.0f;
    // rasterizerState.depthBiasSlopeFactor = 0.0f;

    PipelineStateData pipelineStateData;
    pipelineStateData.dataLayout = vertexDataLayout;
    pipelineStateData.shader =
        Shader::create({ { "Bhazel/shaders/bin/ShadowPassVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                         { "Bhazel/shaders/bin/ShadowPassGeo.spv", VK_SHADER_STAGE_GEOMETRY_BIT } });
    pipelineStateData.layout = rendererData.shadowPassPipelineLayout;
    pipelineStateData.viewports = { { 0.0f, 0.0f, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0.0f, 1.0f } };
    pipelineStateData.scissorRects = { { 0u, 0u, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } };
    pipelineStateData.rasterizerState = rasterizerState;
    pipelineStateData.depthStencilState = depthStencilState;
    pipelineStateData.dynamicStates = { VK_DYNAMIC_STATE_DEPTH_BIAS };
    pipelineStateData.renderPass = rendererData.shadowRenderPass;
    pipelineStateData.subPassIndex = 0;
    rendererData.shadowPassPipelineState = PipelineState::create(pipelineStateData);
    BZ_SET_PIPELINE_DEBUG_NAME(rendererData.shadowPassPipelineState, "Renderer Shadow Pass Pipeline");

    rendererData.passDescriptorSetForShadowPass =
        &DescriptorSet::get(rendererData.passDescriptorSetLayoutForShadowPass);
    Ref<Buffer> constantBuffers[SHADOW_MAPPING_CASCADE_COUNT];
    uint32 offsets[SHADOW_MAPPING_CASCADE_COUNT];
    uint32 sizes[SHADOW_MAPPING_CASCADE_COUNT];
    for (uint32 i = 0; i < SHADOW_MAPPING_CASCADE_COUNT; ++i) {
        constantBuffers[i] = rendererData.constantBuffer;
        offsets[i] = PASS_CONSTANT_BUFFER_OFFSET + sizeof(PassConstantBufferData) * i;
        sizes[i] = sizeof(PassConstantBufferData);
    }
    rendererData.passDescriptorSetForShadowPass->setConstantBuffers(constantBuffers, SHADOW_MAPPING_CASCADE_COUNT, 0, 0,
                                                                    offsets, sizes);

    Sampler::Builder samplerBuilder;
    samplerBuilder.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    samplerBuilder.setBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    samplerBuilder.enableCompare(VK_COMPARE_OP_LESS);
    rendererData.shadowSampler = samplerBuilder.build();
}

void Renderer::initColorPassData() {
    BZ_PROFILE_FUNCTION();

    AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    AttachmentDescription depthStencilAttachmentDesc;
    depthStencilAttachmentDesc.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depthStencilAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthStencilAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilAttachmentDesc.clearValue.depthStencil.depth = 1.0f;
    depthStencilAttachmentDesc.clearValue.depthStencil.stencil = 0;

    SubPassDescription subPassDesc;
    subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
    subPassDesc.depthStencilAttachmentsRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    // Wait on ShadowPass.
    SubPassDependency dependency1;
    dependency1.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
    dependency1.dstSubPassIndex = 0;
    dependency1.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency1.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency1.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency1.dependencyFlags = 0;

    // Wait for the memcpyied constant data to be available before doing actual rendering.
    // No need as long as this pass waits on shadow pass which alaready does this wait.
    // SubPassDependency dependency2;
    // dependency2.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
    // dependency2.dstSubPassIndex = 0;
    // dependency2.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    // dependency2.dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    // dependency2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // dependency2.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    // dependency2.dependencyFlags = 0;

    rendererData.colorRenderPass =
        RenderPass::create({ colorAttachmentDesc, depthStencilAttachmentDesc }, { subPassDesc }, { dependency1 });

    RasterizerState rasterizerState;
    rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerState.frontFaceCounterClockwise = true;

    DepthStencilState depthStencilState;
    depthStencilState.enableDepthTest = true;
    depthStencilState.enableDepthWrite = true;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

    BlendingState blendingState;
    BlendingStateAttachment blendingStateAttachment;
    blendingState.attachmentBlendingStates = { blendingStateAttachment };

    PipelineStateData pipelineStateData;
    pipelineStateData.dataLayout = vertexDataLayout;
    pipelineStateData.shader =
        Shader::create({ { "Bhazel/shaders/bin/RendererVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                         { "Bhazel/shaders/bin/RendererFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
    pipelineStateData.layout = rendererData.pipelineLayout;
    pipelineStateData.rasterizerState = rasterizerState;
    pipelineStateData.depthStencilState = depthStencilState;
    pipelineStateData.blendingState = blendingState;
    pipelineStateData.renderPass = rendererData.colorRenderPass;
    pipelineStateData.subPassIndex = 0;
    rendererData.colorPassPipelineState = PipelineState::create(pipelineStateData);
    BZ_SET_PIPELINE_DEBUG_NAME(rendererData.colorPassPipelineState, "Renderer Color Pass Pipeline");

    const auto WINDOW_DIMS_INT = Engine::get().getWindow().getDimensions();

    auto colorTexture = Texture2D::createRenderTarget(WINDOW_DIMS_INT.x, WINDOW_DIMS_INT.y, 1, 1,
                                                      colorAttachmentDesc.format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    BZ_SET_TEXTURE_DEBUG_NAME(colorTexture, "Renderer Color Texture");
    rendererData.colorTexView = TextureView::create(colorTexture);

    auto depthTexture =
        Texture2D::createRenderTarget(WINDOW_DIMS_INT.x, WINDOW_DIMS_INT.y, 1, 1, depthStencilAttachmentDesc.format);
    BZ_SET_TEXTURE_DEBUG_NAME(depthTexture, "Renderer Depth Texture");
    rendererData.depthTexView = TextureView::create(depthTexture);

    rendererData.colorFramebuffer =
        Framebuffer::create(rendererData.colorRenderPass, { rendererData.colorTexView, rendererData.depthTexView },
                            glm::uvec3(WINDOW_DIMS_INT.x, WINDOW_DIMS_INT.y, 1));


    Sampler::Builder samplerBuilder;
    samplerBuilder.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    rendererData.brdfLookupSampler = samplerBuilder.build();

    auto brdfLookupTexRef = Texture2D::create(reinterpret_cast<const byte *>(brdfLut), brdfLutSize, brdfLutSize,
                                              VK_FORMAT_R16G16_SFLOAT, MipmapData::Options::DoNothing);
    BZ_SET_TEXTURE_DEBUG_NAME(brdfLookupTexRef, "Renderer BRDF Lookup Texture");
    rendererData.brdfLookupTexture = TextureView::create(brdfLookupTexRef);

    rendererData.globalDescriptorSet = &DescriptorSet::get(rendererData.globalDescriptorSetLayout);
    rendererData.globalDescriptorSet->setCombinedTextureSampler(rendererData.brdfLookupTexture,
                                                                rendererData.brdfLookupSampler, 0);

    rendererData.passDescriptorSet = &DescriptorSet::get(rendererData.passDescriptorSetLayout);
    rendererData.passDescriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, PASS_CONSTANT_BUFFER_OFFSET,
                                                      sizeof(PassConstantBufferData));
}

void Renderer::initSkyBoxData() {
    BZ_PROFILE_FUNCTION();

    RasterizerState rasterizerState;
    rasterizerState.cullMode = VK_CULL_MODE_NONE;

    DepthStencilState depthStencilState;
    depthStencilState.enableDepthTest = true;
    depthStencilState.enableDepthWrite = false;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

    BlendingState blendingState;
    BlendingStateAttachment blendingStateAttachment;
    blendingState.attachmentBlendingStates = { blendingStateAttachment };

    PipelineStateData pipelineStateData;
    pipelineStateData.dataLayout = vertexDataLayout;
    pipelineStateData.shader =
        Shader::create({ { "Bhazel/shaders/bin/SkyBoxVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                         { "Bhazel/shaders/bin/SkyBoxFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
    pipelineStateData.layout = rendererData.pipelineLayout;
    pipelineStateData.rasterizerState = rasterizerState;
    pipelineStateData.depthStencilState = depthStencilState;
    pipelineStateData.blendingState = blendingState;
    pipelineStateData.renderPass = rendererData.colorRenderPass;
    pipelineStateData.subPassIndex = 0;
    rendererData.skyBoxPipelineState = PipelineState::create(pipelineStateData);
    BZ_SET_PIPELINE_DEBUG_NAME(rendererData.skyBoxPipelineState, "Renderer Skybox Pipeline");
}

void Renderer::destroy() {
    BZ_PROFILE_FUNCTION();

    rendererData.constantBuffer.reset();

    rendererData.globalDescriptorSetLayout.reset();
    rendererData.sceneDescriptorSetLayout.reset();
    rendererData.passDescriptorSetLayout.reset();
    rendererData.passDescriptorSetLayoutForShadowPass.reset();
    rendererData.materialDescriptorSetLayout.reset();
    rendererData.entityDescriptorSetLayout.reset();
    rendererData.pipelineLayout.reset();

    rendererData.colorPassPipelineState.reset();
    rendererData.skyBoxPipelineState.reset();
    rendererData.shadowPassPipelineState.reset();
    rendererData.shadowPassPipelineLayout.reset();

    rendererData.defaultSampler.reset();
    rendererData.defaultAnisotropicSampler.reset();
    rendererData.brdfLookupSampler.reset();
    rendererData.shadowSampler.reset();

    rendererData.materialOffsetMap.clear();

    rendererData.brdfLookupTexture.reset();

    rendererData.shadowRenderPass.reset();

    rendererData.colorRenderPass.reset();

    rendererData.colorTexView.reset();
    rendererData.depthTexView.reset();

    rendererData.colorFramebuffer.reset();

    rendererData.postProcessor.destroy();
}

void Renderer::renderScene(const Scene &scene) {
    BZ_PROFILE_FUNCTION();

    rendererData.sceneToRender = &scene;
}

void Renderer::shadowPass(const Scene &scene) {
    BZ_PROFILE_FUNCTION();

    CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
    BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "Shadow Pass");

    commandBuffer.bindDescriptorSet(*rendererData.globalDescriptorSet, rendererData.pipelineLayout,
                                    RENDERER_GLOBAL_DESCRIPTOR_SET_IDX, 0, 0);
    commandBuffer.bindDescriptorSet(rendererData.sceneToRender->getDescriptorSet(), rendererData.pipelineLayout,
                                    RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

    commandBuffer.bindPipelineState(rendererData.shadowPassPipelineState);
    commandBuffer.setDepthBias(rendererData.depthBiasData.x, rendererData.depthBiasData.y,
                               rendererData.depthBiasData.z);

    uint32 lightIdx = 0;
    for (auto &dirLight : scene.getDirectionalLights()) {
        uint32 lightOffset = lightIdx * sizeof(PassConstantBufferData) * SHADOW_MAPPING_CASCADE_COUNT;
        uint32 lightOffsetArr[SHADOW_MAPPING_CASCADE_COUNT];
        for (uint32 i = 0; i < SHADOW_MAPPING_CASCADE_COUNT; ++i) {
            lightOffsetArr[i] = lightOffset;
        }
        commandBuffer.bindDescriptorSet(*rendererData.passDescriptorSetForShadowPass,
                                        rendererData.shadowPassPipelineLayout, RENDERER_PASS_DESCRIPTOR_SET_IDX,
                                        lightOffsetArr, SHADOW_MAPPING_CASCADE_COUNT);

        commandBuffer.beginRenderPass(rendererData.shadowRenderPass, dirLight.shadowMapFramebuffer);
        drawEntities(commandBuffer, scene, true);
        commandBuffer.endRenderPass();
        lightIdx++;
    }

    BZ_CB_END_DEBUG_LABEL(commandBuffer);
    commandBuffer.endAndSubmit(false, false);
}

void Renderer::colorPass(const Scene &scene) {
    BZ_PROFILE_FUNCTION();

    CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
    BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "Color Pass");

    commandBuffer.bindDescriptorSet(*rendererData.globalDescriptorSet, rendererData.pipelineLayout,
                                    RENDERER_GLOBAL_DESCRIPTOR_SET_IDX, 0, 0);
    commandBuffer.bindDescriptorSet(rendererData.sceneToRender->getDescriptorSet(), rendererData.pipelineLayout,
                                    RENDERER_SCENE_DESCRIPTOR_SET_IDX, 0, 0);

    uint32 colorPassOffset =
        PASS_CONSTANT_BUFFER_SIZE - sizeof(PassConstantBufferData); // Color pass is the last (after the Depth passes).
    commandBuffer.bindDescriptorSet(*rendererData.passDescriptorSet, rendererData.pipelineLayout,
                                    RENDERER_PASS_DESCRIPTOR_SET_IDX, &colorPassOffset, 1);

    commandBuffer.beginRenderPass(rendererData.colorRenderPass, rendererData.colorFramebuffer);
    commandBuffer.bindPipelineState(rendererData.colorPassPipelineState);

    drawEntities(commandBuffer, scene, false);

    if (scene.hasSkyBox()) {
        commandBuffer.bindPipelineState(rendererData.skyBoxPipelineState);
        drawMesh(commandBuffer, scene.getSkyBox().mesh, Material(), false);
    }

    commandBuffer.endRenderPass();
    BZ_CB_END_DEBUG_LABEL(commandBuffer);
    commandBuffer.endAndSubmit(false, false);
}

void Renderer::drawEntities(CommandBuffer &commandBuffer, const Scene &scene, bool shadowPass) {
    BZ_PROFILE_FUNCTION();

    uint32 entityIndex = 0;
    for (const auto &entity : scene.getEntities()) {
        if (!shadowPass || entity.castShadow) {
            uint32 entityOffset = entityIndex * sizeof(EntityConstantBufferData);
            commandBuffer.bindDescriptorSet(*rendererData.entityDescriptorSet,
                                            shadowPass ? rendererData.shadowPassPipelineLayout :
                                                         rendererData.pipelineLayout,
                                            RENDERER_ENTITY_DESCRIPTOR_SET_IDX, &entityOffset, 1);

            drawMesh(commandBuffer, entity.mesh, entity.overrideMaterial, shadowPass);
            entityIndex++;
        }
    }
}

void Renderer::drawMesh(CommandBuffer &commandBuffer, const Mesh &mesh, const Material &overrideMaterial,
                        bool shadowPass) {
    BZ_PROFILE_FUNCTION();

    commandBuffer.bindBuffer(mesh.getVertexBuffer(), 0);

    if (mesh.hasIndices())
        commandBuffer.bindBuffer(mesh.getIndexBuffer(), 0);

    for (const auto &submesh : mesh.getSubmeshes()) {
        if (!shadowPass) {
            const Material &materialToUse = overrideMaterial.isValid() ? overrideMaterial : submesh.material;

            uint32 materialOffset = rendererData.materialOffsetMap[materialToUse];
            commandBuffer.bindDescriptorSet(materialToUse.getDescriptorSet(),
                                            shadowPass ? rendererData.shadowPassPipelineLayout :
                                                         rendererData.pipelineLayout,
                                            RENDERER_MATERIAL_DESCRIPTOR_SET_IDX, &materialOffset, 1);
        }

        if (mesh.hasIndices())
            commandBuffer.drawIndexed(submesh.indexCount, 1, submesh.indexOffset, 0, 0);
        else
            commandBuffer.draw(submesh.vertexCount, 1, submesh.vertexOffset, 1);

        rendererData.stats.drawCallCount++;
    }

    rendererData.stats.vertexCount += mesh.getVertexCount();
    rendererData.stats.triangleCount += (mesh.hasIndices() ? mesh.getIndexCount() : mesh.getVertexCount()) / 3;
}

void Renderer::fillConstants(const Scene &scene) {
    BZ_PROFILE_FUNCTION();

    // Matrices related to the depth passes for the lights.
    glm::mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT];
    glm::mat4 lightProjectionMatrices[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT];

    const PerspectiveCamera &camera = static_cast<const PerspectiveCamera &>(scene.getCamera());
    const PerspectiveCamera::Parameters &cameraParams = camera.getParameters();

    float cascadeSplits[SHADOW_MAPPING_CASCADE_COUNT];
    computeCascadedShadowMappingSplits(cascadeSplits, SHADOW_MAPPING_CASCADE_COUNT, cameraParams.near,
                                       cameraParams.far);

    uint32 matrixIndex = 0;
    for (auto &dirLight : scene.getDirectionalLights()) {

        for (uint32 cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
            const float cascadeNear = cascadeIdx == 0 ? cameraParams.near : cascadeSplits[cascadeIdx];
            const float cascadeFar = cascadeIdx == 0 ? cascadeSplits[0] : cascadeSplits[cascadeIdx + 1];

            // Compute Sphere that enconpasses the frustum, in camera space.
            // The sphere is useful to have the shadow frustum to be always the same size regardless of the camera
            // orientation, leading to no shadow flickering.
            float t = glm::tan(glm::radians(cameraParams.aspectRatio * cameraParams.fovy * 0.5f));
            glm::vec3 n(t * cascadeNear, 0.0f, cascadeNear);
            glm::vec3 f(t * cascadeFar, 0.0f, cascadeFar);

            // Solve the equation |f-center|=|n-center|, knowing that the sphere center is (0, 0, centerZ).
            float centerZ = ((f.x * f.x + f.z * f.z) - (n.x * n.x + n.z * n.z)) / (2.0f * f.z - 2.0f * n.z);
            float r = glm::distance(glm::vec3(0.0f, 0.0f, centerZ), n);

            glm::vec3 sphereCenterWorld =
                camera.getTransform().getLocalToParentMatrix() * glm::vec4(0.0f, 0.0f, centerZ, 1.0f);

            // Units of view space per shadow map texel (and world space, assuming no scaling between the two spaces).
            const float Q = glm::ceil(r * 2.0f) / static_cast<float>(SHADOW_MAP_SIZE);

            glm::vec3 lightCamPos = sphereCenterWorld - dirLight.getDirection() * r;
            lightMatrices[matrixIndex] = glm::lookAtRH(lightCamPos, sphereCenterWorld, glm::vec3(0, 1, 0));

            // Apply the quantization to translation to stabilize shadows when camera moves. We only move the light
            // camera in texel sized snaps.
            lightMatrices[matrixIndex][3].x = glm::floor(lightMatrices[matrixIndex][3].x / Q) * Q;
            lightMatrices[matrixIndex][3].y = glm::floor(lightMatrices[matrixIndex][3].y / Q) * Q;
            lightMatrices[matrixIndex][3].z = glm::floor(lightMatrices[matrixIndex][3].z / Q) * Q;

            lightProjectionMatrices[matrixIndex] = Utils::ortho(-r, r, -r, r, 0.1f, r * 2.0f);
            matrixIndex++;
        }
    }

    fillScene(scene, lightMatrices, lightProjectionMatrices, cascadeSplits);
    fillPasses(scene, lightMatrices, lightProjectionMatrices);
    fillMaterials(scene);
    fillEntities(scene);
    rendererData.postProcessor.fillData(rendererData.postProcessConstantBufferPtr, scene);
}

void Renderer::fillScene(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices,
                         const float cascadeSplits[]) {
    BZ_PROFILE_FUNCTION();

    SceneConstantBufferData sceneConstantBufferData;
    int lightIdx = 0;
    int totalCascadeIdx = 0;
    for (const auto &dirLight : scene.getDirectionalLights()) {
        for (uint32 cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
            sceneConstantBufferData.lightMatrices[totalCascadeIdx] =
                lightProjectionMatrices[totalCascadeIdx] * lightMatrices[totalCascadeIdx];
            sceneConstantBufferData.cascadeSplits[cascadeIdx] = cascadeSplits[cascadeIdx];
            totalCascadeIdx++;
        }

        const auto &dir = dirLight.getDirection();
        sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].x = dir.x;
        sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].y = dir.y;
        sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].z = dir.z;
        sceneConstantBufferData.dirLightDirectionsAndIntensities[lightIdx].w = dirLight.intensity;
        sceneConstantBufferData.dirLightColors[lightIdx].r = dirLight.color.r;
        sceneConstantBufferData.dirLightColors[lightIdx].g = dirLight.color.g;
        sceneConstantBufferData.dirLightColors[lightIdx].b = dirLight.color.b;
        lightIdx++;
    }
    sceneConstantBufferData.dirLightCount = static_cast<float>(lightIdx);
    memcpy(rendererData.sceneConstantBufferPtr, &sceneConstantBufferData, sizeof(SceneConstantBufferData));
}

void Renderer::fillPasses(const Scene &scene, const glm::mat4 *lightMatrices,
                          const glm::mat4 *lightProjectionMatrices) {
    BZ_PROFILE_FUNCTION();

    // Depth Pass data
    uint32 passIndex = 0;
    for (auto &dirLight : scene.getDirectionalLights()) {
        for (uint32 cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
            uint32 passOffset = passIndex * sizeof(PassConstantBufferData);
            PassConstantBufferData passConstantBufferData;
            passConstantBufferData.viewMatrix = lightMatrices[passIndex];
            passConstantBufferData.projectionMatrix = lightProjectionMatrices[passIndex];
            passConstantBufferData.viewProjectionMatrix =
                passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
            memcpy(rendererData.passConstantBufferPtr + passOffset, &passConstantBufferData,
                   sizeof(PassConstantBufferData));
            passIndex++;
        }
    }

    // Color pass data
    PassConstantBufferData passConstantBufferData;
    passConstantBufferData.viewMatrix = scene.getCamera().getViewMatrix();
    passConstantBufferData.projectionMatrix = scene.getCamera().getProjectionMatrix();
    passConstantBufferData.viewProjectionMatrix =
        passConstantBufferData.projectionMatrix * passConstantBufferData.viewMatrix;
    const glm::vec3 &cameraPosition = scene.getCamera().getTransform().getTranslation();
    passConstantBufferData.cameraPosition.x = cameraPosition.x;
    passConstantBufferData.cameraPosition.y = cameraPosition.y;
    passConstantBufferData.cameraPosition.z = cameraPosition.z;

    uint32 colorPassOffset =
        PASS_CONSTANT_BUFFER_SIZE - sizeof(PassConstantBufferData); // Color pass is the last (after the Depth passes).
    memcpy(rendererData.passConstantBufferPtr + colorPassOffset, &passConstantBufferData,
           sizeof(PassConstantBufferData));
}

// TODO: There's no need to call this every frame, like it's being done now.
void Renderer::fillMaterials(const Scene &scene) {
    BZ_PROFILE_FUNCTION();

    rendererData.materialOffsetMap.clear();

    if (scene.hasSkyBox()) {
        fillMaterial(scene.getSkyBox().mesh.getSubMeshIdx(0).material);
    }

    for (const auto &entity : scene.getEntities()) {
        if (entity.overrideMaterial.isValid()) {
            fillMaterial(entity.overrideMaterial);
        }
        else {
            for (const auto &submesh : entity.mesh.getSubmeshes()) {
                fillMaterial(submesh.material);
            }
        }
    }

    uint32 materialCount = static_cast<uint32>(rendererData.materialOffsetMap.size());
    rendererData.stats.materialCount = materialCount;
    BZ_ASSERT_CORE(materialCount <= MAX_MATERIALS_PER_SCENE, "Reached the max number of Materials!");
}

void Renderer::fillMaterial(const Material &material) {
    BZ_PROFILE_FUNCTION();

    BZ_ASSERT_CORE(material.isValid(), "Trying to use an invalid/uninitialized Material!");

    const auto storedMaterialIt = rendererData.materialOffsetMap.find(material);

    // If it's the first time this Material is used on a Scene set the correspondent data.
    if (storedMaterialIt == rendererData.materialOffsetMap.end()) {
        const auto &uvScale = material.getUvScale();
        MaterialConstantBufferData materialConstantBufferData;
        materialConstantBufferData.normalMetallicRoughnessAndAO.x = material.hasNormalTexture() ? 1.0f : 0.0f;
        materialConstantBufferData.normalMetallicRoughnessAndAO.y =
            material.hasMetallicTexture() ? -1.0f : material.getMetallic();
        materialConstantBufferData.normalMetallicRoughnessAndAO.z =
            material.hasRoughnessTexture() ? -1.0f : material.getRoughness();
        materialConstantBufferData.normalMetallicRoughnessAndAO.w = material.hasAOTexture() ? 1.0f : 0.0f;

        materialConstantBufferData.heightAndUvScale.x =
            material.hasHeightTexture() ? material.getParallaxOcclusionScale() : -1.0f;
        materialConstantBufferData.heightAndUvScale.y = uvScale.x;
        materialConstantBufferData.heightAndUvScale.z = uvScale.y;

        uint32 materialOffset =
            static_cast<uint32>(rendererData.materialOffsetMap.size()) * sizeof(EntityConstantBufferData);
        memcpy(rendererData.materialConstantBufferPtr + materialOffset, &materialConstantBufferData,
               sizeof(MaterialConstantBufferData));

        rendererData.materialOffsetMap[material] = materialOffset;
    }
}

void Renderer::fillEntities(const Scene &scene) {
    BZ_PROFILE_FUNCTION();

    uint32 entityIndex = 0;
    for (const auto &entity : scene.getEntities()) {
        EntityConstantBufferData entityConstantBufferData;
        entityConstantBufferData.modelMatrix = entity.transform.getLocalToParentMatrix();
        entityConstantBufferData.normalMatrix = entity.transform.getNormalMatrix();

        uint32 entityOffset = entityIndex * sizeof(EntityConstantBufferData);
        memcpy(rendererData.entityConstantBufferPtr + entityOffset, &entityConstantBufferData,
               sizeof(EntityConstantBufferData));
        entityIndex++;
    }
    BZ_ASSERT_CORE(entityIndex <= MAX_ENTITIES_PER_SCENE, "Reached the max number of Entities!");
}

void Renderer::render(const Ref<RenderPass> &finalRenderPass, const Ref<Framebuffer> &finalFramebuffer,
                      bool waitForImageAvailable, bool signalFrameEnd) {
    BZ_PROFILE_FUNCTION();

    if (rendererData.sceneToRender) {
        fillConstants(*rendererData.sceneToRender);

        shadowPass(*rendererData.sceneToRender);
        colorPass(*rendererData.sceneToRender);
        rendererData.postProcessor.render(finalRenderPass, finalFramebuffer, waitForImageAvailable, signalFrameEnd);

        rendererData.sceneToRender = nullptr;
    }
    else {
        // Dummy well behaved pass if this renderer is active but without stuff to render. Should not happen
        // frequently.
        CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
        commandBuffer.beginRenderPass(finalRenderPass, finalFramebuffer);
        commandBuffer.endRenderPass();
        commandBuffer.endAndSubmit(waitForImageAvailable, signalFrameEnd);
    }
}

void Renderer::onImGuiRender(const FrameTiming &frameTiming) {
    BZ_PROFILE_FUNCTION();

    rendererData.postProcessor.onImGuiRender(frameTiming);

    if (ImGui::Begin("Renderer")) {
        ImGui::Text("Depth Bias:");
        ImGui::DragFloat("ConstantFactor", &rendererData.depthBiasData.x, 0.05f, 0.0f, 10.0f);
        ImGui::DragFloat("Clamp", &rendererData.depthBiasData.y, 0.05f, -10.0f, 10.0f);
        ImGui::DragFloat("SlopeFactor", &rendererData.depthBiasData.z, 0.05f, 0.0f, 100.0f);
        ImGui::Separator();

        rendererData.statsRefreshTimeAcumMs += frameTiming.deltaTime.asMillisecondsUint32();
        if (rendererData.statsRefreshTimeAcumMs >= rendererData.statsRefreshPeriodMs) {
            rendererData.statsRefreshTimeAcumMs = 0;
            rendererData.visibleStats = rendererData.stats;
        }
        ImGui::Text("Stats:");
        ImGui::Text("Vertex Count: %d.", rendererData.visibleStats.vertexCount);
        ImGui::Text("Triangle Count: %d.", rendererData.visibleStats.triangleCount);
        ImGui::Text("Draw Call Count: %d.", rendererData.visibleStats.drawCallCount);
        ImGui::Text("Material Count: %d.", rendererData.visibleStats.materialCount);
        ImGui::Separator();

        ImGui::Text("Refresh period ms");
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.95f);
        ImGui::SliderInt("##slider", reinterpret_cast<int *>(&rendererData.statsRefreshPeriodMs), 0, 1000);
    }
    ImGui::End();
    rendererData.stats = {};
}

void Renderer::computeCascadedShadowMappingSplits(float out[], uint32 splits, float nearPlane, float farPlane) {
    for (uint32 i = 1; i <= splits; ++i) {
        float log = nearPlane * std::pow(farPlane / nearPlane, static_cast<float>(i) / splits);
        float uni = nearPlane + (farPlane - nearPlane) * (static_cast<float>(i) / splits);
        out[i - 1] = -glm::mix(log, uni, 0.4f); // Negated to be on a right-handed camera space
    }
}

const DataLayout &Renderer::getVertexDataLayout() {
    return vertexDataLayout;
}

const DataLayout &Renderer::getIndexDataLayout() {
    return indexDataLayout;
}

DescriptorSet &Renderer::createSceneDescriptorSet() {
    auto &descriptorSet = DescriptorSet::get(rendererData.sceneDescriptorSetLayout);
    descriptorSet.setConstantBuffer(rendererData.constantBuffer, 0, SCENE_CONSTANT_BUFFER_OFFSET,
                                    sizeof(SceneConstantBufferData));
    return descriptorSet;
}

DescriptorSet &Renderer::createMaterialDescriptorSet() {
    auto &descriptorSet = DescriptorSet::get(rendererData.materialDescriptorSetLayout);
    descriptorSet.setConstantBuffer(rendererData.constantBuffer, 0, MATERIAL_CONSTANT_BUFFER_OFFSET,
                                    sizeof(MaterialConstantBufferData));
    return descriptorSet;
}

Ref<Framebuffer> Renderer::createShadowMapFramebuffer() {
    auto shadowMapRef =
        Texture2D::createRenderTarget(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, SHADOW_MAPPING_CASCADE_COUNT, 1,
                                      rendererData.shadowRenderPass->getDepthStencilAttachmentDescription()->format);
    BZ_SET_TEXTURE_DEBUG_NAME(shadowMapRef, "Renderer Shadow Map Texture");
    glm::uvec3 dimsAndLayers(shadowMapRef->getDimensions().x, shadowMapRef->getDimensions().y,
                             shadowMapRef->getLayers());
    return Framebuffer::create(rendererData.shadowRenderPass, { TextureView::create(shadowMapRef) }, dimsAndLayers);
}

const Ref<Sampler> &Renderer::getDefaultSampler() {
    return rendererData.defaultSampler;
}

const Ref<Sampler> &Renderer::getDefaultAnisotropicSampler() {
    return rendererData.defaultAnisotropicSampler;
}

const Ref<Sampler> &Renderer::getShadowSampler() {
    return rendererData.shadowSampler;
}
}
