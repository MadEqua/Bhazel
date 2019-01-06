#pragma once

#include "bzpch.h"
#include "Bhazel/Core.h"
#include "Bhazel/Events/Event.h"

namespace BZ {

	class Event;

	class BZ_API Layer {
	public:
		Layer(const std::string &name = "Layer");
		virtual ~Layer() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate() {}
		virtual void onEvent(Event &event) {}

		inline const std::string &getName() const { return debugName; }

	protected:
		std::string debugName;
	};
}