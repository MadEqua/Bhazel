#pragma once

#include <string>

namespace BZ {

    class Shader {
    public:
        Shader(const std::string &vertexSrc, const std::string &fragmentSrc);
        ~Shader();

        void bind() const;
        void unbind() const;

    private:
        uint32 rendererId;
    };
}