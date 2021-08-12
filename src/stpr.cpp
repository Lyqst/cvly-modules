#include "plugin.hpp"

struct Stpr : Module
{
	enum ParamIds
	{
		ENUMS(STEP_PARAM, 8),
		ENUMS(STEP_ON_PARAM, 8),
		NUM_PARAMS
	};
	enum InputIds
	{
		CLOCK_INPUT,
		RESET_INPUT,
		ENUMS(TRIGGER_INPUT, 8),
		NUM_INPUTS
	};
	enum OutputIds
	{
		CV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		ENUMS(STEP_LIGHT, 8),
		NUM_LIGHTS
	};

	Stpr()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 8; i++)
		{
			configParam(STEP_PARAM + i, -1.f, 1.f, 0, "Step " + std::to_string(i + 1), "");
			configParam(STEP_ON_PARAM + i, 0, 1, 1, "Step " + std::to_string(i + 1) + " on", "");
		}
	}

	int curr_step = 0;
	int range = 1;
	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger stepTrigger[8];

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		// range
		json_object_set_new(rootJ, "range", json_integer(range));

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		// range
		json_t *rangeJ = json_object_get(rootJ, "range");
		if (rangeJ)
			range = json_integer_value(rangeJ);
	}

	void process(const ProcessArgs &args) override
	{
		processStepTriggers();

		if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
		{
			int old_step = curr_step;
			curr_step = getNextStep();
			updateStepLights(old_step, curr_step);
		}

		if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
		{
			for (int i = 0; i < 8; i++)
			{
				params[STEP_ON_PARAM + i].setValue(1);
			}
			updateStepLights(curr_step);
			curr_step = 0;
		}

		float out_cv = params[STEP_PARAM + curr_step].getValue() * range;
		outputs[CV_OUTPUT].setVoltage(out_cv, 0);
	}

	void processStepTriggers()
	{
		for (int i = 0; i < 8; i++)
		{
			int step_on_value = params[STEP_ON_PARAM + i].getValue();
			if (stepTrigger[i].process(inputs[TRIGGER_INPUT + i].getVoltage()))
				step_on_value = step_on_value ? 0 : 1;
			params[STEP_ON_PARAM + i].setValue(step_on_value);
		}
	}

	int getNextStep()
	{
		int n = 1;
		while (n < 8)
		{
			if (curr_step + n < 8)
			{
				if (params[STEP_ON_PARAM + curr_step + n].getValue())
					return curr_step + n;
			}
			else
			{
				if (params[STEP_ON_PARAM + curr_step + n - 8].getValue())
					return curr_step + n - 8;
			}
			n++;
		}
		return curr_step;
	}

	void updateStepLights(int old_step, int new_step = -1)
	{
		lights[STEP_LIGHT + old_step].setBrightness(0.f);
		if (new_step > -1)
			lights[STEP_LIGHT + new_step].setBrightness(0.75f);
	}
};

struct StprWidget : ModuleWidget
{
	StprWidget(Stpr *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/stpr.svg")));

		addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 6, 0)));
		addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		static const float portY[8] = {45, 83, 121, 159, 197, 235, 273, 311};
		addInput(createInputCentered<CustomPort>(Vec(32, portY[0]), module, Stpr::CLOCK_INPUT));
		addInput(createInputCentered<CustomPort>(Vec(32, portY[2] - 22), module, Stpr::RESET_INPUT));
		for (int i = 0; i < 8; i++)
		{
			addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(85, portY[i]), module, Stpr::STEP_LIGHT + i));
			addParam(createParamCentered<CustomSmallKnob>(Vec(105, portY[i]), module, Stpr::STEP_PARAM + i));
			addParam(createParamCentered<SmallSwitchButtonNoRandom>(Vec(132, portY[i]), module, Stpr::STEP_ON_PARAM + i));
			addInput(createInputCentered<CustomPort>(Vec(152, portY[i]), module, Stpr::TRIGGER_INPUT + i));
		}

		addOutput(createOutputCentered<CustomPortOut>(Vec(32, portY[7]), module, Stpr::CV_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Stpr *module = dynamic_cast<Stpr *>(this->module);

		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Range"));

		struct RangeItem : MenuItem
		{
			Stpr *module;
			int range;
			void onAction(const event::Action &e) override
			{
				module->range = range;
			}
		};

		std::string labels[5] = {"-1v...1v", "-2v...2v", "-3v...3v", "-5v...5v"};
		int ranges[4] = {1, 2, 3, 5};
		for (int i = 0; i < 4; i++)
		{
			RangeItem *rangeItem = createMenuItem<RangeItem>(labels[i]);
			rangeItem->rightText = CHECKMARK(module->range == ranges[i]);
			rangeItem->module = module;
			rangeItem->range = ranges[i];
			menu->addChild(rangeItem);
		}
	}
};

Model *modelStpr = createModel<Stpr, StprWidget>("stpr");