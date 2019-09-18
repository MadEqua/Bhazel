#include "bzpch.h"

#include "OpenGLIncludes.h"
#include "OpenGLRendererAPI.h"

#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Renderer/PipelineSettings.h"


namespace BZ {

    static GLenum blendingFunctionToGLenum(BlendingFunction blendingFunction);
    static GLenum blendingEquationToGLenum(BlendingEquation blendingEquation);
    static GLenum testFunctionToGLenum(TestFunction testFunction);

    void OpenGLRendererAPI::setClearColor(const glm::vec4& color) {
        BZ_ASSERT_GL(glClearColor(color.r, color.g, color.b, color.a));
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

    void OpenGLRendererAPI::setDepthSettings(DepthSettings &settings) {
        if(settings.enableDepthTest) {
            BZ_ASSERT_GL(glEnable(GL_DEPTH_TEST));
            BZ_ASSERT_GL(glDepthFunc(testFunctionToGLenum(settings.testFunction)));
        }
        else {
            BZ_ASSERT_GL(glDisable(GL_DEPTH_TEST));
        }

        BZ_ASSERT_GL(glDepthMask(settings.enableDepthWrite ? GL_TRUE : GL_FALSE));
        BZ_ASSERT_GL(glClearDepth(settings.depthClearValue));
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
        case Renderer::RenderMode::TriangleStrip:
            renderMode = GL_TRIANGLE_STRIP;
            break;
        case Renderer::RenderMode::LineStrip:
            renderMode = GL_LINE_STRIP;
            break;
        default:
            BZ_LOG_ERROR("Unknown RenderMode. Setting triangles.");
            renderMode = GL_TRIANGLES;
        }
    }

    void OpenGLRendererAPI::draw(uint32 vertexCount) {
        BZ_ASSERT_GL(glDrawArrays(renderMode, 0, vertexCount));
    }

    void OpenGLRendererAPI::drawIndexed(uint32 indicesCount) {
        BZ_ASSERT_GL(glDrawElements(renderMode, indicesCount, GL_UNSIGNED_INT, nullptr));
    }

    void OpenGLRendererAPI::submitCompute(uint32 groupsX, uint32 groupsY, uint32 groupsZ) {
        BZ_ASSERT_GL(glDispatchCompute(groupsX, groupsY, groupsZ));
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }


    static GLenum blendingFunctionToGLenum(BlendingFunction blendingFunction) {
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
            return 0;
        }
    }

    static GLenum blendingEquationToGLenum(BlendingEquation blendingEquation) {
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
            return 0;
        }
    }

    static GLenum testFunctionToGLenum(TestFunction testFunction) {
        switch(testFunction) {
        case TestFunction::Never:
            return GL_NEVER;
        case TestFunction::Always:
            return GL_ALWAYS;
        case TestFunction::Less:
            return GL_LESS;
        case TestFunction::LessOrEqual:
            return GL_LEQUAL;
        case TestFunction::Greater:
            return GL_GREATER;
        case TestFunction::GreaterOrEqual:
            return GL_GEQUAL;
        case TestFunction::Equal:
            return GL_EQUAL;
        case TestFunction::NotEqual:
            return GL_NOTEQUAL;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TestFunction!");
            return 0;
        }
    }
}