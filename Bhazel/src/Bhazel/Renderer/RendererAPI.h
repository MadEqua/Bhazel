#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "VertexArray.h"


namespace BZ {
    
    class RendererAPI {
    public:
        enum class API  {
            None,
            OpenGL
        };

        virtual void setClearColor(const glm::vec4 &color) = 0;
        virtual void clear() = 0;

        virtual void drawIndexed(const std::shared_ptr<VertexArray> &vertexArray) = 0;

        static API getAPI() { return api; }

    private:
        static API api;
    };
}