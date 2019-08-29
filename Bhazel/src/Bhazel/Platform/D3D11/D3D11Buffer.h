#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {
    
    class D3D11VertexBuffer : public VertexBuffer {
    public:
        D3D11VertexBuffer(float *vertices, uint32 size, const BufferLayout &layout);

        virtual void bindToPipeline(uint32 unit = 0) const override;
        virtual void unbindFromPipeline(uint32 unit = 0) const override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11Buffer> bufferPtr;
    };


    class D3D11IndexBuffer : public IndexBuffer {
    public:
        D3D11IndexBuffer(uint32 *indices, uint32 count);

        virtual void bindToPipeline(uint32 unit = 0) const override;
        virtual void unbindFromPipeline(uint32 unit = 0) const override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11Buffer> bufferPtr;
    };


    class D3D11ConstantBuffer : public ConstantBuffer {
    public:
        explicit D3D11ConstantBuffer(uint32 size);
        D3D11ConstantBuffer(void *data, uint32 size);

        virtual void bindToPipeline(uint32 unit = 0) const override;
        virtual void unbindFromPipeline(uint32 unit = 0) const override;

        virtual void setData(const void *data, uint32 size) override;

        ID3D11Buffer* getNativeResource() { return bufferPtr.Get(); }

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11Buffer> bufferPtr;
    };
}