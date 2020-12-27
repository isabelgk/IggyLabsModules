#include "../plugin.hpp"

#include "../dsp/pitch.hpp"
#include "../dsp/trigger.hpp"
#include "../model/more-ideas-model.cpp"
#include "../util/util.hpp"


struct More_ideas : Module {
	enum ParamIds {
		RULE_PARAM,
		SEED_PARAM,
		LOW_PARAM,
		HIGH_PARAM,
		SELECT_PARAM,
		SCALE_PARAM,
		CLOCK_OUT_PARAM,
		QUANTIZE_PARAM,
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		RULE_INPUT,
		SEED_INPUT,
		LOW_INPUT,
		HIGH_INPUT,
		SCALE_INPUT,
		SELECT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(BIT_OUTPUTS, 8),
		CV_OUTPUT,
		CLOCK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(SELECTED_LIGHTS, 8),
		ENUMS(TRIGGERED_LIGHTS, 8),
		NUM_LIGHTS
	};

	Trigger clockTrigger;
	dsp::BooleanTrigger resetTrigger;
	MoreIdeas::Model *stateModel = new MoreIdeas::Model();

	int loopCounter = 0;

	int gridWidth = 64;
	MoreIdeas::CA* ca = new MoreIdeas::CA(gridWidth);
	bool caDirty = true;
	bool scaleTextDirty = true;


