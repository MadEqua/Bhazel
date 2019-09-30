#include "bzpch.h"

#include "D3D11InputDescription.h"
#include "Bhazel/Platform/D3D11/D3D11Shader.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    static DXGI_FORMAT shaderDataTypeToD3D11(DataType dataType, DataElements dataElements, bool normalized);

    D3D11InputDescription::D3D11InputDescription() :
        context(static_cast<D3D11Context&>(Application::getInstance().getGraphicsContext())) {
    }

    void D3D11InputDescription::bindToPipeline() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetInputLayout(inputLayoutPtr.Get()));
        
        for(auto &vb : vertexBuffers) {
            vb->bindToPipeline();
        }

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
            ied.Format = shaderDataTypeToD3D11(element.dataType, element.dataElements, element.normalized);
            ied.InputSlot = static_cast<UINT>(vertexBuffers.size());
            ied.AlignedByteOffset = element.offset;
            ied.InputSlotClass = element.perInstanceStep > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
            ied.InstanceDataStepRate = element.perInstanceStep;

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

    static DXGI_FORMAT shaderDataTypeToD3D11(DataType dataType, DataElements dataElements, bool normalized) {
        switch(dataType) {
        case DataType::Float32: {
            switch(dataElements) {
            case DataElements::Scalar:
                return DXGI_FORMAT_R32_FLOAT;
            case DataElements::Vec2:
                return DXGI_FORMAT_R32G32_FLOAT;
            case DataElements::Vec3:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case DataElements::Vec4:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Float16: {
            switch(dataElements) {
            case DataElements::Scalar:
                return DXGI_FORMAT_R16_FLOAT;
            case DataElements::Vec2:
                return DXGI_FORMAT_R16G16_FLOAT;
            case DataElements::Vec3:
                BZ_ASSERT_ALWAYS_CORE("D3D11 does not support Vec3 of Float16.");
                return DXGI_FORMAT_UNKNOWN;
            case DataElements::Vec4:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Int32: {
            switch(dataElements) {
            case DataElements::Scalar:
                return DXGI_FORMAT_R32_SINT;
            case DataElements::Vec2:
                return DXGI_FORMAT_R32G32_SINT;
            case DataElements::Vec3:
                return DXGI_FORMAT_R32G32B32_SINT;
            case DataElements::Vec4:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Int16: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? DXGI_FORMAT_R16_SNORM : DXGI_FORMAT_R16_SINT;
            case DataElements::Vec2:
                return normalized ? DXGI_FORMAT_R16G16_SNORM : DXGI_FORMAT_R16G16_SINT;
            case DataElements::Vec3:
                BZ_ASSERT_ALWAYS_CORE("D3D11 does not support Vec3 of Int16.");
                return DXGI_FORMAT_UNKNOWN;
            case DataElements::Vec4:
                return normalized ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_SINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Int8: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_SINT;
            case DataElements::Vec2:
                return normalized ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_SINT;
            case DataElements::Vec3:
                BZ_ASSERT_ALWAYS_CORE("D3D11 does not support Vec3 of Int8.");
                return DXGI_FORMAT_UNKNOWN;
            case DataElements::Vec4:
                return normalized ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_SINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Uint32: {
            switch(dataElements) {
            case DataElements::Scalar:
                return DXGI_FORMAT_R32_UINT;
            case DataElements::Vec2:
                return DXGI_FORMAT_R32G32_UINT;
            case DataElements::Vec3:
                return DXGI_FORMAT_R32G32B32_UINT;
            case DataElements::Vec4:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Uint16: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R16_UINT;
            case DataElements::Vec2:
                return normalized ? DXGI_FORMAT_R16G16_UNORM : DXGI_FORMAT_R16G16_UINT;
            case DataElements::Vec3:
                BZ_ASSERT_ALWAYS_CORE("D3D11 does not support Vec3 of Uint16.");
                return DXGI_FORMAT_UNKNOWN;
            case DataElements::Vec4:
                return normalized ? DXGI_FORMAT_R16G16B16A16_UNORM : DXGI_FORMAT_R16G16B16A16_UINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        case DataType::Uint8: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8_UINT;
            case DataElements::Vec2:
                return normalized ? DXGI_FORMAT_R8G8_UNORM : DXGI_FORMAT_R8G8_UINT;
            case DataElements::Vec3:
                BZ_ASSERT_ALWAYS_CORE("D3D11 does not support Vec3 of Uint8.");
                return DXGI_FORMAT_UNKNOWN;
            case DataElements::Vec4:
                return normalized ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("D3D11 matrix vertex attributes not implemented.");
                return DXGI_FORMAT_UNKNOWN;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}