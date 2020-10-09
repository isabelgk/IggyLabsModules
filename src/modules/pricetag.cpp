// Calculate the price of your rack
// by iggy.labs

#include "../plugin.hpp"
#include "tinyxml2/tinyxml2.h"

struct PriceTag : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    PriceTag() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
};

struct PriceTagWidget : ModuleWidget {
    PriceTagWidget(PriceTag* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/dev-background.svg")));
    }
};

Model* modelPriceTag = createModel<PriceTag, PriceTagWidget>("pricetag");