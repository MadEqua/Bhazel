#pragma once

#include <glm/glm.hpp>

#include "Bhazel/Renderer/Shader.h"


namespace BZ {

    class OpenGLShader : public Shader {
    public:
        OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);
        virtual ~OpenGLShader() override;

        void bind() const;
        void unbind() const;

    private:
        uint32 rendererId;
    };
}