#include "plugin.hpp"

struct Bss : Module
{
    enum ParamIds
    {
        MODE_PARAM,
        NOTE_PARAM,
        OCT_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CV_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        CV_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(MODE_LIGHT, 3),
        NUM_LIGHTS
    };

    Bss()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configButton(MODE_PARAM, "Mode");
        configParam(OCT_PARAM, 0, 8, 4, "Octave", "");
        configParam(NOTE_PARAM, 0, 11, 0, "Note", "");
        getParamQuantity(MODE_PARAM)->randomizeEnabled = false;
        configInput(CV_INPUT, "CV");
        configInput(GATE_INPUT, "Gate");
        configOutput(CV_OUTPUT, "CV");
    }

    dsp::SchmittTrigger inputTrigger;
    dsp::SchmittTrigger modeTrigger;
    int mode = 0;
    int note = 0;
    int octave = 0;
    int min_polyphony = 1;

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        // mode
        json_object_set_new(rootJ, "mode", json_integer(mode));

        // min_polyphony
        json_object_set_new(rootJ, "min_polyphony", json_integer(min_polyphony));

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        // mode
        json_t *modeJ = json_object_get(rootJ, "mode");
        if (modeJ)
            mode = json_integer_value(modeJ);

        // min_polyphony
        json_t *polyJ = json_object_get(rootJ, "min_polyphony");
        if (polyJ)
            min_polyphony = json_integer_value(polyJ);
    }

    float cv[16] = {};
    bool gates[16] = {false};
    int note_out = 0;
    int scoreMapping[12] = {0, 12, 6, 2, 2, 12, 6, 0, 12, 12, 4, 4};

    void process(const ProcessArgs &args) override
    {
        if (modeTrigger.process(params[MODE_PARAM].getValue()))
        {
            mode++;
            if (mode > 2)
                mode = 0;
        }

        int channels = inputs[CV_INPUT].getChannels();
        int open_gates = 0;
        bool gated = inputs[GATE_INPUT].isConnected();

        bool update = false;
        for (int i = 0; i < channels; i++)
        {
            float in_cv = inputs[CV_INPUT].getVoltage(i);
            bool gate = gated && inputs[GATE_INPUT].getVoltage(i) > 0.0f;

            if (gate != gates[i])
                update = true;

            if (gate || !gated)
            {
                if (cv[open_gates] != in_cv)
                {
                    cv[open_gates] = in_cv;
                    update = true;
                }
                open_gates++;
            }
        }
        channels = open_gates;

        if (update && channels >= min_polyphony)
        {
            if (mode == 0) // harmonic
            {
                int notes[16] = {0};

                for (int i = 0; i < channels; i++)
                {
                    float iPtr;
                    float fract = modff(cv[i], &iPtr);

                    if (fract < 0)
                        fract = (abs(fract) < 1e-7) ? 0.f : fract + 1.f;

                    notes[i] = (int)floor(fract * 12.f + 0.5f);

                    if (notes[i] == 12)
                        notes[i] = 0;
                }

                int min = 9999;
                for (int i = 0; i < channels; i++)
                {
                    int sum = 0;
                    for (int j = 0; j < channels; j++)
                    {
                        int diff = notes[j] - notes[i];
                        if (diff < 0)
                            diff += 12;
                        sum += scoreMapping[diff];
                    }
                    if (sum < min)
                    {
                        min = sum;
                        note_out = notes[i];
                    }
                }
            }

            else if (mode == 1) // lowest
            {
                float lowest = 99;

                for (int i = 0; i < channels; i++)
                {
                    if (cv[i] < lowest)
                        lowest = cv[i];
                }

                float iPtr;
                float fract = modff(lowest, &iPtr);

                if (fract < 0)
                    fract = (abs(fract) < 1e-7) ? 0.f : fract + 1.f;

                note_out = (int)floor(fract * 12.f + 0.5f);
            }
            else
            { // random
                float rand = random::uniform();
                int n = floor(rand * channels);

                float iPtr;
                float fract = modff(cv[n], &iPtr);

                if (fract < 0)
                    fract = (abs(fract) < 1e-7) ? 0.f : fract + 1.f;

                note_out = (int)floor(fract * 12.f + 0.5f);
            }
        }

        for (int i = 0; i < 3; i++)
        {
            lights[MODE_LIGHT + i].setBrightness(mode == i ? 1.f : 0);
        }

        octave = params[OCT_PARAM].getValue();
        note = params[NOTE_PARAM].getValue();

        int oct_out = octave - 4;
        if (note_out < note)
            oct_out++;
        outputs[CV_OUTPUT].setVoltage(oct_out + (note_out / 12.f));
    }
};

