// Plugins for VCV Rack by iggy.labs

#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelTable);
	p->addModel(modelSelect);
	p->addModel(modelMore_ideas);
	p->addModel(modelPriceTag);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
