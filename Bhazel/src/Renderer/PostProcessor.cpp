#include "bzpch.h"

#include "PostProcessor.h"

#include "Core/Application.h"
#include "Core/Window.h"

#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/CommandBuffer.h"

#include "Renderer/Scene.h"
#include "Renderer/Camera.h"


namespace BZ {


    void Bloom::init(const Ref<DescriptorSetLayout> &descriptorSetLayout, const Ref<TextureView> &colorTexView, const Ref<Sampler> &sampler) {
        const auto HALF_WINDOW_DIMS_FLOAT = Application::get().getWindow().getDimensionsFloat() * 0.5f;
        uint32 w = static_cast<uint32>(HALF_WINDOW_DIMS_FLOAT.x);
        uint32 h = static_cast<uint32>(HALF_WINDOW_DIMS_FLOAT.y);

        tex1 = Texture2D::createRenderTarget(w, h, 1, BLOOM_TEXTURE_MIPS, VK_FORMAT_R16G16B16A16_SFLOAT, true);
        tex2 = Texture2D::createRenderTarget(w, h, 1, BLOOM_TEXTURE_MIPS, VK_FORMAT_R16G16B16A16_SFLOAT, true);

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            tex1MipViews[i] = TextureView::create(tex1, i, 1);
            tex2MipViews[i] = TextureView::create(tex2, i, 1);

            viewports[i].x = 0.0f;
            viewports[i].y = 0.0f;
            viewports[i].width = static_cast<float>(w);
            viewports[i].height = static_cast<float>(h);
            viewports[i].minDepth = 0.0f;
            viewports[i].maxDepth = 1.0f;

            //Inverting the space (+y -> up)
            //TODO: aux function
            viewports[i].y = viewports[i].height;
            viewports[i].height = -viewports[i].height;

            w /= 2;
            h /= 2;
        }

