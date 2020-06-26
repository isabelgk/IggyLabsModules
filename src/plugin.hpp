// Plugins for VCV Rack by IggyLabs

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Widget overrides
struct IlKnob : RoundKnob {
    IlKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob.svg")));
    }
};

struct IlPort : SvgPort {
    IlPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port.svg")));
    }
};

// Declare each Model, defined in each module source file
extern Model* modelTable;
