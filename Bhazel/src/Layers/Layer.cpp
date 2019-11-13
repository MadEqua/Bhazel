#include "bzpch.h"

#include "Layer.h"
#include "Core/Application.h"


namespace BZ {

	Layer::Layer(const std::string &name) :
        application(Application::getInstance()),
        debugName(name) {
	}
}