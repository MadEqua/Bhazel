#include "bzpch.h"

#include "RendererCoordinator.h"

#include "Graphics/Framebuffer.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/RenderPass.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RendererImGui.h"

#include "Core/Engine.h"
#include "Core/KeyCodes.h"
#include "Events/KeyEvent.h"


namespace BZ {

void RendererCoordinator::init(bool enable2dRenderer, bool enable3dRenderer, bool enableImGuiRenderer) {
    if (enable2dRenderer)
        Renderer2D::init();
    if (enable3dRenderer)
        Renderer::init();
    if (enableImGuiRenderer)
        RendererImGui::init();

    internalInit();

    if (enable2dRenderer) {
        if (enable3dRenderer) {
            if (enableImGuiRenderer) {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    Renderer::render(firstPass, swapchainFramebuffer, true, false);
                    Renderer2D::render(scene, secondPass, swapchainFramebuffer, false, false);
                    RendererImGui::render(lastPass, swapchainFramebuffer, false, true);
                };
            }
            else {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    Renderer::render(firstPass, swapchainFramebuffer, true, false);
                    Renderer2D::render(scene, lastPass, swapchainFramebuffer, false, true);
                };
            }
        }
        else {
            if (enableImGuiRenderer) {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    Renderer2D::render(scene, firstPass, swapchainFramebuffer, true, false);
                    RendererImGui::render(lastPass, swapchainFramebuffer, false, true);
                };
            }
            else {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    Renderer2D::render(scene, firstAndLastPass, swapchainFramebuffer, true, true);
                };
            }
        }
    }
    else {
        if (enable3dRenderer) {
            if (enableImGuiRenderer) {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    Renderer::render(firstPass, swapchainFramebuffer, true, false);
                    RendererImGui::render(lastPass, swapchainFramebuffer, false, true);
                };
            }
            else {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    Renderer::render(firstAndLastPass, swapchainFramebuffer, true, true);
                };
            }
        }
        else {
            if (enableImGuiRenderer) {
                renderFunction = [this](const Scene &scene) {
                    const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
                    RendererImGui::render(firstAndLastPass, swapchainFramebuffer, true, true);
                };
            }
            else {
                BZ_ASSERT_ALWAYS_CORE("At least one Renderer must be active!");
            }
        }
    }
}

void RendererCoordinator::initEditorMode() {
    Renderer2D::init();
    Renderer::init();
    RendererImGui::init();

    internalInit();

    const Ref<RenderPass> &swapchainRenderPass = BZ_GRAPHICS_CTX.getSwapchainRenderPass();
    const Ref<Framebuffer> &swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
    const glm::uvec3 SWAPCHAIN_DIMS = swapchainFramebuffer->getDimensionsAndLayers();

    for (uint32 i = 0; i < GraphicsContext::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto swapchainReplicaTex =
            Texture2D::createRenderTarget(SWAPCHAIN_DIMS.x, SWAPCHAIN_DIMS.y, 1, 1,
                                          swapchainFramebuffer->getColorAttachmentTextureView(0)->getTextureFormat());
        BZ_SET_TEXTURE_DEBUG_NAME(swapchainReplicaTex, "RendererCoordinator SwapchainReplica Texture");
        auto swapchainReplicaTexView = TextureView::create(swapchainReplicaTex);

        offscreenFramebuffers[i] =
            Framebuffer::create(swapchainRenderPass, { swapchainReplicaTexView }, SWAPCHAIN_DIMS);

        Ref<DescriptorSetLayout> descriptorSetLayout = DescriptorSetLayout::create(
            { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });
        offscreenTextureDescriptorSets[i] = &BZ_GRAPHICS_CTX.getDescriptorPool().getDescriptorSet(descriptorSetLayout);
        offscreenTextureDescriptorSets[i]->setCombinedTextureSampler(swapchainReplicaTexView,
                                                                     Renderer::getDefaultSampler(), 0);
    }

    renderFunction = [this](const Scene &scene) {
        const Ref<Framebuffer> offscreenFramebuffer = offscreenFramebuffers[BZ_GRAPHICS_CTX.getCurrentFrameIndex()];
        const Ref<Framebuffer> swapchainFramebuffer = BZ_GRAPHICS_CTX.getSwapchainAquiredImageFramebuffer();
        Renderer::render(firstPass, offscreenFramebuffer, true, false);
        Renderer2D::render(scene, lastPassEditorMode, offscreenFramebuffer, false, false);
        RendererImGui::render(firstAndLastPass, swapchainFramebuffer, false, true);
    };
}

// TODO: only create the necessary RenderPasses.
void RendererCoordinator::internalInit() {
    // Create the possible combinations of RenderPasses. All compatible with the default Swapchain
    // Renderpass, which is used on the Pipelines and to create the Framebuffers.
    const Ref<RenderPass> &renderPass = BZ_GRAPHICS_CTX.getSwapchainRenderPass();
    const SubPassDescription &subPassDesc = renderPass->getSubPassDescription(0);

    AttachmentDescription colorAttachmentDesc = renderPass->getColorAttachmentDescription(0);
    colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
    colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Wait for previous Renderer.
    SubPassDependency dependency;
    dependency.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
    dependency.dstSubPassIndex = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependency.dependencyFlags = 0;

    // firstPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    firstPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });

    // secondPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    secondPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

    // lastPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    lastPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    lastPassEditorMode = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

    // firstAndLastPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    firstAndLastPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });
}

void RendererCoordinator::destroy() {
    Renderer2D::destroy();
    Renderer::destroy();
    RendererImGui::destroy();

    firstPass.reset();
    secondPass.reset();
    lastPass.reset();
    firstAndLastPass.reset();

    lastPassEditorMode.reset();

    for (uint32 i = 0; i < GraphicsContext::MAX_FRAMES_IN_FLIGHT; ++i) {
        this->offscreenFramebuffers[i].reset();
    }
}

void RendererCoordinator::onEvent(Event &e) {
    RendererImGui::onEvent(e);
}

void RendererCoordinator::render(const Scene &scene) {
    renderFunction(scene);
}

DescriptorSet *RendererCoordinator::getOffscreenTextureDescriptorSet() {
    uint32 currentFrame = BZ_GRAPHICS_CTX.getCurrentFrameIndex();
    return offscreenTextureDescriptorSets[currentFrame];
}

}