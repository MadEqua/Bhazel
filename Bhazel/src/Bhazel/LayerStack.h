#pragma once

#include "bzpch.h"
#include "Bhazel/Core.h"

namespace BZ {

	class Layer;

	class BZ_API LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void pushLayer(Layer *layer);
		void pushOverlay(Layer *overlay);
		void popLayer(Layer *layer);
		void popOverlay(Layer *overlay);

		std::vector<Layer*>::iterator begin() { return layers.begin(); }
		std::vector<Layer*>::iterator end() { return layers.end(); }

	private:
		std::vector<Layer*> layers;
		std::vector<Layer*>::iterator layerInsert;
	};
}