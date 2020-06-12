#pragma once

#include "Panels/Panel.h"

#include <memory>
#include <vector>


namespace BZ {

class GuiRoot {
  public:
    GuiRoot();

    void render();

  private:
    void renderMenuBar();

    std::vector<std::unique_ptr<Panel>> panels;
};

}