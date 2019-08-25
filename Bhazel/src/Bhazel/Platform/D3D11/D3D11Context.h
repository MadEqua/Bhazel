#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"


namespace BZ {

    class D3D11Context : public GraphicsContext {
    public:
        D3D11Context();

        virtual void swapBuffers() override;

    private:
        //GLFWwindow *windowHandle;
    };
}