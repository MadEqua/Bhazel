#include "bzpch.h"

#include "D3D11Buffer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"
#include "Bhazel/Renderer/Shader.h"


namespace BZ {

    D3D11VertexBuffer::D3D11VertexBuffer(float *vertices, uint32 size, const BufferLayout &layout) :
        VertexBuffer(layout),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = layout.getStride();

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = vertices;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateBuffer(&bufferDesc, &data, &bufferPtr));
    }

    void D3D11VertexBuffer::bind(uint32 unit) const {
        UINT stride = layout.getStride();
        UINT offset = 0;
        BZ_LOG_DXGI(context.getDeviceContext()->IASetVertexBuffers(unit, 1, bufferPtr.GetAddressOf(), &stride, &offset));
    }

    void D3D11VertexBuffer::unbind(uint32 unit) const {
        UINT dummy;
        BZ_LOG_DXGI(context.getDeviceContext()->IASetVertexBuffers(unit, 1, nullptr, &dummy, &dummy));
    }


    D3D11IndexBuffer::D3D11IndexBuffer(uint32 *indices, uint32 count) :
        IndexBuffer(count),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {
        
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = sizeof(uint32);

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = indices;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateBuffer(&bufferDesc, &data, &bufferPtr));
    }

    void D3D11IndexBuffer::bind(uint32 unit) const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetIndexBuffer(bufferPtr.Get(), DXGI_FORMAT_R32_UINT, 0));
    }

    void D3D11IndexBuffer::unbind(uint32 unit) const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
    }


    D3D11ConstantBuffer::D3D11ConstantBuffer(uint32 size) :
        D3D11ConstantBuffer(nullptr, size)  {
    }

    D3D11ConstantBuffer::D3D11ConstantBuffer(void *data, uint32 size) :
        ConstantBuffer(size),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {

        BZ_ASSERT_CORE(!(size % 16), "D3D11ConstantBuffer needs to have a size multiple of 16 bytes.")

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA resData = {};
        resData.pSysMem = data;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateBuffer(&bufferDesc, data ? &resData : nullptr, &bufferPtr));
    }

    void D3D11ConstantBuffer::bind(uint32 unit) const {
        //TODO: binding to VS and PS. Better solution needed.
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetConstantBuffers(unit, 1, bufferPtr.GetAddressOf()));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetConstantBuffers(unit, 1, bufferPtr.GetAddressOf()));
    }

    void D3D11ConstantBuffer::unbind(uint32 unit) const {
        BZ_LOG_DXGI(context.getDeviceContext()->VSSetConstantBuffers(unit, 1, nullptr));
        BZ_LOG_DXGI(context.getDeviceContext()->PSSetConstantBuffers(unit, 1, nullptr));
    }

    void D3D11ConstantBuffer::setData(const void *data, uint32 size) {
        BZ_ASSERT_CORE(size <= this->size, "Trying to overflow the buffer!");
        BZ_LOG_DXGI(context.getDeviceContext()->UpdateSubresource(bufferPtr.Get(), 0, nullptr, data, 0, 0));
    }
}
