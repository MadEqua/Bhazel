#pragma once

#include "Bhazel/Renderer/Shader.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {

    class D3D11Shader : public Shader {
    public:
        explicit D3D11Shader(const std::string &filePath);
        D3D11Shader(const std::string &vertexSrc, const std::string &fragmentSrc);

        void bindToPipeline() const;
        void unbindFromPipeline() const;

    private:
        D3D11Context &context;

        wrl::ComPtr<ID3DBlob> vertexShaderByteCodeBlobPtr; //Used for InputDescription

        wrl::ComPtr<ID3D11VertexShader> vertexShaderPtr;
        wrl::ComPtr<ID3D11PixelShader> pixelShaderPtr;

        void compile(const std::unordered_map<ShaderType, std::string> &sources);

        friend class D3D11InputDescription;
    };
}