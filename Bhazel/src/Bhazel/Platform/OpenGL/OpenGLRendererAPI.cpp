#include "bzpch.h"

#include "OpenGLIncludes.h"
#include "OpenGLRendererAPI.h"

#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Renderer/BlendingSettings.h"


namespace BZ {

    void OpenGLRendererAPI::setClearColor(const glm::vec4& color) {
        BZ_ASSERT_GL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void OpenGLRendererAPI::setDepthClearValue(float value) {
        BZ_ASSERT_GL(glClearDepthf(value));
    }

    void OpenGLRendererAPI::setStencilClearValue(int value) {
        BZ_ASSERT_GL(glClearStencil(value));
    }

    void OpenGLRendererAPI::clearColorBuffer() {
        BZ_ASSERT_GL(glClear(GL_COLOR_BUFFER_BIT));
    }

    void OpenGLRendererAPI::clearDepthBuffer() {
        BZ_ASSERT_GL(glClear(GL_DEPTH_BUFFER_BIT));
    }

    void OpenGLRendererAPI::clearStencilBuffer() {
        BZ_ASSERT_GL(glClear(GL_STENCIL_BUFFER_BIT));
    }

    void OpenGLRendererAPI::clearColorAndDepthStencilBuffers() {
        BZ_ASSERT_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    }

    void OpenGLRendererAPI::setBlendingSettings(BlendingSettings &settings) {
        if(settings.enableBlending) {
            BZ_ASSERT_GL(glEnable(GL_BLEND));

            const BlendingSetting &rgbSetting = settings.settingRGB;
            const BlendingSetting &alphaSetting = settings.settingAlpha;

            BZ_ASSERT_GL(glBlendFuncSeparate(blendingFunctionToGLenum(rgbSetting.sourceBlendingFunction),
                                             blendingFunctionToGLenum(rgbSetting.destinationBlendingFunction),
                                             blendingFunctionToGLenum(alphaSetting.sourceBlendingFunction),
                                             blendingFunctionToGLenum(alphaSetting.destinationBlendingFunction)));

            BZ_ASSERT_GL(glBlendEquationSeparate(blendingEquationToGLenum(rgbSetting.blendingEquation),
                                                 blendingEquationToGLenum(alphaSetting.blendingEquation)));

            if(settings.needsConstantColor()) {
                auto &cc = settings.constantColor;
                BZ_ASSERT_GL(glBlendColor(cc.r, cc.g, cc.b, cc.a));
            }
        }
        else {
            BZ_ASSERT_GL(glDisable(GL_BLEND));
        }
    }

    void OpenGLRendererAPI::setViewport(int left, int top, int width, int height) {
        BZ_ASSERT_GL(glViewport(left, top, width, height));
    }

    void OpenGLRendererAPI::setRenderMode(Renderer::RenderMode mode) {
        switch(mode)
        {
        case Renderer::RenderMode::Points:
            renderMode = GL_POINTS;
            break;
        case Renderer::RenderMode::Lines:
            renderMode = GL_LINES;
            break;
        case Renderer::RenderMode::Triangles:
            renderMode = GL_TRIANGLES;
            break;
        default:
            BZ_LOG_ERROR("Unknown RenderMode. Setting triangles.");
            renderMode = GL_TRIANGLES;
        }
    }

    void OpenGLRendererAPI::drawIndexed(uint32 indicesCount) {
        BZ_ASSERT_GL(glDrawElements(renderMode, indicesCount, GL_UNSIGNED_INT, nullptr));
    }


    /*GLenum OpenGLRendererAPI::comparisionFunctionToGLenum(ComparisionFunction comparisionFunction) {
        switch(comparisionFunction) {
        case ComparisionFunction::NEVER:
            return GL_NEVER;
        case ComparisionFunction::ALWAYS:
            return GL_ALWAYS;
        case ComparisionFunction::LESS:
            return GL_LESS;
        case ComparisionFunction::LESS_OR_EQUAL:
            return GL_LEQUAL;
        case ComparisionFunction::GREATER:
            return GL_GREATER;
        case ComparisionFunction::GREATER_OR_EQUAL:
            return GL_GEQUAL;
        case ComparisionFunction::EQUAL:
            return GL_EQUAL;
        case ComparisionFunction::NOT_EQUAL:
            return GL_NOTEQUAL;
        default:
            return 0;
        }
    }*/

    GLenum OpenGLRendererAPI::blendingFunctionToGLenum(BlendingFunction blendingFunction) {
        switch(blendingFunction) {
        case BlendingFunction::Zero:
            return GL_ZERO;
        case BlendingFunction::One:
            return GL_ONE;
        case BlendingFunction::SourceColor:
            return GL_SRC_COLOR;
        case BlendingFunction::OneMinusSourceColor:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendingFunction::DestinationColor:
            return GL_DST_COLOR;
        case BlendingFunction::OneMinusDestinationColor:
            return GL_ONE_MINUS_DST_COLOR;
        case BlendingFunction::SourceAlpha:
            return GL_SRC_ALPHA;
        case BlendingFunction::OneMinusSourceAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendingFunction::DestinationAlpha:
            return GL_DST_ALPHA;
        case BlendingFunction::OneMinusDestinationAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        case BlendingFunction::ConstantColor:
            return GL_CONSTANT_COLOR;
        case BlendingFunction::OneMinusConstantColor:
            return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendingFunction::ConstantAlpha:
            return GL_CONSTANT_ALPHA;
        case BlendingFunction::OneMinusConstantAlpha:
            return GL_ONE_MINUS_CONSTANT_ALPHA;
        case BlendingFunction::AlphaSaturate:
            return GL_SRC_ALPHA_SATURATE;
        case BlendingFunction::Source1Color:
            return GL_SRC1_COLOR;
        case BlendingFunction::OneMinusSource1Color:
            return GL_ONE_MINUS_SRC1_COLOR;
        case BlendingFunction::Source1Alpha:
            return GL_SRC1_ALPHA;
        case BlendingFunction::OneMinusSource1Alpha:
            return GL_ONE_MINUS_SRC1_ALPHA;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BlendingFunction!");
        }
    }

    GLenum OpenGLRendererAPI::blendingEquationToGLenum(BlendingEquation blendingEquation) {
        switch(blendingEquation) {
        case BlendingEquation::Add:
            return GL_FUNC_ADD;
        case BlendingEquation::SourceMinusDestination:
            return GL_FUNC_SUBTRACT;
        case BlendingEquation::DestinationMinusSource:
            return GL_FUNC_REVERSE_SUBTRACT;
        case BlendingEquation::Min:
            return GL_MIN;
        case BlendingEquation::Max:
            return GL_MAX;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BlendingEquation!");
        }
    }
}