#pragma once

#include "Bhazel/Renderer/Shader.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {

    class D3D11Shader : public Shader {
    public:
        D3D11Shader(const std::string &vertexSrc, const std::string &fragmentSrc);

        void bindToPipeline() const;
        void unbindFromPipeline() const;

        ID3DBlob* getVertexShaderBlob() { return vertexShaderBlobPtr.Get(); }

    private:
        D3D11Context &context;

        wrl::ComPtr<ID3DBlob> vertexShaderBlobPtr; //Used for InputDescription 
        wrl::ComPtr<ID3D11VertexShader> vertexShaderPtr;
        wrl::ComPtr<ID3D11PixelShader> pixelShaderPtr;
    };
}