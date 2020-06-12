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

void RendererCoordinator::init() {
    is3dActive = true;
    is2dActive = true;
    isImGuiActive = true;
    isForceOffscreenRendering = false;

    // Create the possible combinations of RenderPasses. All compatible with the default Swapchain
    // Renderpass, which is used on the Pipelines and to create the Framebuffers.
    const Ref<RenderPass> &renderPass = Engine::get().getGraphicsContext().getSwapchainRenderPass();
    const SubPassDescription &subPassDesc = renderPass->getSubPassDescription(0);

    AttachmentDescription colorAttachmentDesc = renderPass->getColorAttachmentDescription(0);
    colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
    colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Wait for previous Renderer. //TODO: confirm if correct.
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
    firstPassOffscreen = firstPass;

    // secondPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    secondPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });
    secondPassOffscreen = secondPass;

    // lastPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    lastPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    lastPassOffscreen = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

    // firstAndLastPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    firstAndLastPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });

    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    firstAndLastPassOffscreen = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });
}

void RendererCoordinator::destroy() {
    firstPass.reset();
    secondPass.reset();
    lastPass.reset();
    firstAndLastPass.reset();

    offscreenFramebuffer.reset();
    firstPassOffscreen.reset();
    secondPassOffscreen.reset();
    lastPassOffscreen.reset();
    firstAndLastPassOffscreen.reset();
}

void RendererCoordinator::enable3dRenderer(bool enable) {
    is3dActive = enable;
}

void RendererCoordinator::enable2dRenderer(bool enable) {
    is2dActive = enable;
}

void RendererCoordinator::enableImGuiRenderer(bool enable) {
    isImGuiActive = enable;
}

void RendererCoordinator::forceOffscreenRendering(bool force) {
    isForceOffscreenRendering = force;

    if (!offscreenFramebuffer) {
        const Ref<RenderPass> &swapchainRenderPass = Engine::get().getGraphicsContext().getSwapchainRenderPass();
        const Ref<Framebuffer> &swapchainFramebuffer =
            Engine::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer();
        const glm::uvec3 SWAPCHAIN_DIMS = swapchainFramebuffer->getDimensionsAndLayers();
        auto swapchainReplicaTex =
            Texture2D::createRenderTarget(SWAPCHAIN_DIMS.x, SWAPCHAIN_DIMS.y, 1, 1,
                                          swapchainFramebuffer->getColorAttachmentTextureView(0)->getTextureFormat());
        BZ_SET_TEXTURE_DEBUG_NAME(swapchainReplicaTex, "RendererCoordinator SwapchainReplica Texture");
        auto swapchainReplicaTexView = TextureView::create(swapchainReplicaTex);

        offscreenFramebuffer = Framebuffer::create(swapchainRenderPass, { swapchainReplicaTexView }, SWAPCHAIN_DIMS);

        Ref<DescriptorSetLayout> descriptorSetLayout = DescriptorSetLayout::create(
            { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });
        offscreenTextureDescriptorSet = &BZ_GRAPHICS_CTX.getDescriptorPool().getDescriptorSet(descriptorSetLayout);
        offscreenTextureDescriptorSet->setCombinedTextureSampler(swapchainReplicaTexView, Renderer::getDefaultSampler(),
                                                                 0);
    }
}

void RendererCoordinator::onEvent(Event &ev) {
    EventDispatcher dispatcher(ev);
    dispatcher.dispatch<KeyPressedEvent>([this](const KeyPressedEvent &ev) -> bool {
        uint32 activeRendererCount = getActiveRendererCount();

        if (ev.getKeyCode() == BZ_KEY_F1) {
            if (is3dActive) {
                if (activeRendererCount > 1) {
                    is3dActive = false;
                }
            }
            else
                is3dActive = true;
        }
        if (ev.getKeyCode() == BZ_KEY_F2) {
            if (is2dActive) {
                if (activeRendererCount > 1) {
                    is2dActive = false;
                }
            }
            else
                is2dActive = true;
        }
        if (ev.getKeyCode() == BZ_KEY_F3) {
            if (isImGuiActive) {
                if (activeRendererCount > 1) {
                    isImGuiActive = false;
                }
            }
            else
                isImGuiActive = true;
        }
        return false;
    });
}

void RendererCoordinator::render() {
    Ref<Framebuffer> swapchainFramebuffer = Engine::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer();
    Ref<Framebuffer> framebuffer = isForceOffscreenRendering ? offscreenFramebuffer : swapchainFramebuffer;

    uint32 activeRendererCount = getActiveRendererCount();
    if (activeRendererCount == 1) {
        Ref<RenderPass> renderPass = isForceOffscreenRendering ? firstAndLastPassOffscreen : firstAndLastPass;

        if (is3dActive) {
            Renderer::render(renderPass, framebuffer, true, true);
        }
        else if (is2dActive) {
            Renderer2D::render(renderPass, framebuffer, true, true);
        }
        else {
            RendererImGui::render(firstAndLastPass, swapchainFramebuffer, true, true);
        }
    }
    else if (activeRendererCount == 2) {
        Ref<RenderPass> firstRenderPass = isForceOffscreenRendering ? firstAndLastPassOffscreen : firstPass;
        Ref<RenderPass> lastRenderPass = isForceOffscreenRendering ? lastPassOffscreen : lastPass;
        Ref<RenderPass> finalRenderPass = isForceOffscreenRendering ? firstAndLastPass : lastPass;

        if (is3dActive) {
            Renderer::render(firstRenderPass, framebuffer, true, false);
            if (is2dActive) {
                Renderer2D::render(lastRenderPass, framebuffer, false, true);
            }
            else if (isImGuiActive) {
                RendererImGui::render(finalRenderPass, swapchainFramebuffer, false, true);
            }
        }
        else {
            Renderer2D::render(firstRenderPass, framebuffer, true, false);
            RendererImGui::render(finalRenderPass, swapchainFramebuffer, false, true);
        }
    }
    else if (activeRendererCount == 3) {
        Ref<RenderPass> firstRenderPass = isForceOffscreenRendering ? firstPassOffscreen : firstPass;
        Ref<RenderPass> secondRenderPass = isForceOffscreenRendering ? secondPassOffscreen : secondPass;
        Ref<RenderPass> finalRenderPass = isForceOffscreenRendering ? firstAndLastPass : lastPass;

        Renderer::render(firstRenderPass, framebuffer, true, false);
        Renderer2D::render(secondRenderPass, framebuffer, false, false);
        RendererImGui::render(firstAndLastPass, swapchainFramebuffer, false, true);
    }
}

uint32 RendererCoordinator::getActiveRendererCount() const {
    uint32 activeRendererCount = 0;
    if (is3dActive)
        activeRendererCount++;
    if (is2dActive)
        activeRendererCount++;
    if (isImGuiActive)
        activeRendererCount++;
    return activeRendererCount;
}
}