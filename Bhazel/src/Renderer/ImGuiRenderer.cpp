#include "bzpch.h"

#include "ImGuiRenderer.h"

#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/RenderPass.h"

#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"

#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include <imgui.h>


namespace BZ {

    static struct ImGuiRendererData {
        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;
        Ref<Buffer> constantBuffer;

        BufferPtr vertexBufferPtr;
        BufferPtr indexBufferPtr;
        BufferPtr constantBufferPtr;

        Ref<TextureView> fontTextureView;
        Ref<Sampler> fontTextureSampler;

        Ref<PipelineState> pipelineState;
        DescriptorSet *descriptorSet;

        Ref<RenderPass> renderPass;
    } rendererData;


    void ImGuiRenderer::init() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
        io.MouseDrawCursor = true;                                  // Enable software cursor

        initInput();
        initGraphics();
    }

    void ImGuiRenderer::destroy() {

        rendererData.vertexBuffer.reset();
        rendererData.indexBuffer.reset();
        rendererData.constantBuffer.reset();

        rendererData.fontTextureView.reset();
        rendererData.fontTextureSampler.reset();

        rendererData.pipelineState.reset();

        rendererData.renderPass.reset();

        ImGui::DestroyContext();
    }

    void ImGuiRenderer::onEvent(Event &event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>([](const KeyPressedEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();

            int keyCode = event.getKeyCode();
            io.KeysDown[keyCode] = true;

            if(keyCode == BZ_KEY_LEFT_CONTROL || keyCode == BZ_KEY_RIGHT_CONTROL)
                io.KeyCtrl = true;
            if(keyCode == BZ_KEY_LEFT_ALT || keyCode == BZ_KEY_RIGHT_ALT)
                io.KeyAlt = true;
            if(keyCode == BZ_KEY_LEFT_SHIFT || keyCode == BZ_KEY_RIGHT_SHIFT)
                io.KeyShift = true;
            if(keyCode == BZ_KEY_LEFT_SUPER || keyCode == BZ_KEY_RIGHT_SUPER)
                io.KeySuper = true;

            return false;
            });

        dispatcher.dispatch<KeyReleasedEvent>([](const KeyReleasedEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();

            int keyCode = event.getKeyCode();
            io.KeysDown[keyCode] = false;

            if(keyCode == BZ_KEY_LEFT_CONTROL || keyCode == BZ_KEY_RIGHT_CONTROL)
                io.KeyCtrl = false;
            if(keyCode == BZ_KEY_LEFT_ALT || keyCode == BZ_KEY_RIGHT_ALT)
                io.KeyAlt = false;
            if(keyCode == BZ_KEY_LEFT_SHIFT || keyCode == BZ_KEY_RIGHT_SHIFT)
                io.KeyShift = false;
            if(keyCode == BZ_KEY_LEFT_SUPER || keyCode == BZ_KEY_RIGHT_SUPER)
                io.KeySuper = false;

            return false;
            });

        dispatcher.dispatch<KeyTypedEvent>([](const KeyTypedEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();
            io.AddInputCharacter(event.getKeyCode());
            return false;
            });

        Window &window = Application::get().getWindow();
        dispatcher.dispatch<MouseMovedEvent>([&window](const MouseMovedEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();
            io.MouseHoveredViewport = 0;
            io.MousePos = ImVec2(static_cast<float>(event.getX()), static_cast<float>(window.getDimensions().y - event.getY()));
            return false;
            });

        dispatcher.dispatch<MouseButtonPressedEvent>([](const MouseButtonPressedEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();
            io.MouseDown[event.getMouseButton()] = true;
            return false;
            });

        dispatcher.dispatch<MouseButtonReleasedEvent>([](const MouseButtonReleasedEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();
            io.MouseDown[event.getMouseButton()] = false;
            return false;
            });

        dispatcher.dispatch<MouseScrolledEvent>([](const MouseScrolledEvent &event) -> bool {
            ImGuiIO &io = ImGui::GetIO();
            io.MouseWheel = event.getYOffset();
            return false;
            });
    }

    void ImGuiRenderer::begin() {
        ImGuiIO &io = ImGui::GetIO();
        Window &window = Application::get().getWindow();
        io.DisplaySize = ImVec2(static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

        ImGui::NewFrame();
    }

    void ImGuiRenderer::end() {
        ImGui::Render();

        ImDrawData *imDrawData = ImGui::GetDrawData();

        uint32 vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        uint32 indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);

        if (vertexBufferSize == 0 || indexBufferSize == 0 || imDrawData->CmdListsCount <= 0) {
            BZ_LOG_CORE_INFO("Nothing to draw from ImGui Vertices size: {}. Indices size: {}. Command List count: {}. Allowing dummy RenderPass to perform image layout transition.",
                vertexBufferSize, indexBufferSize, imDrawData->CmdListsCount);
        }

        byte *vtxDst = rendererData.vertexBufferPtr;
        byte *idxDst = rendererData.indexBufferPtr;
        for (int n = 0; n < imDrawData->CmdListsCount; n++) {
            const ImDrawList *drawList = imDrawData->CmdLists[n];
            memcpy(vtxDst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += drawList->VtxBuffer.Size * sizeof(ImDrawVert);
            idxDst += drawList->IdxBuffer.Size * sizeof(ImDrawIdx);
        }

        commandBuffer.beginRenderPass(rendererData.renderPass, Application::get().getGraphicsContext().getSwapchainAquiredImageFramebuffer());

        ImGuiIO &io = ImGui::GetIO();
        glm::mat4 projMatrix(1.0f);
        projMatrix[0][0] = 2.0f / io.DisplaySize.x;
        projMatrix[1][1] = -2.0f / io.DisplaySize.y;
        projMatrix[3][0] = -1.0f;
        projMatrix[3][1] = 1.0f;

        memcpy(rendererData.constantBufferPtr, &projMatrix[0], sizeof(glm::mat4));

        commandBuffer.bindPipelineState(rendererData.pipelineState);
        commandBuffer.bindDescriptorSet(*rendererData.descriptorSet, rendererData.pipelineState, 0, nullptr, 0);

        int vertexOffset = 0;
        int indexOffset = 0;

        commandBuffer.bindBuffer(rendererData.vertexBuffer, 0);
        commandBuffer.bindBuffer(rendererData.indexBuffer, 0);

        for(int i = 0; i < imDrawData->CmdListsCount; ++i) {
            const ImDrawList *cmdList = imDrawData->CmdLists[i];
            for(int j = 0; j < cmdList->CmdBuffer.Size; ++j) {
                const ImDrawCmd *pcmd = &cmdList->CmdBuffer[j];
                VkRect2D scissorRect;
                scissorRect.offset.x = std::max(static_cast<uint32>(pcmd->ClipRect.x), 0u);
                scissorRect.offset.y = std::max(static_cast<uint32>(pcmd->ClipRect.y), 0u);
                scissorRect.extent.width = static_cast<uint32>(pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissorRect.extent.height = static_cast<uint32>(pcmd->ClipRect.w - pcmd->ClipRect.y);
                commandBuffer.setScissorRects(0, &scissorRect, 1);
                commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);

                indexOffset += pcmd->ElemCount;
            }
            vertexOffset += cmdList->VtxBuffer.Size;
        }

        commandBuffer.endRenderPass();
        commandBuffer.endAndSubmit();
    }

    void ImGuiRenderer::initInput() {
        // Setup back-end capabilities flags
        ImGuiIO &io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
        io.BackendPlatformName = "BhazelEngine";

        // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
        io.KeyMap[ImGuiKey_Tab] = BZ_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = BZ_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = BZ_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = BZ_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = BZ_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = BZ_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = BZ_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = BZ_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = BZ_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = BZ_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = BZ_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = BZ_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = BZ_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = BZ_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = BZ_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = BZ_KEY_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = BZ_KEY_A;
        io.KeyMap[ImGuiKey_C] = BZ_KEY_C;
        io.KeyMap[ImGuiKey_V] = BZ_KEY_V;
        io.KeyMap[ImGuiKey_X] = BZ_KEY_X;
        io.KeyMap[ImGuiKey_Y] = BZ_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = BZ_KEY_Z;

        //io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
        //io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
        //io.ClipboardUserData = g_Window;

        //g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        //g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        //g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
        //g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        //g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        //g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
        //g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
        //g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    }

    void ImGuiRenderer::initGraphics() {
        ImGuiIO &io = ImGui::GetIO();

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        //Init font texture
        unsigned char *fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
        auto fontTextureRef = Texture2D::create(fontData, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, MipmapData::Options::DoNothing);
        rendererData.fontTextureView = TextureView::create(fontTextureRef);
        rendererData.fontTextureSampler = Sampler::Builder().build();

        //VertexLayout
        DataLayout vertexLayout = {
            { DataType::Float32, DataElements::Vec2 },
            { DataType::Float32, DataElements::Vec2 },
            { DataType::Uint8, DataElements::Vec4, true },
        };

        //Buffers
        const uint32 MAX_INDICES = 1 << (sizeof(ImDrawIdx) * 8);
        rendererData.vertexBuffer = Buffer::create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, MAX_INDICES * sizeof(ImDrawVert), MemoryType::CpuToGpu, vertexLayout);
        rendererData.indexBuffer = Buffer::create(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, MAX_INDICES * sizeof(ImDrawIdx), MemoryType::CpuToGpu, { {DataType::Uint16, DataElements::Scalar, ""} });
        
        rendererData.vertexBufferPtr = rendererData.vertexBuffer->map(0);
        rendererData.indexBufferPtr = rendererData.indexBuffer->map(0);

        //Shaders
        Ref<Shader> shader = Shader::create({ { "Bhazel/shaders/bin/ImGuiVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                              { "Bhazel/shaders/bin/ImGuiFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });

        //DescriptorSetLayout
        Ref<DescriptorSetLayout> descriptorSetLayout = 
            DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1 },
                                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

        Window &window = Application::get().getWindow();

        //PipelineStateData
        BlendingState blendingState;
        BlendingStateAttachment blendingStateAttachment;
        blendingStateAttachment.enableBlending = true;
        blendingStateAttachment.srcColorBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendingStateAttachment.dstColorBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendingStateAttachment.colorBlendingOperation = VK_BLEND_OP_ADD;
        blendingStateAttachment.srcAlphaBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendingStateAttachment.dstAlphaBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendingStateAttachment.alphaBlendingOperation = VK_BLEND_OP_ADD;
        blendingState.attachmentBlendingStates = { blendingStateAttachment };

        PipelineStateData pipelineStateData;
        pipelineStateData.dataLayout = vertexLayout;
        pipelineStateData.shader = shader;
        pipelineStateData.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };
        pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()), 0.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, window.getWidth(), window.getHeight() } };
        pipelineStateData.blendingState = blendingState;
        pipelineStateData.dynamicStates = { VK_DYNAMIC_STATE_SCISSOR };

        //Create a Render Pass based on the default Swapchain one.
        const Ref<RenderPass> &renderPass = Application::get().getGraphicsContext().getSwapchainDefaultRenderPass();

        //This Renderer is not the first one, so load. It's the last one so transition to present layout.
        AttachmentDescription colorAttachmentDesc = renderPass->getColorAttachmentDescription(0);
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };

        //Wait on previous Renderers.
        SubPassDependency dependency;
        dependency.srcSubPassIndex = VK_SUBPASS_EXTERNAL;
        dependency.dstSubPassIndex = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependency.dependencyFlags = 0;

        rendererData.renderPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc }, { dependency });

        pipelineStateData.renderPass = rendererData.renderPass;
        pipelineStateData.subPassIndex = 0;

        rendererData.pipelineState = PipelineState::create(pipelineStateData);

        //Constant Buffer
        rendererData.constantBuffer = Buffer::create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, MIN_UNIFORM_BUFFER_OFFSET_ALIGN, MemoryType::CpuToGpu);
        rendererData.constantBufferPtr = rendererData.constantBuffer->map(0);

        //DescriptorSet
        rendererData.descriptorSet = &DescriptorSet::get(descriptorSetLayout);
        rendererData.descriptorSet->setConstantBuffer(rendererData.constantBuffer, 0, 0, sizeof(glm::mat4));
        rendererData.descriptorSet->setCombinedTextureSampler(rendererData.fontTextureView, rendererData.fontTextureSampler, 1);
    }
}