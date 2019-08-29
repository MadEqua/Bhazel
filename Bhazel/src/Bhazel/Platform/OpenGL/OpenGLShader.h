#pragma once

#include "Bhazel/Renderer/Shader.h"


namespace BZ {

    class OpenGLShader : public Shader {
    public:
        OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);
        virtual ~OpenGLShader() override;

        void bindToPipeline() const;
        void unbindFromPipeline() const;

    private:
        uint32 rendererId;
    };
}