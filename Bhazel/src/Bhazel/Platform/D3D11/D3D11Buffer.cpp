#include "bzpch.h"

#include "D3D11Buffer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    D3D11VertexBuffer::D3D11VertexBuffer(float *vertices, uint32 size, const BufferLayout &layout) :
        VertexBuffer(layout),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {

        D3D11_BUFFER_DESC bufferDesc = {0};
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = layout.getStride();

        D3D11_SUBRESOURCE_DATA data = {0};
        data.pSysMem = vertices;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateBuffer(&bufferDesc, &data, &bufferPtr));
    }

    void D3D11VertexBuffer::bind() const {
        UINT stride = layout.getStride();
        UINT offset = 0;
        BZ_LOG_DXGI(context.getDeviceContext()->IASetVertexBuffers(0, 1, bufferPtr.GetAddressOf(), &stride, &offset));
    }

    void D3D11VertexBuffer::unbind() const {
        UINT dummy;
        BZ_LOG_DXGI(context.getDeviceContext()->IASetVertexBuffers(0, 0, nullptr, &dummy, &dummy));
    }


    D3D11IndexBuffer::D3D11IndexBuffer(uint32 *indices, uint32 count) :
        IndexBuffer(count),
        context(static_cast<D3D11Context&>(Application::getInstance().getWindow().getGraphicsContext())) {
        
        D3D11_BUFFER_DESC bufferDesc = {0};
        bufferDesc.ByteWidth = sizeof(uint32) * count;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = sizeof(uint32);

        D3D11_SUBRESOURCE_DATA data = {0};
        data.pSysMem = indices;

        BZ_ASSERT_HRES_DXGI(context.getDevice()->CreateBuffer(&bufferDesc, &data, &bufferPtr));
    }

    void D3D11IndexBuffer::bind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetIndexBuffer(bufferPtr.Get(), DXGI_FORMAT_R32_UINT, 0));
    }

    void D3D11IndexBuffer::unbind() const {
        BZ_LOG_DXGI(context.getDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
    }
}