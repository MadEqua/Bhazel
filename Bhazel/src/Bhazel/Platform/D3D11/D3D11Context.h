#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include <d3d11.h>


namespace BZ {

    class D3D11Context : public GraphicsContext {
    public:
        explicit D3D11Context(HWND windowHandle);
        ~D3D11Context() override;

        virtual void swapBuffers() override;

        ID3D11Device* getDevice() { return device; }
        ID3D11DeviceContext* getDeviceContext() { return deviceContext; }

    private:
        HWND windowHandle;

        ID3D11Device *device = nullptr;
        IDXGISwapChain *swapChain = nullptr;
        ID3D11DeviceContext *deviceContext = nullptr;
    };
}