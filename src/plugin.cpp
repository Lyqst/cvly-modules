#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;

	p->addModel(modelBrst);
	p->addModel(modelBss);
	p->addModel(modelCrcl);
	p->addModel(modelNtrvlc);
	p->addModel(modelNtrvlx);
	p->addModel(modelSpc);
	p->addModel(modelStpr);
	p->addModel(modelTxt);
	p->addModel(modelVbrt);
	p->addModel(modelWhl);
}
