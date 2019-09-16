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

    void D3D11InputDescription::bindToPipeline() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetInputLayout(inputLayoutPtr.Get()));
        
        for(auto &vb : vertexBuffers)
            vb->bindToPipeline();

        if(indexBuffer)
            indexBuffer->bindToPipeline();
    }

    void D3D11InputDescription::unbindFromPipeline() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetInputLayout(nullptr));

        for(auto &vb : vertexBuffers)
            vb->unbindFromPipeline();

        if(indexBuffer)
            indexBuffer->unbindFromPipeline();
    }

    void D3D11InputDescription::addVertexBuffer(const Ref<Buffer> &buffer, const Ref<Shader> &vertexShader) {
        BZ_ASSERT_CORE(buffer->getLayout().getElementCount(), "VertexBuffer has no layout.");

        for(const auto &element : buffer->getLayout()) {
            D3D11_INPUT_ELEMENT_DESC ied = {};
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
                            d3d11VertexShader->vertexShaderByteCodeBlobPtr->GetBufferPointer(),
                            d3d11VertexShader->vertexShaderByteCodeBlobPtr->GetBufferSize(),
                            &inputLayoutPtr));

        vertexBuffers.emplace_back(buffer);
    }

    void D3D11InputDescription::setIndexBuffer(const Ref<Buffer> &buffer) {
        indexBuffer = buffer;
    }

    DXGI_FORMAT D3D11InputDescription::shaderDataTypeToD3D11(DataType dataType, bool normalized) {
        switch(dataType)
        {
        case DataType::Float:
            return DXGI_FORMAT_R32_FLOAT;

        case DataType::Int:
            return  DXGI_FORMAT_R32_SINT;
        case DataType::Int16:
            return normalized ? DXGI_FORMAT_R16_SNORM : DXGI_FORMAT_R16_SINT;
        case DataType::Int8:
            return normalized ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_SINT;

        case DataType::Uint:
            return  DXGI_FORMAT_R32_UINT;
        case DataType::Uint16:
            return normalized ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R16_UINT;
        case DataType::Uint8:
            return normalized ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8_UINT;
        
        case DataType::Bool:
            return DXGI_FORMAT_R32_UINT;

        case DataType::Vec2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case DataType::Vec3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case DataType::Vec4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case DataType::Vec2i:
            return DXGI_FORMAT_R32G32_SINT;
        case DataType::Vec3i:
            return DXGI_FORMAT_R32G32B32_SINT;
        case DataType::Vec4i:
            return DXGI_FORMAT_R32G32B32A32_SINT;

        case DataType::Vec2ui:
            return DXGI_FORMAT_R32G32_UINT;
        case DataType::Vec3ui:
            return DXGI_FORMAT_R32G32B32_UINT;
        case DataType::Vec4ui:
            return DXGI_FORMAT_R32G32B32A32_UINT;

        case DataType::Mat2:
        case DataType::Mat3:
        case DataType::Mat4:
            BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
            return DXGI_FORMAT_UNKNOWN;

        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}