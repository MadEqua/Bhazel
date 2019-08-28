#pragma once

#include "Buffer.h"


namespace BZ {

    enum class ShaderType {
        Vertex, Fragment
    };

    class Shader {
    public:
        virtual ~Shader() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        void addConstantBuffer(Ref<ConstantBuffer> &buffer, ShaderType type);

        static Ref<Shader> create(const std::string &vertexSrc, const std::string &fragmentSrc);

    protected:
        std::vector<Ref<ConstantBuffer>> vsConstantBuffers;
        std::vector<Ref<ConstantBuffer>> fsConstantBuffers;
    };
}