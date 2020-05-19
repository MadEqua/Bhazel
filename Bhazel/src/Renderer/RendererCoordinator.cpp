#include "bzpch.h"

#include "RendererCoordinator.h"

#include "Graphics/RenderPass.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RendererImGui.h"

#include "Core/Application.h"
#include "Events/KeyEvent.h"
#include "Core/KeyCodes.h"


namespace BZ {

    void RendererCoordinator::init() {
        is3dActive = true;
        is2dActive = true;
        isImGuiActive = true;

        //Create the possible combinations of RenderPasses. All compatible with the default Swapchain Renderpass, which
        //is used on the Pipelines and to create the Framebuffers.
        const Ref<RenderPass> &renderPass = Application::get().getGraphicsContext().getSwapchainDefaultRenderPass();
        const SubPassDescription &subPassDesc = renderPass->getSubPassDescription(0);

        AttachmentDescription colorAttachmentDesc = renderPass->getColorAttachmentDescription(0);
        colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        //Wait for previous Renderer. //TODO: confirm if correct.
        SubPassDependency dependency;
        dependency.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
        dependency.dstSubPassIndex = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependency.dependencyFlags = 0;

        //firstPass
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        firstPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });

        //secondPass
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        secondPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

        //lastPass
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        lastPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

        //firstAndLastPass
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
        dispatcher.dispatch<KeyPressedEvent>([this] (const KeyPressedEvent &ev) -> bool {
            if(ev.getKeyCode() == BZ_KEY_F1) is3dActive = !is3dActive;
            if(ev.getKeyCode() == BZ_KEY_F2) is2dActive = !is2dActive;
            if(ev.getKeyCode() == BZ_KEY_F3) isImGuiActive = !isImGuiActive;
            return false;
        });
    }

    void RendererCoordinator::render() {
        int activeRendererCount = 0;
        if(is3dActive) activeRendererCount++;
        if(is2dActive) activeRendererCount++;
        if(isImGuiActive) activeRendererCount++;

        Ref<Framebuffer> currentSwapchainFramebuffer = Application::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer();

        if(activeRendererCount == 1) {
            if(is3dActive) {
                Renderer::render(firstAndLastPass, currentSwapchainFramebuffer);
            }
            else if(is2dActive) {
                Renderer2D::render(firstAndLastPass, currentSwapchainFramebuffer);
            }
            else {
                RendererImGui::render(firstAndLastPass, currentSwapchainFramebuffer);
            }
        }
        else if(activeRendererCount == 2) {
            if(is3dActive) {
                Renderer::render(firstPass, currentSwapchainFramebuffer);
                if(is2dActive) {
                    Renderer2D::render(lastPass, currentSwapchainFramebuffer);
                }
                else if(isImGuiActive) {
                    RendererImGui::render(lastPass, currentSwapchainFramebuffer);
                }
            }
            else {
                Renderer2D::render(firstPass, currentSwapchainFramebuffer);
                RendererImGui::render(lastPass, currentSwapchainFramebuffer);
            }
        }
        else if(activeRendererCount == 3) {
            Renderer::render(firstPass, currentSwapchainFramebuffer);
            Renderer2D::render(secondPass, currentSwapchainFramebuffer);
            RendererImGui::render(lastPass, currentSwapchainFramebuffer);
        }
    }
}