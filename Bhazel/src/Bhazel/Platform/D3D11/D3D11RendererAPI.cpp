#include "bzpch.h"

#include "D3D11RendererAPI.h"
#include "D3D11Context.h"

#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Renderer/PipelineSettings.h"


namespace BZ {

    static D3D11_BLEND blendingFunctionToD3D(BlendingFunction blendingFunction);
    static D3D11_BLEND_OP blendingEquationToD3D(BlendingEquation blendingEquation);
    static D3D11_COMPARISON_FUNC testFunctionToD3D(TestFunction testFunction);

    D3D11RendererAPI::D3D11RendererAPI(D3D11Context &context) :
        context(context), device(context.getDevice()), deviceContext(context.getDeviceContext()), swapChain(context.getSwapChain()) {
    }

    void D3D11RendererAPI::setClearColor(const glm::vec4& color) {
        clearColor = color;
    }

    void D3D11RendererAPI::clearColorBuffer() {
        BZ_LOG_DXGI(deviceContext->ClearRenderTargetView(context.getBackBufferView(), reinterpret_cast<float*>(&clearColor)));
    }

    void D3D11RendererAPI::clearDepthBuffer() {
        BZ_LOG_DXGI(deviceContext->ClearDepthStencilView(context.getDepthStencilView(), D3D11_CLEAR_DEPTH, depthClearValue, stencilClearValue));
    }

    void D3D11RendererAPI::clearStencilBuffer() {
        BZ_LOG_DXGI(deviceContext->ClearDepthStencilView(context.getDepthStencilView(), D3D11_CLEAR_STENCIL, depthClearValue, stencilClearValue));
    }

    void D3D11RendererAPI::clearColorAndDepthStencilBuffers() {
        BZ_LOG_DXGI(deviceContext->ClearRenderTargetView(context.getBackBufferView(), reinterpret_cast<float*>(&clearColor)));
        BZ_LOG_DXGI(deviceContext->ClearDepthStencilView(context.getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClearValue, stencilClearValue));
    }

