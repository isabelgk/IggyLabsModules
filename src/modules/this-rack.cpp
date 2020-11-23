// What's in this rack?
// by iggy.labs

#include <string>
#include <vector>
#include "../plugin.hpp"


struct ThisRack : Module {
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
    
    int numParams = 0;
    int numPorts = 0;
    int numCables = 0;
    int totalHP = 0;
    int numModules = 0;

    ThisRack() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs& args) override {

    }

};

// https://community.vcvrack.com/t/advanced-nanovg-custom-label/6769/12
struct ThisRackTextWidget : OpaqueWidget {
    std::shared_ptr<Font> font;
    int fontSize = 24;
    std::string text;

    ThisRackTextWidget(int x, int y, std::string _text = "") {
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

struct ThisRackWidget : ModuleWidget {
    ThisRack* moduleInstance;
    ThisRackWidget(ThisRack* module) {
        setModule(module);
        moduleInstance = module;
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/this-rack.svg")));
    
        // Create and position labels
        ThisRackTextWidget* portsLabel = new ThisRackTextWidget(37.5, 75, "0");
        // portsLabel->text = std::to_string(moduleInstance->numPorts);

        ThisRackTextWidget* paramLabel = new ThisRackTextWidget(37.5, 135, "0");
        // paramLabel->text = std::to_string(moduleInstance->numParams);

        ThisRackTextWidget* cablesLabel = new ThisRackTextWidget(37.5, 195, "0");
        // cablesLabel->text = std::to_string(moduleInstance->numCables);

        ThisRackTextWidget* hpLabel = new ThisRackTextWidget(37.5, 255, "0");
        // hpLabel->text = std::to_string(moduleInstance->totalHP);

        ThisRackTextWidget* modulesLabel = new ThisRackTextWidget(37.5, 314, "0");
        // modulesLabel->text = std::to_string(moduleInstance->numModules);

        addChild(portsLabel);
        addChild(paramLabel);
        addChild(cablesLabel);
        addChild(hpLabel);
        addChild(modulesLabel);
    }

    /** Count the number of params, ports, cables, and HP*/
    void step() override {
        if (moduleInstance) {
            moduleInstance->numParams = 0;
            moduleInstance->numPorts = 0;
            moduleInstance->numCables = 0;
            moduleInstance->totalHP = 0;
            moduleInstance->numModules = 0;

            // Recompute values
            auto moduleContainer = APP->scene->rack->moduleContainer;
            if (moduleContainer) {
                for (widget::Widget* w : moduleContainer->children) {
                    ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
                    if(moduleWidget) {
                        moduleInstance->numModules++;
                        moduleInstance->numParams += moduleWidget->params.size();
                        moduleInstance->numPorts += moduleWidget->outputs.size() + moduleWidget->inputs.size();
                        moduleInstance->totalHP += moduleWidget->panel->box.size.x / RACK_GRID_WIDTH;
                    }
                }
            }

            auto cableContainer = APP->scene->rack->cableContainer;
            for (widget::Widget* w : cableContainer->children) {
                CableWidget* cw = dynamic_cast<CableWidget*>(w);
                assert(cw);

                if (!cw->isComplete()) continue;
                moduleInstance->numCables += 1;
            }

            ModuleWidget::step();
        }
    }
};

Model* modelThisRack = createModel<ThisRack, ThisRackWidget>("this-rack");