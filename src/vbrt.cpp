#include "plugin.hpp"

struct Vbrt : Module
{
    enum ParamIds
    {
        ENUMS(CENTS_PARAM, 8),
        ENUMS(MIRROR_PARAM, 8),
        ENUMS(FREQ_PARAM, 8),
        ENUMS(TO_POLY_PARAM, 8),
        NUM_PARAMS
    };

    enum InputIds
    {
        ENUMS(PITCH_INPUT, 8),
        ENUMS(SCALE_INPUT, 8),
        NUM_INPUTS
    };

    enum OutputIds
    {
        ENUMS(MONO_OUTPUT, 8),
        POLY_OUTPUT,
        NUM_OUTPUTS
    };

    float maxCents = 50.f;
    float maxFreq = 4.0f;
    float phase[8];
    float poly_v[16];

    Vbrt()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0);
        for (int i = 0; i < 8; i++)
        {
            std::string channel = std::to_string(i + 1);
            configInput(PITCH_INPUT + i, "Channel " + channel + " pitch");
            configInput(SCALE_INPUT + i, "Channel " + channel + " scale");
            configParam(CENTS_PARAM + i, 0.0f, maxCents, 0, "Channel " + channel + " cents ", " cents");
            configParam(FREQ_PARAM + i, 0.0f, maxFreq, 0, "Channel " + channel + " frequency ", " hz");
            configSwitch(MIRROR_PARAM + i, 0, 1, 0, "Channel " + channel + " mirror mode", {"Off", "On"});
            configSwitch(TO_POLY_PARAM + i, 0, 1, 0, "Channel " + channel + " to poly", {"Off", "On"});
            configOutput(MONO_OUTPUT + i, "Channel " + channel + " mono");
        }
        configOutput(POLY_OUTPUT, "Poly output");
    }

    void onReset() override
    {
        for (int i = 0; i < 8; i++)
        {
            phase[i] = 0.0f;
        }
    }

    void process(const ProcessArgs &args) override
    {
        float norm_pitch = 0.0f;
        int p_channel = 0;
        for (int i = 0; i < 8; i++)
        {
            if (inputs[PITCH_INPUT + i].isConnected())
                norm_pitch = inputs[PITCH_INPUT + i].getVoltage();

            float pitch = norm_pitch;

            if (outputs[MONO_OUTPUT + i].isConnected() || (outputs[POLY_OUTPUT].isConnected() && params[TO_POLY_PARAM + i].getValue() == 1))
            {
                float freq = params[FREQ_PARAM + i].getValue();
                if (freq < 0.0001f)
                {
                    phase[i] = 0.0f;
                }
                else
                {
                    float deltaPhase = freq * args.sampleTime;
                    phase[i] += deltaPhase;
                    phase[i] -= simd::trunc(phase[i]);
                }
                float s = inputs[SCALE_INPUT + i].isConnected() ? inputs[SCALE_INPUT + i].getVoltage() / 10.0f : 1.0f;
                float a = simd::cos(2 * M_PI * phase[i]) * s;
                float v = a * params[CENTS_PARAM + i].getValue() / 1200.0f;
                if (params[MIRROR_PARAM + i].getValue() == 1)
                {
                    outputs[MONO_OUTPUT + i].setChannels(2);
                    outputs[MONO_OUTPUT + i].setVoltage(pitch + v, 0);
                    outputs[MONO_OUTPUT + i].setVoltage(pitch - v, 1);
                    if (outputs[POLY_OUTPUT].isConnected() && params[TO_POLY_PARAM + i].getValue() == 1)
                    {
                        poly_v[p_channel++] = pitch + v;
                        poly_v[p_channel++] = pitch - v;
                    }
                }
                else
                {
                    outputs[MONO_OUTPUT + i].setChannels(1);
                    outputs[MONO_OUTPUT + i].setVoltage(pitch + v, 0);
                    if (outputs[POLY_OUTPUT].isConnected() && params[TO_POLY_PARAM + i].getValue() == 1)
                    {
                        poly_v[p_channel++] = pitch + v;
                    }
                }
            }
            if (outputs[POLY_OUTPUT].isConnected())
            {
                outputs[POLY_OUTPUT].setChannels(p_channel);
                for (int i = 0; i < p_channel; i++)
                    outputs[POLY_OUTPUT].setVoltage(poly_v[i], i);
            }
        }
    }
};

struct VbrtWidget : ModuleWidget
{
    VbrtWidget(Vbrt *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/vbrt.svg")));

        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 2, 0)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        for (int i = 0; i < 8; i++)
        {
            addInput(createInputCentered<CustomPort>(Vec(21, 35 + 30 * (i + 1)), module, Vbrt::PITCH_INPUT + i));
            addParam(createParamCentered<CustomSmallKnob>(Vec(46, 35 + 30 * (i + 1)), module, Vbrt::CENTS_PARAM + i));
            addInput(createInputCentered<CustomPort>(Vec(71, 35 + 30 * (i + 1)), module, Vbrt::SCALE_INPUT + i));
            addParam(createParamCentered<MediumSwitchButton>(Vec(96, 35 + 30 * (i + 1)), module, Vbrt::MIRROR_PARAM + i));
            addParam(createParamCentered<CustomSmallKnob>(Vec(121, 35 + 30 * (i + 1)), module, Vbrt::FREQ_PARAM + i));
            addParam(createParamCentered<MediumSwitchButton>(Vec(150, 35 + 30 * (i + 1)), module, Vbrt::TO_POLY_PARAM + i));
            addOutput(createOutputCentered<CustomPortOut>(Vec(175, 35 + 30 * (i + 1)), module, Vbrt::MONO_OUTPUT + i));
        }
        addOutput(createOutputCentered<CustomPortOut>(Vec(175, 305), module, Vbrt::POLY_OUTPUT));
    }
};

Model *modelVbrt = createModel<Vbrt, VbrtWidget>("vbrt");