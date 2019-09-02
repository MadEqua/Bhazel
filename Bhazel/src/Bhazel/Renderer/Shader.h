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

        static Ref<Shader> create(const std::string &filePath);
        static Ref<Shader> create(const std::string &vertexSrc, const std::string &fragmentSrc);

    protected:
        static std::string readFile(const std::string &filePath);
        static std::unordered_map<ShaderType, std::string> preProcessSource(const std::string &source);
        static ShaderType shaderTypeFromString(const std::string &string);
    };
}