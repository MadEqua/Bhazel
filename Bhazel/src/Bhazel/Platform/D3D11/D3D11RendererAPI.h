#pragma once

#include "Bhazel/Renderer/RendererAPI.h"

#include "D3D11Includes.h"


namespace BZ {

    class D3D11RendererAPI : public RendererAPI
    {
    public:
        D3D11RendererAPI(ID3D11Device *device, ID3D11DeviceContext *deviceContext, IDXGISwapChain* swapchain,
                         ID3D11RenderTargetView *backBufferView, ID3D11DepthStencilView *depthStencilView);

        virtual void setClearColor(const glm::vec4& color) override;
        virtual void setDepthClearValue(float value) override;
        virtual void setStencilClearValue(int value) override;

        virtual void clearColorAndDepthStencilBuffers() override;

        virtual void drawIndexed(const Ref<InputDescription> &vertexArray) override;

    private:
        wrl::ComPtr<ID3D11Device> device;
        wrl::ComPtr<ID3D11DeviceContext> deviceContext;
        wrl::ComPtr<IDXGISwapChain> swapChain;

        wrl::ComPtr<ID3D11RenderTargetView> backBufferView;
        wrl::ComPtr<ID3D11DepthStencilView> depthStencilView;

        glm::vec4 clearColor = {0, 0, 0, 0};
        float depthClearValue = 1.0f;
        int stencilClearValue = 0;
    };
}