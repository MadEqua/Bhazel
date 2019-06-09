#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace BZ {

    class OpenGLContext : public GraphicsContext {
    public:
        OpenGLContext(GLFWwindow *windowHandle);

        virtual void init() override;
        virtual void swapBuffers() override;

    private:
        GLFWwindow *windowHandle;
    };
}