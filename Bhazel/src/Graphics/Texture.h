#pragma once


namespace BZ {

    enum class TextureFormat {
        Undefined,
        R8, R8_sRGB,
        R8G8, R8G8_sRGB,
        R8G8B8, R8G8B8_sRGB,
        R8G8B8A8, R8G8B8A8_sRGB,
        B8G8R8A8, B8G8R8A8_sRGB,
        D16S8,
        D24S8,
    };

    struct TextureFormatWrapper {
        TextureFormatWrapper(TextureFormat format);

        TextureFormat format;

        //TextureFormat operator()() { return format; }
        bool isColor() const;
        bool isDepthStencil() const;
    };

    enum class FilterMode {
        Nearest, Linear
    };

    enum class AddressMode {
        Repeat, 
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
        MirrorClampToEdge
    };


    class Texture {
    public:
        virtual ~Texture() = default;

        const TextureFormatWrapper& getFormat() const { return format; }

        uint32 getWidth() const { return dimensions.x; }
        uint32 getHeight() const { return dimensions.y; }
        uint32 getDepth() const { return dimensions.z; }

 
    protected:
        explicit Texture(TextureFormat format);

        static const byte* loadFile(const char* path, bool flip, int &widthOut, int &heightOut);
        static void freeData(const byte *data);

        TextureFormatWrapper format;
        glm::ivec3 dimensions = {1,1,1};
    };


    class Texture2D : public Texture {
    public:
        static Ref<Texture2D> create(const std::string &path, TextureFormat format);
        static Ref<Texture2D> create(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format);

    protected:
        explicit Texture2D(TextureFormat format);
    };


    class TextureView {
    public:
        static Ref<TextureView> create(const Ref<Texture> &texture);

        //TextureFormat getFormat() const { return texture->getFormat(); } TODO: TextureView own format
        const TextureFormatWrapper& getTextureFormat() const { return texture->getFormat(); }
        Ref<Texture> getTexture() const { return texture; }

    protected:
        explicit TextureView(const Ref<Texture> &texture);
        virtual ~TextureView() = default;

        Ref<Texture> texture;
    };


    class Sampler {
    public:
        class Builder {
            void setMinFilterMode(FilterMode filterMode) { minFilter = filterMode; }
            void setMagFilterMode(FilterMode filterMode) { magFilter = filterMode; }
            void setMipmapFilterMode(FilterMode filterMode) { mipmapFilter = filterMode; }

            void setAddressModeAll(AddressMode addressMode) { addressModeU = addressMode; addressModeV = addressMode; addressModeW = addressMode; }
            void setAddressModeU(AddressMode addressMode) { addressModeU = addressMode; }
            void setAddressModeV(AddressMode addressMode) { addressModeV = addressMode; }
            void setAddressModeW(AddressMode addressMode) { addressModeW = addressMode; }

            void setUnnormalizedCoordinates(bool enable) { unnormalizedCoordinate = enable; }

        private:
            FilterMode minFilter = FilterMode::Linear;
            FilterMode magFilter = FilterMode::Linear;
            FilterMode mipmapFilter = FilterMode::Linear;
            AddressMode addressModeU  = AddressMode::Repeat;
            AddressMode addressModeV = AddressMode::Repeat;
            AddressMode addressModeW = AddressMode::Repeat;
            bool unnormalizedCoordinate = false;

            friend class VulkanSampler;
        };

        static Ref<Sampler> create(const Builder &builder);

    protected:
        virtual ~Sampler() = default;
    };
}