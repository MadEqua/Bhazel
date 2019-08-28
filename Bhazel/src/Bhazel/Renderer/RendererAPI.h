#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "InputDescription.h"


namespace BZ {

    enum class RenderMode;

    class RendererAPI {
    public:
        enum class API  {
            OpenGL,
            D3D11
        };

        virtual ~RendererAPI() = default;

        virtual void setClearColor(const glm::vec4 &color) = 0;
        virtual void setDepthClearValue(float value) = 0;
        virtual void setStencilClearValue(int value) = 0;

        virtual void clearColorAndDepthStencilBuffers() = 0;

        virtual void setViewport(int left, int top, int width, int height) = 0;
        virtual void setRenderMode(RenderMode mode) = 0;

        virtual void drawIndexed(const Ref<InputDescription> &inputDesc) = 0;

        static API getAPI() { return api; }

    private:
        static const API api = API::D3D11;
    };
}