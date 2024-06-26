#ifndef VRNAVIGATOR_H_INCLUDED
#define VRNAVIGATOR_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include "core/math/OSGMathFwd.h"
#include <map>
#include <string>
#include <vector>
#include "core/objects/VRObjectFwd.h"
#include "core/utils/VRFunctionFwd.h"
#include "core/utils/VRDeviceFwd.h"
#include "core/utils/VRStorage.h"
#include "core/utils/VRName.h"
#include "VRNavigationFwd.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

struct VRNavBinding {
    int key = 0;
    int state = 0;
    VRSignalWeakPtr sig;
    VRDeviceCbPtr cb;
    string sig_name;
    string cb_name;
    bool active = true;
    bool doRepeat = false;

    VRNavBinding(VRDeviceCbPtr c, int k, int s, bool repeat);
    ~VRNavBinding();

    void clearSignal();
};

class VRNavPreset : public VRName {
    private:
        vector<VRNavBinding> bindings;
        VRDevicePtr dev = 0;
        VRTransformWeakPtr target;
        bool active = false;
        double speedX = 1;
        double speedY = 1;

    public:
        VRNavPreset();
        virtual ~VRNavPreset();
        static VRNavPresetPtr create();

        void updateBinding(VRNavBinding& b);

        void setActive(bool b);
        bool isActive();

        void setDevice(VRDevicePtr _dev);
        void setTarget(VRTransformPtr _target);
        void setSpeed(float vt, float vr);

        void activate();
        void deactivate();
        void setBindingState(size_t i, bool b);

        void addKeyBinding(VRNavBinding b);
        vector<VRNavBinding>& getBindings();

        vector<VRNavBinding>::iterator begin();
        vector<VRNavBinding>::iterator end();
};

class VRNavigator_base : public VRStorage {
    protected:
        map<string, VRDeviceCbPtr> library;
        map<string, VRNavPresetPtr> presets;
        map<string, bool> presetStates;

    public:
        VRNavigator_base();
        ~VRNavigator_base();

        void addNavigation(VRNavPresetPtr ps);
        void remNavigation(string name);

        VRNavPresetPtr getNavigation(string s);
        map<string, VRNavPresetPtr> getNavigations();
        vector<string> getNavigationNames();
        string getNavigationTip(string name);
        void setNavigationState(string name, bool active);

        void storeNavigationCallback(VRDeviceCbPtr cb);
        map<string, VRDeviceCbPtr>& getNavigationCallbacks();
        VRDeviceCbPtr getNavigationCallback(string s);
};

class VRNavigator : public VRNavigator_base {
    private:
        VRTransformWeakPtr target;
        VRDeviceWeakPtr device;

        vector<VRTransformWeakPtr> walk_surfaces;
        static float clip_dist_down;
        std::shared_ptr<VRFunction<float> > focus_fkt;

        bool isCtrlDown();
        bool isShiftDown();
        bool isKeyDown(int k);

        // callbacks
        bool zoom(VRDeviceWeakPtr dev, int dir);
        bool walk(VRDeviceWeakPtr dev);
        bool fly_walk(VRDeviceWeakPtr dev);
        bool hyd_walk(VRDeviceWeakPtr dev);
        bool orbit(VRDeviceWeakPtr dev);
        bool orbit2D(VRDeviceWeakPtr dev);
        bool focus(VRDeviceWeakPtr dev);

    protected:
        VRNavigator();
        ~VRNavigator();

        void update();

    public:
        // init presets
        void initWalk(VRTransformPtr target, VRDevicePtr dev);
        void initOrbit(VRTransformPtr target, VRDevicePtr dev);
        void initOrbit2D(VRTransformPtr target, VRDevicePtr dev);
        void initFlyOrbit(VRTransformPtr target, VRDevicePtr dev);
        void initFlyWalk(VRTransformPtr target, VRDevicePtr dev);
        void initHydraFly(VRTransformPtr target, VRDevicePtr dev);
};

OSG_END_NAMESPACE;

#endif // VRNAVIGATOR_H_INCLUDED