	More_ideas() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RULE_PARAM, 0.f, 255.f, 90.f, "Rule");
		configParam(SEED_PARAM, 0.f, 255.f, 30.f, "Seed");
		configParam(LOW_PARAM, 0.f, 28.f, 0.f, "Low");
		configParam(HIGH_PARAM, 0.f, 28.f, 14.f, "High");
		configParam(SCALE_PARAM, 0.f, 16.f, 0.f, "Scale");
		configParam(SELECT_PARAM, 0.f, 7.f, 0.f, "Select");
		configParam(CLOCK_OUT_PARAM, 0.f, 1.f, 0.f, "Clock output mode");
		configParam(QUANTIZE_PARAM, 0.0, 1.f, 0.f, "Quantize output");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
	}

	// If Initializing the plugin were to reset the raw CV range, we would
	// do it like this, but bogaudio doesn't reset output ranges, so this
	// would be unexpected behavior for the user.
	// void onReset() {
	// 	this->stateModel->cvRangeIndex = 0;
	// }

	void onTrigger(const ProcessArgs& args) {
		this->stateModel->quantizeOutput = !int(params[QUANTIZE_PARAM].getValue());
		this->stateModel->onTrigger();

		// Update the pitch if the mode is set to always update the pitch
		// or if the mode is set to update when the selected trigger is on
		// and the trigger is indeed on
		bool alwaysUpdate = int(params[CLOCK_OUT_PARAM].getValue());
		int currentBit = this->stateModel->bit;
		bool currentBitOn;
		if (this->stateModel->generation == nullptr) {
			currentBitOn = this->stateModel->seed->binaryArray[currentBit];
		} else {
			currentBitOn = this->stateModel->generation->binaryArray[currentBit];
		}

		if ((!alwaysUpdate && currentBitOn) || alwaysUpdate){
			if (this->stateModel->quantizeOutput) {
				outputs[CV_OUTPUT].setVoltage(iggylabs::dsp::semitoneToCV(float(this->stateModel->note)));
			} else {
				outputs[CV_OUTPUT].setVoltage(this->stateModel->rawCvOut);
			}
		}
	}

	void onReset(const ProcessArgs& args) {
		this->stateModel->onReset();
	}

	void subSampledProcess(const ProcessArgs& args) {
		for (int i = 0; i < 8; i++) {
			if (this->stateModel->generation == nullptr) {
				outputs[BIT_OUTPUTS + i].setVoltage(clockTrigger.isHigh() && this->stateModel->seed->binaryArray[i] ? 10.f : 0.f);

				if (i == this->stateModel->bit) {
					outputs[CLOCK_OUTPUT].setVoltage(clockTrigger.isHigh() && this->stateModel->seed->binaryArray[i] ? 10.f : 0.f);
				}

			} else {
				outputs[BIT_OUTPUTS + i].setVoltage(clockTrigger.isHigh() && this->stateModel->generation->binaryArray[i] ? 10.f : 0.f);
				if (i == this->stateModel->bit) {
					outputs[CLOCK_OUTPUT].setVoltage(clockTrigger.isHigh() && this->stateModel->generation->binaryArray[i] ? 10.f : 0.f);
				}
			}
		}

		for (int i = 0; i < 8; i++) {
			if (this->stateModel->generation == nullptr) {
				lights[TRIGGERED_LIGHTS + i].setBrightness(this->stateModel->seed->binaryArray[i] ? 1.f : 0.f);
			} else {
				lights[TRIGGERED_LIGHTS + i].setBrightness(this->stateModel->generation->binaryArray[i] ? 1.f : 0.f);
			}

			lights[SELECTED_LIGHTS + i].setBrightness(this->stateModel->bit == i ? 1.f : 0.f);
		}

		// Rule
		float rule = params[RULE_PARAM].getValue();
		if (inputs[RULE_INPUT].isConnected()) {
			rule += inputs[RULE_INPUT].getVoltageSum();
		}
		rule = clamp(rule, 0.f, 255.f);

		// Only update if it changes
		if (floor(rule) != this->stateModel->rule->integer) {
			stateModel->setRule(floor(rule));
			this->caDirty = true;
			this->ca->setCells(floor(rule), this->stateModel->seed->integer);
		}

		// SEED
		float seed = params[SEED_PARAM].getValue();
		if (inputs[SEED_INPUT].isConnected()) {
			seed += inputs[SEED_INPUT].getVoltageSum();
		}
		seed = clamp(seed, 0.f, 255.f);
		if (floor(seed) != this->stateModel->seed->integer) {
			stateModel->setSeed(floor(seed));
			this->caDirty = true;
			this->ca->setCells(floor(rule), this->stateModel->seed->integer);
		}

		// LOW
		float low = params[LOW_PARAM].getValue();
		if (inputs[LOW_INPUT].isConnected()) {
			float inputLow = inputs[LOW_INPUT].getVoltageSum();
			low += iggylabs::util::rescale(inputLow, -10.f, 10.f, 0.f, 28.f);
		}
		low = clamp(low, 0.f, 28.f);
		stateModel->low = floor(low);

		// HIGH
		float high = params[HIGH_PARAM].getValue();
		if (inputs[HIGH_INPUT].isConnected()) {
			float inputHigh = inputs[HIGH_INPUT].getVoltageSum();
			high += iggylabs::util::rescale(inputHigh, -10.f, 10.f, 0.f, 28.f);
		}
		high = clamp(high, 0.f, 28.f);
		stateModel->high = floor(high);

		// SCALE
		float scale = params[SCALE_PARAM].getValue();
		if (inputs[SCALE_INPUT].isConnected()) {
			float inputscale = inputs[SCALE_INPUT].getVoltageSum();
			scale += iggylabs::util::rescale(inputscale, -10.f, 10.f, 0.f, 16.f);
		}
		scale = clamp(scale, 0.f, 16.f);
		if (stateModel->scaleIndex != floor(scale)) {
			this->scaleTextDirty = true;
		}
		stateModel->scaleIndex = floor(scale);

		// SELECT
		float select = params[SELECT_PARAM].getValue();
		if (inputs[SELECT_INPUT].isConnected()) {
			float inputSelect = inputs[SELECT_INPUT].getVoltageSum();
			select += iggylabs::util::rescale(inputSelect, -10.f, 10.f, 0.f, 7.f);
		}
		select = clamp(select, 0.f, 7.f);
		stateModel->bit = floor(select);
	}

	void process(const ProcessArgs& args) override {
		if (loopCounter-- == 0) {
			loopCounter = 8;
			subSampledProcess(args);
		}
		
		if (this->clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
			onTrigger(args);
		}

		if (resetTrigger.process(params[RESET_PARAM].getValue() > 0.f)) {
			onReset(args);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_object_set_new(rootJ, "lastCvRangeIndex", json_integer(this->stateModel->cvRangeIndex));

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* lastCvRangeIndexJ = json_object_get(rootJ, "lastCvRangeIndex");

		if (lastCvRangeIndexJ) {
			int lastCvRangeIndex = json_integer_value(lastCvRangeIndexJ);
			this->stateModel->cvRangeIndex = lastCvRangeIndex;
		}
	}
};


