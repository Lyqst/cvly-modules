#include "plugin.hpp"

struct Ntrvlc : Module
{
	enum ParamIds
	{
		STACK_PARAM,
		QUANT1_PARAM,
		QUANT2_PARAM,
		QUANT3_PARAM,
		QUANT4_PARAM,
		QUANT5_PARAM,
		QUANT6_PARAM,
		QUANT7_PARAM,
		QUANT8_PARAM,
		QUANT9_PARAM,
		QUANT10_PARAM,
		QUANT11_PARAM,
		QUANT12_PARAM,
		ENUMS(LENGTH_PARAM, 4),
		ENUMS(ROW1_PARAM, 8),
		ENUMS(ROW2_PARAM, 8),
		ENUMS(ROW3_PARAM, 8),
		ENUMS(ROW4_PARAM, 8),
		NUM_PARAMS
	};
	enum InputIds
	{
		ENUMS(CLOCK_INPUT, 4),
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		POLY_OUTPUT,
		ENUMS(CV_OUTPUT, 4),
		NUM_OUTPUTS
	};

	enum LightIds
	{
		STACK_LED_LIGHT,
		ENUMS(ROW1_LIGHT, 8),
		ENUMS(ROW2_LIGHT, 8),
		ENUMS(ROW3_LIGHT, 8),
		ENUMS(ROW4_LIGHT, 8),
		NUM_LIGHTS
	};

	float rightMessages[2][4] = {};

