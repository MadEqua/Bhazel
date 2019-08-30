#pragma once

#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    enum class BlendingFunction;
    enum class BlendingEquation;

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void setClearColor(const glm::vec4& color) override;
        virtual void setDepthClearValue(float value) override;
        virtual void setStencilClearValue(int value) override;
        virtual void clearColorBuffer() override;
        virtual void clearDepthBuffer() override;
        virtual void clearStencilBuffer() override;
        virtual void clearColorAndDepthStencilBuffers() override;

        virtual void setBlendingSettings(BlendingSettings &settings) override;

        virtual void setViewport(int left, int top, int width, int height) override;

        virtual void setRenderMode(Renderer::RenderMode mode) override;

        virtual void drawIndexed(uint32 indicesCount) override;

    private:
        int renderMode;

        GLenum blendingFunctionToGLenum(BlendingFunction blendingFunction);
        GLenum blendingEquationToGLenum(BlendingEquation blendingEquation);
    };
}