#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {
    
    class D3D11VertexBuffer : public VertexBuffer {
    public:
        D3D11VertexBuffer(float *vertices, uint32 size, const BufferLayout &layout);

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11Buffer> bufferPtr;
    };


    class D3D11IndexBuffer : public IndexBuffer {
    public:
        D3D11IndexBuffer(uint32 *indices, uint32 count);

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11Buffer> bufferPtr;
    };
}