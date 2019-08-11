#pragma once

#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void setClearColor(const glm::vec4& color) override;
        virtual void clear() override;

        virtual void drawIndexed(const Ref<VertexArray> &vertexArray) override;
    };
}