	Ntrvlc()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STACK_PARAM, 0, 1, 1, "Stack intervals");
		configParam(QUANT1_PARAM, 0.f, 1.f, 0.f, "C");
		configParam(QUANT2_PARAM, 0.f, 1.f, 0.f, "C#");
		configParam(QUANT3_PARAM, 0.f, 1.f, 0.f, "D");
		configParam(QUANT4_PARAM, 0.f, 1.f, 0.f, "D#");
		configParam(QUANT5_PARAM, 0.f, 1.f, 0.f, "E");
		configParam(QUANT6_PARAM, 0.f, 1.f, 0.f, "F");
		configParam(QUANT7_PARAM, 0.f, 1.f, 0.f, "F#");
		configParam(QUANT8_PARAM, 0.f, 1.f, 0.f, "G");
		configParam(QUANT9_PARAM, 0.f, 1.f, 0.f, "G#");
		configParam(QUANT10_PARAM, 0.f, 1.f, 0.f, "A");
		configParam(QUANT11_PARAM, 0.f, 1.f, 0.f, "A#");
		configParam(QUANT12_PARAM, 0.f, 1.f, 0.f, "B");
		configParam(LENGTH_PARAM, 1, 8, 8, "Seq 1 length");
		configParam(LENGTH_PARAM + 1, 1, 8, 8, "Seq 2 length");
		configParam(LENGTH_PARAM + 2, 1, 8, 8, "Seq 3 length");
		configParam(LENGTH_PARAM + 3, 1, 8, 8, "Seq 4 length");
		for (int i = 0; i < 8; i++)
		{
			configParam(ROW1_PARAM + i, -1.f, 1.f, 0, "Seq 1, Step " + std::to_string(i + 1), "");
			configParam(ROW2_PARAM + i, -1.f, 1.f, 0, "Seq 2, Step " + std::to_string(i + 1), "");
			configParam(ROW3_PARAM + i, -1.f, 1.f, 0, "Seq 3, Step " + std::to_string(i + 1), "");
			configParam(ROW4_PARAM + i, -1.f, 1.f, 0, "Seq 4, Step " + std::to_string(i + 1), "");
		}

		rightExpander.producerMessage = rightMessages[0];
		rightExpander.consumerMessage = rightMessages[1];
	}

	int range = 1;
	int num_notes = 1;
	float input_scale[12];
	float scale[13];
	int step[4] = {-1};
	int length[4] = {8};
	float step_cv[4] = {0};
	float out_cv[4] = {0};
	bool stack = true;
	bool row_on[4] = {false};
	dsp::SchmittTrigger clockTrigger[4];
	dsp::SchmittTrigger resetTrigger;
	int cycleCount = 1;
	int quant_cycle_count = 50;
	float stack_weight = 1.f;
	bool stack_snap = false;

	void process(const ProcessArgs &args) override
	{
		if (--cycleCount < 0)
		{
			cycleCount = quant_cycle_count;
		}

		bool expanderPresent = (rightExpander.module && rightExpander.module->model == modelNtrvlx);
		float *messagesFromExpander = (float *)rightExpander.consumerMessage;

		if (expanderPresent)
		{
			stack_weight = messagesFromExpander[0];

			if (messagesFromExpander[1] > 0.1f)
				stack_snap = true;
			else
				stack_snap = false;

			rightExpander.module->leftExpander.messageFlipRequested = true;
		}
		else
		{
			stack_weight = 1.f;
			stack_snap = false;
		}

		processParams();

		bool update = processSequences();

		if (update)
		{
			updateLights();

			float last_cv = 0;
			int count = 0;

			for (int i = 0; i < 4; i++)
			{
				if (row_on[i])
				{
					step_cv[i] = (stack ? last_cv * stack_weight : 0) + params[ROW1_PARAM + i * 8 + step[i]].getValue() * range;
					step_cv[i] = stack && stack_snap && count > 0 ? step_cv[i] / (1.f + stack_weight) : step_cv[i];
					last_cv = step_cv[i];

					if (num_notes > 0)
						out_cv[i] = quantizeCv(step_cv[i]);
					else
						out_cv[i] = step_cv[i];

					count++;
				}
			}
		}

		int p = 0;
		for (int i = 0; i < 4; i++)
		{
			outputs[CV_OUTPUT + i].setVoltage(out_cv[i], 0);
			if (row_on[i])
				outputs[POLY_OUTPUT].setVoltage(out_cv[i], p++);

			if (expanderPresent)
			{
				float *messagesToExpander = (float *)(rightExpander.module->leftExpander.producerMessage);
				messagesToExpander[i] = out_cv[i];
			}
		}
		outputs[POLY_OUTPUT].setChannels(p);
	}

	void processParams()
	{
		if (cycleCount == 0)
		{
			input_scale[0] = params[QUANT1_PARAM].getValue();
			input_scale[1] = params[QUANT2_PARAM].getValue();
			input_scale[2] = params[QUANT3_PARAM].getValue();
			input_scale[3] = params[QUANT4_PARAM].getValue();
			input_scale[4] = params[QUANT5_PARAM].getValue();
			input_scale[5] = params[QUANT6_PARAM].getValue();
			input_scale[6] = params[QUANT7_PARAM].getValue();
			input_scale[7] = params[QUANT8_PARAM].getValue();
			input_scale[8] = params[QUANT9_PARAM].getValue();
			input_scale[9] = params[QUANT10_PARAM].getValue();
			input_scale[10] = params[QUANT11_PARAM].getValue();
			input_scale[11] = params[QUANT12_PARAM].getValue();

			int note = 0;
			for (int i = 0; i < 12; i++)
			{
				if (input_scale[i] > 0.0)
				{
					scale[note++] = i / 12.f;
				}
			}
			num_notes = note;
			scale[num_notes] = scale[0] + 1;
		}

		stack = params[STACK_PARAM].getValue() > 0.f;
	}

	bool processSequences()
	{
		bool update = false;

		for (int i = 0; i < 4; i++)
		{
			row_on[i] = inputs[CLOCK_INPUT + i].isConnected();

			if (length[i] != params[LENGTH_PARAM + i].getValue())
			{
				length[i] = params[LENGTH_PARAM + i].getValue();
				update = true;
			}

			if (row_on[i])
			{
				if (clockTrigger[i].process(inputs[CLOCK_INPUT + i].getVoltage()))
				{
					if (++step[i] >= length[i])
						step[i] = 0;
					update = true;
				}
			}
			else if (step[i] > -1)
			{
				step[i] = -1;
				update = true;
			}
		}

		if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
		{
			for (int i = 0; i < 4; i++)
			{
				step[i] = 0;
			}
			update = true;
		}

		return update;
	}

	float quantizeCv(float v)
	{
		int note;
		float oct;
		float fract = modff(v, &oct);
		if (oct < 0.f || fract < 0.f)
		{
			if (abs(fract) < 1e-7)
				fract = 0.f;
			else
			{
				fract += 1.f;
				oct -= 1.f;
			}
		}
		note = (int)(floor(num_notes * fract + 0.5f));

		if (note == 12)
		{
			note = 0;
			oct++;
		}

		return oct + scale[note];
	}

	void updateLights()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				float brightness = 0.f;
				if (step[i] == j)
					brightness = 0.9f;
				else if (j < length[i])
					brightness = 0.1f;
				lights[ROW1_LIGHT + i * 8 + j].setBrightness(brightness);
			}
		}
	}
};

