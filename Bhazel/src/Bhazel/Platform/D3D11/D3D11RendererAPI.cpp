#include "bzpch.h"

#include "D3D11RendererAPI.h"
#include "D3D11Context.h"

#include "Bhazel/Renderer/Renderer.h"


namespace BZ {

    D3D11RendererAPI::D3D11RendererAPI(D3D11Context &context, ID3D11RenderTargetView *backBufferView, ID3D11DepthStencilView *depthStencilView) :
        context(context), device(context.getDevice()), deviceContext(context.getDeviceContext()), swapChain(context.getSwapChain()),
        backBufferView (backBufferView), depthStencilView (depthStencilView) {
    }

    void D3D11RendererAPI::setClearColor(const glm::vec4& color) {
        clearColor = color;
    }

    void D3D11RendererAPI::setDepthClearValue(float value) {
        depthClearValue = value;
    }

    void D3D11RendererAPI::setStencilClearValue(int value) {
        stencilClearValue = value;
    }

    void D3D11RendererAPI::clearColorAndDepthStencilBuffers() {
        deviceContext->ClearRenderTargetView(backBufferView.Get(), reinterpret_cast<float*>(&clearColor));
        deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClearValue, stencilClearValue);
    }

    void D3D11RendererAPI::setViewport(int left, int top, int width, int height) {
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = static_cast<FLOAT>(left);
        viewport.TopLeftY = static_cast<FLOAT>(top);
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);
        viewport.MinDepth = 0;
        viewport.MaxDepth = 1;
        BZ_LOG_DXGI(deviceContext->RSSetViewports(1, &viewport));
    }

    void D3D11RendererAPI::setRenderMode(RenderMode mode) {
        D3D_PRIMITIVE_TOPOLOGY topology;
        switch(mode) {
        case RenderMode::Points:
            topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case BZ::RenderMode::Lines:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case BZ::RenderMode::Triangles:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        default:
            BZ_LOG_ERROR("Unknown RenderMode. Setting triangles.");
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        BZ_LOG_DXGI(deviceContext->IASetPrimitiveTopology(topology));
    }

    void D3D11RendererAPI::drawIndexed(const Ref<InputDescription> &inputDesc) {
        deviceContext->DrawIndexed(inputDesc->getIndexBuffer()->getCount(), 0, 0);
    }
}