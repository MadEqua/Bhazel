#pragma once

#include <Bhazel.h>

#include "MainLayer.h"


namespace BZ {

class BhazelEdApp : public Application {
  public:
    BhazelEdApp() { pushLayer(new MainLayer()); }
};

}