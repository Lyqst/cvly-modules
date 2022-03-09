#include "plugin.hpp"
#include "manuals.hpp"

struct Txt : Module
{
    std::string slug;
    std::string text;
    bool dirty = false;

    Txt()
    {
        config(0, 0, 0, 0);
    }

    void step() override
    {
        Module *module = leftExpander.module;

        if ((module && module->model->slug != slug) || !module)
        {
            slug = module ? module->model->slug : "";
            text = manuals::text[slug];
            dirty = true;
        }
    }
};

struct TxtDisplayWidget : rack::LedDisplayTextField
{
    Txt *module;

    void step() override
    {
        TextField::cursor = 0;
        TextField::selection = 0;
        TextField::step();

        if (module && module->dirty)
        {
            setText(module->text);
            module->dirty = false;
        }
    }
};

struct TxtWidget : ModuleWidget
{
    TxtDisplayWidget *txtDisplay;

    TxtWidget(Txt *module)
    {
        setModule(module);
        setPanel(Svg::load(asset::plugin(pluginInstance, "res/txt.svg")));

        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        txtDisplay = createWidget<TxtDisplayWidget>(Vec(8, 16));
        txtDisplay->box.size = Vec(224, 310);
        txtDisplay->multiline = true;
        txtDisplay->color = nvgRGB(100, 246, 237);
        txtDisplay->setText("cvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules");
        txtDisplay->module = dynamic_cast<Txt *>(module);
        addChild(txtDisplay);
    }
};

Model *modelTxt = createModel<Txt, TxtWidget>("txt");