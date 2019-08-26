#include "bzpch.h"

#include "D3D11Context.h"
#include "D3D11RendererAPI.h"
#include "D3D11Debug.h"

#include "Bhazel/Renderer/RenderCommand.h"


namespace BZ {

    D3D11Context::D3D11Context(HWND windowHandle) :
        windowHandle (windowHandle) {

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
        swapChainDesc.BufferDesc.Width = 0;
        swapChainDesc.BufferDesc.Height = 0;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //TODO: srgb?
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.OutputWindow = windowHandle;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Flags = 0;

        uint32 flags = 0;
#ifndef BZ_DIST
        flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

        BZ_ASSERT_HRES_DXGI(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
                            &swapChainDesc, &swapChain, &device, nullptr, &deviceContext));

        BZ_ASSERT_CORE(swapChain, "Error creating SwapChain!");
        BZ_ASSERT_CORE(device, "Error creating Device!");
        BZ_ASSERT_CORE(deviceContext, "Error creating DeviceContext!");

        rendererAPI = std::make_unique<D3D11RendererAPI>();
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    D3D11Context::~D3D11Context() {
        if(deviceContext)
            deviceContext->Release();
        if(swapChain)
            swapChain->Release();
        if(device)
            device->Release();
    }

    void D3D11Context::swapBuffers() {
        //TESTING
        ID3D11Resource *backBuffer = nullptr;
        swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**) &backBuffer);

        ID3D11RenderTargetView *target;
        device->CreateRenderTargetView(backBuffer, nullptr, &target);

        backBuffer->Release();

        float color[] = {0.2f, 0.4f, 0.2f};
        deviceContext->ClearRenderTargetView(target, color);

        //deviceContext->OMSetRenderTargets(1, &target, nullptr);


        HRESULT hr;
        if(FAILED(hr = swapChain->Present(static_cast<uint32>(vsync), 0))) {
            BZ::DXGIDebug::getInstance().printMessages();

            if(hr == DXGI_ERROR_DEVICE_REMOVED) {
                //This has a special treatment because we have extra info
                BZ_ASSERT_ALWAYS_CORE("SwapChain Present failed! Device Removed. Reason: {0}", device->GetDeviceRemovedReason());
            }
            else
                BZ_ASSERT_ALWAYS_CORE("SwapChain Present failed! Error: 0x{0:08x}", static_cast<uint32>(hr));
        }
    }
}