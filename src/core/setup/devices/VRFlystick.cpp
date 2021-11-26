#include "VRFlystick.h"
#include "VRSignal.h"
#include "core/utils/VRFunction.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

VRFlystick::VRFlystick() : VRDevice("flystick") {
    addBeacon();
    enableAvatar("cone");
    enableAvatar("ray");
    store("tracker", &trackerName);
}

VRFlystickPtr VRFlystick::create() {
    auto d = VRFlystickPtr(new VRFlystick());
    d->initIntersect(d);
    d->clearSignals();
    return d;
}

VRFlystickPtr VRFlystick::ptr() { return static_pointer_cast<VRFlystick>( shared_from_this() ); }

void VRFlystick::clearSignals() {
    VRDevice::clearSignals();

    newSignal( 0, 0)->add( getDrop() );
    newSignal( 0, 1)->add( addDrag( getBeacon() ) );
}

void VRFlystick::update(vector<int> buttons) {
    for(unsigned int k=0; k<buttons.size(); k++) {
        if (BStates[k] != buttons[k] || BStates.count(k) == 0) change_button(k, buttons[k]);
    }
}

void VRFlystick::update(vector<float> sliders) {
    for(unsigned int i=0; i<sliders.size(); i++) change_slider(i+10, sliders[i]); // art slider key has an offset of 10
}

string VRFlystick::getTrackerName() { return trackerName; }

OSG_END_NAMESPACE;
