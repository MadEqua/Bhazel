#pragma once

#include "Buffer.h"


namespace BZ {

    enum class ShaderType {
        Vertex,
        Fragment,
        Unknown
    };

    class Shader {
    public:
        virtual ~Shader() = default;

        virtual void bindToPipeline() const = 0;
        virtual void unbindFromPipeline() const = 0;

        static Ref<Shader> create(const std::string &filePath);
        static Ref<Shader> create(const std::string &vertexSrc, const std::string &fragmentSrc);

    protected:
        static std::unordered_map<ShaderType, std::string> readAndPreprocessFile(const std::string &filePath);
        static ShaderType shaderTypeFromString(const std::string &string);
    };
}