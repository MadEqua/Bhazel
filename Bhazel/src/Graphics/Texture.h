#pragma once

#include "PipelineState.h"

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    struct TextureFormat {
        TextureFormat(VkFormat format);

        bool isColor() const;
        bool isDepth() const;
        bool isStencil() const;
        bool isDepthStencil() const;
        bool isDepthOnly() const;
        bool isSRGB() const;
        bool isFloatingPoint() const;

        int getChannelCount() const;
        int getSizePerChannel() const;
        int getSizePerTexel() const;

        bool operator==(const TextureFormat &other) const {
            return format == other.format;
        }

        operator VkFormat() const { return format; }

    private:
        VkFormat format;
    };

    struct MipmapData {
        enum class Options {
            Generate, Load, DoNothing
        };

        MipmapData(Options option) : option(option), mipLevels(0) {}
        MipmapData(Options option, uint32 mipLevels) : option(option), mipLevels(mipLevels) {}

        Options option;
        uint32 mipLevels; //Used when Load is the option.
    };

    struct TextureHandles {
        VkImage imageHandle;
        VmaAllocation allocationHandle;

        VkBuffer stagingBufferHandle;
        VmaAllocation stagingBufferAllocationHandle;
    };

    class Texture : public GpuObject<TextureHandles> {
    public:
        ~Texture();

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

        static FileData loadFile(const char* path, int desiredChannels, bool flip, float isFloatingPoint);
        static void freeData(const FileData &fileData);

        TextureFormat format;
        glm::ivec3 dimensions = { 1, 1, 1 };
        uint32 layers = 1;
        uint32 mipLevels = 1;

        bool isWrapping = false;
    };


    /*-------------------------------------------------------------------------------------------*/
    class Texture2D : public Texture {
    public:
        static Ref<Texture2D> create(const char *path, TextureFormat format, MipmapData mipmapData);
        static Ref<Texture2D> create(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData);

        static Ref<Texture2D> createRenderTarget(uint32 width, uint32 height, uint32 layers, TextureFormat format);

        static Ref<Texture2D> wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        Texture2D(const char *path, TextureFormat format, MipmapData mipmapData);
        Texture2D(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData);
        Texture2D(uint32 width, uint32 height, uint32 layers, TextureFormat format);

    private:
        void createImage(bool hasData, MipmapData mipmapData);

        //Coming from an already existent VkImage. Used on the swapchain images.
        Texture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);
    };


    /*-------------------------------------------------------------------------------------------*/
    class TextureCube : public Texture {
    public:
        //fileNames -> +x, -x, +y, -y, +z, -z
        static Ref<TextureCube> create(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData);
 
        TextureCube(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData);

    private:
        void createImage(bool hasData, MipmapData mipmapData);
    };


    /*-------------------------------------------------------------------------------------------*/
    class TextureView : public GpuObject<VkImageView> {
    public:
        static Ref<TextureView> create(const Ref<Texture2D> &texture2D);
        static Ref<TextureView> create(const Ref<Texture2D> &texture2D, uint32 baseLayer, uint32 layerCount);
        static Ref<TextureView> create(const Ref<TextureCube> &textureCube);

        explicit TextureView(const Ref<Texture2D> &texture2D);
        TextureView(const Ref<Texture2D> &texture2D, uint32 baseLayer, uint32 layerCount);
        explicit TextureView(const Ref<TextureCube> &textureCube);

        ~TextureView();

        //const TextureFormat& getFormat() const { return texture->getFormat(); } TODO: TextureView own format
        const TextureFormat& getTextureFormat() const { return texture->getFormat(); }
        Ref<Texture> getTexture() const { return texture; }

    private:
        void init(VkImageViewType viewType, VkImage vkImage, uint32 baseLayer, uint32 layerCount);

        Ref<Texture> texture;
    };


    /*-------------------------------------------------------------------------------------------*/
    class Sampler : public GpuObject<VkSampler> {
    public:
        class Builder {
        public:
            void setMinFilterMode(VkFilter filter) { minFilter = filter; }
            void setMagFilterMode(VkFilter filter) { magFilter = filter; }

            void setMipmapFilter(VkSamplerMipmapMode filter) { mipmapFilter = filter; }
            void setMinMipmap(uint32 min) { minMipmap = min; };
            void setMaxMipmap(uint32 max) { maxMipmap = max; };

            void setAddressModeAll(VkSamplerAddressMode addressMode) { addressModeU = addressMode; addressModeV = addressMode; addressModeW = addressMode; }
            void setAddressModeU(VkSamplerAddressMode addressMode) { addressModeU = addressMode; }
            void setAddressModeV(VkSamplerAddressMode addressMode) { addressModeV = addressMode; }
            void setAddressModeW(VkSamplerAddressMode addressMode) { addressModeW = addressMode; }

            void setUnnormalizedCoordinates(bool enable) { unnormalizedCoordinate = enable; }

            void enableCompare(VkCompareOp compareOp) { compareEnabled = true; this->compareOp = compareOp; }

            void setBorderColor(VkBorderColor borderColor) { this->borderColor = borderColor; }

            Ref<Sampler> build() const;

        private:
            VkFilter minFilter = VK_FILTER_LINEAR;
            VkFilter magFilter = VK_FILTER_LINEAR;
            VkSamplerMipmapMode mipmapFilter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            uint32 minMipmap = 0;
            uint32 maxMipmap = 0xffffffff;
            VkSamplerAddressMode addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            bool unnormalizedCoordinate = false;
            bool compareEnabled = false;
            VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
            VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

            friend class Sampler;
        };

        Sampler(const Builder &builder);
        ~Sampler();
    };
}