#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include <d3d11.h>
#include <wrl.h>

namespace wrl = Microsoft::WRL;


namespace BZ {

    class D3D11Context : public GraphicsContext {
    public:
        explicit D3D11Context(HWND windowHandle);

        virtual void swapBuffers() override;

        ID3D11Device* getDevice() { return device.Get(); }
        ID3D11DeviceContext* getDeviceContext() { return deviceContext.Get(); }

    private:
        HWND windowHandle;

        wrl::ComPtr<ID3D11Device> device;
        wrl::ComPtr<IDXGISwapChain> swapChain;
        wrl::ComPtr<ID3D11DeviceContext> deviceContext;

        //TESTING
        void testinit();
        void testdraw();

        wrl::ComPtr<ID3D11Buffer> vertexBuffer;
        wrl::ComPtr<ID3D11VertexShader> vertexShader;
        wrl::ComPtr<ID3D11PixelShader> pixelShader;

        wrl::ComPtr<ID3D11RenderTargetView> backBufferView;
    };
}