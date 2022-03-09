#include "plugin.hpp"
#include <array>

struct Brst : Module
{
    enum ParamIds
    {
        MODE_PARAM,
        TIME_PARAM,
        VARIATON_PARAM,
        ENUMS(WEIGHT_PARAM, 8),
        ENUMS(SH_PARAM, 8),
        NUM_PARAMS
    };

    enum InputIds
    {
        GATE_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        ENUMS(CV_OUTPUT, 8),
        ENUMS(GATE_OUTPUT, 8),
        NUM_OUTPUTS
    };

    enum LightIds
    {
        MODE_1_LIGHT,
        MODE_2_LIGHT,
        MODE_3_LIGHT,
        ENUMS(OUTPUT_LIGHT, 8),
        NUM_LIGHTS
    };

    float time = 0.0f;
    float tgt = 0.0f;
    int mode = 1;
    bool keep_on = true;
    dsp::SchmittTrigger modeTrigger;
    std::array<int, 8> currentSteps = {0, 0, 0, 0, 0, 0, 0, 0};

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        // mode
        json_object_set_new(rootJ, "mode", json_integer(mode));

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        // mode
        json_t *modeJ = json_object_get(rootJ, "mode");
        if (modeJ)
            mode = json_integer_value(modeJ);
    }

    Brst()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configButton(MODE_PARAM, "Mode");
        configParam(TIME_PARAM, 0.0f, 10.0f, 1.0f, "Burst base time", " seconds");
        configParam(VARIATON_PARAM, 0.0f, 100.0f, 50.0f, "Burst time variation", "%");
        configInput(GATE_INPUT, "Gate");
        configInput(CV_INPUT, "CV");
        for (int i = 0; i < 8; i++)
        {
            std::string step = std::to_string(i + 1);
            configParam(WEIGHT_PARAM + i, 0.0f, 1.0f, 1.0f, "Step " + step + " weight");
            configSwitch(SH_PARAM + i, 0, 1, 0, "Step " + step + " sample & hold", {"Off", "On"});
            configOutput(CV_OUTPUT + i, "Step " + step + " CV");
            configOutput(GATE_OUTPUT + i, "Step " + step + " Gate");
        }
        random::init();
    };

    void process(const ProcessArgs &args) override
    {
        if (modeTrigger.process(params[MODE_PARAM].getValue()))
        {
            mode++;
            if (mode > 3)
                mode = 1;
            currentSteps.fill(0);
            if (mode == 3)
            {
                currentSteps[0] = 1;
            }
        }

        lights[MODE_1_LIGHT].setBrightness((mode == 1) ? 1.0f : 0.0f);
        lights[MODE_2_LIGHT].setBrightness((mode == 2) ? 1.0f : 0.0f);
        lights[MODE_3_LIGHT].setBrightness((mode == 3) ? 1.0f : 0.0f);

        float *in_cv = inputs[CV_INPUT].getVoltages();
        int channels = inputs[CV_INPUT].getChannels();

        bool gate = inputs[GATE_INPUT].isConnected() && inputs[GATE_INPUT].getVoltage() > 0.0f;

        if (gate)
        {
            bool out = false;

            if (tgt == 0.0f)
            {
                out = true;
                float base = params[TIME_PARAM].getValue();
                float var = params[VARIATON_PARAM].getValue() / 100.0f;
                tgt = base * (1 + var * (random::uniform() * 2 - 1));
            }

            if (time > tgt)
            {
                out = true;
                time = 0.0f;
            }

            time += args.sampleTime;

            if (out)
            {
                float sum = 0.0f;

                for (int i = 0; i < 8; i++)
                {
                    sum += params[WEIGHT_PARAM + i].getValue();
                }

                if (sum > 0.0f)
                {
                    switch (mode)
                    {
                    case 1:
                    {
                        int selected = 0;
                        float r = random::uniform() * sum;
                        sum = 0.0f;
                        for (int i = 0; i < 8; i++)
                        {
                            sum += params[WEIGHT_PARAM + i].getValue();
                            if (sum > r)
                            {
                                selected = i;
                                break;
                            }
                        }
                        currentSteps.fill(0);
                        currentSteps[selected] = 1;
                        break;
                    }
                    case 2:
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            currentSteps[i] = params[WEIGHT_PARAM + i].getValue() > random::uniform() ? 1 : 0;
                        }
                        break;
                    }
                    case 3:
                    {
                        int current = 0;
                        for (int i = 0; i < 8; i++)
                        {
                            if (currentSteps[i] == 1)
                            {
                                current = i;
                                break;
                            }
                        }
                        int next = current == 7 ? 0 : current + 1;
                        while (params[WEIGHT_PARAM + next].getValue() == 0.0f)
                            next = next == 7 ? 0 : next + 1;
                        if (params[WEIGHT_PARAM + next].getValue() > random::uniform())
                        {
                            currentSteps[current] = 0;
                            currentSteps[next] = 1;
                        }
                    }
                    }

                    for (int i = 0; i < 8; i++)
                    {
                        if (currentSteps[i] == 1 && params[SH_PARAM + i].getValue() == 1)
                        {
                            outputs[CV_OUTPUT + i].setChannels(channels);
                            for (int c = 0; c < channels; c++)
                            {
                                outputs[CV_OUTPUT + i].setVoltage(in_cv[c], c);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            tgt = 0.0f;
            time = 0.0f;
        }
        for (int i = 0; i < 8; i++)
        {
            if (currentSteps[i] == 1 && (gate || keep_on))
            {
                outputs[GATE_OUTPUT + i].setVoltage(10.0f);
                lights[OUTPUT_LIGHT + i].setBrightness(1.0f);
                if (params[SH_PARAM + i].getValue() == 0)
                {
                    outputs[CV_OUTPUT + i].setChannels(channels);
                    for (int c = 0; c < channels; c++)
                    {
                        outputs[CV_OUTPUT + i].setVoltage(in_cv[c], c);
                    }
                }
            }
            else
            {
                outputs[GATE_OUTPUT + i].setVoltage(0.0f);
                lights[OUTPUT_LIGHT + i].setBrightness(0.0f);
                if (params[SH_PARAM + i].getValue() == 0)
                {
                    outputs[CV_OUTPUT + i].setChannels(channels);
                    for (int c = 0; c < channels; c++)
                    {
                        outputs[CV_OUTPUT + i].setVoltage(0.0f, c);
                    }
                }
            }
        }
    };
};

struct BrstWidget : ModuleWidget
{
    BrstWidget(Brst *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/brst.svg")));

        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<CustomPort>(Vec(22, 55), module, Brst::CV_INPUT));
        addInput(createInputCentered<CustomPort>(Vec(22, 95), module, Brst::GATE_INPUT));
        addParam(createParamCentered<MediumButton>(Vec(22, 135), module, Brst::MODE_PARAM));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(8, 155), module, Brst::MODE_1_LIGHT));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(8, 175), module, Brst::MODE_2_LIGHT));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(8, 195), module, Brst::MODE_3_LIGHT));
        addParam(createParamCentered<CustomSmallKnob>(Vec(22, 235), module, Brst::TIME_PARAM));
        addParam(createParamCentered<CustomSmallKnob>(Vec(22, 275), module, Brst::VARIATON_PARAM));
        for (int i = 0; i < 8; i++)
        {
            addParam(createParamCentered<CustomSmallKnob>(Vec(57, 40 + 30 * (i + 1)), module, Brst::WEIGHT_PARAM + i));
            addParam(createParamCentered<MediumSwitchButton>(Vec(82, 40 + 30 * (i + 1)), module, Brst::SH_PARAM + i));
            addOutput(createOutputCentered<CustomPortOut>(Vec(110, 40 + 30 * (i + 1)), module, Brst::CV_OUTPUT + i));
            addOutput(createOutputCentered<CustomPortOut>(Vec(135, 40 + 30 * (i + 1)), module, Brst::GATE_OUTPUT + i));
            addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(153, 40 + 30 * (i + 1)), module, Brst::OUTPUT_LIGHT + i));
        }
    }
};

Model *modelBrst = createModel<Brst, BrstWidget>("brst");