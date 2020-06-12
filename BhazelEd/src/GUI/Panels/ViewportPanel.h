#pragma once

#include "Panel.h"


namespace BZ {

class ViewportPanel : public Panel {
  public:
    ViewportPanel();

  protected:
    void internalRender() override;
};

}
