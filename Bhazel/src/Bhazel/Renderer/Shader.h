#pragma once

#include "Buffer.h"


namespace BZ {

    enum class ShaderType {
        Vertex, Fragment
    };

    class Shader {
    public:
        virtual ~Shader() = default;

        virtual void bindToPipeline() const = 0;
        virtual void unbindFromPipeline() const = 0;

        static Ref<Shader> create(const std::string &vertexSrc, const std::string &fragmentSrc);
    };
}