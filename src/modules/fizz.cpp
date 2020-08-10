#include "../plugin.hpp"

struct Fizz : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        VEL_INPUT,
        GATE_INPUT,
        PITCH_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    Fizz() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

};

struct GreenPort : SvgPort {
    GreenPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/green/port.svg")));
    }
};

struct FizzWidget : ModuleWidget {
    FizzWidget(Fizz* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fizz.svg")));

		// Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<GreenPort>(mm2px(Vec(5.1, 70.56)), module, Fizz::VEL_INPUT));
        addInput(createInputCentered<GreenPort>(mm2px(Vec(5.1, 84.73)), module, Fizz::GATE_INPUT));
        addInput(createInputCentered<GreenPort>(mm2px(Vec(5.1, 98.9)), module, Fizz::PITCH_INPUT));
        addOutput(createOutputCentered<GreenPort>(mm2px(Vec(5.1, 112.0)), module, Fizz::OUTPUT));
    }
};

Model* modelFizz = createModel<Fizz, FizzWidget>("fizz");