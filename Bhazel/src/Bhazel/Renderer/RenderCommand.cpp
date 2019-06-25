#include "bzpch.h"

#include "RenderCommand.h"

#include "Bhazel/Platform/OpenGL/OpenGLRendererAPI.h"


namespace BZ {

    RendererAPI *RenderCommand::rendererAPI = new OpenGLRendererAPI();
}