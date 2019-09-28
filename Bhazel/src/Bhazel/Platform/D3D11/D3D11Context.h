#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "D3D11Includes.h"


namespace BZ {

    class D3D11Context : public GraphicsContext {
    public:
        explicit D3D11Context(HWND windowHandle);

        virtual void presentBuffer() override;

        void onWindowResize(uint32 width, uint32 height) override;

        ID3D11Device* getDevice() { return device.Get(); }
        ID3D11DeviceContext* getDeviceContext() { return deviceContext.Get(); }
        IDXGISwapChain* getSwapChain() { return swapChain.Get(); }

        ID3D11RenderTargetView* getBackBufferView() { return backBufferView.Get(); }
        ID3D11DepthStencilView* getDepthStencilView() { return depthStencilView.Get(); }

    private:
        HWND windowHandle;

        wrl::ComPtr<ID3D11Device> device;
        wrl::ComPtr<ID3D11DeviceContext> deviceContext;
        wrl::ComPtr<IDXGISwapChain> swapChain;

        wrl::ComPtr<ID3D11RenderTargetView> backBufferView;
        wrl::ComPtr<ID3D11DepthStencilView> depthStencilView;

        void setupRenderTargets();
    };
}