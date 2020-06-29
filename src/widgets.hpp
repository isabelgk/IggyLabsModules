// Knobs

struct IlKnobXXS : RoundKnob {
    IlKnobXXS() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/knob_xxs.svg")));
    }
};

struct IlKnobXS : RoundKnob {
    IlKnobXS() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/knob_xs.svg")));
    }
};

struct IlKnobS : RoundKnob {
    IlKnobS() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/knob_s.svg")));
    }
};

struct IlKnobM : RoundKnob {
    IlKnobM() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/knob_m.svg")));
    }
};

struct IlKnobL : RoundKnob {
    IlKnobL() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/knob_l.svg")));
    }
};

struct IlKnobXL : RoundKnob {
    IlKnobL() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/knob_xl.svg")));
    }
};


// Ports

struct IlPort : SvgPort {
    IlPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/widgets/port.svg")));
    }
};