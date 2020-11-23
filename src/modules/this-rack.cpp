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

    ThisRack() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

};

// https://community.vcvrack.com/t/advanced-nanovg-custom-label/6769/12
struct ThisRackTextWidget : OpaqueWidget {
    std::shared_ptr<Font> font;
    int fontSize = 24;
    int* val = 0;

    ThisRackTextWidget(int x, int y, int* v) {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/font/open24displaySt/Open 24 Display St.ttf"));
        box.pos = Vec(x, y);
        val = v;
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
            
            nvgText(args.vg, 0, 0, std::to_string(*val).c_str(), NULL);
            nvgStroke(args.vg);

            bndSetFont(APP->window->uiFont->handle);
        }
    }
};

struct ThisRackWidget : ModuleWidget {
    int numParams = 0;
    int numPorts = 0;
    int numCables = 0;
    int numModules = 0;
    int totalHP = 0;
    int loopCounter = 0;

    ThisRackWidget(ThisRack* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/this-rack.svg")));

        ThisRackTextWidget* portsLabel = new ThisRackTextWidget(37.5, 75, &numPorts);
        addChild(portsLabel);
        
        ThisRackTextWidget* paramLabel = new ThisRackTextWidget(37.5, 135, &numParams);
        addChild(paramLabel);
        
        ThisRackTextWidget* cablesLabel = new ThisRackTextWidget(37.5, 195, &numCables);
        addChild(cablesLabel);
        
        ThisRackTextWidget* hpLabel = new ThisRackTextWidget(37.5, 255, &totalHP);
        addChild(hpLabel);

        ThisRackTextWidget* modulesLabel = new ThisRackTextWidget(37.5, 314, &numModules);
        addChild(modulesLabel);
    }

    /** Count the number of params, ports, cables, and HP*/
    void step() override {
        if (module) {
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
                if (moduleWidget) {
                    numModules++;
                    numParams += moduleWidget->params.size();
                    numPorts += moduleWidget->outputs.size() + moduleWidget->inputs.size();
                    totalHP += moduleWidget->panel->box.size.x / RACK_GRID_WIDTH;
                }

            }

            auto cableContainer = APP->scene->rack->cableContainer;
            for (widget::Widget* w : cableContainer->children) {
                CableWidget* cw = dynamic_cast<CableWidget*>(w);
                if (cw) {
                    if (!cw->isComplete()) continue;
                    numCables += 1;
                }

            }
        }

        ModuleWidget::step();
    }
};

Model* modelThisRack = createModel<ThisRack, ThisRackWidget>("this-rack");