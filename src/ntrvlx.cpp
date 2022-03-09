#include "plugin.hpp"

struct Ntrvlx : Module
{
    enum ParamIds
    {
        WEIGHT_PARAM,
        SNAP_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
        ENUMS(TRIGGER_OUTPUT, 4),
        NUM_OUTPUTS
    };

    enum LightIds
    {
        CONNECTED_LIGHT,
        NUM_LIGHTS
    };

    dsp::PulseGenerator pulseGenerator[4];
    float leftMessages[2][4] = {};
    bool poly_out = false;

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        // poly out
        json_object_set_new(rootJ, "poly out", json_boolean(poly_out));

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        // poly out
        json_t *polyJ = json_object_get(rootJ, "poly out");
        if (polyJ)
            poly_out = json_boolean_value(polyJ);
    }

    Ntrvlx()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(WEIGHT_PARAM, 0.f, 1.f, 1.f, "Stacking weight");
        configSwitch(SNAP_PARAM, 0, 1, 0, "Snap stacking", {"Off", "On"});
        for (int i = 0; i < 4; i++)
        {
            configOutput(TRIGGER_OUTPUT + i, "Seq " + std::to_string(i + 1) + " trigger");
        }
        leftExpander.producerMessage = leftMessages[0];
        leftExpander.consumerMessage = leftMessages[1];
    }

    void process(const ProcessArgs &args) override
    {
        bool motherPresent = leftExpander.module && leftExpander.module->model == modelNtrvlc;
        if (motherPresent)
        {
            float *messagesToMother = (float *)leftExpander.module->rightExpander.producerMessage;
            messagesToMother[0] = params[WEIGHT_PARAM].getValue();
            messagesToMother[1] = params[SNAP_PARAM].getValue();

            leftExpander.module->rightExpander.messageFlipRequested = true;

            float *messagesFromMother = (float *)leftExpander.consumerMessage;

            int p = 0;

            for (int i = 0; i < 4; i++)
            {
                if (messagesFromMother[i] > 0.1f)
                    pulseGenerator[i].trigger(1e-3f);

                bool pulse = pulseGenerator[i].process(args.sampleTime);
                outputs[TRIGGER_OUTPUT + i].setVoltage(pulse ? 10.0 : 0.0);

                if (poly_out && messagesFromMother[i] >= 0)
                    outputs[TRIGGER_OUTPUT].setVoltage(pulse ? 10.0 : 0.0, p++);
            }

            outputs[TRIGGER_OUTPUT].setChannels(poly_out ? p : 1);
        }

        lights[CONNECTED_LIGHT].setBrightness(motherPresent ? 1.f : 0.0f);
    }
};

struct NtrvlxWidget : ModuleWidget
{
    NtrvlxWidget(Ntrvlx *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ntrvlx.svg")));

        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 4, 0)));
        addChild(createWidget<CustomScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        int posX = 44;
        addParam(createParamCentered<CustomKnob>(Vec(posX, 32), module, Ntrvlx::WEIGHT_PARAM));
        addParam(createParamCentered<MediumSwitchButton>(Vec(posX, 74), module, Ntrvlx::SNAP_PARAM));
        addOutput(createOutputCentered<CustomPortOut>(Vec(posX, 123), module, Ntrvlx::TRIGGER_OUTPUT));
        addOutput(createOutputCentered<CustomPortOut>(Vec(posX, 183), module, Ntrvlx::TRIGGER_OUTPUT + 1));
        addOutput(createOutputCentered<CustomPortOut>(Vec(posX, 243), module, Ntrvlx::TRIGGER_OUTPUT + 2));
        addOutput(createOutputCentered<CustomPortOut>(Vec(posX, 303), module, Ntrvlx::TRIGGER_OUTPUT + 3));

        addChild(createLightCentered<SmallLight<CustomGreenLight>>(Vec(11, 311), module, Ntrvlx::CONNECTED_LIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        Ntrvlx *module = dynamic_cast<Ntrvlx *>(this->module);

        menu->addChild(new MenuEntry);

        struct PolyItem : MenuItem
        {
            Ntrvlx *module;
            void onAction(const event::Action &e) override
            {
                module->poly_out = !module->poly_out;
            }
        };
        PolyItem *polyItem = createMenuItem<PolyItem>("First output as poly");
        polyItem->rightText = CHECKMARK(module->poly_out);
        polyItem->module = module;
        menu->addChild(polyItem);
    }
};

Model *modelNtrvlx = createModel<Ntrvlx, NtrvlxWidget>("ntrvlx");