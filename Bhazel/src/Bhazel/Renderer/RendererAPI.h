#pragma once

#include "Renderer.h"

namespace BZ {

    class InputDescription;
    struct BlendingSettings;
    struct DepthSettings;

    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        virtual void setClearColor(const glm::vec4 &color) = 0;
        virtual void clearColorBuffer() = 0;
        virtual void clearDepthBuffer() = 0;
        virtual void clearStencilBuffer() = 0;
        virtual void clearColorAndDepthStencilBuffers() = 0;

        virtual void setBlendingSettings(BlendingSettings &settings) = 0;
        virtual void setDepthSettings(DepthSettings &settings) = 0;
        //virtual void setStencilSettings() = 0;

        //virtual void setBackfaceCullingSettings();

        virtual void setViewport(int left, int top, int width, int height) = 0;
        
        virtual void setRenderMode(Renderer::RenderMode mode) = 0;

        virtual void drawIndexed(uint32 indicesCount) = 0;
    };
}