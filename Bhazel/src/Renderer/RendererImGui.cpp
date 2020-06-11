#include "bzpch.h"

#include "RendererImGui.h"

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"

#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include <imgui.h>


namespace BZ {

static struct ImGuiRendererData {
    Ref<Buffer> vertexBuffer;
    Ref<Buffer> indexBuffer;

    BufferPtr vertexBufferPtr;
    BufferPtr indexBufferPtr;

    Ref<TextureView> fontTextureView;
    Ref<Sampler> fontTextureSampler;

    Ref<PipelineLayout> pipelineLayout;
    Ref<PipelineState> pipelineState;
    DescriptorSet *descriptorSet;
} rendererData;


void RendererImGui::init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform
    // Windows io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons; io.ConfigFlags |=
    // ImGuiConfigFlags_ViewportsNoMerge;
    io.MouseDrawCursor = true; // Enable software cursor

    initInput();
    initGraphics();
}

void RendererImGui::destroy() {

    rendererData.vertexBuffer.reset();
    rendererData.indexBuffer.reset();

    rendererData.fontTextureView.reset();
    rendererData.fontTextureSampler.reset();

    rendererData.pipelineLayout.reset();
    rendererData.pipelineState.reset();

    ImGui::DestroyContext();
}

void RendererImGui::initInput() {
    // Setup back-end capabilities flags
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests
                                                          // (optional, rarely used)
    // io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create
    // multi-viewports on the Platform side (optional)
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
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

    // io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    // io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    // io.ClipboardUserData = g_Window;

    // g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    // g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    // g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); //
    // FIXME: GLFW doesn't have this. g_MouseCursors[ImGuiMouseCursor_ResizeNS] =
    // glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR); g_MouseCursors[ImGuiMouseCursor_ResizeEW] =
    // glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR); g_MouseCursors[ImGuiMouseCursor_ResizeNESW] =
    // glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
    // g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); //
    // FIXME: GLFW doesn't have this. g_MouseCursors[ImGuiMouseCursor_Hand] =
    // glfwCreateStandardCursor(GLFW_HAND_CURSOR);
}

