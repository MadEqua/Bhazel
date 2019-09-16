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

        static void setDepthSettings(DepthSettings &settings) {
            rendererAPI->setDepthSettings(settings);
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
        static void draw(uint32 vertexCount) {
            rendererAPI->draw(vertexCount);
        }

        static void drawIndexed(uint32 indicesCount) {
            rendererAPI->drawIndexed(indicesCount);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        static void submitCompute(uint32 groupsX, uint32 groupsY, uint32 groupsZ) {
            rendererAPI->submitCompute(groupsX, groupsY, groupsZ);
        }

    private:
        static RendererAPI *rendererAPI;
    };
}
