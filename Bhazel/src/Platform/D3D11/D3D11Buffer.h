#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include "Bhazel/Platform/D3D11/D3D11Context.h"
#include "Bhazel/Platform/D3D11/D3D11Includes.h"


namespace BZ {
    
    class D3D11Buffer : public Buffer {
    public:
        D3D11Buffer(BufferType type, uint32 size);
        D3D11Buffer(BufferType type, uint32 size, const void *data);
        D3D11Buffer(BufferType type, uint32 size, const void *data, const BufferLayout &layout);

        virtual void setData(const void *data, uint32 size) override;

        virtual void bindToPipeline(uint32 unit = 0) const override;
        virtual void unbindFromPipeline(uint32 unit = 0) const override;

        virtual void bindToPipelineAsGeneric(uint32 unit = 0) const override;
        virtual void unbindFromPipelineAsGeneric(uint32 unit = 0) const override;

    private:
        D3D11Context &context;
        wrl::ComPtr<ID3D11Buffer> bufferPtr;

        mutable wrl::ComPtr<ID3D11UnorderedAccessView> unorderedAccessViewPtr;
    };
}