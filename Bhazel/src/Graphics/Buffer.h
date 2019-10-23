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
        Vertex, Index, Constant
    };

    class Buffer {
    public:
        virtual ~Buffer() = default;

        //Utility create functions
        static Ref<Buffer> createVertexBuffer(const void *data, uint32 size, const DataLayout& layout, bool dynamic);
        static Ref<Buffer> createIndexBuffer(const void *data, uint32 size, bool dynamic);
        static Ref<Buffer> createConstantBuffer(uint32 size, bool dynamic);

        static Ref<Buffer> create(BufferType type, uint32 size, const void *data, bool dynamic);
        static Ref<Buffer> create(BufferType type, uint32 size, const void *data, const DataLayout& layout, bool dynamic);

        virtual void setData(const void *data, uint32 size, uint32 offset = 0) = 0;

        uint32 getSize() const { return size; }
        BufferType getType() const { return type; }
        const DataLayout& getLayout() const { return layout; }

    protected:
        uint32 size;
        BufferType type;
        DataLayout layout;
        bool dynamic;

        Buffer(BufferType type, uint32 size, bool dynamic);
        Buffer(BufferType type, uint32 size, const DataLayout &layout, bool dynamic);
    };
}