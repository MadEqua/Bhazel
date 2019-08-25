#include "bzpch.h"

#include "OpenGLTexture.h"
#include <glad/glad.h>
#include <stb_image.h>


namespace BZ {

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path) : 
        path(path) {

        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        BZ_ASSERT_CORE(data, "Failed to load image!");
        
        this->width = width;
        this->height = height;

        glGenTextures(1, &rendererId);
        glBindTexture(GL_TEXTURE_2D, rendererId);

        //GL 4.5
        //glCreateTextures(GL_TEXTURE_2D, 1, &rendererId);

        glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8, width, height);

        //GL 4.5
        //glTextureStorage2D(rendererId, 1, GL_SRGB8, width, height);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
        
        //GL 4.5
        //glTextureSubImage2D(rendererId, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //GL 4.5
        //glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D() {
        glDeleteTextures(1, &rendererId);
    }

    void OpenGLTexture2D::bind(uint32 unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, rendererId);

        //GL 4.5
        //glBindTextureUnit(unit, rendererId);
    }
}