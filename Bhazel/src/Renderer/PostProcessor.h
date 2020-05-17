#pragma once

#include "Graphics/PipelineState.h"


namespace BZ {

    class DescriptorSetLayout;
    class DescriptorSet;
    class RenderPass;
    class Texture2D;
    class TextureView;
    class Framebuffer;
    class Sampler;
    class CommandBuffer;
    class Scene;
    class Buffer;
    class BufferPtr;


    class Bloom {
    public:
        void init(const Ref<DescriptorSetLayout> &descriptorSetLayout, const Ref<TextureView> &colorTexView, const Ref<Sampler> &sampler);
        void destroy();

        void render(CommandBuffer &commandBuffer, const DescriptorSet &descriptorSet, const Ref<TextureView> &colorTexView);

    private:

        //Ammont of textures to apply the blur filter.
        static const uint32 BLOOM_TEXTURE_MIPS = 5u;

        void downsamplePass(CommandBuffer &commandBuffer, const Ref<TextureView> &colorTexView);

        void initBlurPass(const Ref<DescriptorSetLayout> &descriptorSetLayout, const Ref<Sampler> &sampler);
        void blurPass(CommandBuffer &commandBuffer);

        void initFinalPass(const Ref<TextureView> &colorTexView, const Ref<Sampler> &sampler);
        void finalPass(CommandBuffer &commandBuffer);

        Ref<Texture2D> tex1;
        Ref<Texture2D> tex2;

        //Views into the whole texture.
        //Ref<TextureView> tex1View;
        //Ref<TextureView> tex2View;

        //Views into each mipmap of the textures.
        Ref<TextureView> tex1MipViews[BLOOM_TEXTURE_MIPS];
        Ref<TextureView> tex2MipViews[BLOOM_TEXTURE_MIPS];

        //Corresponding to each of the above views.
        Ref<Framebuffer> tex1Framebuffers[BLOOM_TEXTURE_MIPS];
        Ref<Framebuffer> tex2Framebuffers[BLOOM_TEXTURE_MIPS];

        VkViewport viewports[BLOOM_TEXTURE_MIPS];

        Ref<PipelineState> blurPipelineState;
        Ref<RenderPass> blurRenderPass;

        Ref<DescriptorSetLayout> blurDescriptorSetLayout;
        DescriptorSet *blurDescriptorSets1[BLOOM_TEXTURE_MIPS];
        DescriptorSet *blurDescriptorSets2[BLOOM_TEXTURE_MIPS];

        Ref<PipelineState> finalPipelineState;
        Ref<RenderPass> finalRenderPass;
        Ref<Framebuffer> finalFramebuffer;

        Ref<DescriptorSetLayout> finalDescriptorSetLayout;
        DescriptorSet *finalDescriptorSet;
     };


    /*-------------------------------------------------------------------------------------------*/
    class ToneMap {
    public:
        void init(const Ref<DescriptorSetLayout> &descriptorSetLayout);
        void destroy();

        void render(CommandBuffer &commandBuffer, const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer,
                    const DescriptorSet &descriptorSet);

    private:
        Ref<PipelineState> pipelineState;
    };


    /*-------------------------------------------------------------------------------------------*/
    struct alignas(MIN_UNIFORM_BUFFER_OFFSET_ALIGN) PostProcessConstantBufferData {
        glm::vec4 cameraExposure;
    };


    class PostProcessor {
    public:
        void init(const Ref<TextureView> &colorTexView, const Ref<Buffer> &constantBuffer, uint32 bufferOffset);
        void destroy();

        void fillData(const BufferPtr &ptr, const Scene &scene);
        void render(CommandBuffer &commandBuffer, const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer);

    private:
        Ref<TextureView> colorTexView;

        Ref<Sampler> sampler;
        Ref<DescriptorSetLayout> descriptorSetLayout;
        DescriptorSet *descriptorSet;
        
        Bloom bloom;
        ToneMap toneMap;
    };
}