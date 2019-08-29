#pragma once

#include "Bhazel/Renderer/Texture.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {

    class D3D11Texture2D : public Texture2D
    {
    public:
        explicit D3D11Texture2D(const std::string &path);

        virtual void bind(uint32 unit) const override;

    private:
        std::string path;

        D3D11Context &context;
        wrl::ComPtr<ID3D11Texture2D> texturePtr;
        wrl::ComPtr<ID3D11ShaderResourceView> textureViewPtr;
        wrl::ComPtr<ID3D11SamplerState> samplerViewPtr;
    };
}