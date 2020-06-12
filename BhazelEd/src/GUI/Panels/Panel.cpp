#include "Panel.h"

#include <imgui.h>


namespace BZ {

void Panel::render() {
    if (open) {
        internalRender();
    }
}

}