struct CaFramebufferWidget : FramebufferWidget {
	More_ideas* module;
	CaFramebufferWidget(More_ideas* module) {
		this->module = module;
	}

	void step() override {
		if (module) {
			if (module->caDirty) {
				FramebufferWidget::dirty = true;

				module->caDirty = false;
			}
			FramebufferWidget::step();
		}
	}
};

struct CaDrawWidget : OpaqueWidget {
	More_ideas* module;

	CaDrawWidget(More_ideas* module) {
		this->module = module;
	}

	void draw(const DrawArgs& args) override {
		if (!module || module->ca == nullptr) return;
		if (!module->ca->built) return;

		float sizeX = box.size.x / float(module->gridWidth);
		float sizeY = box.size.y / float(module->gridWidth);

		for (int i = 0; i < (int) module->ca->cells.size(); i++) {
			for (int j = 0; j < (int) module->ca->cells[i].size(); j++) {
				if (module->ca->cells[i][j]) {
					nvgBeginPath(args.vg);
					nvgRect(args.vg, j * sizeX, i * sizeY, sizeX, sizeY);
					nvgFillColor(args.vg, nvgRGB(224, 247, 250));
					nvgFill(args.vg);
				}
			}
		}
	}
};

struct TextFramebufferWidget : FramebufferWidget {
	More_ideas* module;
	TextFramebufferWidget(More_ideas* module) {
		this->module = module;
	}

	void step() override {
		if (module) {
			if (module->scaleTextDirty) {
				FramebufferWidget::dirty = true;

				module->scaleTextDirty = false;
			}
			FramebufferWidget::step();
		}
	}
};

struct TextDrawWidget : OpaqueWidget {
	More_ideas* module;
	std::string text = "";
	std::shared_ptr<Font> font;

	TextDrawWidget(More_ideas* module) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/font/Londrina_Solid/LondrinaSolid-Regular.ttf"));
		this->module = module;
	}

	void draw(const DrawArgs& args) override {
		if (!module) return;
		nvgFontSize(args.vg, 9);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, nvgRGB(0, 131, 143));
		text = module->stateModel->scaleNames[module->stateModel->scaleIndex];

		nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
		nvgTextAlign(args.vg, NVG_ALIGN_TOP);

		nvgText(args.vg, 0, 0, text.c_str(), NULL);
	}
};

struct CyanSwitch : app::SvgSwitch {
	CyanSwitch() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/cyan/switch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/cyan/switch_1.svg")));
	}
};

struct CyanButton : app::SvgSwitch {
	CyanButton() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/cyan/button_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/cyan/button_1.svg")));
	}
};

struct CyanKnob : RoundKnob {
	CyanKnob() {
		snap = true;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/cyan/knob_m.svg")));
	}
};

struct CyanPort : SvgPort {
	CyanPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/cyan/port.svg")));
	}
};

struct MenuItemRawCvOutRange : MenuItem {
	More_ideas* module;
	int ind;

	void onAction(const event::Action& e) override {
		module->stateModel->cvRangeIndex = ind;
	}
};

