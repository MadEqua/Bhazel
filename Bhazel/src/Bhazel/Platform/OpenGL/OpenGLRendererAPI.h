#pragma once

#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void setClearColor(const glm::vec4& color) override;
        virtual void setDepthClearValue(float value) override;
        virtual void setStencilClearValue(int value) override;

        virtual void clearColorAndDepthStencilBuffers() override;

        virtual void drawIndexed(const Ref<InputDescription> &inputDesc) override;
    };
}