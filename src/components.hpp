using namespace rack;

extern Plugin *pluginInstance;

struct CustomKnob : SvgKnob
{
	CustomKnob()
	{
		minAngle = -0.78 * M_PI;
		maxAngle = 0.78 * M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob.svg")));
	}
};

struct CustomSmallKnob : SvgKnob
{
	CustomSmallKnob()
	{
		minAngle = -0.78 * M_PI;
		maxAngle = 0.78 * M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobS.svg")));
	}
};

struct CustomSmallSwitchKnob : SvgKnob
{
	CustomSmallSwitchKnob()
	{
		minAngle = -0.78 * M_PI;
		maxAngle = 0.78 * M_PI;
		snap = true;
		smooth = false;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobS.svg")));
	}
};

struct SmallSwitchButton : SvgSwitch
{
	SmallSwitchButton()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttonS0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttonS1.svg")));
		fb->removeChild(shadow);
		delete shadow;
	}
};

struct MediumButton : SvgSwitch
{
	MediumButton()
	{
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttonM0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttonM1.svg")));
		fb->removeChild(shadow);
		delete shadow;
	}
};

struct MediumSwitchButton : SvgSwitch
{
	MediumSwitchButton()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttonM0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttonM1.svg")));
		fb->removeChild(shadow);
		delete shadow;
	}
};

struct CustomPort : app::SvgPort
{
	CustomPort()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port.svg")));
	}
};

struct CustomPortOut : app::SvgPort
{
	CustomPortOut()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port-out.svg")));
	}
};

// Random rotation screw code by stoermelder
struct CustomScrew : app::SvgScrew
{
	widget::TransformWidget* tw;

	CustomScrew()
	{
		fb->removeChild(sw);

		tw = new TransformWidget();
		tw->addChild(sw);
		fb->addChild(tw);

		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw.svg")));

		tw->box.size = sw->box.size;
		box.size = tw->box.size;

		float angle = random::uniform() * M_PI;
		tw->identity();
		// Rotate SVG
		math::Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
	}
};