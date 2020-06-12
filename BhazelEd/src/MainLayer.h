#pragma once

#include <Bhazel.h>
#include "GUI/GuiRoot.h"


namespace BZ {

class MainLayer : public Layer {
  public:
    MainLayer() : Layer("MainLayer") {}

    void onAttachToEngine() override;

    void onUpdate(const FrameTiming &frameTiming) override;
    void onEvent(Event &event) override;
    void onImGuiRender(const FrameTiming &frameTiming) override;

  private:
    GuiRoot guiRoot;
};

}