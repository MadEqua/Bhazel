#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    enum class DataType {
        Float32, Float16,
        Int32, Int16, Int8,
        Uint32, Uint16, Uint8
    };

    enum class DataElements {
        Scalar,
        Vec2, Vec3, Vec4,
        Mat2, Mat3, Mat4
    };

    class DataElement {
    public:
        DataElement(DataType dataType, DataElements dataElements, bool normalized = false);

        const DataType& getDataType() const { return dataType; }

        uint32 getDataTypeSizeBytes() const;
        uint32 getElementCount() const;
        uint32 getSizeBytes() const { return sizeBytes; }
        uint32 getOffsetBytes() const { return offsetBytes; }

        VkFormat toVkFormat() const;

    private:
        DataType dataType;
        DataElements dataElements;
        bool normalized;

        uint32 sizeBytes;
        uint32 offsetBytes;

        friend class DataLayout;
    };


    /*-------------------------------------------------------------------------------------------*/
    class DataLayout {
    public:
        DataLayout() = default;
        DataLayout(const std::initializer_list<DataElement>& elements, VkVertexInputRate vertexInputRate = VK_VERTEX_INPUT_RATE_VERTEX);

        const auto& getElements() const { return elements; }
        uint32 getElementCount() const { return static_cast<uint32>(elements.size()); }
        uint32 getSizeBytes() const { return sizeBytes; }
        VkVertexInputRate getVertexInputRate() const { return vertexInputRate; }

        std::vector<DataElement>::iterator begin() { return elements.begin(); }
        std::vector<DataElement>::iterator end() { return elements.end(); }
        std::vector<DataElement>::const_iterator begin() const { return elements.cbegin(); }
        std::vector<DataElement>::const_iterator end() const { return elements.cend(); }

    private:
        std::vector<DataElement> elements;
        uint32 sizeBytes = 0;
        VkVertexInputRate vertexInputRate;

        void calculateOffsetsAndStride();
    };



    /*-------------------------------------------------------------------------------------------*/
    class Buffer;

    /*
    * This will wrap pointers to mapped buffer memory.
    * It will hide from the client the existence of different replicas for dynamic buffers.
    */
    class BufferPtr {
    public:
        BufferPtr() = default;
        BufferPtr(Buffer &buffer, byte* const basePtr) : buffer(&buffer), basePtr(basePtr) {}

        operator void*() const;
        operator byte*() const;

    private:
        Buffer *buffer = nullptr;
        byte* basePtr = nullptr;

        friend BufferPtr operator+(const BufferPtr &lhs, int offset);
        friend BufferPtr operator-(const BufferPtr &lhs, int offset);
        friend BufferPtr operator+(const BufferPtr &lhs, uint32 offset);
        friend BufferPtr operator-(const BufferPtr &lhs, uint32 offset);
        friend BufferPtr operator+(const BufferPtr &lhs, uint64 offset);
        friend BufferPtr operator-(const BufferPtr &lhs, uint64 offset);
    };

    BufferPtr operator+(const BufferPtr &lhs, int offset);
    BufferPtr operator-(const BufferPtr &lhs, int offset);
    BufferPtr operator+(const BufferPtr &lhs, uint32 offset);
    BufferPtr operator-(const BufferPtr &lhs, uint32 offset);
    BufferPtr operator+(const BufferPtr &lhs, uint64 offset);
    BufferPtr operator-(const BufferPtr &lhs, uint64 offset);


    /*-------------------------------------------------------------------------------------------*/
    enum class MemoryType {
        GpuOnly,
        CpuToGpu,
        GpuToCpu,
        Staging //Same as CpuToGpu but without the replicas.
    };

    struct BufferHandles {
        VkBuffer bufferHandle;
        VmaAllocation allocationHandle;
    };

    /*
    * Generic buffer of data. Will replicate data if considered dynamic (one replica per frame in-flight) to avoid race conditions.
    */
    class Buffer : public GpuObject<BufferHandles> {
    public:
        static Ref<Buffer> create(VkBufferUsageFlags usageFlags, uint32 size, MemoryType memoryType);
        static Ref<Buffer> create(VkBufferUsageFlags usageFlags, uint32 size, MemoryType memoryType, const DataLayout &layout);

        Buffer(VkBufferUsageFlags usageFlags, uint32 size, MemoryType memoryType, const DataLayout *layout);
        ~Buffer();

        BZ_NON_COPYABLE(Buffer);

        void setData(const void *data, uint32 dataSize, uint32 offset);
        BufferPtr map(uint32 offset);
        void unmap();

        uint32 getDimensions() const { return size; }
        uint32 getRealSize() const { return realSize; }

        bool isReplicated() const { return memoryType == MemoryType::CpuToGpu || memoryType == MemoryType::GpuToCpu; }
        bool isMappable() const { return memoryType == MemoryType::CpuToGpu || memoryType == MemoryType::GpuToCpu || memoryType == MemoryType::Staging; }

        const DataLayout& getLayout() const { return layout; }

        uint32 getCurrentBaseOfReplicaOffset() const;

        //VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
        bool isVertex() const { return BZ_FLAG_CHECK(usageFlags, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT); }
        bool isIndex() const { return BZ_FLAG_CHECK(usageFlags, VK_BUFFER_USAGE_INDEX_BUFFER_BIT); }
        bool isUniform() const { return BZ_FLAG_CHECK(usageFlags, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT); }

    private:
        VkBufferUsageFlags usageFlags;
        MemoryType memoryType;

        uint32 size;
        uint32 realSize;

        DataLayout layout;

        bool isMapped = false;

        VkBufferUsageFlags toRequiredVkBufferUsageFlags() const;
        VkBufferUsageFlags toPreferredVkBufferUsageFlags() const;
    };
}