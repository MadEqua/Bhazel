#pragma once


namespace BZ {

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual void bindToPipeline(uint32 unit = 0u) const = 0;
        //virtual void unbindFromPipeline() const = 0;

    };

    class Texture2D : public Texture {
    public:
        static Ref<Texture2D> create(const std::string &path);

        uint32 getWidth() const { return width; }
        uint32 getHeight() const { return height; }

    protected:
        uint32 width, height;
    };
}