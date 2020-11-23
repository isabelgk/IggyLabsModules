// Calculate the price of your rack
// by iggy.labs

#include <string>
#include "../plugin.hpp"


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

// https://community.vcvrack.com/t/advanced-nanovg-custom-label/6769/12
struct PriceTagTextWidget : OpaqueWidget {
    std::shared_ptr<Font> font;
    int fontSize = 24;
    std::string text;

    PriceTagTextWidget(int x, int y, std::string _text = "") {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/font/open24displaySt/Open 24 Display St.ttf"));
        box.pos = Vec(x, y);
        text = _text;
    }

    void draw(const DrawArgs& args) override {
        if (font->handle >= 0) {
            bndSetFont(font->handle);
            nvgFontSize(args.vg, fontSize);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 1);
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

            nvgBeginPath(args.vg);
            nvgFillColor(args.vg, nvgRGB(236, 239, 241));  // Blue Grey 50 - Material color scheme
            nvgText(args.vg, 0, 0, text.c_str(), NULL);
            nvgStroke(args.vg);

            bndSetFont(APP->window->uiFont->handle);
        }
    }
};

struct PriceTagWidget : ModuleWidget {
    // Create and position labels
    PriceTagTextWidget* portsLabel = new PriceTagTextWidget(37.5, 75, "ports");
    PriceTagTextWidget* paramLabel = new PriceTagTextWidget(37.5, 135, "params");
    PriceTagTextWidget* cablesLabel = new PriceTagTextWidget(37.5, 195, "cables");
    PriceTagTextWidget* hpLabel = new PriceTagTextWidget(37.5, 255, "hp");
    PriceTagTextWidget* modulesLabel = new PriceTagTextWidget(37.5, 314, "modules");

    int numParams = 0;
    int numPorts = 0;
    int numCables = 0;
    int numModules = 0;
    int totalHP = 0;
    int loopCounter = 0;


    PriceTagWidget(PriceTag* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/pricetag.svg")));

        addChild(portsLabel);
        addChild(paramLabel);
        addChild(cablesLabel);
        addChild(hpLabel);
        addChild(modulesLabel);
    }


    void updateStats() {
        // Reset stats
        numParams = 0;
        numPorts = 0;
        numCables = 0;
        totalHP = 0;
        numModules = 0;

        // Recompute values
        auto moduleContainer = APP->scene->rack->moduleContainer;
        for (widget::Widget* w : moduleContainer->children) {
            ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
            assert(moduleWidget);

            numModules++;
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

        // Update GUI
        paramLabel->text = std::to_string(numParams);
        portsLabel->text = std::to_string(numPorts);
        cablesLabel->text = std::to_string(numCables);
        hpLabel->text = std::to_string(totalHP);
        modulesLabel->text = std::to_string(numModules);
    }

    /** Count the number of params, ports, cables, and HP*/
    void step() override {
        updateStats();
        Widget::step();
    }
};

Model* modelPriceTag = createModel<PriceTag, PriceTagWidget>("pricetag");