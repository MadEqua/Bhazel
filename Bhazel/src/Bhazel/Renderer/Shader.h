#pragma once

#include <string>
#include <glm/glm.hpp>

namespace BZ {

    class Shader {
    public:
        Shader(const std::string &vertexSrc, const std::string &fragmentSrc);
        ~Shader();

        void bind() const;
        void unbind() const;

        void setUniformFloat3(const std::string &name, const glm::vec3 &vec);
        void setUniformFloat4(const std::string &name, const glm::vec4 &vec);
        void setUniformMat4(const std::string &name, const glm::mat4 &mat);

    private:
        uint32 rendererId;
    };
}