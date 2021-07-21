#include "plugin.hpp"

struct Ntrvlc : Module
{
	enum ParamIds
	{
		STACK_PARAM,
		RESET_PARAM,
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
		LENGTH1_PARAM,
		LENGTH2_PARAM,
		LENGTH3_PARAM,
		LENGTH4_PARAM,
		ENUMS(ROW1_PARAM, 8),
		ENUMS(ROW2_PARAM, 8),
		ENUMS(ROW3_PARAM, 8),
		ENUMS(ROW4_PARAM, 8),
		NUM_PARAMS
	};
	enum InputIds
	{
		CLOCK1_INPUT,
		CLOCK2_INPUT,
		CLOCK3_INPUT,
		CLOCK4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		POLY_OUT_OUTPUT,
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
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

	Ntrvlc()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STACK_PARAM, 0, 1, 1, "Stack intervals");
		configParam(RESET_PARAM, 0, 1, 0, "Reset");
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
		configParam(LENGTH1_PARAM, 1, 8, 8, "Seq 1 length");
		configParam(LENGTH2_PARAM, 1, 8, 8, "Seq 2 length");
		configParam(LENGTH3_PARAM, 1, 8, 8, "Seq 3 length");
		configParam(LENGTH4_PARAM, 1, 8, 8, "Seq 4 length");
		for (int i = 0; i < 8; i++)
		{
			configParam(ROW1_PARAM + i, -1.f, 1.f, 0, "Seq 1, Step " + std::to_string(i + 1), "");
			configParam(ROW2_PARAM + i, -1.f, 1.f, 0, "Seq 2, Step " + std::to_string(i + 1), "");
			configParam(ROW3_PARAM + i, -1.f, 1.f, 0, "Seq 3, Step " + std::to_string(i + 1), "");
			configParam(ROW4_PARAM + i, -1.f, 1.f, 0, "Seq 4, Step " + std::to_string(i + 1), "");
		}
	}

	int range = 1;
	int num_notes = 1;
	float input_scale[12];
	float scale[13];
	int step1 = 0;
	int step2 = 0;
	int step3 = 0;
	int step4 = 0;
	int num_steps1 = 8;
	int num_steps2 = 8;
	int num_steps3 = 8;
	int num_steps4 = 8;
	bool stack = true;
	bool row1_on = false;
	bool row2_on = false;
	bool row3_on = false;
	bool row4_on = false;
	dsp::SchmittTrigger clock1Trigger;
	dsp::SchmittTrigger clock2Trigger;
	dsp::SchmittTrigger clock3Trigger;
	dsp::SchmittTrigger clock4Trigger;
	dsp::SchmittTrigger resetTrigger;
	int cycleCount = 1;
	int quant_cycle_count = 50;

	void process(const ProcessArgs &args) override
	{
		if (--cycleCount < 0)
		{
			cycleCount = quant_cycle_count;
		}

		process_params();

		float note = 0;
		float last_note = 0;
		float row_value;

		int i = 0;

		if (row1_on)
		{
			row_value = params[ROW1_PARAM + step1].getValue() * range;
			note = row_value;
			if (num_notes > 0)
				note = quantize_cv(note);
			last_note = note;
			outputs[OUT1_OUTPUT].setVoltage(note, 0);
			outputs[POLY_OUT_OUTPUT].setVoltage(note, i++);
		}
		else
			outputs[OUT1_OUTPUT].setVoltage(0, 0);

		if (row2_on)
		{

			row_value = params[ROW2_PARAM + step2].getValue() * range;
			note = (stack ? last_note : 0) + row_value;
			if (num_notes > 0)
				note = quantize_cv(note);
			last_note = note;
			outputs[OUT2_OUTPUT].setVoltage(note, 0);
			outputs[POLY_OUT_OUTPUT].setVoltage(note, i++);
		}
		else
			outputs[OUT2_OUTPUT].setVoltage(0, 0);

		if (row3_on)
		{
			row_value = params[ROW3_PARAM + step3].getValue() * range;
			note = (stack ? last_note : 0) + row_value;
			if (num_notes > 0)
				note = quantize_cv(note);
			last_note = note;
			outputs[OUT3_OUTPUT].setVoltage(note, 0);
			outputs[POLY_OUT_OUTPUT].setVoltage(note, i++);
		}
		else
			outputs[OUT3_OUTPUT].setVoltage(0, 0);

		if (row4_on)
		{
			row_value = params[ROW4_PARAM + step4].getValue() * range;
			note = (stack ? last_note : 0) + row_value;
			if (num_notes > 0)
				note = quantize_cv(note);
			outputs[OUT4_OUTPUT].setVoltage(note, 0);
			outputs[POLY_OUT_OUTPUT].setVoltage(note, i++);
		}
		else
			outputs[OUT4_OUTPUT].setVoltage(0, 0);

		outputs[POLY_OUT_OUTPUT].setChannels(i);
	}

	void process_params()
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

		row1_on = inputs[CLOCK1_INPUT].isConnected();
		row2_on = inputs[CLOCK2_INPUT].isConnected();
		row3_on = inputs[CLOCK3_INPUT].isConnected();
		row4_on = inputs[CLOCK4_INPUT].isConnected();

		num_steps1 = params[LENGTH1_PARAM].getValue();
		num_steps2 = params[LENGTH2_PARAM].getValue();
		num_steps3 = params[LENGTH3_PARAM].getValue();
		num_steps4 = params[LENGTH4_PARAM].getValue();

		if (row1_on)
		{
			if (clock1Trigger.process(inputs[CLOCK1_INPUT].getVoltage()))
			{
				int old_step = step1++;
				if (step1 >= num_steps1)
					step1 = 0;
				updateRowLights(ROW1_LIGHT, old_step, step1);
			}
		}
		else
		{
			updateRowLights(ROW1_LIGHT, step1);
			step1 = -1;
		}

		if (row2_on)
		{
			if (clock2Trigger.process(inputs[CLOCK2_INPUT].getVoltage()))
			{
				int old_step = step2++;
				if (step2 >= num_steps2)
					step2 = 0;
				updateRowLights(ROW2_LIGHT, old_step, step2);
			}
		}
		else
		{
			updateRowLights(ROW2_LIGHT, step2);
			step2 = -1;
		}

		if (row3_on)
		{
			if (clock3Trigger.process(inputs[CLOCK3_INPUT].getVoltage()))
			{
				int old_step = step3++;
				if (step3 >= num_steps3)
					step3 = 0;
				updateRowLights(ROW3_LIGHT, old_step, step3);
			}
		}
		else
		{
			updateRowLights(ROW3_LIGHT, step3);
			step3 = -1;
		}

		if (row4_on)
		{
			if (clock4Trigger.process(inputs[CLOCK4_INPUT].getVoltage()))
			{
				int old_step = step4++;
				if (step4 >= num_steps4)
					step4 = 0;
				updateRowLights(ROW4_LIGHT, old_step, step4);
			}
		}
		else
		{
			updateRowLights(ROW4_LIGHT, step4);
			step4 = -1;
		}

		if (resetTrigger.process(params[RESET_PARAM].getValue()))
		{
			updateRowLights(ROW1_LIGHT, step1, 0);
			updateRowLights(ROW1_LIGHT, step1, 0);
			updateRowLights(ROW1_LIGHT, step1, 0);
			updateRowLights(ROW1_LIGHT, step1, 0);
			step1 = 0;
			step2 = 0;
			step3 = 0;
			step4 = 0;
		}
	}

	float quantize_cv(float v)
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

	float quantize_note(float n, bool asc)
	{
		if (num_notes == 0)
		{
			return 0;
		}

		float oct;
		float fract = modff(n, &oct);
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
		int note = fract * 12;
		bool found = false;
		while (!found)
		{
			if (input_scale[note] > 0.f)
			{
				found = true;
			}
			else
			{
				if (asc)
				{
					note += 1.f;
					if (note > 11)
					{
						note = 0;
						oct += 1.f;
					}
				}
				else
				{
					note -= 1.f;
					if (note < 0)
					{
						note = 11;
						oct -= 1.f;
					}
				}
			}
		}

		return (oct + note / 12.f);
	}

	void updateRowLights(int row, int old_step, int new_step = -1)
	{
		lights[row + old_step].setBrightness(0.f);
		if (new_step > -1)
			lights[row + new_step].setBrightness(0.75f);
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

		addParam(createParamCentered<MediumSwitchButton>(Vec(233, 48), module, Ntrvlc::STACK_PARAM));
		addParam(createParamCentered<MediumButton>(Vec(30, 48), module, Ntrvlc::RESET_PARAM));
		addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(274, 48), module, Ntrvlc::LENGTH1_PARAM));
		addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(296, 48), module, Ntrvlc::LENGTH2_PARAM));
		addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(318, 48), module, Ntrvlc::LENGTH3_PARAM));
		addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(340, 48), module, Ntrvlc::LENGTH4_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(70, 65), module, Ntrvlc::QUANT1_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(80, 42), module, Ntrvlc::QUANT2_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(90, 65), module, Ntrvlc::QUANT3_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(100, 42), module, Ntrvlc::QUANT4_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(110, 65), module, Ntrvlc::QUANT5_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(130, 65), module, Ntrvlc::QUANT6_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(140, 42), module, Ntrvlc::QUANT7_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(150, 65), module, Ntrvlc::QUANT8_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(160, 42), module, Ntrvlc::QUANT9_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(170, 65), module, Ntrvlc::QUANT10_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(180, 42), module, Ntrvlc::QUANT11_PARAM));
		addParam(createParamCentered<MediumSwitchButton>(Vec(190, 65), module, Ntrvlc::QUANT12_PARAM));

		static const float portX[10] = {30, 70, 109, 148, 187, 226, 265, 304, 343, 386};
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 123), module, Ntrvlc::CLOCK1_INPUT));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 183), module, Ntrvlc::CLOCK2_INPUT));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 243), module, Ntrvlc::CLOCK3_INPUT));
		addInput(createInputCentered<CustomPort>(Vec(portX[0], 303), module, Ntrvlc::CLOCK4_INPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 57), module, Ntrvlc::POLY_OUT_OUTPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 123), module, Ntrvlc::OUT1_OUTPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 183), module, Ntrvlc::OUT2_OUTPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 243), module, Ntrvlc::OUT3_OUTPUT));
		addOutput(createOutputCentered<CustomPortOut>(Vec(portX[9], 303), module, Ntrvlc::OUT4_OUTPUT));
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