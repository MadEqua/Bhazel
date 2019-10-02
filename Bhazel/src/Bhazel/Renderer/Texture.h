#pragma once


namespace BZ {

    enum class TextureFormat {
        Unknown
    };

    class Texture {
    public:
        virtual ~Texture() = default;

        TextureFormat getFormat() const { return format; }

        uint32 getWidth() const { return dimensions.x; }
        uint32 getHeight() const { return dimensions.y; }
        uint32 getDepth() const { return dimensions.z; }

    protected:
        Texture(TextureFormat format);

        const byte* loadFile(const char* path, bool flip, int &widthOut, int &heightOut);

        TextureFormat format;
        glm::ivec3 dimensions = {1,1,1};
    };


    class Texture2D : public Texture {
    public:
        static Ref<Texture2D> create(const std::string &path, TextureFormat format);

    protected:
        Texture2D(TextureFormat format);
    };


    class TextureView {
    public:
        static Ref<TextureView> create(const Ref<Texture> &texture);
        virtual ~TextureView() = default;

        Ref<Texture> getTexture() const { return texture; }

    protected:
        TextureView(const Ref<Texture> &texture);

        Ref<Texture> texture;
    };
}