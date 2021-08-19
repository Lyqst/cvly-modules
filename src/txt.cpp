#include "plugin.hpp"
#include "manuals.hpp"

struct Txt : Module
{
    int currModule;

    Txt()
    {
        config(0, 0, 0, 0);
    }

    void process(const ProcessArgs &args) override
    {
        Module *module = leftExpander.module;

        currModule = manuals::NONE;

        if (module)
        {
            for (int i = 1; i < manuals::NUM_MODULES; i++)
            {
                if (module->model->slug == manuals::slugs[i])
                {
                    currModule = i;
                    break;
                }
            }
        }
    }
};

struct TxtDisplayWidget : rack::LedDisplayTextField
{
    Txt *module;
    int currModule;
    TxtDisplayWidget(rack::Vec pos, rack::Vec size, Txt *module)
    {
        box.size = size;
        box.pos = pos;
        this->multiline = true;
        this->color = nvgRGB(0x78, 0xD8, 0xC8);
        this->module = module;
    }

    void onSelectText(const event::SelectText &e) override
    {
        e.stopPropagating();
    }

    void onSelectKey(const event::SelectKey &e) override
    {
        e.stopPropagating();
    }

    void draw(const DrawArgs &args) override
    {
        if (module != NULL)
        {
            if (currModule != module->currModule)
            {
                currModule = module->currModule;
                setText(manuals::text[currModule]);
            }
        }
        else
            setText("cvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules\ncvly modules");

        LedDisplayTextField::draw(args);
    }
};

struct TxtWidget : ModuleWidget
{
    TxtWidget(Txt *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/txt.svg")));

        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<CustomScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<CustomScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addChild(new TxtDisplayWidget(Vec(10, 25), Vec(220, 305), module));
    }
};

Model *modelTxt = createModel<Txt, TxtWidget>("txt");