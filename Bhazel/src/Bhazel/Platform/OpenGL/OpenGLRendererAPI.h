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

        virtual void setViewport(int left, int top, int width, int height) override;
        virtual void setRenderMode(Renderer::RenderMode mode) override;

        virtual void drawIndexed(const Ref<InputDescription> &inputDesc) override;

    private:
        int renderMode;
    };
}