#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "D3D11Includes.h"


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
        wrl::ComPtr<ID3D11DeviceContext> deviceContext;
        wrl::ComPtr<IDXGISwapChain> swapChain;

        //TESTING
        void testinit();
        void testdraw();

        wrl::ComPtr<ID3D11Buffer> vertexBuffer;
        wrl::ComPtr<ID3D11VertexShader> vertexShader;
        wrl::ComPtr<ID3D11PixelShader> pixelShader;

    };
}