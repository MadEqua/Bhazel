#pragma once

#include <vector>
#include <string>

namespace BZ {

    enum class ShaderDataType {
        Float, Int, Bool,
        Vec2i, Vec3i, Vec4i,
        Vec2, Vec3, Vec4,
        Mat2, Mat3, Mat4
    };

    static uint32 shaderDataTypeSize(ShaderDataType type) {
        switch(type)
        {
        case ShaderDataType::Float:
            return sizeof(float);
        case ShaderDataType::Int:
            return sizeof(int);
        case ShaderDataType::Bool:
            return sizeof(bool);
        case ShaderDataType::Vec2i:
            return sizeof(int) * 2;
        case ShaderDataType::Vec3i:
            return sizeof(int) * 3;
        case ShaderDataType::Vec4i:
            return sizeof(int) * 4;
        case ShaderDataType::Vec2:
            return sizeof(float) * 2;
        case ShaderDataType::Vec3:
            return sizeof(float) * 3;
        case ShaderDataType::Vec4:
            return sizeof(float) * 4;
        case ShaderDataType::Mat2:
            return sizeof(float) * 4;
        case ShaderDataType::Mat3:
            return sizeof(float) * 9;
        case ShaderDataType::Mat4:
            return sizeof(float) * 16;
        default:
            BZ_CORE_ASSERT_ALWAYS("Unknown ShaderDataType.");
            return 0;
        }
    }


    class BufferElement {
    public:
        ShaderDataType dataType;
        std::string name;
        uint32 sizeBytes;
        uint32 offset;
        bool normalized;

        BufferElement(ShaderDataType dataType, const std::string &name, bool normalized = false) :
            dataType(dataType), name(name), sizeBytes(shaderDataTypeSize(dataType)), offset(0), normalized(normalized) {
        }

        uint32 getElementCount() const;
    };


    class BufferLayout {
    public:
        BufferLayout() = default;
        BufferLayout(const std::initializer_list<BufferElement> &elements) : 
            elements(elements), stride(0) {
            calculateOffsetsAndStride();
        }

        const auto& getElements() const { return elements; }
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

        //virtual void setData() = 0;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;
    };


    class VertexBuffer : public Buffer {
    public:
        static Ref<VertexBuffer> create(float *vertices, uint32 size);

        void setLayout(const BufferLayout &layout) { this->layout = layout; }
        const BufferLayout& getLayout() const { return layout; };

    protected:
        BufferLayout layout;
    };


    class IndexBuffer : public Buffer {
    public:
        static Ref<IndexBuffer> create(uint32 *indices, uint32 count);

        uint32 getCount() const { return count; }

    protected:
        IndexBuffer(uint32 count) : count(count) {}

        uint32 count;
    };
}