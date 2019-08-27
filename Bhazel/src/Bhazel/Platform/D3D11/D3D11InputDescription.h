#pragma once

#include "Bhazel/Renderer/InputDescription.h"
#include "Bhazel/Renderer/Shader.h"
#include "Bhazel/Renderer/Buffer.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {

    class D3D11InputDescription : public InputDescription
    {
    public:
        D3D11InputDescription();

        virtual void bind() const override;
        virtual void unbind() const override;

        virtual void addVertexBuffer(const Ref<VertexBuffer> &buffer, const Ref<Shader> &vertexShader) override;
        virtual void setIndexBuffer(const Ref<IndexBuffer> &buffer) override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11InputLayout> inputLayoutPtr;

        std::vector<D3D11_INPUT_ELEMENT_DESC> ieds;

        static DXGI_FORMAT shaderDataTypeToD3D11(ShaderDataType dataType, bool normalized);
    };
}