#pragma once

#include "Panels/Panel.h"
#include <vector>


namespace BZ {

class GuiRoot {
  public:
    GuiRoot();

    void render();

  private:
    void renderMenuBar();

    std::vector<Panel> panels; // Pointer?
};

}