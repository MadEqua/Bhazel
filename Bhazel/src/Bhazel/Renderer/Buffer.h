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

    class BufferElement {
    public:
        DataType dataType;
        DataElements dataElements;
        std::string name;
        bool normalized;
        uint32 perInstanceStep;
        
        uint32 sizeBytes;
        uint32 offset;

        BufferElement(DataType dataType, DataElements dataElements, const std::string &name, bool normalized = false, uint32 perInstanceStep = 0);
        uint32 getElementCount() const;
    };


    class BufferLayout {
    public:
        BufferLayout() = default;
        BufferLayout(const std::initializer_list<BufferElement> &elements);

        const auto& getElements() const { return elements; }
        uint32 getElementCount() const { return static_cast<uint32>(elements.size()); }
        uint32 getStride() const { return stride; }

        std::vector<BufferElement>::iterator begin() { return elements.begin(); }
        std::vector<BufferElement>::iterator end() { return elements.end(); }
        std::vector<BufferElement>::const_iterator begin() const { return elements.cbegin(); }
        std::vector<BufferElement>::const_iterator end() const { return elements.cend(); }

    private:
        std::vector<BufferElement> elements;
        uint32 stride;

        void calculateOffsetsAndStride();
    };


    class Buffer {
    public:
        virtual ~Buffer() = default;

        //Utility create functions
        static Ref<Buffer> createVertexBuffer(const void *data, uint32 size, const BufferLayout &layout);
        static Ref<Buffer> createIndexBuffer(const void *data, uint32 size);
        static Ref<Buffer> createConstantBuffer(uint32 size);

        static Ref<Buffer> create(BufferType type, uint32 size);
        static Ref<Buffer> create(BufferType type, uint32 size, const void *data);
        static Ref<Buffer> create(BufferType type, uint32 size, const void *data, const BufferLayout &layout);

        virtual void setData(const void *data, uint32 size) = 0;

        virtual void bindToPipeline(uint32 unit = 0) const = 0;
        virtual void unbindFromPipeline(uint32 unit = 0) const = 0;

        //Bind and unbind as generic Read/Write buffers. Useful for compute shader usage.
        virtual void bindToPipelineAsGeneric(uint32 unit = 0) const = 0;
        virtual void unbindFromPipelineAsGeneric(uint32 unit = 0) const = 0;

        uint32 getSize() const { return size; }
        BufferType getType() const { return type; }
        const BufferLayout& getLayout() const { return layout; }

    protected:
        uint32 size;
        BufferType type;
        BufferLayout layout;

        Buffer(BufferType type, uint32 size) : type(type), size(size) {}
        Buffer(BufferType type, uint32 size, const BufferLayout &layout) : type(type), size(size), layout(layout) {}
    };
}