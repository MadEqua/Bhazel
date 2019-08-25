#pragma once

#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    class D3D11RendererAPI : public RendererAPI
    {
    public:
        virtual void setClearColor(const glm::vec4& color) override;
        virtual void clear() override;

        virtual void drawIndexed(const Ref<VertexArray> &vertexArray) override;
    };
}