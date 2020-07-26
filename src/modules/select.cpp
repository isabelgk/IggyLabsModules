#include "../plugin.hpp"

struct Select : Module {
	enum ParamIds {
		SELECT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(INPUTS, 16),
		NUM_INPUTS
	};
	enum OutputIds {
		THRU_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LIGHTS, 16),
		NUM_LIGHTS
	};

	int loopCounter = 0;
	int maxPolyphony = 1;

	Select() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SELECT_PARAM, 0.f, 15.f, 0.f, "Selected");
	}

	void slowerProcess(const ProcessArgs& args, int sel) {
		// Set lights
		for (int i = 0; i < 16; i++) {
			if (i == sel) {
				lights[i].setBrightness(1.f);
			} else {
				lights[i].setBrightness(0.f);
			}
		}
	}

	void process(const ProcessArgs& args) override {
		int selection = int(params[SELECT_PARAM].getValue());

		// Update lights and check polyphony less frequently
		if (loopCounter-- == 0) {
			loopCounter = 16;
			slowerProcess(args, selection);
		}

		maxPolyphony = inputs[INPUTS + selection].isConnected() ? inputs[INPUTS + selection].getChannels() : 1;

		for (int c = 0; c < maxPolyphony; c++) {
			outputs[THRU_OUTPUT].setVoltage(inputs[INPUTS + selection].getVoltage(c), c);
		}

		outputs[THRU_OUTPUT].setChannels(maxPolyphony);
	}
};

struct PurpleKnob : RoundKnob {
    PurpleKnob() {
		snap = true;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/purple/knob_m.svg")));
    }
};

struct PurplePort : SvgPort {
    PurplePort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/purple/port.svg")));
    }
};


struct SelectWidget : ModuleWidget {
	SelectWidget(Select* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/select.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<PurpleKnob>(mm2px(Vec(7.877, 107.036)), module, Select::SELECT_PARAM));

		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 23.805)), module, Select::INPUTS + 0));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 34.187)), module, Select::INPUTS + 1));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 44.569)), module, Select::INPUTS + 2));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 54.951)), module, Select::INPUTS + 3));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 65.333)), module, Select::INPUTS + 4));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 75.715)), module, Select::INPUTS + 5));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 86.097)), module, Select::INPUTS + 6));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(7.877, 96.478)), module, Select::INPUTS + 7));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 23.805)), module, Select::INPUTS + 8));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 34.187)), module, Select::INPUTS + 9));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 44.569)), module, Select::INPUTS + 10));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 54.951)), module, Select::INPUTS + 11));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 65.333)), module, Select::INPUTS + 12));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 75.715)), module, Select::INPUTS + 13));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 86.097)), module, Select::INPUTS + 14));
		addInput(createInputCentered<PurplePort>(mm2px(Vec(18.933, 96.478)), module, Select::INPUTS + 15));

		addOutput(createOutputCentered<PurplePort>(mm2px(Vec(18.933, 107.036)), module, Select::THRU_OUTPUT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 20.017)), module, Select::LIGHTS + 0));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 30.397)), module, Select::LIGHTS + 1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 40.778)), module, Select::LIGHTS + 2));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 51.159)), module, Select::LIGHTS + 3));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 61.54)), module, Select::LIGHTS + 4));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 71.921)), module, Select::LIGHTS + 5));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 82.302)), module, Select::LIGHTS + 6));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.505, 92.683)), module, Select::LIGHTS + 7));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 20.017)), module, Select::LIGHTS + 8));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 30.397)), module, Select::LIGHTS + 9));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 40.778)), module, Select::LIGHTS + 10));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 51.159)), module, Select::LIGHTS + 11));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 61.54)), module, Select::LIGHTS + 12));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 71.921)), module, Select::LIGHTS + 13));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 82.302)), module, Select::LIGHTS + 14));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(14.584, 92.683)), module, Select::LIGHTS + 15));
	}
};


Model* modelSelect = createModel<Select, SelectWidget>("select");