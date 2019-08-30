#pragma once

#include "RendererAPI.h"


namespace BZ {


    class RenderCommand
    {
    public:
        static void initRendererAPI(RendererAPI *api);

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        static void setClearColor(const glm::vec4& color) {
            rendererAPI->setClearColor(color);
        }

        static void setDepthClearValue(float value) {
            rendererAPI->setDepthClearValue(value);
        }

        static void setStencilClearValue(int value) {
            rendererAPI->setStencilClearValue(value);
        }

        static void clearColorBuffer() {
            rendererAPI->clearColorBuffer();
        }

        static void clearDepthBuffer() {
            rendererAPI->clearDepthBuffer();
        }

        static void clearStencilBuffer() {
            rendererAPI->clearStencilBuffer();
        }

        static void clearColorAndDepthStencilBuffers() {
            rendererAPI->clearColorAndDepthStencilBuffers();
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        static void setBlendingSettings(BlendingSettings &settings) {
            rendererAPI->setBlendingSettings(settings);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        static void setViewport(int left, int top, int width, int height) {
            rendererAPI->setViewport(left, top, width, height);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        static void setRenderMode(Renderer::RenderMode mode) {
            rendererAPI->setRenderMode(mode);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        static void drawIndexed(uint32 indicesCount) {
            rendererAPI->drawIndexed(indicesCount);
        }

    private:
        static RendererAPI *rendererAPI;
    };
}
