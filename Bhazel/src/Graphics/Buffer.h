#pragma once


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

    enum class DataRate {
        PerVertex, PerInstance
    };


    class DataElement {
    public:
        DataType dataType;
        DataElements dataElements;
        std::string name;
        bool normalized;

        DataElement(DataType dataType, DataElements dataElements, const char *name, bool normalized = false);

        uint32 getDataTypeSizeBytes() const;
        uint32 getElementCount() const;
        uint32 getSizeBytes() const { return sizeBytes; }
        uint32 getOffsetBytes() const { return offsetBytes; }

    private:
        uint32 sizeBytes;
        uint32 offsetBytes;

        friend class DataLayout;
    };


    class DataLayout {
    public:
        DataLayout() = default;
        DataLayout(const std::initializer_list<DataElement>& elements, DataRate dataRate = DataRate::PerVertex);

        const auto& getElements() const { return elements; }
        uint32 getElementCount() const { return static_cast<uint32>(elements.size()); }
        uint32 getSizeBytes() const { return sizeBytes; }
        DataRate getDataRate() const { return dataRate; }

        std::vector<DataElement>::iterator begin() { return elements.begin(); }
        std::vector<DataElement>::iterator end() { return elements.end(); }
        std::vector<DataElement>::const_iterator begin() const { return elements.cbegin(); }
        std::vector<DataElement>::const_iterator end() const { return elements.cend(); }

    private:
        std::vector<DataElement> elements;
        uint32 sizeBytes;
        DataRate dataRate;

        void calculateOffsetsAndStride();
    };

    enum class BufferType {
        Vertex,
        Index,
        Constant
    };

    enum class MemoryType {
        GpuOnly,
        CpuToGpu,
        GpuToCpu
    };


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
        Buffer *buffer;
        byte* basePtr;

        friend BufferPtr operator+(const BufferPtr &lhs, uint32 offset);
        friend BufferPtr operator-(const BufferPtr &lhs, uint32 offset);
    };

    BufferPtr operator+(const BufferPtr &lhs, uint32 offset);
    BufferPtr operator-(const BufferPtr &lhs, uint32 offset);


    /*
    * Generic buffer of data. Will replicate data if considered dynamic (one replica per frame in-flight) to avoid race conditions.
    * Dynamic buffers assume that the data will change *every* frame, so the buffer client must set new data every frame.
    */
    class Buffer {
    public:
        static Ref<Buffer> create(BufferType type, uint32 size, MemoryType memoryType);
        static Ref<Buffer> create(BufferType type, uint32 size, MemoryType memoryType, const DataLayout &layout);

        void setData(const void *data, uint32 dataSize, uint32 offset);
        BufferPtr map(uint32 offset);
        void unmap();

        uint32 getDimensions() const { return size; }
        uint32 getRealSize() const { return realSize; }

        bool isDynamic() const { return memoryType != MemoryType::GpuOnly; }
        const DataLayout& getLayout() const { return layout; }

        uint32 getCurrentBaseOfReplicaOffset() const;
        BufferType getType() const { return type; }

    protected:
        BufferType type;

        uint32 size;
        uint32 realSize;

        DataLayout layout;
        MemoryType memoryType;

        bool isMapped = false;

        Buffer(BufferType type, uint32 size, MemoryType memoryType, const DataLayout *layout);
        virtual ~Buffer() = default;

        //void initBufferData(const void *data);

        virtual void internalSetData(const void *data, uint32 dataSize, uint32 offset) = 0;
        virtual BufferPtr internalMap(uint32 offset) = 0;
        virtual void internalUnmap() = 0;
    };
}