struct BssNoteWidget : rack::TransparentWidget
{
    Bss *module;
    char str[4];
    static constexpr const char *notes = "CCDDEFFGGAAB";
    static constexpr const char *sharps = " # #  # # # ";
    const std::string fontPath = "res/fonts/fx-4000p.ttf";

    BssNoteWidget(rack::Vec pos, rack::Vec size, Bss *module)
    {
        box.size = size;
        box.pos = pos.minus(size.div(2));
        this->module = module;
    }

    void getString()
    {
        if (this->module != NULL)
        {
            int note = this->module->note;
            int octave = this->module->octave;
            snprintf(str, sizeof(str), "%c%c%d", notes[note], sharps[note], octave);
        }
        else
        {
            snprintf(str, sizeof(str), "bss");
        }
    }

    void draw(const DrawArgs &args) override
    {
        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, fontPath));
        if (font)
        {
            nvgFontFaceId(args.vg, font->handle);

            nvgFontSize(args.vg, 11);
            nvgTextLetterSpacing(args.vg, 1);
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

            nvgFillColor(args.vg, nvgRGB(100, 246, 237));
            getString();
            Vec textPos = Vec(box.size.x - 6, 18);
            nvgText(args.vg, textPos.x, textPos.y, str, NULL);
        }
    }
};

struct BssPolyValueItem : MenuItem
{
    Bss *module;
    int poly;
    void onAction(const event::Action &e) override
    {
        module->min_polyphony = poly;
    }
};

struct BssPolyItem : MenuItem
{
    Bss *module;
    Menu *createChildMenu() override
    {
        Menu *menu = new Menu;
        for (int poly = 1; poly <= 16; poly++)
        {
            BssPolyValueItem *item = new BssPolyValueItem;
            item->text = std::to_string(poly);
            item->rightText = CHECKMARK(module->min_polyphony == poly);
            item->module = module;
            item->poly = poly;
            menu->addChild(item);
        }
        return menu;
    }
};

struct BssWidget : ModuleWidget
{
    BssWidget(Bss *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/bss.svg")));

        addChild(createWidget<CustomScrew>(Vec(0, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        static const float xPos = RACK_GRID_WIDTH * 1.5;

        addInput(createInputCentered<CustomPort>(Vec(xPos, 33), module, Bss::CV_INPUT));
        addInput(createInputCentered<CustomPort>(Vec(xPos, 73), module, Bss::GATE_INPUT));

        addParam(createParamCentered<MediumButton>(Vec(xPos, 113), module, Bss::MODE_PARAM));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(xPos - 15, 130), module, Bss::MODE_LIGHT));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(xPos - 15, 141), module, Bss::MODE_LIGHT + 1));
        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(xPos - 15, 152), module, Bss::MODE_LIGHT + 2));

        addChild(new BssNoteWidget(Vec(9, 190), Vec(39, 27), module));
        addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(xPos, 216), module, Bss::OCT_PARAM));
        addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(xPos, 248), module, Bss::NOTE_PARAM));

        addOutput(createOutputCentered<CustomPortOut>(Vec(xPos, 298), module, Bss::CV_OUTPUT));
    }

    void appendContextMenu(Menu *menu) override
    {
        Bss *bss = dynamic_cast<Bss *>(module);
        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        BssPolyItem *polyItem = new BssPolyItem;
        polyItem->text = "Minimum polyphony";
        polyItem->rightText = std::to_string(bss->min_polyphony) + " " + RIGHT_ARROW;
        polyItem->module = bss;
        menu->addChild(polyItem);
    }
};

Model *modelBss = createModel<Bss, BssWidget>("bss");