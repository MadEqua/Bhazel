#include "bzpch.h"

#include "D3D11Shader.h"

//#include <glm/gtc/type_ptr.hpp>

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    D3D11Shader::D3D11Shader(const std::string &vertexSrc, const std::string &fragmentSrc) :
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {

        //TODO: error checking
        BZ_ASSERT_HRES_DXGI(D3DCompile(
            vertexSrc.c_str(),
            vertexSrc.length(),
            nullptr,
            nullptr,
            nullptr,
            "main", "vs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &vertexShaderBlobPtr,
            nullptr));

        wrl::ComPtr<ID3DBlob> fsBlob;
        BZ_ASSERT_HRES_DXGI(D3DCompile(
            fragmentSrc.c_str(),
            fragmentSrc.length(),
            nullptr,
            nullptr,
            nullptr,
            "main", "ps_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &fsBlob,
            nullptr));

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateVertexShader(vertexShaderBlobPtr->GetBufferPointer(), vertexShaderBlobPtr->GetBufferSize(), nullptr, &vertexShaderPtr));
        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreatePixelShader(fsBlob->GetBufferPointer(), fsBlob->GetBufferSize(), nullptr, &pixelShaderPtr));
    }

    void D3D11Shader::bind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetShader(vertexShaderPtr.Get(), nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShader(pixelShaderPtr.Get(), nullptr, 0));
    }

    void D3D11Shader::unbind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetShader(nullptr, nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShader(nullptr, nullptr, 0));
    }

    /*void D3D11Shader::setUniformInt(const std::string &name, int v) {
    }

    void D3D11Shader::setUniformFloat(const std::string &name, float v) {
    }

    void D3D11Shader::setUniformFloat2(const std::string &name, const glm::vec2 &vec) {
    }

    void D3D11Shader::setUniformFloat3(const std::string &name, const glm::vec3 &vec) {
    }

    void D3D11Shader::setUniformFloat4(const std::string &name, const glm::vec4 &vec) {
    }

    void D3D11Shader::setUniformMat3(const std::string &name, const glm::mat3 &mat) {
    }

    void D3D11Shader::setUniformMat4(const std::string &name, const glm::mat4 &mat) {
    }*/
}