struct NtrvlcWidget : ModuleWidget
{
	NtrvlcWidget(Ntrvlc *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ntrvlc.svg")));

		addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 4, 0)));
		addChild(createWidget<CustomScrew>(Vec(box.size.x - 5 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(233, 48), module, Ntrvlc::STACK_PARAM));
		addParam(createParamCentered<CustomSmallSwitchKnobNoRandom>(Vec(274, 48), module, Ntrvlc::LENGTH_PARAM));
		addParam(createParamCentered<CustomSmallSwitchKnobNoRandom>(Vec(296, 48), module, Ntrvlc::LENGTH_PARAM + 1));
		addParam(createParamCentered<CustomSmallSwitchKnobNoRandom>(Vec(318, 48), module, Ntrvlc::LENGTH_PARAM + 2));
		addParam(createParamCentered<CustomSmallSwitchKnobNoRandom>(Vec(340, 48), module, Ntrvlc::LENGTH_PARAM + 3));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(70, 65), module, Ntrvlc::QUANT1_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(80, 42), module, Ntrvlc::QUANT2_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(90, 65), module, Ntrvlc::QUANT3_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(100, 42), module, Ntrvlc::QUANT4_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(110, 65), module, Ntrvlc::QUANT5_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(130, 65), module, Ntrvlc::QUANT6_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(140, 42), module, Ntrvlc::QUANT7_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(150, 65), module, Ntrvlc::QUANT8_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(160, 42), module, Ntrvlc::QUANT9_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(170, 65), module, Ntrvlc::QUANT10_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(180, 42), module, Ntrvlc::QUANT11_PARAM));
		addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(190, 65), module, Ntrvlc::QUANT12_PARAM));

		static const float portX[10] = {30, 70, 109, 148, 187, 226, 265, 304, 343, 386};
		addInput(createInputCentered<CustomPort>(Vec(30, 48), module, Ntrvlc::RESET_INPUT));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 123), module, Ntrvlc::CLOCK_INPUT));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 183), module, Ntrvlc::CLOCK_INPUT + 1));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 243), module, Ntrvlc::CLOCK_INPUT + 2));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 303), module, Ntrvlc::CLOCK_INPUT + 3));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 57), module, Ntrvlc::POLY_OUTPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 123), module, Ntrvlc::CV_OUTPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 183), module, Ntrvlc::CV_OUTPUT + 1));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 243), module, Ntrvlc::CV_OUTPUT + 2));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 303), module, Ntrvlc::CV_OUTPUT + 3));
		for (int i = 0; i < 8; i++)
		{
			addParam(createParamCentered<CustomKnob>(Vec(portX[i + 1], 131), module, Ntrvlc::ROW1_PARAM + i));
			addParam(createParamCentered<CustomKnob>(Vec(portX[i + 1], 191), module, Ntrvlc::ROW2_PARAM + i));
			addParam(createParamCentered<CustomKnob>(Vec(portX[i + 1], 251), module, Ntrvlc::ROW3_PARAM + i));
			addParam(createParamCentered<CustomKnob>(Vec(portX[i + 1], 311), module, Ntrvlc::ROW4_PARAM + i));
			addChild(createLight<SmallLight<GreenLight>>(Vec(portX[i + 1] - 3, 102), module, Ntrvlc::ROW1_LIGHT + i));
			addChild(createLight<SmallLight<GreenLight>>(Vec(portX[i + 1] - 3, 162), module, Ntrvlc::ROW2_LIGHT + i));
			addChild(createLight<SmallLight<GreenLight>>(Vec(portX[i + 1] - 3, 222), module, Ntrvlc::ROW3_LIGHT + i));
			addChild(createLight<SmallLight<GreenLight>>(Vec(portX[i + 1] - 3, 282), module, Ntrvlc::ROW4_LIGHT + i));
		}
	}

	void appendContextMenu(Menu *menu) override
	{
		Ntrvlc *module = dynamic_cast<Ntrvlc *>(this->module);

		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Range"));

		struct RangeItem : MenuItem
		{
			Ntrvlc *module;
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

Model *modelNtrvlc = createModel<Ntrvlc, NtrvlcWidget>("ntrvlc");