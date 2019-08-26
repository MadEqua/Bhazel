#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace BZ {

    class OpenGLContext : public GraphicsContext {
    public:
        explicit OpenGLContext(GLFWwindow *windowHandle);

        void swapBuffers() override;
        void setVSync(bool enabled) override;

    private:
        GLFWwindow *windowHandle;
    };
}