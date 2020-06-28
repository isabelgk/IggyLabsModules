// Minimal wavetable oscillator by iggy.labs

#include "plugin.hpp"
#include "osdialog.h"
#include <vector>
#include "osc/wavetable.cpp"


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
		NUM_LIGHTS
	};

	Wavetable::Table* wavetable = nullptr;
	Wavetable::Oscillator oscillator[16];  // Maximum 16 channels of polyphony

	Table() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(Table::POS_PARAM, 0.0f, 1.0f, 0.0f, "Wavetable position");
		configParam(Table::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Coarse");
		configParam(Table::FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine");

		// This needs to exist for the child menu to get possible frame sizes for now
		// even though it will get overwritten when you load a real file.
		wavetable = new Wavetable::Table();
	}


	void loadWavetable(std::string path, int frameSize) {

		wavetable = new Wavetable::Table();
		wavetable->loadWavetable(path, frameSize);

		for (int i = 0; i < 16; i++) {
			oscillator[i].table = wavetable;
		}
	}


	void process(const ProcessArgs& args) override {
		int numChannels = std::max(1, inputs[FREQ_INPUT].getChannels());
		outputs[OUTPUT].setChannels(numChannels);

		for (int c = 0; c < 1; c++) {
			if (wavetable == nullptr) {
				outputs[OUTPUT].setVoltage(0.f, c);
			} else if (wavetable->loading) {
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
				oscillator[c].setPitch(pitch, args.sampleRate);

				// Set position (in terms of frame/cycle) in wavetable
				float pos = params[POS_PARAM].getValue();
				if (inputs[POS_INPUT].isConnected()) {
					pos += inputs[POS_INPUT].getPolyVoltage(c) / 10.f;
					pos = clamp(pos, 0.f, 1.f);
				}

				// Step forward in the table
				oscillator[c].step();

				// Get sample
				float out = oscillator[c].getSample(pos);

				// Send out
				outputs[OUTPUT].setVoltage(out, c);
			}
		}
	}

	// Save CPU by processing certain parameters less frequently
	void slowerProcess() {

	}
};

struct LoadFileItem : MenuItem {
	Table* module;
	int frameSize;
	void onAction(const event::Action& e) override {
		if (module->wavetable != nullptr) {
			char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
			if (path) {
				module->loadWavetable(path, frameSize);
				free(path);
			}
		}
	}
};

struct LoadFileMenu : MenuItem {
	Table* module;
	Menu* createChildMenu() override {
		Menu* menu = new Menu;
		for (int i = 0; i < 4; i++) {
			LoadFileItem* item = new LoadFileItem;
			std::vector<int> frameSizes = module->wavetable->frameSizes;

			item->text = string::f("%d samples/cycle", frameSizes[i]);
			item->rightText = CHECKMARK(module->wavetable->frameSize == frameSizes[i]);
			item->module = module;
			item->frameSize = frameSizes[i];
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

		// Knobs
		addParam(createParamCentered<IlKnob19>(mm2px(Vec(5.1, 46.0)), module, Table::POS_PARAM));
		addParam(createParamCentered<IlKnob19>(mm2px(Vec(5.1, 68.0)), module, Table::FINE_PARAM));
		addParam(createParamCentered<IlKnob19>(mm2px(Vec(5.1, 90.0)), module, Table::FREQ_PARAM));

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
		// LoadWavetableItem* sampleDirItem = new LoadWavetableItem;
		// sampleDirItem->text = "Load wavetable";
		// sampleDirItem->module = module;
		// menu->addChild(sampleDirItem);

		LoadFileMenu* loadFileMenu = new LoadFileMenu;
		loadFileMenu->text = "Load wavetable";
		loadFileMenu->module = module;
		menu->addChild(loadFileMenu);
	}
};


Model* modelTable = createModel<Table, TableWidget>("table");