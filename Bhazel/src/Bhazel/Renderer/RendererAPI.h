#pragma once


namespace BZ {

    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        virtual void setClearColor(const glm::vec4 &color) = 0;
        virtual void clearColorBuffer() = 0;
        virtual void clearDepthBuffer() = 0;
        virtual void clearStencilBuffer() = 0;
        virtual void clearColorAndDepthStencilBuffers() = 0;

        //virtual void setBlendingSettings(BlendingSettings &settings) = 0;
        //virtual void setDepthSettings(DepthSettings &settings) = 0;
        //virtual void setStencilSettings() = 0;

        //virtual void setBackfaceCullingSettings();

        virtual void setViewport(int left, int top, int width, int height) = 0;
        
        //virtual void setRenderMode(Renderer::PrimitiveTopology mode) = 0;

        virtual void draw(uint32 vertexCount) = 0;
        virtual void drawIndexed(uint32 indicesCount) = 0;
        virtual void drawInstanced(uint32 vertexCount, uint32 instanceCount) = 0;
        virtual void drawInstancedIndexed(uint32 indicesCount, uint32 instanceCount) = 0;

        virtual void submitCompute(uint32 groupsX, uint32 groupsY, uint32 groupsZ) = 0;
    };
}