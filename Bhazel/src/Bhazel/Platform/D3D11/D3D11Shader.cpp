#include "bzpch.h"

#include "D3D11Shader.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"

#include "D3D11Buffer.h"


namespace BZ {

    D3D11Shader::D3D11Shader(const std::string &vertexSrc, const std::string &fragmentSrc) :
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {


        UINT flags1 = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef BZ_DIST
        flags1 |= D3DCOMPILE_DEBUG;
#endif

        wrl::ComPtr<ID3DBlob> errorsBlob;

        BZ_LOG_HRES_DXGI(D3DCompile(
            vertexSrc.c_str(),
            vertexSrc.length(),
            nullptr,
            nullptr,
            nullptr,
            "main", "vs_5_0",
            flags1, 0,
            &vertexShaderBlobPtr,
            &errorsBlob));

        if(errorsBlob.Get()) {
            BZ_ASSERT_ALWAYS_CORE("Vertex shader compilation error:\n{0}", static_cast<char*>(errorsBlob->GetBufferPointer()));
        }

        wrl::ComPtr<ID3DBlob> fsBlob;
        BZ_LOG_HRES_DXGI(D3DCompile(
            fragmentSrc.c_str(),
            fragmentSrc.length(),
            nullptr,
            nullptr,
            nullptr,
            "main", "ps_5_0",
            flags1, 0,
            &fsBlob,
            &errorsBlob));

        if(errorsBlob.Get()) {
            BZ_ASSERT_ALWAYS_CORE("Pixel shader compilation error:\n{0}", static_cast<char*>(errorsBlob->GetBufferPointer()));
        }

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateVertexShader(vertexShaderBlobPtr->GetBufferPointer(), vertexShaderBlobPtr->GetBufferSize(), nullptr, &vertexShaderPtr));
        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreatePixelShader(fsBlob->GetBufferPointer(), fsBlob->GetBufferSize(), nullptr, &pixelShaderPtr));
    }

    void D3D11Shader::bind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetShader(vertexShaderPtr.Get(), nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShader(pixelShaderPtr.Get(), nullptr, 0));

        //Bind the associated Constant Buffers
        int i = 0;
        for(auto &constantBuffer : vsConstantBuffers) {
            D3D11ConstantBuffer &d3dConstantBuffer = static_cast<D3D11ConstantBuffer&>(*constantBuffer);
            bufferArray[i++] = d3dConstantBuffer.getNativeResource();
        }
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetConstantBuffers(0, static_cast<UINT>(vsConstantBuffers.size()), bufferArray));

        i = 0;
        for(auto &constantBuffer : fsConstantBuffers) {
            D3D11ConstantBuffer &d3dConstantBuffer = static_cast<D3D11ConstantBuffer&>(*constantBuffer);
            bufferArray[i++] = d3dConstantBuffer.getNativeResource();
        }
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetConstantBuffers(0, static_cast<UINT>(fsConstantBuffers.size()), bufferArray));
    }

    void D3D11Shader::unbind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetShader(nullptr, nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShader(nullptr, nullptr, 0));
    }
}