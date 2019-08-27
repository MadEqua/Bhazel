#include "bzpch.h"

#include "D3D11Context.h"
#include "D3D11RendererAPI.h"

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
        swapChainDesc.BufferCount = 1;
        swapChainDesc.OutputWindow = windowHandle;
        swapChainDesc.Windowed = TRUE;
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


        //Create Depth/Stencil Buffer
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
        wrl::ComPtr<ID3D11RenderTargetView> backBufferView;
        wrl::ComPtr<ID3D11DepthStencilView> depthStencilView;

        BZ_ASSERT_HRES_DXGI(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &backBufferView));
        BZ_ASSERT_HRES_DXGI(device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, &depthStencilView));

        BZ_LOG_DXGI(deviceContext->OMSetRenderTargets(1, backBufferView.GetAddressOf(), depthStencilView.Get()));

        //TODO: making some default assumptions here
        D3D11_DEPTH_STENCIL_DESC dsDesc = {0};
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = false;

        wrl::ComPtr<ID3D11DepthStencilState> dsState;
        BZ_ASSERT_HRES_DXGI(device->CreateDepthStencilState(&dsDesc, &dsState))
        deviceContext->OMSetDepthStencilState(dsState.Get(), 1);

        testinit();

        rendererAPI = std::make_unique<D3D11RendererAPI>(device.Get(), deviceContext.Get(), swapChain.Get(), 
                                                         backBufferView.Get(), depthStencilView.Get());
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void D3D11Context::testinit() {
        float vx[] = {
            -0.5f, -0.5f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        D3D11_BUFFER_DESC bufferDesc = {0};
        bufferDesc.ByteWidth = sizeof(vx);
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 6 * sizeof(float);

        D3D11_SUBRESOURCE_DATA data = {0};
        data.pSysMem = vx;

        BZ_ASSERT_HRES_DXGI(device->CreateBuffer(&bufferDesc, &data, &vertexBuffer));

        UINT stride = 6 * sizeof(float);
        UINT offset = 0;
        BZ_LOG_DXGI(deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset));
       

        const char* vs = R"(
            struct VsOut {
                float3 col : COLOR;
                float4 pos : SV_POSITION;
            };

            VsOut main(float3 pos : POSITION, float3 col : COLOR) {
                VsOut res;
                res.pos = float4(pos, 1.0);
                res.col = col;
                return res;
            }
        )";

        const char* fs = R"(
            struct PsIn {
                float3 col : COLOR;
            };

            float4 main(PsIn input) : SV_TARGET {
                return float4(input.col, 1.0);
            }
        )";

        wrl::ComPtr<ID3DBlob> vsBlob;
        BZ_ASSERT_HRES_DXGI(D3DCompile(
            vs,
            strlen(vs),
            nullptr,
            nullptr,
            nullptr,
            "main", "vs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &vsBlob,
            nullptr));

        wrl::ComPtr<ID3DBlob> fsBlob;
        BZ_ASSERT_HRES_DXGI(D3DCompile(
            fs,
            strlen(fs),
            nullptr,
            nullptr,
            nullptr,
            "main", "ps_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &fsBlob,
            nullptr));

        BZ_ASSERT_HRES_DXGI(device->CreateVertexShader(
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            nullptr,
            &vertexShader));

        BZ_ASSERT_HRES_DXGI(device->CreatePixelShader(
            fsBlob->GetBufferPointer(),
            fsBlob->GetBufferSize(),
            nullptr,
            &pixelShader));

        BZ_LOG_DXGI(deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0));
        BZ_LOG_DXGI(deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0));


        wrl::ComPtr<ID3D11InputLayout> inputLayout;
        D3D11_INPUT_ELEMENT_DESC ied[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        BZ_ASSERT_HRES_DXGI(device->CreateInputLayout(ied, ARRAYSIZE(ied), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout));
        BZ_LOG_DXGI(deviceContext->IASetInputLayout(inputLayout.Get()));





        D3D11_VIEWPORT viewport = {0};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = 1280;
        viewport.Height = 800;
        viewport.MinDepth = 0;
        viewport.MaxDepth = 1;
        BZ_LOG_DXGI(deviceContext->RSSetViewports(1, &viewport));

        BZ_LOG_DXGI(deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    }

    void D3D11Context::testdraw() {
        /*float color[] = {0.2f, 0.4f, 0.2f};
        deviceContext->ClearRenderTargetView(backBufferView.Get(), color);
        BZ_LOG_DXGI(deviceContext->Draw(3, 0));*/
    }

    void D3D11Context::swapBuffers() {
        //testdraw();

        BZ_ASSERT_HRES_DXGI(swapChain->Present(static_cast<uint32>(vsync), 0));
    }
}