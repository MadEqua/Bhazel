#include "bzpch.h"

#include "D3D11Buffer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"
#include "Bhazel/Renderer/Shader.h"


namespace BZ {

    static uint32 bufferTypeToD3D11(BufferType bufferType);


    D3D11Buffer::D3D11Buffer(BufferType type, uint32 size) :
        D3D11Buffer(type, size, nullptr, BufferLayout()) {
    }

    D3D11Buffer::D3D11Buffer(BufferType type, uint32 size, const void *data) :
        D3D11Buffer(type, size, data, BufferLayout()) {
    }

    D3D11Buffer::D3D11Buffer(BufferType type, uint32 size, const void *data, const BufferLayout &layout) :
        Buffer(type, size, layout),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: correct usage flags
        bufferDesc.BindFlags = bufferTypeToD3D11(type);
        bufferDesc.CPUAccessFlags = 0;

        //UINT stride = 0;
        UINT miscFlags = 0;
        switch(type) {
        case BufferType::Vertex:
            //stride = layout.getStride();
            miscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            break;
        case BufferType::Index:
            //stride = sizeof(uint32);
            miscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            break;
        }

        //bufferDesc.StructureByteStride = stride;
        bufferDesc.MiscFlags = miscFlags;

        D3D11_SUBRESOURCE_DATA resData = {};
        resData.pSysMem = data;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateBuffer(&bufferDesc, data ? &resData : nullptr, &bufferPtr));
    }

    void D3D11Buffer::bindToPipeline(uint32 unit) const {

        switch(type) {
        case BufferType::Vertex:
        {
            UINT stride = layout.getStride();
            UINT offset = 0;
            unbindFromPipelineAsGeneric();
            BZ_LOG_DXGI(context.getDeviceContext()->IASetVertexBuffers(unit, 1, bufferPtr.GetAddressOf(), &stride, &offset));
        }
            break;
        case BufferType::Index:
            BZ_LOG_DXGI(context.getDeviceContext()->IASetIndexBuffer(bufferPtr.Get(), DXGI_FORMAT_R32_UINT, 0));
            break;
        case BufferType::Constant:
            //TODO: binding to all shaders. Better solution needed.
            BZ_LOG_DXGI(context.getDeviceContext()->VSSetConstantBuffers(unit, 1, bufferPtr.GetAddressOf()));
            BZ_LOG_DXGI(context.getDeviceContext()->PSSetConstantBuffers(unit, 1, bufferPtr.GetAddressOf()));
            BZ_LOG_DXGI(context.getDeviceContext()->CSSetConstantBuffers(unit, 1, bufferPtr.GetAddressOf()));
            break;
        default:
            break;
        }
    }

    void D3D11Buffer::unbindFromPipeline(uint32 unit) const {
        ID3D11Buffer* nullViews[] = {nullptr};

        switch(type) {
        case BufferType::Vertex:
            UINT dummy;
            BZ_LOG_DXGI(context.getDeviceContext()->IASetVertexBuffers(unit, 1, nullViews, &dummy, &dummy));
            break;
        case BufferType::Index:
            BZ_LOG_DXGI(context.getDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
            break;
        case BufferType::Constant:
            //TODO: binding to all shaders. Better solution needed.
            BZ_LOG_DXGI(context.getDeviceContext()->VSSetConstantBuffers(unit, 1, nullViews));
            BZ_LOG_DXGI(context.getDeviceContext()->PSSetConstantBuffers(unit, 1, nullViews));
            BZ_LOG_DXGI(context.getDeviceContext()->CSSetConstantBuffers(unit, 1, nullViews));
            break;
        default:
            break;
        }
    }

    void D3D11Buffer::bindToPipelineAsGeneric(uint32 unit) const {
        if(!unorderedAccessViewPtr) {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferPtr->GetDesc(&bufferDesc);

            uint32 stride = layout.getStride();

            D3D11_UNORDERED_ACCESS_VIEW_DESC descUnorderedAccessView = {};
            descUnorderedAccessView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            descUnorderedAccessView.Buffer.FirstElement = 0;
            descUnorderedAccessView.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
            descUnorderedAccessView.Buffer.NumElements = stride > 0 ? bufferDesc.ByteWidth / stride : bufferDesc.ByteWidth;
            descUnorderedAccessView.Format = DXGI_FORMAT_R32_TYPELESS;
            BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateUnorderedAccessView(bufferPtr.Get(), &descUnorderedAccessView, &unorderedAccessViewPtr));
        }

        unbindFromPipeline(unit);
        BZ_LOG_DXGI(context.getDeviceContext()->CSSetUnorderedAccessViews(unit, 1, unorderedAccessViewPtr.GetAddressOf(), nullptr));
    }

    void D3D11Buffer::unbindFromPipelineAsGeneric(uint32 unit) const {
        ID3D11UnorderedAccessView* nullViews[] = {nullptr};
        BZ_LOG_DXGI(context.getDeviceContext()->CSSetUnorderedAccessViews(unit, 1, nullViews, nullptr));
    }

    void D3D11Buffer::setData(const void *data, uint32 size) {
        BZ_ASSERT_CORE(size <= this->size, "Trying to overflow the buffer!");
        BZ_LOG_DXGI(context.getDeviceContext()->UpdateSubresource(bufferPtr.Get(), 0, nullptr, data, 0, 0));
    }

    static uint32 bufferTypeToD3D11(BufferType bufferType) {
        switch(bufferType) {
        case BufferType::Vertex:
            return D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
        case BZ::BufferType::Index:
            return D3D11_BIND_INDEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
        case BZ::BufferType::Constant:
            return D3D11_BIND_CONSTANT_BUFFER;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BufferType!");
        }
    }
}
