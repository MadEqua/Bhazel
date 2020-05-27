#include "bzpch.h"

#include "PostProcessor.h"

#include "Core/Application.h"

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

#include <imgui.h>


namespace BZ {

    void Bloom::init() {
        const auto INPUT_DIMENSIONS = postProcessor.getInputTextureDimensions();

        //Start at half dimensions.
        const uint32 W = INPUT_DIMENSIONS.x / 2;
        const uint32 H = INPUT_DIMENSIONS.y / 2;

        tex1 = Texture2D::createRenderTarget(W, H, 1, BLOOM_TEXTURE_MIPS, postProcessor.getInputTextureFormat(), VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        BZ_SET_TEXTURE_DEBUG_NAME(tex1, "Bloom Aux Texture 1");

        tex2 = Texture2D::createRenderTarget(W, H, 1, BLOOM_TEXTURE_MIPS, postProcessor.getInputTextureFormat());
        BZ_SET_TEXTURE_DEBUG_NAME(tex2, "Bloom Aux Texture 2");

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            tex1MipViews[i] = TextureView::create(tex1, 0, 1, i, 1);
            tex2MipViews[i] = TextureView::create(tex2, 0, 1, i, 1);
        }

        intensity = 0.05f;

        initBlurPass();
        initFinalPass();
    }

    void Bloom::destroy() {
        tex1.reset();
        tex2.reset();

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            tex1MipViews[i].reset();
            tex2MipViews[i].reset();
            tex1Framebuffers[i].reset();
            tex2Framebuffers[i].reset();
        }

        blurPipelineLayout.reset();
        blurPipelineState.reset();
        blurRenderPass.reset();
        blurDescriptorSetLayout.reset();

