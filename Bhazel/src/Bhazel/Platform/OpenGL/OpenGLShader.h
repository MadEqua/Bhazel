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

        void setUniformInt(const std::string &name, int v);

        void setUniformFloat(const std::string &name, float v);
        void setUniformFloat2(const std::string &name, const glm::vec2 &vec);
        void setUniformFloat3(const std::string &name, const glm::vec3 &vec);
        void setUniformFloat4(const std::string &name, const glm::vec4 &vec);

        void setUniformMat3(const std::string &name, const glm::mat3 &mat);
        void setUniformMat4(const std::string &name, const glm::mat4 &mat);

    private:
        uint32 rendererId;
    };
}