void RendererImGui::initGraphics() {
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
    // identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Init font texture
    unsigned char *fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    auto fontTextureRef =
        Texture2D::create(fontData, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, MipmapData::Options::DoNothing);
    BZ_SET_TEXTURE_DEBUG_NAME(fontTextureRef, "ImGuiRenderer Font Texture");
    rendererData.fontTextureView = TextureView::create(fontTextureRef);
    rendererData.fontTextureSampler = Sampler::Builder().build();

    // VertexLayout
    DataLayout vertexLayout = {
        { DataType::Float32, DataElements::Vec2 },
        { DataType::Float32, DataElements::Vec2 },
        { DataType::Uint8, DataElements::Vec4, true },
    };

    // Buffers
    const uint32 MAX_INDICES = 1 << (sizeof(ImDrawIdx) * 8);
    rendererData.vertexBuffer = Buffer::create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, MAX_INDICES * sizeof(ImDrawVert),
                                               MemoryType::CpuToGpu, vertexLayout);
    rendererData.indexBuffer = Buffer::create(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, MAX_INDICES * sizeof(ImDrawIdx),
                                              MemoryType::CpuToGpu, { { DataType::Uint16, DataElements::Scalar, "" } });
    BZ_SET_BUFFER_DEBUG_NAME(rendererData.vertexBuffer, "RendererImGui Vertex Buffer");
    BZ_SET_BUFFER_DEBUG_NAME(rendererData.indexBuffer, "RendererImGui Index Buffer");

    rendererData.vertexBufferPtr = rendererData.vertexBuffer->map(0);
    rendererData.indexBufferPtr = rendererData.indexBuffer->map(0);

    // Shaders
    Ref<Shader> shader = Shader::create({ { "Bhazel/shaders/bin/ImGuiVert.spv", VK_SHADER_STAGE_VERTEX_BIT },
                                          { "Bhazel/shaders/bin/ImGuiFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } });
    // DescriptorSetLayout
    Ref<DescriptorSetLayout> descriptorSetLayout =
        DescriptorSetLayout::create({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1 } });

    rendererData.pipelineLayout =
        PipelineLayout::create({ descriptorSetLayout }, { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ImVec2) } });

    // PipelineStateData
    BlendingState blendingState;
    BlendingStateAttachment blendingStateAttachment;
    blendingStateAttachment.enableBlending = true;
    blendingStateAttachment.srcColorBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendingStateAttachment.dstColorBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendingStateAttachment.colorBlendingOperation = VK_BLEND_OP_ADD;
    blendingStateAttachment.srcAlphaBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendingStateAttachment.dstAlphaBlendingFactor = VK_BLEND_FACTOR_ZERO;
    blendingStateAttachment.alphaBlendingOperation = VK_BLEND_OP_ADD;
    blendingState.attachmentBlendingStates = { blendingStateAttachment };

    PipelineStateData pipelineStateData;
    pipelineStateData.dataLayout = vertexLayout;
    pipelineStateData.shader = shader;
    pipelineStateData.layout = rendererData.pipelineLayout;
    pipelineStateData.blendingState = blendingState;
    pipelineStateData.dynamicStates = { VK_DYNAMIC_STATE_SCISSOR };
    pipelineStateData.renderPass = Engine::get().getGraphicsContext().getSwapchainRenderPass();
    pipelineStateData.subPassIndex = 0;

    rendererData.pipelineState = PipelineState::create(pipelineStateData);
    BZ_SET_PIPELINE_DEBUG_NAME(rendererData.pipelineState, "RendererImGui Pipeline");

    // DescriptorSet
    rendererData.descriptorSet = &DescriptorSet::get(descriptorSetLayout);
    rendererData.descriptorSet->setCombinedTextureSampler(rendererData.fontTextureView, rendererData.fontTextureSampler,
                                                          0);
}

