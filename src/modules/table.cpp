// Minimal wavetable oscillator by iggy.labs

#include <array>
#include <string>
#include <vector>
#include "../plugin.hpp"
#include "osdialog.h"

#include "../widgets.hpp"
#include "../osc/wavetable.cpp"


struct Table : Module {
	enum ParamIds {
		FINE_PARAM,
		POS_PARAM,
		FREQ_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FINE_INPUT,
		POS_INPUT,
		FREQ_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LOADED_LIGHT,
		NUM_LIGHTS
	};

	Wavetable::Wavetable* wavetable;
	int currentPolyphony = 1;
	int loopCounter = 0;

	Table() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(Table::POS_PARAM, 0.0f, 1.0f, 0.0f, "Wavetable position");
		configParam(Table::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Coarse");
		configParam(Table::FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine");

		wavetable = new Wavetable::Wavetable();
	}



	void loadWavetable(std::string path, int cycleLength) {
		wavetable = new Wavetable::Wavetable();
		wavetable->loadWavetable(path, cycleLength);
	}

	// Save CPU by processing certain parameters less frequently
	void slowerProcess(const ProcessArgs& args) {
		// if (wavetables[0] == nullptr || !wavetables[0]->loaded) {
		if (wavetable == nullptr || !wavetable->loaded) {
			lights[LOADED_LIGHT].setBrightness(0.f);
		} else {
			lights[LOADED_LIGHT].setBrightness(1.f);
		}
	}

	void process(const ProcessArgs& args) override {
		if (loopCounter-- == 0) {
			loopCounter = 8;
			slowerProcess(args);
		}

		currentPolyphony = std::max(1, inputs[FREQ_INPUT].getChannels());
		outputs[OUTPUT].setChannels(currentPolyphony);
		for (int c = 0; c < currentPolyphony; c++) {
			if (wavetable == nullptr || wavetable->loading) {
				outputs[OUTPUT].setVoltage(0.f, c);
			} else {
				// Set pitch
				float pitch = params[FREQ_PARAM].getValue();
				if (inputs[FREQ_INPUT].isConnected()) {
					pitch += inputs[FREQ_INPUT].getPolyVoltage(c);
				}
				pitch += params[FINE_PARAM].getValue();
				if (inputs[FINE_INPUT].isConnected()) {
					pitch += inputs[FINE_INPUT].getPolyVoltage(c) / 5.f;
				}
				pitch = clamp(pitch, -3.5f, 3.5f);
				// wavetables[c]->setPitch(pitch, args.sampleRate);

				// Set position in wavetable (which cycle to access)
				float pos = params[POS_PARAM].getValue();
				if (inputs[POS_INPUT].isConnected()) {
					pos += inputs[POS_INPUT].getPolyVoltage(c) / 10.f;
					pos = clamp(pos, 0.f, 1.f);
				}

				// This does everything to update the phase, frequency, etc. before
				// returning the sample * 5 (to be in the 5V output range)
				float out = wavetable->process(c, pos, pitch, args.sampleRate) * 5.f;

				outputs[OUTPUT].setVoltage(out, c);
			}
		}
	}


	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_object_set_new(rootJ, "lastPath", json_string(wavetable->lastPath.c_str()));
		json_object_set_new(rootJ, "lastCycleLength", json_integer(wavetable->cycleLength));

		return rootJ; 
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* lastPathJ = json_object_get(rootJ, "lastPath");
		json_t* lastCycleLengthJ = json_object_get(rootJ, "lastCycleLength");

		if (lastPathJ && lastCycleLengthJ) {
			std::string lastPath = json_string_value(lastPathJ);
			int lastCycleLength = json_integer_value(lastCycleLengthJ);

			loadWavetable(lastPath, lastCycleLength);
		}
	}
};

struct LoadFileItem : MenuItem {
	Table* module;
	int cycleLength;
	void onAction(const event::Action& e) override {

		if (module->wavetable != nullptr) {
			osdialog_filters* filters = osdialog_filters_parse(".wav files:wav");
			char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
			if (path) {
				module->loadWavetable(path, cycleLength);
				free(path);
			}
			osdialog_filters_free(filters);
		}
	}
};

struct LoadFileMenu : MenuItem {
	Table* module;
	Menu* createChildMenu() override {
		Menu* menu = new Menu;
		for (int i = 0; i < 4; i++) {
			LoadFileItem* item = new LoadFileItem;
			std::vector<int> cycleLengths= Wavetable::cycleLengths;

			item->text = string::f("%d samples/cycle", cycleLengths[i]);
			item->rightText = CHECKMARK(module->wavetable->cycleLength == cycleLengths[i]);
			item->module = module;
			item->cycleLength = cycleLengths[i];
			menu->addChild(item);
		}

		return menu;
	}
};


struct TableWidget : ModuleWidget {
	TableWidget(Table* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/table.svg")));

		// Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Lights
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(5.1, 39.0)), module, Table::LOADED_LIGHT));

		// Knobs
		addParam(createParamCentered<IlKnobS>(mm2px(Vec(5.1, 46.0)), module, Table::POS_PARAM));
		addParam(createParamCentered<IlKnobS>(mm2px(Vec(5.1, 68.0)), module, Table::FINE_PARAM));
		addParam(createParamCentered<IlKnobS>(mm2px(Vec(5.1, 90.0)), module, Table::FREQ_PARAM));

		// Inputs
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 57.0)), module, Table::POS_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 79.0)), module, Table::FINE_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 101.0)), module, Table::FREQ_INPUT));

		// Outputs
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(5.1, 112.0)), module, Table::OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Table* module = dynamic_cast<Table*>(this->module);

		menu->addChild(new MenuEntry);
		LoadFileMenu* loadFileMenu = new LoadFileMenu;
		loadFileMenu->text = "Load wavetable";
		loadFileMenu->module = module;
		menu->addChild(loadFileMenu);
	}
};


Model* modelTable = createModel<Table, TableWidget>("table");