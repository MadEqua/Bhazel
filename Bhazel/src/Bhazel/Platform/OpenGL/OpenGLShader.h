#pragma once

#include "Bhazel/Renderer/Shader.h"


namespace BZ {

    class OpenGLShader : public Shader {
    public:
        explicit OpenGLShader(const std::string &filePath);
        OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);
        virtual ~OpenGLShader() override;

        void bindToPipeline() const;
        void unbindFromPipeline() const;

    private:
        uint32 rendererId;

        void compile(const std::unordered_map<ShaderType, std::string> &sources);
    };
}