        finalDescriptorSetLayout.reset();
        finalPipelineLayout.reset();
        finalPipelineState.reset();
        finalRenderPass.reset();
        finalFramebuffer.reset();
    }

    void Bloom::render(CommandBuffer &commandBuffer) {
        BZ_CB_INSERT_DEBUG_LABEL(commandBuffer, "Downsample Pass");
        downsamplePass(commandBuffer);

        BZ_CB_INSERT_DEBUG_LABEL(commandBuffer, "Blur Pass");
        blurPass(commandBuffer);

        BZ_CB_INSERT_DEBUG_LABEL(commandBuffer, "Final Pass");
        finalPass(commandBuffer);
    }

    void Bloom::onImGuiRender(const FrameTiming &frameTiming) {
        ImGui::Text("Bloom:");
        ImGui::Checkbox("Enabled", &enabled);
        ImGui::DragFloat("Intensity", &intensity, 0.001f, 0.0f, 0.5f);

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            ImGui::PushID(i);
            char buf[32];
            sprintf_s(buf, "Blur #%d Weight", i);
            ImGui::DragFloat(buf, &blurWeights[i], 0.005f, 0.0f, 2.0f);
            ImGui::PopID();
        }
    }

    void Bloom::downsamplePass(CommandBuffer &commandBuffer) {
        const auto INPUT_DIMENSIONS = postProcessor.getInputTextureDimensions();
        uint32 w = INPUT_DIMENSIONS.x;
        uint32 h = INPUT_DIMENSIONS.y;

        //Populate tex1 with blits from the input image.
        uint32 i;
        for(i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {

            Ref<Texture> src = i == 0 ? postProcessor.getInputTexView()->getTexture() : tex1;
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
            blit.srcOffsets[1] = { (int)w, (int)h, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = srcMip;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { (int)w / 2, (int)h / 2, 1 };
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
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
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

    void Bloom::initBlurPass() {
        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        blurDescriptorSetLayout = DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                                                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        //Create the RenderPass.
        AttachmentDescription colorAttachmentDesc;
        colorAttachmentDesc.format = postProcessor.getInputTextureFormat();
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

        blurPipelineLayout = PipelineLayout::create({ postProcessor.getDescriptorSetLayout(), blurDescriptorSetLayout },
                                                    { { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32) * 2 } });
        //Half the input texture.
        uint32 w = tex1->getWidth();
        uint32 h = tex1->getHeight();

        PipelineStateData blurPipelineStateData;
        blurPipelineStateData.dataLayout = {};
        blurPipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                        { "Bhazel/shaders/bin/BloomBlurAndSumFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        blurPipelineStateData.layout = blurPipelineLayout;
        blurPipelineStateData.blendingState = blendingState;
        blurPipelineStateData.viewports = { {} }; //Dynamic.
        blurPipelineStateData.scissorRects = { { 0, 0, w, h } }; //Scissor with the largest mip dimensions.
        blurPipelineStateData.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT };
        blurPipelineStateData.renderPass = blurRenderPass;
        blurPipelineStateData.subPassIndex = 0;
        blurPipelineState = PipelineState::create(blurPipelineStateData);
        BZ_SET_PIPELINE_DEBUG_NAME(blurPipelineState, "Bloom Blur Pass Pipeline");

        for(uint32 i = 0; i < BLOOM_TEXTURE_MIPS; ++i) {
            blurWeights[i] = (1.0f - (glm::exp(float(i)) - 75.0f)) * 0.02f;

            viewports[i].x = 0.0f;
            viewports[i].y = 0.0f;
            viewports[i].width = static_cast<float>(w);
            viewports[i].height = static_cast<float>(h);
            viewports[i].minDepth = 0.0f;
            viewports[i].maxDepth = 1.0f;

            tex1Framebuffers[i] = Framebuffer::create(blurRenderPass, { tex1MipViews[i] }, { w, h, 1 });
            tex2Framebuffers[i] = Framebuffer::create(blurRenderPass, { tex2MipViews[i] }, { w, h, 1 });
            
            w /= 2;
            h /= 2;

            blurDescriptorSets1[i] = &DescriptorSet::get(blurDescriptorSetLayout);
            blurDescriptorSets1[i]->setCombinedTextureSampler(tex1MipViews[i], postProcessor.getSamplerNearest(), 0);
            //Previous mip (smaller), if exists. If not, send a dummy.
            blurDescriptorSets1[i]->setCombinedTextureSampler(i < BLOOM_TEXTURE_MIPS - 1 ? tex2MipViews[i + 1] : tex1MipViews[i], postProcessor.getSamplerLinear(), 1);

            blurDescriptorSets2[i] = &DescriptorSet::get(blurDescriptorSetLayout);
            blurDescriptorSets2[i]->setCombinedTextureSampler(tex2MipViews[i], postProcessor.getSamplerNearest(), 0);
            blurDescriptorSets2[i]->setCombinedTextureSampler(i < BLOOM_TEXTURE_MIPS - 1 ? tex1MipViews[i + 1] : tex2MipViews[i], postProcessor.getSamplerLinear(), 1);
        }
    }

    void Bloom::blurPass(CommandBuffer &commandBuffer) {
        commandBuffer.bindPipelineState(blurPipelineState);
        commandBuffer.bindDescriptorSet(postProcessor.getDescriptorSet(), blurPipelineLayout, 0, nullptr, 0);

        //One horizontal pass and another vertical for each mip, ping-ponging between tex1 and tex2.
        //The vertical (second) pass will simultaneously do a sum of the current mip with the previous one, gathering all data on the top mip.
        for(uint32 blurPass = 0; blurPass < 2; ++blurPass) {

            for(int mip = BLOOM_TEXTURE_MIPS - 1; mip >= 0; --mip) {
                uint32 push[] = { blurPass, static_cast<uint32>(mip) };
                commandBuffer.bindDescriptorSet(blurPass ? *blurDescriptorSets2[mip] : *blurDescriptorSets1[mip], blurPipelineLayout, 1, nullptr, 0);
                commandBuffer.beginRenderPass(blurRenderPass, blurPass ? tex1Framebuffers[mip] : tex2Framebuffers[mip]);
                commandBuffer.setViewports(0, &viewports[mip], 1);
                commandBuffer.setPushConstants(blurPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, &push, 0, sizeof(push));
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

    void Bloom::initFinalPass() {

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
        colorAttachmentDesc.format = postProcessor.getInputTextureFormat();
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

        finalPipelineLayout = PipelineLayout::create({ postProcessor.getDescriptorSetLayout(), finalDescriptorSetLayout });

        const auto INPUT_DIMENSIONS = postProcessor.getInputTextureDimensions();
        const auto INPUT_DIMENSIONS_F = postProcessor.getInputTextureDimensionsFloat();

        PipelineStateData finalPipelineStateData;
        finalPipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                         { "Bhazel/shaders/bin/BloomFinalFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        finalPipelineStateData.layout = finalPipelineLayout;
        finalPipelineStateData.blendingState = blendingState;
        finalPipelineStateData.viewports = { {0.0f, 0.0f, INPUT_DIMENSIONS_F.x, INPUT_DIMENSIONS_F.y } };
        finalPipelineStateData.scissorRects = { { 0, 0, INPUT_DIMENSIONS.x, INPUT_DIMENSIONS.y} };
        finalPipelineStateData.renderPass = finalRenderPass;
        finalPipelineStateData.subPassIndex = 0;
        finalPipelineState = PipelineState::create(finalPipelineStateData);
        BZ_SET_PIPELINE_DEBUG_NAME(finalPipelineState, "Bloom Final Pass Pipeline");

        finalFramebuffer = Framebuffer::create(finalRenderPass, { postProcessor.getInputTexView() }, { INPUT_DIMENSIONS.x, INPUT_DIMENSIONS.y, 1 });

        finalDescriptorSet = &DescriptorSet::get(finalDescriptorSetLayout);
        finalDescriptorSet->setCombinedTextureSampler(tex1MipViews[0], postProcessor.getSamplerLinear(), 0);
    }

    void Bloom::finalPass(CommandBuffer &commandBuffer) {
        commandBuffer.beginRenderPass(finalRenderPass, finalFramebuffer);
        commandBuffer.bindPipelineState(finalPipelineState);
        commandBuffer.bindDescriptorSet(postProcessor.getDescriptorSet(), finalPipelineLayout, 0, nullptr, 0);
        commandBuffer.bindDescriptorSet(*finalDescriptorSet, finalPipelineLayout, 1, nullptr, 0);
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }


    /*-------------------------------------------------------------------------------------------*/
    void ToneMap::init() {
        const auto INPUT_DIMENSIONS = postProcessor.getInputTextureDimensions();
        const auto INPUT_DIMENSIONS_F = postProcessor.getInputTextureDimensionsFloat();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/ToneMapFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        pipelineStateData.layout = postProcessor.getPipelineLayout();
        pipelineStateData.viewports = { { 0.0f, 0.0f, INPUT_DIMENSIONS_F.x, INPUT_DIMENSIONS_F.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, INPUT_DIMENSIONS.x, INPUT_DIMENSIONS.y } };
        pipelineStateData.blendingState = blendingState;
        pipelineStateData.renderPass = Application::get().getGraphicsContext().getSwapchainRenderPass();
        pipelineStateData.subPassIndex = 0;
        pipelineState = PipelineState::create(pipelineStateData);
        BZ_SET_PIPELINE_DEBUG_NAME(pipelineState, "ToneMap Pipeline");
    }

    void ToneMap::destroy() {
        pipelineState.reset();
    }

     void ToneMap::render(CommandBuffer &commandBuffer, const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer) {
        commandBuffer.beginRenderPass(renderPass, framebuffer);
        commandBuffer.bindPipelineState(pipelineState);
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }

    void ToneMap::onImGuiRender(const FrameTiming &frameTiming) {
        ImGui::Text("Tone Mapping:");
        ImGui::Text("Check camera exposure.");
    }


    /*-------------------------------------------------------------------------------------------*/
    void FXAA::init(const Ref<TextureView> &inTexture) {
        this->inTexture = inTexture;

        auto descriptorSetLayout = DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });
        pipelineLayout = PipelineLayout::create({ descriptorSetLayout });
        descriptorSet = &DescriptorSet::get(descriptorSetLayout);
        descriptorSet->setCombinedTextureSampler(inTexture, postProcessor.getSamplerLinear(), 0);

        const auto INPUT_DIMENSIONS = inTexture->getTexture()->getDimensions();
        const auto INPUT_DIMENSIONS_F = inTexture->getTexture()->getDimensionsFloat();

        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        PipelineStateData pipelineStateData;
        pipelineStateData.shader = Shader::create({ { "Bhazel/shaders/bin/FullScreenVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                                    { "Bhazel/shaders/bin/FxaaFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
        pipelineStateData.layout = pipelineLayout;
        pipelineStateData.viewports = { { 0.0f, 0.0f, INPUT_DIMENSIONS_F.x, INPUT_DIMENSIONS_F.y, 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, INPUT_DIMENSIONS.x, INPUT_DIMENSIONS.y } };
        pipelineStateData.blendingState = blendingState;
        pipelineStateData.renderPass = Application::get().getGraphicsContext().getSwapchainRenderPass();
        pipelineStateData.subPassIndex = 0;
        pipelineState = PipelineState::create(pipelineStateData);
        BZ_SET_PIPELINE_DEBUG_NAME(pipelineState, "FXAA Pipeline");
    }

    void FXAA::destroy() {
        inTexture.reset();
        pipelineState.reset();
        pipelineLayout.reset();
    }

    void FXAA::render(CommandBuffer &commandBuffer, const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer) {
        commandBuffer.beginRenderPass(renderPass, framebuffer);
        commandBuffer.bindPipelineState(pipelineState);
        commandBuffer.bindDescriptorSet(*descriptorSet, pipelineLayout, 0, nullptr, 0);
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }

    void FXAA::onImGuiRender(const FrameTiming &frameTiming) {
        ImGui::Text("FXAA:");
        ImGui::Checkbox("Enabled", &enabled);
    }


    /*-------------------------------------------------------------------------------------------*/
    PostProcessor::PostProcessor() :
        bloom(*this), 
        toneMap(*this),
        fxaa(*this) {
    }

    void PostProcessor::init(const Ref<TextureView> &colorTexView, const Ref<Buffer> &constantBuffer, uint32 bufferOffset) {
        inputTexView = colorTexView;

        const Ref<RenderPass> &swapchainRenderPass = Application::get().getGraphicsContext().getSwapchainRenderPass();
        const Ref<Framebuffer> &swapchainFramebuffer = Application::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer();
        const glm::uvec3 SWAPCHAIN_DIMS = swapchainFramebuffer->getDimensionsAndLayers();
        auto swapchainReplicaTex = Texture2D::createRenderTarget(SWAPCHAIN_DIMS.x, SWAPCHAIN_DIMS.y, 1, 1,
            swapchainFramebuffer->getColorAttachmentTextureView(0)->getTextureFormat());
        BZ_SET_TEXTURE_DEBUG_NAME(swapchainReplicaTex, "PostProcessor SwapchainReplica Texture");
        auto swapchainReplicaTexView = TextureView::create(swapchainReplicaTex);


        //Create the RenderPass.
        AttachmentDescription colorAttachmentDesc;
        colorAttachmentDesc.format = swapchainReplicaTex->getFormat();
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };

        swapchainReplicaRenderPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });
        swapchainReplicaFramebuffer = Framebuffer::create(swapchainReplicaRenderPass, { swapchainReplicaTexView }, SWAPCHAIN_DIMS);

        Sampler::Builder samplerBuilderN;
        samplerBuilderN.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        samplerBuilderN.setMinFilterMode(VK_FILTER_NEAREST);
        samplerBuilderN.setMagFilterMode(VK_FILTER_NEAREST);
        samplerNearest = samplerBuilderN.build();

        Sampler::Builder samplerBuilderL;
        samplerBuilderL.setAddressModeAll(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        samplerBuilderL.setMinFilterMode(VK_FILTER_LINEAR);
        samplerBuilderL.setMagFilterMode(VK_FILTER_LINEAR);
        samplerLinear = samplerBuilderL.build();

        descriptorSetLayout =
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        pipelineLayout = PipelineLayout::create({ descriptorSetLayout });

        descriptorSet = &DescriptorSet::get(descriptorSetLayout);
        descriptorSet->setCombinedTextureSampler(colorTexView, samplerNearest, 0);
        descriptorSet->setConstantBuffer(constantBuffer, 1, bufferOffset, sizeof(PostProcessConstantBufferData));

        bloom.init();
        toneMap.init();
        fxaa.init(swapchainReplicaTexView);
    }

    void PostProcessor::destroy() {
        inputTexView.reset();

        swapchainReplicaRenderPass.reset();
        swapchainReplicaFramebuffer.reset();

        samplerNearest.reset();
        samplerLinear.reset();
        descriptorSetLayout.reset();
        pipelineLayout.reset();

        toneMap.destroy();
        fxaa.destroy();
        bloom.destroy();
    }

    void PostProcessor::fillData(const BufferPtr &ptr, const Scene &scene) {
        PostProcessConstantBufferData data;
        data.cameraExposureAndBloomIntensity.x = scene.getCamera().getExposure();
        data.cameraExposureAndBloomIntensity.y = bloom.getIntensity();

        for(uint32 i = 0; i < Bloom::BLOOM_TEXTURE_MIPS; ++i) {
            data.bloomBlurWeights[i].x = bloom.getBlurWeights()[i];
        }
 
        memcpy(ptr, &data, sizeof(PostProcessConstantBufferData));
    }

    void PostProcessor::render(const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer, bool waitForImageAvailable, bool signalFrameEnd) {
        CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);

        BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "PostProcessing");
        commandBuffer.bindDescriptorSet(*descriptorSet, pipelineLayout, 0, nullptr, 0);

        if(bloom.isEnabled()) {
            BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "Bloom");
            addBarrier(commandBuffer);
            bloom.render(commandBuffer);
            BZ_CB_END_DEBUG_LABEL(commandBuffer);
        }

        BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "ToneMapping");
        addBarrier(commandBuffer);
        toneMap.render(commandBuffer, 
            fxaa.isEnabled() ? swapchainReplicaRenderPass : swapchainRenderPass,
            fxaa.isEnabled() ? swapchainReplicaFramebuffer : swapchainFramebuffer);
        BZ_CB_END_DEBUG_LABEL(commandBuffer);

        if(fxaa.isEnabled()) {
            BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "FXAA");
            addBarrier(commandBuffer);
            fxaa.render(commandBuffer, swapchainRenderPass, swapchainFramebuffer);
            BZ_CB_END_DEBUG_LABEL(commandBuffer);
        }

        BZ_CB_END_DEBUG_LABEL(commandBuffer);
        commandBuffer.endAndSubmit(waitForImageAvailable, signalFrameEnd);
    }

    void PostProcessor::onImGuiRender(const FrameTiming &frameTiming) {
        if(ImGui::Begin("Post-Processing")) {
            ImGui::PushID(1);
            bloom.onImGuiRender(frameTiming);
            ImGui::PopID();

            ImGui::Separator();

            ImGui::PushID(2);
            toneMap.onImGuiRender(frameTiming);
            ImGui::PopID();

            ImGui::Separator();

            ImGui::PushID(3);
            fxaa.onImGuiRender(frameTiming);
            ImGui::PopID();
        }
        ImGui::End();
    }

    void PostProcessor::addBarrier(CommandBuffer &commandBuffer) {
        commandBuffer.pipelineBarrierMemory(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    }
}