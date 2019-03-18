#pragma once

#include "Bhazel/Layer.h"

namespace BZ {

    class BZ_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void onAttach() override;
        void onDetach() override;
        void onImGuiRender() override;

        void begin();
        void end();

        float time;
    };
}