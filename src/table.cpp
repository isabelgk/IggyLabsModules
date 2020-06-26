// Minimal wavetable oscillator by IggyLabs
#include "plugin.hpp"
#include "osdialog.h"
#include <vector>

#include "../lib/dr_wav.h"


struct Table : Module {
	enum ParamIds {
		FINE_PARAM,
		FM_PARAM,
		POS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FINE_INPUT,
		FM_INPUT,
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

	Table() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	// Number of samples in a single frame, default to 256
	const std::vector<int> frameSizes{ 256, 512, 1024, 2048 };
	int frameSize = 256;


	void loadWavetable(std::string path) {
		// unsigned int channels;
		// unsigned int sampleRate;
		// drwav_uint64 totalSampleCount;

		// float* sampleData;
		// sampleData = drwav_open_and_read_file_f32(path.c_str(), &channels, &sampleRate, &totalSampleCount);
	
		// if (sampleData != NULL) {

		// }
		DEBUG("%s", path);

	}

	void process(const ProcessArgs& args) override {

	}

	// Save CPU by processing certain parameters less frequently
	void slowerProcess() {

	}
};

struct SetFrameSizeItem : MenuItem {
	Table* module;
	int frameSize;
	void onAction(const event::Action& e) override {
		module->frameSize = frameSize;
	}
};

struct SetFrameSizeChildMenu : MenuItem {
	Table* module;
	Menu* createChildMenu() override {
		Menu* menu = new Menu;
		for (int i = 0; i < 4; i++) {
			SetFrameSizeItem* item = new SetFrameSizeItem;
			item->text = string::f("%d", module->frameSizes[i]);
			// item->rightText = CHECKMARK(module->frameSize == module->frameSizes[i]);
			item->module = module;
			item->frameSize = 1;
			menu->addChild(item);
		}

		return menu;
	}
};

struct LoadWavetableItem : MenuItem {
	Table* module;
	
	void onAction(const event::Action &e) override {
		char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		if (path) {
			
			module->loadWavetable(path);
			free(path);
		}
	}
};


struct TableWidget : ModuleWidget {
	TableWidget(Table* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/table.svg")));

		// Screws
		addChild(createWidget<ScrewSilver>(Vec((box.size.x - RACK_GRID_WIDTH) / 2, 0)));
		addChild(createWidget<ScrewSilver>(Vec((box.size.x - RACK_GRID_WIDTH) / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<IlKnob>(mm2px(Vec(5.1, 35.0)), module, Table::FINE_PARAM));
		addParam(createParamCentered<IlKnob>(mm2px(Vec(5.1, 57.0)), module, Table::FM_PARAM));
		addParam(createParamCentered<IlKnob>(mm2px(Vec(5.1, 79.0)), module, Table::POS_PARAM));

		// Inputs
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 46.0)), module, Table::FINE_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 68.0)), module, Table::FM_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 90.0)), module, Table::POS_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(5.1, 101.0)), module, Table::FREQ_INPUT));

		// Outputs
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(5.1, 112.0)), module, Table::OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Table* module = dynamic_cast<Table*>(this->module);

		menu->addChild(new MenuEntry);
		LoadWavetableItem* sampleDirItem = new LoadWavetableItem;
		sampleDirItem->text = "Load sample";
		sampleDirItem->module = module;
		menu->addChild(sampleDirItem);

		SetFrameSizeChildMenu* frameSizeMenu = new SetFrameSizeChildMenu;
		frameSizeMenu->text = "Set frame size";
		frameSizeMenu->module = module;
		menu->addChild(frameSizeMenu);
	}
};


Model* modelTable = createModel<Table, TableWidget>("table");