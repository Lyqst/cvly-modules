#include "plugin.hpp"
#include "scales.hpp"

struct Crcl : Module
{
    enum ParamIds
    {
        MODE_PARAM,
        ENUMS(CIRCLE_PARAM, 12),
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        TRIGGER_INPUT,
        RESET_INPUT,
        ROOT_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CV_OUTPUT,
        TRIGGER_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        ENUMS(MODE_LIGHT, 3),
        ENUMS(CIRCLE_LIGHT, 12),
        NUM_LIGHTS
    };

    dsp::SchmittTrigger modeTrigger;
    int mode = 0;

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

    int notes = 12;
    int input_scale[12];
    float scale[13];
    int normalMapping[13] = {0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5, 0};
    int negativeMapping[13] = {7, 6, 5, 4, 3, 2, 1, 0, 11, 10, 9, 8, 7};
    dsp::SchmittTrigger inputTrigger[16];
    dsp::SchmittTrigger resetTrigger;
    dsp::PulseGenerator pulseGenerator[16];
    float prev_out_cv[16] = {};
    float in_cv[16] = {};
    float out_cv[16] = {};
    float out_trigger[16] = {};
    float out_count = 0;
    int step = 0;

    Crcl()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override
    {
        if (modeTrigger.process(params[MODE_PARAM].getValue()))
        {
            mode++;
            if (mode > 2)
                mode = 0;
        }

        int cv_channels = inputs[CV_INPUT].getChannels();
        int tg_channels = inputs[TRIGGER_INPUT].getChannels();
        int channels = std::max(cv_channels, tg_channels);

        if (mode == 1) // no polyphony in sequencer mode
            channels = 1;

        bool trigger_bool[16];
        for (int c = 0; c < tg_channels; c++)
        {
            trigger_bool[c] = inputTrigger[c].process(inputs[TRIGGER_INPUT].getVoltage(c));
        }

        bool light_bool[12] = {false};

        input_scale[0] = params[CIRCLE_PARAM].getValue();
        input_scale[1] = params[CIRCLE_PARAM + 7].getValue();   // m2
        input_scale[2] = params[CIRCLE_PARAM + 2].getValue();   // M2
        input_scale[3] = params[CIRCLE_PARAM + 9].getValue();   // m3
        input_scale[4] = params[CIRCLE_PARAM + 4].getValue();   // M3
        input_scale[5] = params[CIRCLE_PARAM + 11].getValue();  // P4
        input_scale[6] = params[CIRCLE_PARAM + 6].getValue();   // TT
        input_scale[7] = params[CIRCLE_PARAM + 1].getValue();   // P5
        input_scale[8] = params[CIRCLE_PARAM + 8].getValue();   // m6
        input_scale[9] = params[CIRCLE_PARAM + 3].getValue();   // M6
        input_scale[10] = params[CIRCLE_PARAM + 10].getValue(); // m7
        input_scale[11] = params[CIRCLE_PARAM + 5].getValue();  // M7

        int n = 0;

        for (int i = 0; i < 12; i++)
        {
            if (input_scale[mode == 1 ? normalMapping[i] : i] > 0.1)
                scale[n++] = (mode == 1 ? normalMapping[i] : (mode == 2 ? negativeMapping[i] : i)) / 12.f;
        }

        if (n == 0)
        {
            n = 1;
            scale[0] = 0;
        }

        scale[n] = scale[0] + 1;
        notes = n;

        if (mode == 0 || mode == 2)
        {
            for (int c = 0; c < channels; c++)
            {

                if (tg_channels == 0 || trigger_bool[c >= tg_channels ? tg_channels - 1 : c])
                {
                    in_cv[c] = inputs[CV_INPUT].getVoltage(c >= cv_channels ? cv_channels - 1 : c);
                }

                float root = inputs[ROOT_INPUT].getVoltage() - floor(inputs[ROOT_INPUT].getVoltage());

                int oct = floor(in_cv[c]);
                float diff = in_cv[c] - oct - root;

                if (diff < 0)
                {
                    diff = 1 + diff;
                    oct--;
                }

                int note = floor(diff * notes + 0.5);

                out_cv[c] = oct + root + scale[note];

                light_bool[normalMapping[(int)floor(scale[note] * 12)]] = true;
            }
        }
        else if (mode == 1)
        {
            if (inputs[TRIGGER_INPUT].isConnected() && trigger_bool[0])
            {
                if (inputs[CV_INPUT].getVoltage() >= 0 || !inputs[CV_INPUT].isConnected())
                {
                    if (++step >= notes)
                        step = 0;
                }
                else
                {
                    if (--step < 0)
                        step = notes - 1;
                }
            }

            if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
            {
                step = 0;
            }

            out_cv[0] = inputs[ROOT_INPUT].getVoltage() + scale[step];

            light_bool[normalMapping[(int)floor(scale[step] * 12)]] = true;
        }

        for (int i = 0; i < 3; i++)
        {
            lights[MODE_LIGHT + i].setBrightness(mode == i ? 1.f : 0);
        }

        for (int i = 0; i < 12; i++)
        {
            lights[CIRCLE_LIGHT + i].setBrightness(light_bool[i] ? 1.f : 0);
        }

        for (int i = 0; i < channels; i++)
        {
            if (out_cv[i] != prev_out_cv[i])
            {
                prev_out_cv[i] = out_cv[i];
                pulseGenerator[i].trigger(1e-3f);
            }
            outputs[CV_OUTPUT].setVoltage(out_cv[i], i);
            bool pulse = pulseGenerator[i].process(args.sampleTime);
            outputs[TRIGGER_OUTPUT].setVoltage(pulse ? 10.0 : 0.0, i);
        }

        outputs[CV_OUTPUT].setChannels(channels);
        outputs[TRIGGER_OUTPUT].setChannels(channels);
    }
};

