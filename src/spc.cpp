#include "plugin.hpp"

struct Spc : Module
{
    enum ParamIds
    {
        DIST_PARAM,
        OCT_PARAM,
        NOTE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        POLY_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        POLY_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    int base_note = 0;
    int base_octave = 0;
    int min_interval = 0;
    float cv[16] = {};
    float notes_out[16] = {};

    bool sort = false;

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        // sort
        json_object_set_new(rootJ, "sort", json_boolean(sort));

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        // sort
        json_t *sortJ = json_object_get(rootJ, "sort");
        if (sortJ)
            sort = json_boolean_value(sortJ);
    }

    Spc()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(DIST_PARAM, 0, 12, 0, "Min. interval", " semitones");
        configParam(OCT_PARAM, 0, 8, 4, "Octave", "");
        configParam(NOTE_PARAM, 0, 11, 0, "Note", "");
    }

    void process(const ProcessArgs &args) override
    {

        int channels = inputs[POLY_INPUT].getChannels();

        if (channels > 0)
        {
            bool update = false;
            for (int i = 0; i < channels; i++)
            {
                float in_cv = inputs[POLY_INPUT].getVoltage(i);
                if (cv[i] != in_cv)
                {
                    cv[i] = in_cv;
                    update = true;
                }
            }

            int note = params[NOTE_PARAM].getValue();
            if (base_note != note)
            {
                base_note = note;
                update = true;
            }

            int octave = params[OCT_PARAM].getValue();
            if (base_octave != octave)
            {
                base_octave = octave;
                update = true;
            }

            int interval = params[DIST_PARAM].getValue();
            if (min_interval != interval)
            {
                min_interval = interval;
                update = true;
            }

            if (update)
            {
                float temp_cv[16] = {};

                std::copy(cv, cv + 16, temp_cv);

                if (sort)
                {
                    std::sort(temp_cv, temp_cv + channels);
                }

                int notes[16] = {0};

                for (int i = 0; i < channels; i++)
                {
                    float iPtr;
                    float fract = modff(temp_cv[i], &iPtr);

                    if (fract < 0)
                        fract = (abs(fract) < 1e-7) ? 0.f : fract + 1.f;

                    notes[i] = (int)floor(fract * 12.f + 0.5f);

                    if (notes[i] == 12)
                        notes[i] = 0;
                }

                int oct = base_octave - 4;

                if (notes[0] < base_note)
                    oct++;

                notes_out[0] = oct + (notes[0] / 12.f);

                for (int i = 1; i < channels; i++)
                {
                    if (notes[i] - notes[i - 1] < min_interval)
                        oct++;

                    notes_out[i] = oct + (notes[i] / 12.f);
                }
            }

            for (int i = 0; i < channels; i++)
                outputs[POLY_OUTPUT].setVoltage(notes_out[i], i);

            outputs[POLY_OUTPUT].setChannels(channels);
        }
    }
};

struct SpcNoteWidget : rack::TransparentWidget
{
    Spc *module;
    std::shared_ptr<rack::Font> font;
    char str[4];
    static constexpr const char *notes = "CCDDEFFGGAAB";
    static constexpr const char *sharps = " # #  # # # ";

    SpcNoteWidget(rack::Vec pos, rack::Vec size, Spc *module)
    {
        box.size = size;
        box.pos = pos.minus(size.div(2));
        this->module = module;
        this->font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ninepin.regular.ttf"));
    }

    void getString()
    {
        if (this->module != NULL)
        {
            int note = this->module->base_note;
            int octave = this->module->base_octave;
            snprintf(str, sizeof(str), "%c%c%d", notes[note], sharps[note], octave);
        }
        else
        {
            snprintf(str, sizeof(str), "spc");
        }
    }

    void draw(const DrawArgs &args) override
    {
        NVGcolor textColor = nvgRGB(0x78, 0xD8, 0xC8);

        nvgFontSize(args.vg, 12);
        nvgFontFaceId(args.vg, this->font->handle);
        nvgTextLetterSpacing(args.vg, 1);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

        Vec textPos = Vec(box.size.x - 6, 18);
        nvgFillColor(args.vg, textColor);
        getString();
        nvgText(args.vg, textPos.x, textPos.y, str, NULL);
    }
};

struct SpcWidget : ModuleWidget
{
    SpcWidget(Spc *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/spc.svg")));

        addChild(createWidget<CustomScrew>(Vec(0, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        static const float xPos = RACK_GRID_WIDTH * 1.5;

        addInput(createInputCentered<CustomPort>(Vec(xPos, 38), module, Spc::POLY_INPUT));

        addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(xPos, 80), module, Spc::DIST_PARAM));

        addChild(new SpcNoteWidget(Vec(9, 137), Vec(39, 27), module));
        addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(xPos, 169), module, Spc::OCT_PARAM));
        addParam(createParamCentered<CustomSmallSwitchKnob>(Vec(xPos, 205), module, Spc::NOTE_PARAM));

        addOutput(createOutputCentered<CustomPortOut>(Vec(xPos, 285), module, Spc::POLY_OUTPUT));
    }

    void appendContextMenu(Menu *menu) override
    {
        Spc *module = dynamic_cast<Spc *>(this->module);

        menu->addChild(new MenuEntry);

        struct SortItem : MenuItem
        {
            Spc *module;
            void onAction(const event::Action &e) override
            {
                module->sort = !module->sort;
            }
        };
        SortItem *sortItem = createMenuItem<SortItem>("Sort input");
        sortItem->module = module;
        sortItem->rightText = CHECKMARK(module->sort);
        menu->addChild(sortItem);
    }
};

Model *modelSpc = createModel<Spc, SpcWidget>("spc");