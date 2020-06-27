// Plugins for VCV Rack by iggy.labs

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Widget overrides
struct IlKnob16 : RoundKnob {
    IlKnob16() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_16px.svg")));
    }
};

struct IlKnob19 : RoundKnob {
    IlKnob19() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_19px.svg")));
    }
};

struct IlPort : SvgPort {
    IlPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port.svg")));
    }
};

// Declare each Model, defined in each module source file
extern Model* modelTable;
