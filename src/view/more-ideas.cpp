#include "../plugin.hpp"
#include "widgets.hpp"

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
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
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
		PITCH_OUTPUT,
		SELECTED_TRIGGER_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(SELECTED_LIGHTS, 8),
		ENUMS(TRIGGERED_LIGHTS, 8),
		NUM_LIGHTS
	};

	Trigger clockTrigger;
	MoreIdeas::Model *stateModel = new MoreIdeas::Model();
	bool caDirty = true;

	More_ideas() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RULE_PARAM, 0.f, 255.f, 0.f, "Rule");
		configParam(SEED_PARAM, 0.f, 255.f, 0.f, "Seed");
		configParam(LOW_PARAM, 0.f, 28.f, 0.f, "Low");
		configParam(HIGH_PARAM, 0.f, 28.f, 14.f, "High");
		configParam(SCALE_PARAM, 0.f, 16.f, 0.f, "Scale");
		configParam(SELECT_PARAM, 0.f, 7.f, 0.f, "Select");
	}

	void onTrigger(const ProcessArgs& args) {
		this->stateModel->onTrigger();

		outputs[PITCH_OUTPUT].setVoltage(iggylabs::dsp::semitoneToCV(float(this->stateModel->note)));
	}

	void process(const ProcessArgs& args) override {
		if (this->clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
			onTrigger(args);
		}

		for (int i = 0; i < 8; i++) {
			if (this->stateModel->generation == nullptr) {
				outputs[BIT_OUTPUTS + i].setVoltage(clockTrigger.isHigh() && this->stateModel->seed->binaryArray[i] ? 10.f : 0.f);

				if (i == this->stateModel->bit) {
					outputs[SELECTED_TRIGGER_OUTPUT].setVoltage(clockTrigger.isHigh() && this->stateModel->seed->binaryArray[i] ? 10.f : 0.f);
				}

			} else {
				outputs[BIT_OUTPUTS + i].setVoltage(clockTrigger.isHigh() && this->stateModel->generation->binaryArray[i] ? 10.f : 0.f);
				if (i == this->stateModel->bit) {
					outputs[SELECTED_TRIGGER_OUTPUT].setVoltage(clockTrigger.isHigh() && this->stateModel->generation->binaryArray[i] ? 10.f : 0.f);
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
		}

		// SEED
		float seed = params[SEED_PARAM].getValue();
		if (inputs[SEED_INPUT].isConnected()) {
			seed += inputs[SEED_INPUT].getVoltageSum();
		}
		seed = clamp(seed, 0.f, 255.f);
		if (floor(seed) != this->stateModel->seed->integer) {
			stateModel->setSeed(floor(seed));
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
	int gridWidth = 128;

	CaDrawWidget(More_ideas* module) {
		this->module = module;
	}

	void draw(const DrawArgs& args) override {
		if (!module) return;

		float sizeX = box.size.x / float(gridWidth);
		float sizeY = box.size.y / float(gridWidth);

		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.f, 0.f, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(0, 16, 90));
		nvgFill(args.vg);
	}
};

struct More_ideasWidget : ModuleWidget {
	More_ideasWidget(More_ideas* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/more-ideas.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		CaFramebufferWidget* cfb = new CaFramebufferWidget(module);
		CaDrawWidget* cdw = new CaDrawWidget(module);
		cdw->box.size = mm2px(Vec(42.0, 42.0));
		cfb->box.pos = mm2px(Vec(2.54, 17.5));
		cfb->addChild(cdw);
		addChild(cfb);

		addParam(createParamCentered<IlKnobM>(mm2px(Vec(17.229, 66.425)), module, More_ideas::RULE_PARAM));
		addParam(createParamCentered<IlKnobM>(mm2px(Vec(39.99, 66.421)), module, More_ideas::SEED_PARAM));
		addParam(createParamCentered<IlKnobM>(mm2px(Vec(17.229, 79.201)), module, More_ideas::LOW_PARAM));
		addParam(createParamCentered<IlKnobM>(mm2px(Vec(39.99, 79.196)), module, More_ideas::HIGH_PARAM));
		addParam(createParamCentered<IlKnobM>(mm2px(Vec(17.229, 91.976)), module, More_ideas::SCALE_PARAM));
		addParam(createParamCentered<IlKnobM>(mm2px(Vec(39.99, 91.972)), module, More_ideas::SELECT_PARAM));

		addInput(createInputCentered<IlPort>(mm2px(Vec(7.428, 66.436)), module, More_ideas::RULE_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(30.279, 66.436)), module, More_ideas::SEED_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(7.428, 79.204)), module, More_ideas::LOW_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(30.279, 79.204)), module, More_ideas::HIGH_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(7.428, 91.973)), module, More_ideas::SCALE_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(30.279, 91.973)), module, More_ideas::SELECT_INPUT));
		addInput(createInputCentered<IlPort>(mm2px(Vec(7.428, 107.341)), module, More_ideas::CLOCK_INPUT));

		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 23.84)), module, More_ideas::BIT_OUTPUTS + 0));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 34.223)), module, More_ideas::BIT_OUTPUTS + 1));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 44.606)), module, More_ideas::BIT_OUTPUTS + 2));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 54.989)), module, More_ideas::BIT_OUTPUTS + 3));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 65.373)), module, More_ideas::BIT_OUTPUTS + 4));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 75.756)), module, More_ideas::BIT_OUTPUTS + 5));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 86.139)), module, More_ideas::BIT_OUTPUTS + 6));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.548, 96.522)), module, More_ideas::BIT_OUTPUTS + 7));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(44.056, 107.304)), module, More_ideas::PITCH_OUTPUT));
		addOutput(createOutputCentered<IlPort>(mm2px(Vec(54.545, 107.304)), module, More_ideas::SELECTED_TRIGGER_OUTPUT));

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
};


Model* modelMore_ideas = createModel<More_ideas, More_ideasWidget>("more-ideas");