#include "bzpch.h"

#include "D3D11RendererAPI.h"
#include "D3D11Context.h"


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

    void D3D11RendererAPI::drawIndexed(const Ref<InputDescription> &inputDesc) {
        deviceContext->DrawIndexed(inputDesc->getIndexBuffer()->getCount(), 0, 0);
    }
}