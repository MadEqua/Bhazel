#pragma once

#include "Graphics/GraphicsContext.h"
#include "Graphics/PipelineState.h"
#include "Graphics/Texture.h"


namespace BZ {

class DescriptorSetLayout;
class DescriptorSet;
class PipelineLayout;
class RenderPass;
class Framebuffer;
class CommandBuffer;
class Scene;
class Buffer;
class BufferPtr;
struct FrameTiming;


/*-------------------------------------------------------------------------------------------*/
class PostProcessor;

class PostProcessEffect {
  public:
    BZ_NON_COPYABLE(PostProcessEffect);

    explicit PostProcessEffect(const PostProcessor &postProcessor) : postProcessor(postProcessor) {}
    bool isEnabled() const { return enabled; }

  protected:
    const PostProcessor &postProcessor;
    bool enabled = true;
};


/*-------------------------------------------------------------------------------------------*/
class Bloom : public PostProcessEffect {
  public:
    explicit Bloom(const PostProcessor &postProcessor) : PostProcessEffect(postProcessor) {}

    void init();
    void destroy();

    void render(CommandBuffer &commandBuffer);
    void onImGuiRender(const FrameTiming &frameTiming);

    // Ammont of textures to apply the blur filter.
    static constexpr uint32 BLOOM_TEXTURE_MIPS = 5u;

    float getIntensity() const { return intensity; }
    const float *getBlurWeights() const { return blurWeights; }

  private:
    float intensity;
    float blurWeights[BLOOM_TEXTURE_MIPS];

    void downsamplePass(CommandBuffer &commandBuffer);

    void initBlurPass();
    void blurPass(CommandBuffer &commandBuffer);

    void initFinalPass();
    void finalPass(CommandBuffer &commandBuffer);

    // Aux textures, mipmapped.
    Ref<Texture2D> tex1;
    Ref<Texture2D> tex2;

    // Views into each mipmap of the textures.
    Ref<TextureView> tex1MipViews[BLOOM_TEXTURE_MIPS];
    Ref<TextureView> tex2MipViews[BLOOM_TEXTURE_MIPS];

    // Corresponding to each of the above views.
    Ref<Framebuffer> tex1Framebuffers[BLOOM_TEXTURE_MIPS];
    Ref<Framebuffer> tex2Framebuffers[BLOOM_TEXTURE_MIPS];

    // Viewports to render into each of the mips.
    VkViewport viewports[BLOOM_TEXTURE_MIPS];

    Ref<PipelineLayout> blurPipelineLayout;
    Ref<PipelineState> blurPipelineState;
    Ref<RenderPass> blurRenderPass;

    Ref<DescriptorSetLayout> blurDescriptorSetLayout;
    DescriptorSet *blurDescriptorSets1[BLOOM_TEXTURE_MIPS];
    DescriptorSet *blurDescriptorSets2[BLOOM_TEXTURE_MIPS];

    Ref<PipelineLayout> finalPipelineLayout;
    Ref<PipelineState> finalPipelineState;
    Ref<RenderPass> finalRenderPass;
    Ref<Framebuffer> finalFramebuffer;

    Ref<DescriptorSetLayout> finalDescriptorSetLayout;
    DescriptorSet *finalDescriptorSet;
};


/*-------------------------------------------------------------------------------------------*/
class ToneMap : public PostProcessEffect {
  public:
    explicit ToneMap(const PostProcessor &postProcessor) : PostProcessEffect(postProcessor) {}

    void init();
    void destroy();

    void render(CommandBuffer &commandBuffer, const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer);
    void onImGuiRender(const FrameTiming &frameTiming);

  private:
    Ref<PipelineState> pipelineState;
};


/*-------------------------------------------------------------------------------------------*/
class FXAA : public PostProcessEffect {
  public:
    explicit FXAA(const PostProcessor &postProcessor) : PostProcessEffect(postProcessor) {}

    void init(const Ref<TextureView> &inTexture);
    void destroy();

    void render(CommandBuffer &commandBuffer, const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer);
    void onImGuiRender(const FrameTiming &frameTiming);

  private:
    Ref<TextureView> inTexture;
    Ref<PipelineLayout> pipelineLayout;
    Ref<PipelineState> pipelineState;
    DescriptorSet *descriptorSet;
};


/*-------------------------------------------------------------------------------------------*/
struct alignas(GraphicsContext::MIN_UNIFORM_BUFFER_OFFSET_ALIGN) PostProcessConstantBufferData {
    glm::vec4 cameraExposureAndBloomIntensity;

    // glm::vec4 is a float! Using only the first vector component, respecting std140 aligment.
    glm::vec4 bloomBlurWeights[Bloom::BLOOM_TEXTURE_MIPS];
};


class PostProcessor {
  public:
    BZ_NON_COPYABLE(PostProcessor);

    PostProcessor();

    void init(const Ref<TextureView> &colorTexView, const Ref<Buffer> &constantBuffer, uint32 bufferOffset);
    void destroy();

    void fillData(const BufferPtr &ptr, const Scene &scene);
    void render(const Ref<RenderPass> &finalRenderPass, const Ref<Framebuffer> &finalFramebuffer,
                bool waitForImageAvailable, bool signalFrameEnd);
    void onImGuiRender(const FrameTiming &frameTiming);

    const Ref<TextureView> &getInputTexView() const { return inputTexView; }
    const Ref<Sampler> &getSamplerNearest() const { return samplerNearest; }
    const Ref<Sampler> &getSamplerLinear() const { return samplerLinear; }
    const Ref<PipelineLayout> &getPipelineLayout() const { return pipelineLayout; }
    const Ref<DescriptorSetLayout> &getDescriptorSetLayout() const { return descriptorSetLayout; }
    const DescriptorSet &getDescriptorSet() const { return *descriptorSet; }

    glm::uvec3 getInputTextureDimensions() const { return inputTexView->getTexture()->getDimensions(); }
    glm::vec3 getInputTextureDimensionsFloat() const { return inputTexView->getTexture()->getDimensionsFloat(); }
    TextureFormat getInputTextureFormat() const { return inputTexView->getTextureFormat(); }

  private:
    void addBarrier(CommandBuffer &commandBuffer);

    Ref<TextureView> inputTexView;
    Ref<Sampler> samplerNearest;
    Ref<Sampler> samplerLinear;

    Ref<RenderPass> swapchainReplicaRenderPass;
    Ref<Framebuffer> swapchainReplicaFramebuffer;

    // Descriptors to the input texture and the constant buffer.
    Ref<DescriptorSetLayout> descriptorSetLayout;
    Ref<PipelineLayout> pipelineLayout;
    DescriptorSet *descriptorSet;

    Bloom bloom;
    ToneMap toneMap;
    FXAA fxaa;
};
}