        initBlurPass(descriptorSetLayout, sampler);
        initFinalPass(colorTexView, sampler);
    }

    void Bloom::destroy() {
        tex1.reset();
        tex2.reset();

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            tex1MipViews[i].reset();
            tex2MipViews[i].reset();
            tex1Framebuffers[i].reset();
            tex1Framebuffers[i].reset();
        }

        blurPipelineState.reset();
        blurRenderPass.reset();
        blurDescriptorSetLayout.reset();

        finalDescriptorSetLayout.reset();
        finalPipelineState.reset();
        finalRenderPass.reset();
        finalFramebuffer.reset();
    }

    void Bloom::render(CommandBuffer &commandBuffer, const DescriptorSet &descriptorSet, const Ref<TextureView> &colorTexView) {
        downsamplePass(commandBuffer, colorTexView);
        blurPass(commandBuffer);
        finalPass(commandBuffer);
    }

    void Bloom::downsamplePass(CommandBuffer &commandBuffer, const Ref<TextureView> &colorTexView) {
        int w = colorTexView->getTexture()->getWidth();
        int h = colorTexView->getTexture()->getHeight();

        //Wait for color pass to finish. This is only needed while the downsample pass is the first on post-processing.
        //Otherwise this would be on a RenderPass dependency.
        commandBuffer.pipelineBarrierMemory(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);

        //Populate tex1 with blits from the input image.
        uint32 i;
        for(i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {

            Ref<Texture> src = i == 0 ? colorTexView->getTexture() : tex1;
            uint32 srcMip = i == 0 ? 0 : i - 1;
            uint32 dstMip = i;
            VkImageLayout srcLayout = i == 0 ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            //Layout transition to SRC_OPTIMAL.
            commandBuffer.pipelineBarrierTexture(src,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                srcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                srcMip, 1);


            //Layout transition to DST_OPTIMAL.
            commandBuffer.pipelineBarrierTexture(tex1,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                dstMip, 1);

            VkImageBlit blit = {};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { w, h, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = srcMip;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { w / 2, h / 2, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = dstMip;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            commandBuffer.blitTexture(src, tex1,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &blit, 1, VK_FILTER_LINEAR);

            //Layout transition from SRC_OPTIMAL to SHADER_READ_ONLY_OPTIMAL.
            commandBuffer.pipelineBarrierTexture(src,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                srcMip, 1);

            w /= 2;
            h /= 2;
        }

        //Layout transition last mipmap from DST_OPTIMAL to SHADER_READ_ONLY_OPTIMAL.
        commandBuffer.pipelineBarrierTexture(tex1,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            i - 1, 1);
    }

    void Bloom::initBlurPass(const Ref<DescriptorSetLayout> &descriptorSetLayout, const Ref<Sampler> &sampler) {
        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        blurDescriptorSetLayout = DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                                                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        //Create the RenderPass.
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

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };

        //Wait on downsample pass.
        SubPassDependency dependencyBefore;
        dependencyBefore.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
        dependencyBefore.dstSubPassIndex = 0;
        dependencyBefore.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dependencyBefore.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencyBefore.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dependencyBefore.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencyBefore.dependencyFlags = 0;

        blurRenderPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc}, { dependencyBefore });

        uint32 w = tex1->getWidth();
        uint32 h = tex2->getHeight();

        PipelineStateData blurPipelineStateData;
        blurPipelineStateData.dataLayout = {};
        blurPipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenQuadVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                        { "Bhazel/shaders/bin/GaussianBlurFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        blurPipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        blurPipelineStateData.descriptorSetLayouts = { blurDescriptorSetLayout };
        blurPipelineStateData.pushConstants = { { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32) * 2 } };
        blurPipelineStateData.blendingState = blendingState;
        blurPipelineStateData.viewports = { {} };
        blurPipelineStateData.scissorRects = { { 0, 0, w, h } };
        blurPipelineStateData.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT };
        blurPipelineStateData.renderPass = blurRenderPass;
        blurPipelineStateData.subPassIndex = 0;
        blurPipelineState = PipelineState::create(blurPipelineStateData);

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            tex1Framebuffers[i] = Framebuffer::create(blurRenderPass, { tex1MipViews[i] }, { w, h, 1 });
            tex2Framebuffers[i] = Framebuffer::create(blurRenderPass, { tex2MipViews[i] }, { w, h, 1 });
            w /= 2;
            h /= 2;

            blurDescriptorSets1[i] = &DescriptorSet::get(blurDescriptorSetLayout);
            blurDescriptorSets1[i]->setCombinedTextureSampler(tex1MipViews[i], sampler, 0);
            //Next mip, if exists. If not, send a dummy.
            blurDescriptorSets1[i]->setCombinedTextureSampler(i < BLOOM_TEXTURE_MIPS - 1 ? tex2MipViews[i + 1] : tex1MipViews[i], sampler, 1);

            blurDescriptorSets2[i] = &DescriptorSet::get(blurDescriptorSetLayout);
            blurDescriptorSets2[i]->setCombinedTextureSampler(tex2MipViews[i], sampler, 0);
            blurDescriptorSets2[i]->setCombinedTextureSampler(i < BLOOM_TEXTURE_MIPS - 1 ? tex1MipViews[i + 1] : tex2MipViews[i], sampler, 1);
        }
    }

    void Bloom::blurPass(CommandBuffer &commandBuffer) {
        commandBuffer.bindPipelineState(blurPipelineState);

        //One horizontal pass and another vertical for each mip.
        //The vertical (second) pass will simultaneously do a sum of the current mip with the previous one, gathering all data on the top mip.
        for(uint32 blurPass = 0; blurPass < 2; ++blurPass) {

            for(int mip = BLOOM_TEXTURE_MIPS - 1; mip >= 0; --mip) {
                uint32 push[] = { blurPass, mip < BLOOM_TEXTURE_MIPS - 1 };

                commandBuffer.bindDescriptorSet(blurPass ? *blurDescriptorSets2[mip] : *blurDescriptorSets1[mip], blurPipelineState, 0, nullptr, 0);
                commandBuffer.beginRenderPass(blurRenderPass, blurPass ? tex1Framebuffers[mip] : tex2Framebuffers[mip]);
                commandBuffer.setViewports(0, &viewports[mip], 1);
                commandBuffer.setPushConstants(blurPipelineState, VK_SHADER_STAGE_FRAGMENT_BIT, &push, sizeof(push), 0);
                commandBuffer.draw(3, 1, 0, 0);
                commandBuffer.endRenderPass();

                //Vertical pass mip must wait for the previous mip to be done to sum the results into itself.
                //Horizontal pass mips may run concurrently.
                if(blurPass == 1) {
                    commandBuffer.pipelineBarrierMemory(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                }
            }

             //Vertical pass must wait for horizontal pass.
            if(blurPass == 0) {
                commandBuffer.pipelineBarrierMemory(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
            }
        }
    }

    void Bloom::initFinalPass(const Ref<TextureView> &colorTexView, const Ref<Sampler> &sampler) {

        BlendingStateAttachment blendingStateAttachment;
        blendingStateAttachment.enableBlending = true;
        blendingStateAttachment.srcColorBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendingStateAttachment.dstColorBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendingStateAttachment.colorBlendingOperation = VK_BLEND_OP_ADD;
        //blendingStateAttachment.srcAlphaBlendingFactor = VK_BLEND_FACTOR_ZERO;
        //blendingStateAttachment.dstAlphaBlendingFactor = VK_BLEND_FACTOR_ONE;
        //blendingStateAttachment.alphaBlendingOperation = VK_BLEND_OP_ADD;
        blendingStateAttachment.writeMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

        BlendingState blendingState;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        finalDescriptorSetLayout = DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        //Create the RenderPass.
        AttachmentDescription colorAttachmentDesc;
        colorAttachmentDesc.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };

        //Wait on blur pass.
        SubPassDependency dependencyBefore;
        dependencyBefore.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
        dependencyBefore.dstSubPassIndex = 0;
        dependencyBefore.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencyBefore.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencyBefore.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencyBefore.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencyBefore.dependencyFlags = 0;

        finalRenderPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependencyBefore });

        uint32 w = colorTexView->getTexture()->getWidth();
        uint32 h = colorTexView->getTexture()->getHeight();

        PipelineStateData finalPipelineStateData;
        finalPipelineStateData.dataLayout = {};
        finalPipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenQuadVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                         { "Bhazel/shaders/bin/BloomFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        finalPipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        finalPipelineStateData.descriptorSetLayouts = { finalDescriptorSetLayout };
        finalPipelineStateData.blendingState = blendingState;
        finalPipelineStateData.viewports = { {0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h) } };
        finalPipelineStateData.scissorRects = { { 0, 0, w, h } };
        finalPipelineStateData.renderPass = finalRenderPass;
        finalPipelineStateData.subPassIndex = 0;
        finalPipelineState = PipelineState::create(finalPipelineStateData);

        finalFramebuffer = Framebuffer::create(finalRenderPass, { colorTexView }, { w, h, 1 });

        finalDescriptorSet = &DescriptorSet::get(finalDescriptorSetLayout);
        finalDescriptorSet->setCombinedTextureSampler(tex1MipViews[0], sampler, 0);
    }

    void Bloom::finalPass(CommandBuffer &commandBuffer) {
        commandBuffer.beginRenderPass(finalRenderPass, finalFramebuffer);
        commandBuffer.bindPipelineState(finalPipelineState);
        commandBuffer.bindDescriptorSet(*finalDescriptorSet, finalPipelineState, 0, nullptr, 0);
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }


    /*-------------------------------------------------------------------------------------------*/
    void ToneMap::init(const Ref<DescriptorSetLayout> &descriptorSetLayout) {
        const auto WINDOW_DIMS_INT = Application::get().getWindow().getDimensions();
        const auto WINDOW_DIMS_FLOAT = Application::get().getWindow().getDimensionsFloat();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = {};
        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenQuadVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/ToneMapFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        pipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };
        pipelineStateData.viewports = { { 0.0f, 0.0f, WINDOW_DIMS_FLOAT.x, WINDOW_DIMS_FLOAT.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(WINDOW_DIMS_INT.x), static_cast<uint32>(WINDOW_DIMS_INT.y) } };
        pipelineStateData.blendingState = blendingState;

        //Tone mapping will write to a Swapchain framebuffer.
        //TODO: sync?
        pipelineStateData.renderPass = Application::get().getGraphicsContext().getSwapchainDefaultRenderPass();
        pipelineStateData.subPassIndex = 0;
        pipelineState = PipelineState::create(pipelineStateData);
    }

    void ToneMap::destroy() {
        pipelineState.reset();
    }

    //TODO: the descriptorSet argument could removed. Bind it on PostProcessor only once, if the PipelineLayout was separated from the PipelineState.
    //bindDescriptorSet() would only take the PipelineLayout as argument and that should be the same for all PostProcessing passes.
    void ToneMap::render(CommandBuffer &commandBuffer, const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer,
                         const DescriptorSet &descriptorSet) {
        commandBuffer.beginRenderPass(swapchainRenderPass, swapchainFramebuffer);
        commandBuffer.bindPipelineState(pipelineState);
        commandBuffer.bindDescriptorSet(descriptorSet, pipelineState, 0, nullptr, 0);
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }


    /*-------------------------------------------------------------------------------------------*/
    void PostProcessor::init(const Ref<TextureView> &colorTexView, const Ref<Buffer> &constantBuffer, uint32 bufferOffset) {
        this->colorTexView = colorTexView;

        Sampler::Builder samplerBuilder;
        sampler = samplerBuilder.build();

        descriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        descriptorSet = &DescriptorSet::get(descriptorSetLayout);
        descriptorSet->setCombinedTextureSampler(colorTexView, sampler, 0);
        descriptorSet->setConstantBuffer(constantBuffer, 1, bufferOffset, sizeof(PostProcessConstantBufferData));

        bloom.init(descriptorSetLayout, colorTexView, sampler);
        toneMap.init(descriptorSetLayout);
    }

    void PostProcessor::destroy() {
        colorTexView.reset();

        sampler.reset();
        descriptorSetLayout.reset();

        toneMap.destroy();
        bloom.destroy();
    }

    void PostProcessor::fillData(const BufferPtr &ptr, const Scene &scene) {
        PostProcessConstantBufferData data;
        data.cameraExposure.x = scene.getCamera().getExposure();
        memcpy(ptr, &data, sizeof(PostProcessConstantBufferData));
    }

    void PostProcessor::render(CommandBuffer &commandBuffer, const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer) {
        bloom.render(commandBuffer, *descriptorSet, colorTexView);
        toneMap.render(commandBuffer, swapchainRenderPass, swapchainFramebuffer, *descriptorSet);
    }
}