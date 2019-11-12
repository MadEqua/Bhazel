#pragma once

#include "Layers/Layer.h"

#include "Graphics/Graphics.h"

namespace BZ {

    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();

        void onGraphicsContextCreated() override;
        void onDetach() override;
        void onImGuiRender(const FrameStats &frameStats) override;

        void onEvent(Event &event) override;

        void begin();
        void end();

    private:
        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;
        BufferPtr vertexBufferPtr;
        BufferPtr indexBufferPtr;

        //Ref<Shader> vertexShader;
        //Ref<Shader> fragmentShader;

        Ref<Texture> fontTexture;
        Ref<TextureView> fontTextureView;
        Ref<Sampler> fontTextureSampler;

        Ref<DescriptorSet> descriptorSet;

        Ref<PipelineState> pipelineState;

        void initInput();
        void initGraphics();
    };
}