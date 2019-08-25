#include "bzpch.h"
#include "D3D11Context.h"
#include "D3D11RendererAPI.h"


namespace BZ {

    D3D11Context::D3D11Context() {
        rendererAPI = std::make_unique<D3D11RendererAPI>();
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void D3D11Context::swapBuffers() {
    }
}