struct CrclWidget : ModuleWidget
{
    CrclWidget(Crcl *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/crcl.svg")));

        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 4, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 15, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 18, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<CustomPort>(Vec(28, 38), module, Crcl::CV_INPUT));
        addInput(createInputCentered<CustomPort>(Vec(28, 90), module, Crcl::TRIGGER_INPUT));
        addInput(createInputCentered<CustomPort>(Vec(28, 142), module, Crcl::RESET_INPUT));

        addParam(createParamCentered<MediumButtonNoRandom>(Vec(150, 37), module, Crcl::MODE_PARAM));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(130, 57), module, Crcl::MODE_LIGHT));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(130, 68), module, Crcl::MODE_LIGHT + 1));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(130, 79), module, Crcl::MODE_LIGHT + 2));

        addOutput(createOutputCentered<CustomPortOut>(Vec(267, 38), module, Crcl::CV_OUTPUT));
        addOutput(createOutputCentered<CustomPortOut>(Vec(267, 90), module, Crcl::TRIGGER_OUTPUT));

        Vec center = Vec(150, 213);
        addInput(createInputCentered<CustomPort>(center, module, Crcl::ROOT_INPUT));
        float r = 92;
        float r2 = 109;
        for (int i = 0; i < 12; i++)
        {
            addParam(createParamCentered<MediumSwitchButtonNoRandom>(Vec(center.x + r * sin(M_PI * i / 6), center.y - r * cos(M_PI * i / 6)), module, Crcl::CIRCLE_PARAM + i));
            addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(center.x + r2 * sin(M_PI * i / 6), center.y - r2 * cos(M_PI * i / 6)), module, Crcl::CIRCLE_LIGHT + i));
        }
    }

    void appendContextMenu(Menu *menu) override
    {
        Crcl *module = dynamic_cast<Crcl *>(this->module);

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Mode"));

        struct ModeItem : MenuItem
        {
            Crcl *module;
            int mode;
            void onAction(const event::Action &e) override
            {
                module->mode = mode;
            }
        };

        std::string labels[5] = {"Quant + SH", "Sequencer", "Negative harmony"};
        int modes[3] = {0, 1, 2};
        for (int i = 0; i < 3; i++)
        {
            ModeItem *modeItem = createMenuItem<ModeItem>(labels[i]);
            modeItem->rightText = CHECKMARK(module->mode == modes[i]);
            modeItem->module = module;
            modeItem->mode = modes[i];
            menu->addChild(modeItem);
        }

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Load Scale:"));

        struct ScaleItem : MenuItem
        {
            Crcl *module;
            const int *scale;
            void onAction(const event::Action &e) override
            {
                for (int i = 0; i < 12; i++)
                {
                    module->params[module->CIRCLE_PARAM + module->normalMapping[i]].setValue(scale[i]);
                }
            }
        };

        for (int i = 0; i < scales::getNumberOfScales(); i++)
        {
            ScaleItem *scaleItem = createMenuItem<ScaleItem>(scales::getScaleName(i));
            scaleItem->module = module;
            scaleItem->scale = scales::getScale(i);
            menu->addChild(scaleItem);
        }
    }
};

Model *modelCrcl = createModel<Crcl, CrclWidget>("crcl");