#pragma once
#include <rack.hpp>
#include "components.hpp"

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model* modelBss;
extern Model* modelCrcl;
extern Model* modelNtrvlc;
extern Model* modelNtrvlx;
extern Model* modelSpc;
extern Model* modelStpr;
