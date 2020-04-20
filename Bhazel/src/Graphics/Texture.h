#pragma once

#include "PipelineState.h"


namespace BZ {

    enum class TextureFormatEnum {
        Undefined,
        R8, R8_SRGB,
        R8G8, R8G8_SRGB,
        R8G8B8, R8G8B8_SRGB,
        R8G8B8A8, R8G8B8A8_SRGB,
        B8G8R8A8, B8G8R8A8_SRGB,
        D32, D16S8, D24S8,
    };

    struct TextureFormat {
        TextureFormat(TextureFormatEnum formaEnum);

        //TextureFormat operator()() { return format; }
        bool isColor() const;
        bool isDepth() const;
        bool isStencil() const;
        bool isDepthStencil() const;
        bool isDepthOnly() const;
        bool isSRGB() const;

        int getChannelCount() const;

        bool operator==(const TextureFormat &other) const {
            return formaEnum == other.formaEnum;
        }

        TextureFormatEnum getEnum() const { return formaEnum; }

    private:
        TextureFormatEnum formaEnum;
    };


    struct MipmapData {
        enum class Options {
            Generate, Load, DoNothing
        };

        MipmapData(Options option) : option(option), mipLevels(0) {}
        MipmapData(Options option, uint32 mipLevels) : option(option), mipLevels(mipLevels) {}

        Options option;
        uint32 mipLevels; //Used when FromData is the option.
    };

    class Texture {
    public:
        virtual ~Texture() = default;

        const TextureFormat& getFormat() const { return format; }

        const glm::ivec3& getDimensions() const { return dimensions; }
        uint32 getWidth() const { return dimensions.x; }
        uint32 getHeight() const { return dimensions.y; }
        uint32 getDepth() const { return dimensions.z; }
        uint32 getMipLevels() const { return mipLevels; }
        uint32 getLayers() const { return layers; }

    protected:
        explicit Texture(TextureFormat format);

        struct FileData {
            const byte *data;
            uint32 width, height;
        };

        static FileData loadFile(const char* path, int desiredChannels, bool flip);
        static void freeData(const FileData &fileData);

        TextureFormat format;
        glm::ivec3 dimensions = { 1, 1, 1 };
        uint32 layers = 1;
        uint32 mipLevels = 1;
    };


    class Texture2D : public Texture {
    public:
        static Ref<Texture2D> create(const char *path, TextureFormat format, MipmapData mipmapData);
        static Ref<Texture2D> create(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData);
        
        static Ref<Texture2D> createRenderTarget(uint32 width, uint32 height, TextureFormat format);

    protected:
        explicit Texture2D(TextureFormat format);
    };


    class TextureCube : public Texture {
    public:
        //fileNames -> +x, -x, +y, -y, +z, -z
        static Ref<TextureCube> create(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData);
 
    protected:
        explicit TextureCube(TextureFormat format);
    };


    class TextureView {
    public:
        static Ref<TextureView> create(const Ref<Texture2D> &texture2D);
        static Ref<TextureView> create(const Ref<TextureCube> &textureCube);

        //const TextureFormat& getFormat() const { return texture->getFormat(); } TODO: TextureView own format
        const TextureFormat& getTextureFormat() const { return texture->getFormat(); }
        Ref<Texture> getTexture() const { return texture; }

    protected:
        explicit TextureView(const Ref<Texture> &texture);
        virtual ~TextureView() = default;

        Ref<Texture> texture;
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

            void enableCompare(CompareFunction compareFunction) { compareEnabled = true; this->compareFunction = compareFunction; }

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
            bool compareEnabled = false;
            CompareFunction compareFunction = CompareFunction::Always;

            friend class VulkanSampler;
        };

    protected:
        virtual ~Sampler() = default;
    };
}