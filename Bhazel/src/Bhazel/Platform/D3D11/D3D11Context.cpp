#include "bzpch.h"

#include "D3D11Context.h"
#include "Bhazel/Platform/D3D11/D3D11RendererAPI.h"
#include "Bhazel/Renderer/RenderCommand.h"


namespace BZ {

    D3D11Context::D3D11Context(void *windowHandle) :
        windowHandle(static_cast<HWND>(windowHandle)) {

        //Create SwapChain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferDesc.Width = 0;
        swapChainDesc.BufferDesc.Height = 0;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 1;
        swapChainDesc.OutputWindow = this->windowHandle;
        swapChainDesc.Windowed = true;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
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

        wrl::ComPtr<IDXGIDevice> dXGIDevice;
        wrl::ComPtr<IDXGIAdapter> dXGIAdapter;
        DXGI_ADAPTER_DESC adapterDesc;
        BZ_ASSERT_HRES_DXGI(device->QueryInterface(__uuidof(IDXGIDevice), (void **) &dXGIDevice));
        BZ_ASSERT_HRES_DXGI(dXGIDevice->GetAdapter(&dXGIAdapter));
        BZ_ASSERT_HRES_DXGI(dXGIAdapter->GetDesc(&adapterDesc));

        BZ_LOG_CORE_INFO("D3D11 Context:");
        std::wstring wdesc(adapterDesc.Description);
        std::string desc(wdesc.begin(), wdesc.end());
        BZ_LOG_CORE_INFO("  Description: {0}.", desc);
        BZ_LOG_CORE_INFO("  VendorId: 0x{0:04x}.", adapterDesc.VendorId);
        BZ_LOG_CORE_INFO("  DeviceId: 0x{0:04x}.", adapterDesc.DeviceId);
        BZ_LOG_CORE_INFO("  SubSysId: 0x{0:04x}.", adapterDesc.SubSysId);
        BZ_LOG_CORE_INFO("  Revision: 0x{0:04x}.", adapterDesc.Revision);
        BZ_LOG_CORE_INFO("  DedicatedVideoMemory: {0} bytes.", adapterDesc.DedicatedVideoMemory);
        BZ_LOG_CORE_INFO("  DedicatedSystemMemory: {0} bytes.", adapterDesc.DedicatedSystemMemory);
        BZ_LOG_CORE_INFO("  SharedSystemMemory: {0} bytes.", adapterDesc.SharedSystemMemory);

        //Set Rasterizer settings
        wrl::ComPtr<ID3D11RasterizerState> rsState;
        D3D11_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.CullMode = D3D11_CULL_BACK;
        rsDesc.FrontCounterClockwise = true;
        rsDesc.DepthBias = 0;
        rsDesc.DepthBiasClamp = 0;
        rsDesc.SlopeScaledDepthBias = 0;
        rsDesc.DepthClipEnable = true;
        rsDesc.ScissorEnable = false;
        rsDesc.MultisampleEnable = false;
        rsDesc.AntialiasedLineEnable = false;

        BZ_ASSERT_HRES_DXGI(device->CreateRasterizerState(&rsDesc, &rsState));
        BZ_LOG_DXGI(deviceContext->RSSetState(rsState.Get()));

        //Set default viewport
        wrl::ComPtr<ID3D11Texture2D> backBuffer;
        BZ_ASSERT_HRES_DXGI(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
        D3D11_TEXTURE2D_DESC backBufferDesc = {};
        BZ_LOG_DXGI(backBuffer->GetDesc(&backBufferDesc));

        D3D11_VIEWPORT vp;
        vp.Width = static_cast<float>(backBufferDesc.Width);
        vp.Height = static_cast<float>(backBufferDesc.Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        BZ_LOG_DXGI(deviceContext->RSSetViewports(1, &vp));

        rendererAPI = std::make_unique<D3D11RendererAPI>(*this);
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void D3D11Context::presentBuffer() {
        BZ_ASSERT_HRES_DXGI(swapChain->Present(static_cast<uint32>(vsync), 0));
    }

    void D3D11Context::handleWindowResize(int witdh, int height) {
        BZ_LOG_DXGI(deviceContext->OMSetRenderTargets(0, nullptr, nullptr));
        backBufferView.Reset();
        depthStencilView.Reset();

        //Preserve the existing buffer count and format.
        //Automatically choose the width and height to match the client rect for HWNDs.
        BZ_LOG_DXGI(swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

        setupRenderTargets();
    }

    void D3D11Context::setupRenderTargets() {
        wrl::ComPtr<ID3D11Texture2D> backBuffer;
        BZ_ASSERT_HRES_DXGI(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

        wrl::ComPtr<ID3D11Texture2D> depthStencilBuffer;
        D3D11_TEXTURE2D_DESC descDepthStencil = {};
        BZ_LOG_DXGI(backBuffer->GetDesc(&descDepthStencil)); //Reuse back buffer description
        descDepthStencil.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepthStencil.Usage = D3D11_USAGE_DEFAULT;
        descDepthStencil.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        BZ_ASSERT_HRES_DXGI(device->CreateTexture2D(&descDepthStencil, nullptr, &depthStencilBuffer));

        //Create Views
        BZ_ASSERT_HRES_DXGI(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &backBufferView));
        BZ_ASSERT_HRES_DXGI(device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, &depthStencilView));

        BZ_LOG_DXGI(deviceContext->OMSetRenderTargets(1, backBufferView.GetAddressOf(), depthStencilView.Get()));
    }
}