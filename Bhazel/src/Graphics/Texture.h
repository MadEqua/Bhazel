#pragma once


namespace BZ {

    enum class TextureFormatEnum {
        Undefined,
        R8, R8_sRGB,
        R8G8, R8G8_sRGB,
        R8G8B8, R8G8B8_sRGB,
        R8G8B8A8, R8G8B8A8_sRGB,
        B8G8R8A8, B8G8R8A8_sRGB,
        D16S8,
        D24S8,
    };

    struct TextureFormat {
        TextureFormat(TextureFormatEnum format);

        TextureFormatEnum format;

        TextureFormatEnum operator()() { return format; }
        bool isColor() const;
        bool isDepthStencil() const;
    };

    class Texture {
    public:
        virtual ~Texture() = default;

        const TextureFormat& getFormat() const { return format; }

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

        TextureFormat getFormat() const { return texture->getFormat(); } //TODO: have its own format?
        Ref<Texture> getTexture() const { return texture; }

    protected:
        TextureView(const Ref<Texture> &texture);

        Ref<Texture> texture;
    };
}