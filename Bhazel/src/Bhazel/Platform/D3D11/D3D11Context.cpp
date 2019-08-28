#include "bzpch.h"

#include "D3D11Context.h"
#include "D3D11RendererAPI.h"


namespace BZ {

    D3D11Context::D3D11Context(HWND windowHandle) :
        windowHandle (windowHandle) {

        //Create SwapChain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
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
        swapChainDesc.OutputWindow = windowHandle;
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
       
        //Set Depth and Stencil settings
        D3D11_DEPTH_STENCIL_DESC dsDesc = {0};
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = false;

        wrl::ComPtr<ID3D11DepthStencilState> dsState;
        BZ_ASSERT_HRES_DXGI(device->CreateDepthStencilState(&dsDesc, &dsState));
        deviceContext->OMSetDepthStencilState(dsState.Get(), 1);

        //Set Rasterizer settings
        wrl::ComPtr<ID3D11RasterizerState> rsState;
        D3D11_RASTERIZER_DESC rsDesc = {0};
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
        D3D11_TEXTURE2D_DESC backBufferDesc = {0};
        backBuffer->GetDesc(&backBufferDesc);

        D3D11_VIEWPORT vp;
        vp.Width = static_cast<float>(backBufferDesc.Width);
        vp.Height = static_cast<float>(backBufferDesc.Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        deviceContext->RSSetViewports(1, &vp);

        rendererAPI = std::make_unique<D3D11RendererAPI>(*this);
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void D3D11Context::swapBuffers() {
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
        D3D11_TEXTURE2D_DESC descDepthStencil = {0};
        backBuffer->GetDesc(&descDepthStencil); //Reuse back buffer description
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