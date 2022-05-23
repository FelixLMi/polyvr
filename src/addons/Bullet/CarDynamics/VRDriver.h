#ifndef VRDRIVER_H_INCLUDED
#define VRDRIVER_H_INCLUDED

#include "core/math/OSGMathFwd.h"
#include "addons/Bullet/VRPhysicsFwd.h"
#include "core/math/VRMathFwd.h"
#include "core/utils/VRFunctionFwd.h"

#include <OpenSG/OSGConfig.h>

using namespace std;
OSG_BEGIN_NAMESPACE;

class VRDriver {
    private:
        VRCarDynamicsPtr car;
        bool active = false;
        float to = 1;
        float lastPathT = -1;
        PathPtr p_path;
        PathPtr v_path;
        float target_speed = 8;
        bool useClutch = true;
        VRUpdateCbPtr updatePtr;

        void update();

    public:
        VRDriver();
        ~VRDriver();
        static VRDriverPtr create();

        //void setTask(); // TODO
        void setCar( VRCarDynamicsPtr car );
        void followPath(PathPtr p, PathPtr v, float to = 1);
        void stop();
        void resume();
        void setTargetSpeed( float speed, bool useClutch = true );
        bool isDriving();
};

/* TODOs

- add statemachine to utils or math folder
- add VRTask to addons/training, with a state machine

*/

OSG_END_NAMESPACE;

#endif // VRDRIVER_H_INCLUDED
