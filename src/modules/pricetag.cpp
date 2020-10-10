// Calculate the price of your rack
// by iggy.labs

#include <string>
#include "../plugin.hpp"
#include "components.hpp"


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

    /** Get the rack price based on number of knobs, ports, cables, and total HP */
    void getPrice() {
        int numParams = 0;
        int numPorts = 0;
        int numCables = 0;
        int totalHP = 0;

        auto moduleContainer = APP->scene->rack->moduleContainer;
        for (widget::Widget* w : moduleContainer->children) {
            ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
            assert(moduleWidget);

            numParams += moduleWidget->params.size();
            numPorts += moduleWidget->outputs.size() + moduleWidget->inputs.size();
            totalHP += moduleWidget->panel->box.size.x / RACK_GRID_WIDTH;
        }

        auto cableContainer = APP->scene->rack->cableContainer;
        for (widget::Widget* w : cableContainer->children) {
            CableWidget* cw = dynamic_cast<CableWidget*>(w);
            assert(cw);

            if (!cw->isComplete()) continue;
            numCables += 1;
        }

        // This formula is arbitrarily weighted
        return numParams * 17 + numPorts * 12 + totalHP * 20 + numCables * 2;
    }


    int loopCounter = 0;
    void process(const ProcessArgs& args) override {
        if (loopCounter == 0) {
            getPrice();
        }

        loopCounter++;

        // Compute every second
        if (loopCounter > args.sampleRate) {
            loopCounter = 0;
        }
    }
};


struct PriceTagWidget : ModuleWidget {
    PriceTagWidget(PriceTag* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/pricetag.svg")));
    }
};

Model* modelPriceTag = createModel<PriceTag, PriceTagWidget>("pricetag");