void RendererImGui::render(const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer,
                           bool waitForImageAvailable, bool signalFrameEnd) {
    ImDrawData *imDrawData = ImGui::GetDrawData();

    CommandBuffer &commandBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
    BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, "RendererImGui");

    if (imDrawData->TotalVtxCount == 0 || imDrawData->TotalIdxCount == 0 || imDrawData->CmdListsCount <= 0) {
        BZ_LOG_CORE_INFO("Nothing to draw from ImGui. Vertices count: {}. Indices count: {}. "
                         "Command List count: {}. Bailing Out.",
                         imDrawData->TotalVtxCount, imDrawData->TotalIdxCount, imDrawData->CmdListsCount);

        // Still need to submit a command buffer to respect the frame sync and layout transitions
        // coming from the RendererCoordinator.
        commandBuffer.beginRenderPass(swapchainRenderPass, swapchainFramebuffer);
        commandBuffer.endRenderPass();
        commandBuffer.endAndSubmit(waitForImageAvailable, signalFrameEnd);
        return;
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

    ImGuiIO &io = ImGui::GetIO();

    commandBuffer.bindBuffer(rendererData.vertexBuffer, 0);
    commandBuffer.bindBuffer(rendererData.indexBuffer, 0);
    commandBuffer.bindPipelineState(rendererData.pipelineState);
    commandBuffer.bindDescriptorSet(*rendererData.descriptorSet, rendererData.pipelineLayout, 0, nullptr, 0);
    commandBuffer.setPushConstants(rendererData.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, &io.DisplaySize, 0,
                                   sizeof(ImVec2));

    // Wait for the memcpyied index/vertex data to be available before doing actual rendering.
    commandBuffer.pipelineBarrierMemory(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

    commandBuffer.beginRenderPass(swapchainRenderPass, swapchainFramebuffer);

    int globalIndexOffset = 0;
    int globalVertexOffset = 0;
    for (int i = 0; i < imDrawData->CmdListsCount; ++i) {
        const ImDrawList *cmdList = imDrawData->CmdLists[i];
        for (int j = 0; j < cmdList->CmdBuffer.Size; ++j) {
            const ImDrawCmd *pcmd = &cmdList->CmdBuffer[j];

            // User callback, registered via ImDrawList::AddCallback()
            if (pcmd->UserCallback != NULL) {
                pcmd->UserCallback(cmdList, pcmd);
            }
            else {
                VkRect2D scissorRect;
                scissorRect.offset.x = std::max(static_cast<uint32>(pcmd->ClipRect.x), 0u);
                scissorRect.offset.y = std::max(static_cast<uint32>(pcmd->ClipRect.y), 0u);
                scissorRect.extent.width = static_cast<uint32>(pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissorRect.extent.height = static_cast<uint32>(pcmd->ClipRect.w - pcmd->ClipRect.y);
                commandBuffer.setScissorRects(0, &scissorRect, 1);
                commandBuffer.drawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + globalIndexOffset,
                                          pcmd->VtxOffset + globalVertexOffset, 0);
            }
        }
        globalIndexOffset += cmdList->IdxBuffer.Size;
        globalVertexOffset += cmdList->VtxBuffer.Size;
    }

    commandBuffer.endRenderPass();
    BZ_CB_END_DEBUG_LABEL(commandBuffer);
    commandBuffer.endAndSubmit(waitForImageAvailable, signalFrameEnd);
}

void RendererImGui::onEvent(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<KeyPressedEvent>([](const KeyPressedEvent &event) -> bool {
        ImGuiIO &io = ImGui::GetIO();

        int keyCode = event.getKeyCode();
        io.KeysDown[keyCode] = true;

        if (keyCode == BZ_KEY_LEFT_CONTROL || keyCode == BZ_KEY_RIGHT_CONTROL)
            io.KeyCtrl = true;
        if (keyCode == BZ_KEY_LEFT_ALT || keyCode == BZ_KEY_RIGHT_ALT)
            io.KeyAlt = true;
        if (keyCode == BZ_KEY_LEFT_SHIFT || keyCode == BZ_KEY_RIGHT_SHIFT)
            io.KeyShift = true;
        if (keyCode == BZ_KEY_LEFT_SUPER || keyCode == BZ_KEY_RIGHT_SUPER)
            io.KeySuper = true;

        return false;
    });

    dispatcher.dispatch<KeyReleasedEvent>([](const KeyReleasedEvent &event) -> bool {
        ImGuiIO &io = ImGui::GetIO();

        int keyCode = event.getKeyCode();
        io.KeysDown[keyCode] = false;

        if (keyCode == BZ_KEY_LEFT_CONTROL || keyCode == BZ_KEY_RIGHT_CONTROL)
            io.KeyCtrl = false;
        if (keyCode == BZ_KEY_LEFT_ALT || keyCode == BZ_KEY_RIGHT_ALT)
            io.KeyAlt = false;
        if (keyCode == BZ_KEY_LEFT_SHIFT || keyCode == BZ_KEY_RIGHT_SHIFT)
            io.KeyShift = false;
        if (keyCode == BZ_KEY_LEFT_SUPER || keyCode == BZ_KEY_RIGHT_SUPER)
            io.KeySuper = false;

        return false;
    });

    dispatcher.dispatch<KeyTypedEvent>([](const KeyTypedEvent &event) -> bool {
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(event.getKeyCode());
        return false;
    });

    Window &window = Engine::get().getWindow();
    dispatcher.dispatch<MouseMovedEvent>([&window](const MouseMovedEvent &event) -> bool {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseHoveredViewport = 0;
        io.MousePos =
            ImVec2(static_cast<float>(event.getX()), static_cast<float>(window.getDimensions().y - event.getY()));
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

void RendererImGui::begin() {
    ImGuiIO &io = ImGui::GetIO();
    const auto &windowDims = Engine::get().getWindow().getDimensionsFloat();
    io.DisplaySize = ImVec2(windowDims.x, windowDims.y);

    ImGui::NewFrame();
}

void RendererImGui::end() {
    ImGui::Render();
}
}