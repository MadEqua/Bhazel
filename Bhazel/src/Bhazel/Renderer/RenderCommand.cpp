#include "bzpch.h"

#include "RenderCommand.h"
#include "Bhazel/Application.h"
#include "Bhazel/Window.h"
#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    RendererAPI* RenderCommand::rendererAPI = nullptr;

    void RenderCommand::initRendererAPI(RendererAPI *api) {
        rendererAPI = api;
    }
}