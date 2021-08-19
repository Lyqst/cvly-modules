#include "plugin.hpp"

struct Whl : Module
{
    Whl()
    {
        config(0, 0, 0, 0);
    }
};

struct WhlWidget : ModuleWidget
{
    WhlWidget(Whl *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/whl.svg")));

        addChild(createWidget<CustomScrew>(Vec(0, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH * 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }
};

Model *modelWhl = createModel<Whl, WhlWidget>("whl");