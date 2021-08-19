#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelBss);
	p->addModel(modelCrcl);
	p->addModel(modelNtrvlc);
	p->addModel(modelNtrvlx);
	p->addModel(modelSpc);
	p->addModel(modelStpr);
	p->addModel(modelTxt);
	p->addModel(modelWhl);
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
