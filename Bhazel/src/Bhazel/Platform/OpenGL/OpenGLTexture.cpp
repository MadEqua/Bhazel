#include "bzpch.h"

#include "OpenGLTexture.h"
#include "OpenGLIncludes.h"
#include <stb_image.h>


namespace BZ {

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path) : 
        path(path) {

        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
        BZ_ASSERT_CORE(data, "Failed to load image!");
        
        this->width = width;
        this->height = height;

        BZ_ASSERT_GL(glGenTextures(1, &rendererId));
        BZ_ASSERT_GL(glBindTexture(GL_TEXTURE_2D, rendererId));

        //GL 4.5
        //glCreateTextures(GL_TEXTURE_2D, 1, &rendererId);

        BZ_ASSERT_GL(glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, width, height));

        //GL 4.5
        //glTextureStorage2D(rendererId, 1, GL_SRGB8, width, height);

        BZ_ASSERT_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data));
        
        //GL 4.5
        //glTextureSubImage2D(rendererId, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

        BZ_ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        BZ_ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        //GL 4.5
        //glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D() {
        BZ_ASSERT_GL(glDeleteTextures(1, &rendererId));
    }

    void OpenGLTexture2D::bindToPipeline(uint32 unit) const {
        BZ_ASSERT_GL(glActiveTexture(GL_TEXTURE0 + unit));
        BZ_ASSERT_GL(glBindTexture(GL_TEXTURE_2D, rendererId));

        //GL 4.5
        //glBindTextureUnit(unit, rendererId);
    }
}