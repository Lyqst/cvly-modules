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
        NUM_LIGHTS
    };

    dsp::PulseGenerator pulseGenerator[4];
    float leftMessages[2][4] = {};
    float last_out_cv[4] = {0.f};

    Ntrvlx()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(WEIGHT_PARAM, 0.f, 1.f, 1.f, "Stacking weight");
        configParam(SNAP_PARAM, 0, 1, 0, "Snap stacking");

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
            for (int i = 0; i < 4; i++)
            {
                if (messagesFromMother[i] != last_out_cv[i])
                {
                    pulseGenerator[i].trigger(1e-3f);
                    last_out_cv[i] = messagesFromMother[i];
                }
                bool pulse = pulseGenerator[i].process(args.sampleTime);
                outputs[TRIGGER_OUTPUT + i].setVoltage(pulse ? 10.0 : 0.0);
            }
        }
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
    }
};

Model *modelNtrvlx = createModel<Ntrvlx, NtrvlxWidget>("ntrvlx");