#pragma once

#include "Graphics/GraphicsContext.h"

struct GLFWwindow;

namespace BZ {

    class OpenGLContext : public GraphicsContext {
    public:
        explicit OpenGLContext(void *windowHandle);

        void setVSync(bool enabled) override;

    private:
        GLFWwindow *windowHandle;
    };
}