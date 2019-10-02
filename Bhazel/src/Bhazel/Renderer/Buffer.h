#pragma once


namespace BZ {

    enum class BufferType {
        Vertex, Index, Constant
    };

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

    struct DataElement {
        DataType dataType;
        DataElements dataElements;
        std::string name;
        bool normalized;

        uint32 sizeBytes;
        uint32 offsetBytes;

        DataElement(DataType dataType, DataElements dataElements, const std::string& name, bool normalized = false);
        uint32 getElementCount() const;
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


    class Buffer {
    public:
        virtual ~Buffer() = default;

        //Utility create functions
        static Ref<Buffer> createVertexBuffer(const void *data, uint32 size, const DataLayout& layout);
        static Ref<Buffer> createIndexBuffer(const void *data, uint32 size);
        static Ref<Buffer> createConstantBuffer(uint32 size);

        static Ref<Buffer> create(BufferType type, uint32 size);
        static Ref<Buffer> create(BufferType type, uint32 size, const void *data);
        static Ref<Buffer> create(BufferType type, uint32 size, const void *data, const DataLayout& layout);

        virtual void setData(const void *data, uint32 size) = 0;

        uint32 getSize() const { return size; }
        BufferType getType() const { return type; }
        const DataLayout& getLayout() const { return layout; }

    protected:
        uint32 size;
        BufferType type;
        DataLayout layout;

        Buffer(BufferType type, uint32 size) : type(type), size(size) {}
        Buffer(BufferType type, uint32 size, const DataLayout&layout) : type(type), size(size), layout(layout) {}
    };
}