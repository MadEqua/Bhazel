#pragma once

#include "Bhazel/Renderer/InputDescription.h"
#include "Bhazel/Renderer/Shader.h"
#include "Bhazel/Renderer/Buffer.h"

#include "Bhazel/Platform/D3D11/D3D11Context.h"
#include "Bhazel/Platform/D3D11/D3D11Includes.h"


namespace BZ {

    class D3D11InputDescription : public InputDescription
    {
    public:
        D3D11InputDescription();

        virtual void bindToPipeline() const override;
        virtual void unbindFromPipeline() const override;

        virtual void addVertexBuffer(const Ref<Buffer> &buffer, const Ref<Shader> &vertexShader) override;
        virtual void setIndexBuffer(const Ref<Buffer> &buffer) override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11InputLayout> inputLayoutPtr;

        std::vector<D3D11_INPUT_ELEMENT_DESC> ieds;
    };
}