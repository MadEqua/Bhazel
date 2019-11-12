#pragma once

#include "Bhazel/Renderer/Texture.h"

#include "Bhazel/Platform/D3D11/D3D11Context.h"
#include "Bhazel/Platform/D3D11/D3D11Debug.h"


namespace BZ {

    class D3D11Texture2D : public Texture2D
    {
    public:
        explicit D3D11Texture2D(const std::string &path);

        virtual void bindToPipeline(uint32 unit) const override;

    private:
        std::string path;

        D3D11Context &context;
        wrl::ComPtr<ID3D11Texture2D> texturePtr;
        wrl::ComPtr<ID3D11ShaderResourceView> textureViewPtr;
        wrl::ComPtr<ID3D11SamplerState> samplerViewPtr;
    };
}