    void D3D11RendererAPI::setBlendingSettings(BlendingSettings &settings) {
        const BlendingSetting &rgbSetting = settings.settingRGB;
        const BlendingSetting &alphaSetting = settings.settingAlpha;

        D3D11_BLEND_DESC blendDesc = {0};
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].BlendEnable = settings.enableBlending;
        if(settings.enableBlending) {
            blendDesc.RenderTarget[0].SrcBlend = blendingFunctionToD3D(rgbSetting.sourceBlendingFunction);
            blendDesc.RenderTarget[0].DestBlend = blendingFunctionToD3D(rgbSetting.destinationBlendingFunction);
            blendDesc.RenderTarget[0].BlendOp = blendingEquationToD3D(rgbSetting.blendingEquation);
            blendDesc.RenderTarget[0].SrcBlendAlpha = blendingFunctionToD3D(alphaSetting.sourceBlendingFunction);
            blendDesc.RenderTarget[0].DestBlendAlpha = blendingFunctionToD3D(alphaSetting.destinationBlendingFunction);
            blendDesc.RenderTarget[0].BlendOpAlpha = blendingEquationToD3D(alphaSetting.blendingEquation);
        }
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        wrl::ComPtr<ID3D11BlendState> blendState;
        BZ_ASSERT_HRES_DXGI(device->CreateBlendState(&blendDesc, &blendState));
        BZ_LOG_DXGI(deviceContext->OMSetBlendState(blendState.Get(), nullptr, 0xffffffff));
    }

    void D3D11RendererAPI::setDepthSettings(DepthSettings &settings) {
        depthClearValue = settings.depthClearValue;

        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        wrl::ComPtr<ID3D11DepthStencilState> dsState;
        UINT stencilRef;
        BZ_LOG_DXGI(deviceContext->OMGetDepthStencilState(&dsState, &stencilRef));
        if(dsState) 
            BZ_LOG_DXGI(dsState->GetDesc(&dsDesc));

        dsDesc.DepthEnable = settings.enableDepthTest;
        dsDesc.DepthFunc = testFunctionToD3D(settings.testFunction);
        dsDesc.DepthWriteMask = settings.enableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

        BZ_ASSERT_HRES_DXGI(device->CreateDepthStencilState(&dsDesc, &dsState));
        BZ_LOG_DXGI(deviceContext->OMSetDepthStencilState(dsState.Get(), stencilRef));
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

    void D3D11RendererAPI::setRenderMode(Renderer::RenderMode mode) {
        D3D_PRIMITIVE_TOPOLOGY topology;
        switch(mode) {
        case Renderer::RenderMode::Points:
            topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case Renderer::RenderMode::Lines:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case Renderer::RenderMode::Triangles:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case Renderer::RenderMode::LineStrip:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case Renderer::RenderMode::TriangleStrip:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        default:
            BZ_LOG_ERROR("Unknown RenderMode. Setting triangles.");
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        BZ_LOG_DXGI(deviceContext->IASetPrimitiveTopology(topology));
    }

    void D3D11RendererAPI::draw(uint32 vertexCount) {
        BZ_LOG_DXGI(deviceContext->Draw(vertexCount, 0));
    }

    void D3D11RendererAPI::drawIndexed(uint32 indicesCount) {
        BZ_LOG_DXGI(deviceContext->DrawIndexed(indicesCount, 0, 0));
    }

    void D3D11RendererAPI::submitCompute(uint32 groupsX, uint32 groupsY, uint32 groupsZ) {
        //TODO
        BZ_ASSERT_ALWAYS_CORE("Not implemented yet!");
    }

    static D3D11_BLEND blendingFunctionToD3D(BlendingFunction blendingFunction) {
        switch(blendingFunction) {
        case BlendingFunction::Zero:
            return D3D11_BLEND_ZERO;
        case BlendingFunction::One:
            return D3D11_BLEND_ONE;
        case BlendingFunction::SourceColor:
            return D3D11_BLEND_SRC_COLOR;
        case BlendingFunction::OneMinusSourceColor:
            return D3D11_BLEND_INV_SRC_COLOR;
        case BlendingFunction::DestinationColor:
            return D3D11_BLEND_DEST_COLOR;
        case BlendingFunction::OneMinusDestinationColor:
            return D3D11_BLEND_INV_DEST_COLOR;
        case BlendingFunction::SourceAlpha:
            return D3D11_BLEND_SRC_ALPHA;
        case BlendingFunction::OneMinusSourceAlpha:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case BlendingFunction::DestinationAlpha:
            return D3D11_BLEND_DEST_ALPHA;
        case BlendingFunction::OneMinusDestinationAlpha:
            return D3D11_BLEND_INV_DEST_ALPHA;
        case BlendingFunction::ConstantColor:
            BZ_ASSERT_ALWAYS_CORE("Blending mode not implemented on D3D11!");
        case BlendingFunction::OneMinusConstantColor:
            BZ_ASSERT_ALWAYS_CORE("Blending mode not implemented on D3D11!");
        case BlendingFunction::ConstantAlpha:
            BZ_ASSERT_ALWAYS_CORE("Blending mode not implemented on D3D11!");
        case BlendingFunction::OneMinusConstantAlpha:
            BZ_ASSERT_ALWAYS_CORE("Blending mode not implemented on D3D11!");
        case BlendingFunction::AlphaSaturate:
            return D3D11_BLEND_SRC_ALPHA_SAT;
        case BlendingFunction::Source1Color:
            return D3D11_BLEND_SRC1_COLOR;
        case BlendingFunction::OneMinusSource1Color:
            return D3D11_BLEND_INV_SRC1_COLOR;
        case BlendingFunction::Source1Alpha:
            return D3D11_BLEND_SRC1_ALPHA;
        case BlendingFunction::OneMinusSource1Alpha:
            return D3D11_BLEND_INV_SRC1_ALPHA;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BlendingFunction!");
        }
    }

    static D3D11_BLEND_OP blendingEquationToD3D(BlendingEquation blendingEquation) {
        switch(blendingEquation) {
        case BlendingEquation::Add:
            return D3D11_BLEND_OP_ADD;
        case BlendingEquation::SourceMinusDestination:
            return D3D11_BLEND_OP_SUBTRACT;
        case BlendingEquation::DestinationMinusSource:
            return D3D11_BLEND_OP_REV_SUBTRACT;
        case BlendingEquation::Min:
            return D3D11_BLEND_OP_MIN;
        case BlendingEquation::Max:
            return D3D11_BLEND_OP_MAX;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BlendingEquation!");
        }
    }

    static D3D11_COMPARISON_FUNC testFunctionToD3D(TestFunction testFunction) {
        switch(testFunction)
        {
        case TestFunction::Always:
            return D3D11_COMPARISON_ALWAYS;
        case TestFunction::Never:
            return D3D11_COMPARISON_NEVER;
        case TestFunction::Less:
            return D3D11_COMPARISON_LESS;
        case TestFunction::LessOrEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case TestFunction::Greater:
            return D3D11_COMPARISON_GREATER;
        case TestFunction::GreaterOrEqual:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case TestFunction::Equal:
            return D3D11_COMPARISON_EQUAL;
        case TestFunction::NotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TestFunction!");
        }
    }
}