#pragma once

namespace BZ {

    enum class RendererAPI {
        None,
        OpenGL
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        RendererAPI static getAPI() { return rendererAPI; }

    private:
        static RendererAPI rendererAPI;
    };
}