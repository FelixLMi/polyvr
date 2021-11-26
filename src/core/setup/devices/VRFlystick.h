#ifndef VRFLYSTICK_H_INCLUDED
#define VRFLYSTICK_H_INCLUDED

#include "VRDevice.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRFlystick : public VRDevice {
    private:
        string trackerName;

    public:
        VRFlystick();

        static VRFlystickPtr create();
        VRFlystickPtr ptr();

        void clearSignals() override;

        void update(vector<int> buttons);
        void update(vector<float> sliders);

        string getTrackerName();
};


OSG_END_NAMESPACE;

#endif // VRFLYSTICK_H_INCLUDED