struct More_ideasWidget : ModuleWidget {
	More_ideasWidget(More_ideas* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/more-ideas.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		TextFramebufferWidget* tfb = new TextFramebufferWidget(module);
		TextDrawWidget* tdw = new TextDrawWidget(module);
		tdw->box.size = mm2px(Vec(40.0, 18.0));
		tfb->box.pos = mm2px(Vec(14, 96.66));
		tfb->addChild(tdw);
		addChild(tfb);

		CaFramebufferWidget* cfb = new CaFramebufferWidget(module);
		CaDrawWidget* cdw = new CaDrawWidget(module);
		cdw->box.size = mm2px(Vec(40.0, 40.0));
		cfb->box.pos = mm2px(Vec(3.54, 18.5));
		cfb->addChild(cdw);
		addChild(cfb);

		addParam(createParam<CyanSwitch>(mm2px(Vec(45.9, 106)), module, More_ideas::CLOCK_OUT_PARAM));
		addParam(createParam<CyanSwitch>(mm2px(Vec(30.16, 106)), module, More_ideas::QUANTIZE_PARAM));

		addParam(createParam<CyanButton>(mm2px(Vec(13.6, 102.85)), module, More_ideas::RESET_PARAM));

		addParam(createParamCentered<CyanKnob>(mm2px(Vec(17.229, 66.425)), module, More_ideas::RULE_PARAM));
		addParam(createParamCentered<CyanKnob>(mm2px(Vec(39.99, 66.421)), module, More_ideas::SEED_PARAM));
		addParam(createParamCentered<CyanKnob>(mm2px(Vec(17.229, 79.201)), module, More_ideas::LOW_PARAM));
		addParam(createParamCentered<CyanKnob>(mm2px(Vec(39.99, 79.196)), module, More_ideas::HIGH_PARAM));
		addParam(createParamCentered<CyanKnob>(mm2px(Vec(17.229, 91.976)), module, More_ideas::SCALE_PARAM));
		addParam(createParamCentered<CyanKnob>(mm2px(Vec(39.99, 91.972)), module, More_ideas::SELECT_PARAM));

		addInput(createInputCentered<CyanPort>(mm2px(Vec(7.428, 66.436)), module, More_ideas::RULE_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(30.279, 66.436)), module, More_ideas::SEED_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(7.428, 79.204)), module, More_ideas::LOW_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(30.279, 79.204)), module, More_ideas::HIGH_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(7.428, 91.973)), module, More_ideas::SCALE_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(30.279, 91.973)), module, More_ideas::SELECT_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(7.428, 107.341)), module, More_ideas::CLOCK_INPUT));
		addInput(createInputCentered<CyanPort>(mm2px(Vec(20.789, 107.341)), module, More_ideas::RESET_INPUT));

		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 23.84)), module, More_ideas::BIT_OUTPUTS + 0));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 34.223)), module, More_ideas::BIT_OUTPUTS + 1));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 44.606)), module, More_ideas::BIT_OUTPUTS + 2));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 54.989)), module, More_ideas::BIT_OUTPUTS + 3));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 65.373)), module, More_ideas::BIT_OUTPUTS + 4));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 75.756)), module, More_ideas::BIT_OUTPUTS + 5));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 86.139)), module, More_ideas::BIT_OUTPUTS + 6));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.548, 96.522)), module, More_ideas::BIT_OUTPUTS + 7));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(39.537, 107.304)), module, More_ideas::CV_OUTPUT));
		addOutput(createOutputCentered<CyanPort>(mm2px(Vec(54.545, 107.304)), module, More_ideas::CLOCK_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 21.204)), module, More_ideas::SELECTED_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 31.519)), module, More_ideas::SELECTED_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 41.835)), module, More_ideas::SELECTED_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 52.15)), module, More_ideas::SELECTED_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 62.465)), module, More_ideas::SELECTED_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 72.781)), module, More_ideas::SELECTED_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 83.096)), module, More_ideas::SELECTED_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(49.798, 93.412)), module, More_ideas::SELECTED_LIGHTS + 7));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 19.408)), module, More_ideas::TRIGGERED_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 29.724)), module, More_ideas::TRIGGERED_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 40.039)), module, More_ideas::TRIGGERED_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 50.355)), module, More_ideas::TRIGGERED_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 60.67)), module, More_ideas::TRIGGERED_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 70.985)), module, More_ideas::TRIGGERED_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 81.301)), module, More_ideas::TRIGGERED_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(51.971, 91.616)), module, More_ideas::TRIGGERED_LIGHTS + 7));
	}

	void appendContextMenu(Menu* menu) override {
		More_ideas* module = dynamic_cast<More_ideas*>(this->module);

		// CV Ranges
		menu->addChild(new MenuSeparator());
		MenuItem* label = new MenuItem;
		label->disabled = true;
		label->text = "Raw CV output range";
		menu->addChild(label);

		for (int i = 0; i < module->stateModel->NUM_CV_RANGES; i++) {
			MenuItemRawCvOutRange* item = new MenuItemRawCvOutRange;
			item->module = module;
			item->text = module->stateModel->cvRangeNames[i];
			item->rightText = CHECKMARK(module->stateModel->cvRangeIndex == i);
			item->ind = i;
			menu->addChild(item);
		}
	}
};


Model* modelMore_ideas = createModel<More_ideas, More_ideasWidget>("more-ideas");