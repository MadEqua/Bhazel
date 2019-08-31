#pragma once

#include "Bhazel/Renderer/RendererAPI.h"

#include "D3D11Includes.h"


namespace BZ {

    class D3D11Context;
    enum class BlendingFunction;
    enum class BlendingEquation;
    enum class TestFunction;

    class D3D11RendererAPI : public RendererAPI
    {
    public:
        D3D11RendererAPI(D3D11Context &context);

        virtual void setClearColor(const glm::vec4& color) override;
        virtual void clearColorBuffer() override;
        virtual void clearDepthBuffer() override;
        virtual void clearStencilBuffer() override;
        virtual void clearColorAndDepthStencilBuffers() override;

        virtual void setBlendingSettings(BlendingSettings &settings) override;
        virtual void setDepthSettings(DepthSettings &settings) override;

        virtual void setViewport(int left, int top, int width, int height) override;

        virtual void setRenderMode(Renderer::RenderMode mode) override;

        virtual void drawIndexed(uint32 indicesCount) override;

    private:
        D3D11Context &context;

        ID3D11Device *device;
        ID3D11DeviceContext *deviceContext;
        IDXGISwapChain *swapChain;

        glm::vec4 clearColor = {0, 0, 0, 0};
        float depthClearValue = 1.0f;
        int stencilClearValue = 0;

        D3D11_BLEND blendingFunctionToD3D(BlendingFunction blendingFunction);
        D3D11_BLEND_OP blendingEquationToD3D(BlendingEquation blendingEquation);
        D3D11_COMPARISON_FUNC testFunctionToD3D(TestFunction testFunction);
    };
}