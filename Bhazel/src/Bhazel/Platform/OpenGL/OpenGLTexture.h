#pragma once

#include "Bhazel/Renderer/Texture.h"


namespace BZ {

    class OpenGLTexture2D : public Texture2D
    {
    public:
        explicit OpenGLTexture2D(const std::string &path);
        virtual ~OpenGLTexture2D() override;

        virtual void bind(uint32 unit) const override;

    private:
        std::string path;
        uint32 rendererId;
    };
}