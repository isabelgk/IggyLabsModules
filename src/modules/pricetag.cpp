// Calculate the price of your rack
// by iggy.labs

#include <string>
#include "../plugin.hpp"


struct PriceTag : Module {
    enum ParamIds {
        COMPUTE_PARAM,
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
        configParam(COMPUTE_PARAM, 0.0f, 1.0f, 0.0f, "Compute");
    }

    int numParams = 0;
    int numPorts = 0;
    int numCables = 0;
    int totalHP = 0;

    dsp::SchmittTrigger buttonTrigger;


    /** Get plugin info from current Rack session */
    void getPluginInfo() {
        numParams = 0;
        numPorts = 0;
        numCables = 0;
        totalHP = 0;

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
    }

    int getPrice() {
        return numParams * 17 + numPorts * 12 + numCables * 2 + totalHP * 20;
    }

    int loopCounter = 0;

    void process(const ProcessArgs& args) override {
        if (loopCounter == 0) {
            getPluginInfo();
            DEBUG("[IGGYLABS] numParams = %d", numParams);
            DEBUG("[IGGYLABS] numPorts = %d", numPorts);
            DEBUG("[IGGYLABS] numCables = %d", numCables);
            DEBUG("[IGGYLABS] Total HP = %d", totalHP);
            DEBUG("[IGGYLABS] price = %d", getPrice());
        }

        loopCounter++;
        
        // Compute every two seconds
        if (loopCounter > args.sampleRate * 2) {
            loopCounter = 0;
        }
    }
};


struct PriceTagWidget : ModuleWidget {
    PriceTagWidget(PriceTag* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/dev-background.svg")));

        addParam(createParamCentered<LEDButton>(Vec(25, 25), module, PriceTag::COMPUTE_PARAM));	
    }

};

Model* modelPriceTag = createModel<PriceTag, PriceTagWidget>("pricetag");