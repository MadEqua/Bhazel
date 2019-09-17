#include "bzpch.h"

#include "D3D11Shader.h"

#include "Bhazel/Core/Utils.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    static const char* shaderTypeToD3D(ShaderType shaderType);

    D3D11Shader::D3D11Shader(const std::string &filePath) :
        Shader(Utils::getFileNameFromPath(filePath)),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {

        auto &sources = readAndPreprocessFile(filePath);
        compile(sources);
    }

    D3D11Shader::D3D11Shader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc) :
        Shader(name),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {
        std::unordered_map<ShaderType, std::string> sources;
        sources[ShaderType::Vertex] = vertexSrc;
        sources[ShaderType::Fragment] = fragmentSrc;
        compile(sources);
    }

    void D3D11Shader::bindToPipeline() const {
        BZ_LOG_DXGI(context.getDeviceContext()->CSSetShader(computeShaderPtr.Get(), nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetShader(vertexShaderPtr.Get(), nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShader(pixelShaderPtr.Get(), nullptr, 0));
    }

    void D3D11Shader::unbindFromPipeline() const {
        BZ_LOG_DXGI(context.getDeviceContext()->CSSetShader(nullptr, nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetShader(nullptr, nullptr, 0));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetShader(nullptr, nullptr, 0));
    }

    void D3D11Shader::compile(const std::unordered_map<ShaderType, std::string> &sources) {
        BZ_ASSERT_CORE(sources.size() <= 5, "Shader sources need to have at maximum 5 entries!")
        BZ_ASSERT_CORE(sources.find(ShaderType::Vertex) != sources.end() || sources.find(ShaderType::Compute) != sources.end(), "Shader code should contain at least a Vertex or Compute shader!")
        BZ_ASSERT_CORE(!(sources.find(ShaderType::Compute) != sources.end() && sources.size() > 1), "Compute shaders should be used as standalone!")

        UINT flags1 = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef BZ_DIST
        flags1 |= D3DCOMPILE_DEBUG;
#endif

        wrl::ComPtr<ID3DBlob> errorsBlob;

        for(auto &kv : sources) {

            wrl::ComPtr<ID3DBlob> byteCodeBlob;

            BZ_LOG_HRES_DXGI(D3DCompile(
                kv.second.c_str(),
                kv.second.length(),
                nullptr,
                nullptr,
                nullptr,
                "main", shaderTypeToD3D(kv.first),
                flags1, 0,
                &byteCodeBlob,
                &errorsBlob));

            if(errorsBlob.Get()) {
                BZ_ASSERT_ALWAYS_CORE("Shader compilation error:\n{0}", static_cast<char*>(errorsBlob->GetBufferPointer()));
            }

            switch(kv.first) {
            case ShaderType::Vertex:
                vertexShaderByteCodeBlobPtr = byteCodeBlob;
                BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateVertexShader(byteCodeBlob->GetBufferPointer(),
                                    byteCodeBlob->GetBufferSize(), nullptr, &vertexShaderPtr));
                break;
            case ShaderType::Fragment:
                BZ_ASSERT_HRES_DXGI(context.getDevice()->CreatePixelShader(byteCodeBlob->GetBufferPointer(),
                                    byteCodeBlob->GetBufferSize(), nullptr, &pixelShaderPtr));
                break;
            case ShaderType::Compute:
                BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateComputeShader(byteCodeBlob->GetBufferPointer(),
                                    byteCodeBlob->GetBufferSize(), nullptr, &computeShaderPtr));
                break;
            default:
                BZ_ASSERT_ALWAYS("Unknown ShaderType!")
                break;
            }
        }
    }

    static const char* shaderTypeToD3D(ShaderType shaderType) {
        switch(shaderType)
        {
        case ShaderType::Vertex:
            return "vs_5_0";
        case ShaderType::Fragment:
            return "ps_5_0";
        case ShaderType::Compute:
            return "cs_5_0";
        default:
            BZ_ASSERT_ALWAYS("Unknown ShaderType!")
        }
    }
}