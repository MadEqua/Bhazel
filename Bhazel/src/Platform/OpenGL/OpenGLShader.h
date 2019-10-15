#pragma once

#include "Graphics/Shader.h"


namespace BZ {

    class OpenGLShader : public Shader {
    public:
        explicit OpenGLShader(const std::string &filePath);
        OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);
        virtual ~OpenGLShader() override;

        void bindToPipeline() const;
        void unbindFromPipeline() const;

    private:
        uint32 rendererId;

        void compile(const std::unordered_map<ShaderStage, std::string> &sources);
    };
}