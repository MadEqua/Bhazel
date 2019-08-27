#include "bzpch.h"

#include "D3D11InputDescription.h"
#include "D3D11Shader.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    D3D11InputDescription::D3D11InputDescription() :
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {
    }

    void D3D11InputDescription::bind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetInputLayout(inputLayoutPtr.Get()));
        
        for(auto &vb : vertexBuffers)
            vb->bind();

        if(indexBuffer)
            indexBuffer->bind();
    }

    void D3D11InputDescription::unbind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetInputLayout(nullptr));

        for(auto &vb : vertexBuffers)
            vb->unbind();

        if(indexBuffer)
            indexBuffer->unbind();
    }

    void D3D11InputDescription::addVertexBuffer(const Ref<VertexBuffer> &buffer, const Ref<Shader> &vertexShader) {
        BZ_ASSERT_CORE(buffer->getLayout().getElementCount(), "VertexBuffer has no layout.");

        for(const auto &element : buffer->getLayout()) {
            D3D11_INPUT_ELEMENT_DESC ied = {0};
            ied.SemanticName = element.name.c_str();
            ied.SemanticIndex = 0; //TODO: inexistent on abstract API
            ied.Format = shaderDataTypeToD3D11(element.dataType, element.normalized);
            ied.InputSlot = static_cast<UINT>(vertexBuffers.size());
            ied.AlignedByteOffset = element.offset;
            ied.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            ied.InstanceDataStepRate = 0;

            ieds.emplace_back(ied);
        }

        const Ref<D3D11Shader> &d3d11VertexShader = std::static_pointer_cast<D3D11Shader>(vertexShader);
        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateInputLayout(ieds.data(), static_cast<UINT>(ieds.size()),
                            d3d11VertexShader->getVertexShaderBlob()->GetBufferPointer(),
                            d3d11VertexShader->getVertexShaderBlob()->GetBufferSize(),
                            &inputLayoutPtr));

        vertexBuffers.emplace_back(buffer);
    }

    void D3D11InputDescription::setIndexBuffer(const Ref<IndexBuffer> &buffer) {
        indexBuffer = buffer;
    }

    DXGI_FORMAT D3D11InputDescription::shaderDataTypeToD3D11(ShaderDataType dataType, bool normalized) {
        switch(dataType)
        {
        case ShaderDataType::Float:
            return DXGI_FORMAT_R32_FLOAT;

        case ShaderDataType::Int:
            return  DXGI_FORMAT_R32_SINT;
        case ShaderDataType::Int16:
            return normalized ? DXGI_FORMAT_R16_SNORM : DXGI_FORMAT_R16_SINT;
        case ShaderDataType::Int8:
            return normalized ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_SINT;

        case ShaderDataType::Uint:
            return  DXGI_FORMAT_R32_UINT;
        case ShaderDataType::Uint16:
            return normalized ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R16_UINT;
        case ShaderDataType::Uint8:
            return normalized ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8_UINT;
        
        case ShaderDataType::Bool:
            return DXGI_FORMAT_R32_UINT;

        case ShaderDataType::Vec2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case ShaderDataType::Vec3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case ShaderDataType::Vec4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case ShaderDataType::Vec2i:
            return DXGI_FORMAT_R32G32_SINT;
        case ShaderDataType::Vec3i:
            return DXGI_FORMAT_R32G32B32_SINT;
        case ShaderDataType::Vec4i:
            return DXGI_FORMAT_R32G32B32A32_SINT;

        case ShaderDataType::Vec2ui:
            return DXGI_FORMAT_R32G32_UINT;
        case ShaderDataType::Vec3ui:
            return DXGI_FORMAT_R32G32B32_UINT;
        case ShaderDataType::Vec4ui:
            return DXGI_FORMAT_R32G32B32A32_UINT;

        case ShaderDataType::Mat2:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4:
            BZ_LOG_CORE_ERROR("D3D11 matrix vertex attributes not implemented.");
            return DXGI_FORMAT_UNKNOWN;

        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown ShaderDataType.");
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}