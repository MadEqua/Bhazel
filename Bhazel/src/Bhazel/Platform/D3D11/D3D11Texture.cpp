#include "bzpch.h"

#include "D3D11Texture.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"
#include "Bhazel/Renderer/Shader.h"

#include <stb_image.h>


namespace BZ {

    D3D11Texture2D::D3D11Texture2D(const std::string &path) :
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())),
        path(path) {

        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
        BZ_ASSERT_CORE(data, "Failed to load image '{0}'.", path);

        this->width = width;
        this->height = height;

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_IMMUTABLE;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA resData = {};
        resData.pSysMem = data;
        resData.SysMemPitch = width * sizeof(uint8) * 4;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateTexture2D(&texDesc, &resData, &texturePtr));
        stbi_image_free(data);

        /*D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        viewDesc.Format = texDesc.Format;
        viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MostDetailedMip = 0;
        viewDesc.Texture2D.MipLevels = 1;*/

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateShaderResourceView(texturePtr.Get(), nullptr, &textureViewPtr));

        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        sampDesc.MipLODBias = 0.0f;
        sampDesc.MaxAnisotropy = 0;
        //sampDesc.BorderColor;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateSamplerState(&sampDesc, &samplerViewPtr));
    }

    void D3D11Texture2D::bindToPipeline(uint32 unit) const {
        //TODO: only binding to Pixel Shader. What to do?
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShaderResources(unit, 1, textureViewPtr.GetAddressOf()));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetSamplers(unit, 1, samplerViewPtr.GetAddressOf()));
    }
}