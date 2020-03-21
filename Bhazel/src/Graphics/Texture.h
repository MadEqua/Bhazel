#pragma once


namespace BZ {

    enum class TextureFormat {
        Undefined,
        R8, R8_SRGB,
        R8G8, R8G8_SRGB,
        R8G8B8, R8G8B8_SRGB,
        R8G8B8A8, R8G8B8A8_SRGB,
        B8G8R8A8, B8G8R8A8_SRGB,
        D32, D16S8, D24S8,
    };

    struct TextureFormatWrapper {
        TextureFormatWrapper(TextureFormat format);

        TextureFormat format;

        //TextureFormat operator()() { return format; }
        bool isColor() const;
        bool isDepth() const;
        bool isStencil() const;
        bool isDepthStencil() const;
        bool isDepthOnly() const;
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

        const glm::ivec3& getDimensions() const { return dimensions; }
        uint32 getWidth() const { return dimensions.x; }
        uint32 getHeight() const { return dimensions.y; }
        uint32 getDepth() const { return dimensions.z; }
        uint32 getMipLevels() const { return mipLevels; }

    protected:
        explicit Texture(TextureFormat format);

        static const byte* loadFile(const char* path, bool flip, int &widthOut, int &heightOut);
        static void freeData(const byte *data);

        TextureFormatWrapper format;
        glm::ivec3 dimensions = { 1, 1, 1 };
        uint32 mipLevels = 1;
    };


    class Texture2D : public Texture {
    public:
        static Ref<Texture2D> create(const std::string &path, TextureFormat format, bool generateMipmaps);
        static Ref<Texture2D> create(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format, bool generateMipmaps);
        
        static Ref<Texture2D> createRenderTarget(uint32 width, uint32 height, TextureFormat format);

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
        public:
            void setMinFilterMode(FilterMode filterMode) { minFilter = filterMode; }
            void setMagFilterMode(FilterMode filterMode) { magFilter = filterMode; }

            void setMipmapFilterMode(FilterMode filterMode) { mipmapFilter = filterMode; }
            void setMinMipmap(uint32 min) { minMipmap = min; };
            void setMaxMipmap(uint32 max) { maxMipmap = max; };

            void setAddressModeAll(AddressMode addressMode) { addressModeU = addressMode; addressModeV = addressMode; addressModeW = addressMode; }
            void setAddressModeU(AddressMode addressMode) { addressModeU = addressMode; }
            void setAddressModeV(AddressMode addressMode) { addressModeV = addressMode; }
            void setAddressModeW(AddressMode addressMode) { addressModeW = addressMode; }

            void setUnnormalizedCoordinates(bool enable) { unnormalizedCoordinate = enable; }

            Ref<Sampler> build() const;

        private:
            FilterMode minFilter = FilterMode::Linear;
            FilterMode magFilter = FilterMode::Linear;
            FilterMode mipmapFilter = FilterMode::Linear;
            uint32 minMipmap = 0;
            uint32 maxMipmap = 0xffffffff;
            AddressMode addressModeU  = AddressMode::Repeat;
            AddressMode addressModeV = AddressMode::Repeat;
            AddressMode addressModeW = AddressMode::Repeat;
            bool unnormalizedCoordinate = false;

            friend class VulkanSampler;
        };

    protected:
        virtual ~Sampler() = default;
    };
}