#include "bzpch.h"

#include "RendererCoordinator.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/RenderPass.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RendererImGui.h"

#include "Core/Application.h"
#include "Core/KeyCodes.h"
#include "Events/KeyEvent.h"


namespace BZ {

void RendererCoordinator::init() {
    is3dActive = true;
    is2dActive = true;
    isImGuiActive = true;

    // Create the possible combinations of RenderPasses. All compatible with the default Swapchain
    // Renderpass, which is used on the Pipelines and to create the Framebuffers.
    const Ref<RenderPass> &renderPass = Application::get().getGraphicsContext().getSwapchainRenderPass();
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

    // firstAndLastPass
    colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    firstAndLastPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });
}

void RendererCoordinator::destroy() {
    firstPass.reset();
    secondPass.reset();
    lastPass.reset();
    firstAndLastPass.reset();
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
    Ref<Framebuffer> currentSwapchainFramebuffer =
        Application::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer();

    uint32 activeRendererCount = getActiveRendererCount();
    if (activeRendererCount == 1) {
        if (is3dActive) {
            Renderer::render(firstAndLastPass, currentSwapchainFramebuffer, true, true);
        }
        else if (is2dActive) {
            Renderer2D::render(firstAndLastPass, currentSwapchainFramebuffer, true, true);
        }
        else {
            RendererImGui::render(firstAndLastPass, currentSwapchainFramebuffer, true, true);
        }
    }
    else if (activeRendererCount == 2) {
        if (is3dActive) {
            Renderer::render(firstPass, currentSwapchainFramebuffer, true, false);
            if (is2dActive) {
                Renderer2D::render(lastPass, currentSwapchainFramebuffer, false, true);
            }
            else if (isImGuiActive) {
                RendererImGui::render(lastPass, currentSwapchainFramebuffer, false, true);
            }
        }
        else {
            Renderer2D::render(firstPass, currentSwapchainFramebuffer, true, false);
            RendererImGui::render(lastPass, currentSwapchainFramebuffer, false, true);
        }
    }
    else if (activeRendererCount == 3) {
        Renderer::render(firstPass, currentSwapchainFramebuffer, true, false);
        Renderer2D::render(secondPass, currentSwapchainFramebuffer, false, false);
        RendererImGui::render(lastPass, currentSwapchainFramebuffer, false, true);
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