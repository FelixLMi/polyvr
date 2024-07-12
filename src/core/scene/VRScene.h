#ifndef VRSCENE_H_INCLUDED
#define VRSCENE_H_INCLUDED

#include "VRSceneFwd.h"
#include "VRObjectGroupManager.h"
#include "VRCameraManager.h"
#include "VRAnimationManager.h"
#include "VRPhysicsManager.h"
#include "VRCallbackManager.h"
#include "VRMaterialManager.h"
#include "VRThreadManager.h"
#include "core/scripting/VRScriptManager.h"
#include "core/navigation/VRNavigator.h"
#include "core/networking/VRNetworkManager.h"
#include "core/utils/VRName.h"
#include "core/utils/VRFlags.h"
#include "core/utils/VRUtilsFwd.h"
#include "core/utils/VRTimer.h"
#include "core/utils/VRGlobals.h"
#include "VRBackground.h"
#include "rendering/VRRenderManager.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRObject;
class VRVisualLayer;

class VRScene :
                public VRThreadManager,
                public VRObjectGroupManager,
                public VRCameraManager,
                public VRAnimationManager,
#ifndef WITHOUT_BULLET
                public VRPhysicsManager,
#endif
                public VRCallbackManager,
                public VRScriptManager,
                public VRNavigator,
                public VRNetworkManager,
                public VRMaterialManager,
                public VRBackground,
                public VRRenderManager,
                public VRFlags,
                public VRName {
    private:
        string path;
        string icon;
        //physics run in own thread
        int physicsThreadID;
        VRGlobals::Int loadingFrame = 0;

        VRVisualLayerPtr referentials_layer;
        VRVisualLayerPtr cameras_layer;
        VRVisualLayerPtr lights_layer;

        VRToggleCbPtr layer_ref_toggle;
        VRToggleCbPtr layer_cam_toggle;
        VRToggleCbPtr layer_light_toggle;

        VRDeviceCbPtr onSelectObjCb;

        VRSemanticManagerPtr semanticManager;

        VRProgressPtr loadingProgress;
        int loadingProgressThread;
        VRThreadCbPtr loadingProgressThreadCb;
        int loadingTime = 0;
        VRTimer loadingTimer;
        VRUpdateCbPtr loadingTimeCb;

        void updateLoadingProgress(VRThreadWeakPtr t);
        void recLoadingTime();

        bool onSelectObject(VRDeviceWeakPtr dev);

    public:
        VRScene();
        ~VRScene();

        static VRScenePtr getCurrent();

        void setPath(string path);
        void setIcon(string icon);
        string getPath();
        string getWorkdir();
        string getFile();
        string getFileName();
        string getIcon();

        void initDevices();

        void add(VRObjectPtr obj, int parentID = -1);
        void add(NodeMTRecPtr n);

        VRObjectPtr get(int ID);
        VRObjectPtr get(string name, bool strict = true);
        VRObjectPtr getRoot();
        //VRObjectPtr getSystemRoot();

        void setActiveCamera(string name = "");

        void printTree();
        void showReferentials(bool b, VRObjectPtr o);
        void showLights(bool b);
        void showCameras(bool b);

        VRSemanticManagerPtr getSemanticManager();
        VRProgressPtr getLoadingProgress();

        void update();

        void saveScene(XMLElementPtr e);
        void loadScene(XMLElementPtr e);
        void importScene(XMLElementPtr e, string path);
};

typedef std::shared_ptr<VRScene> VRScenePtr;
typedef std::weak_ptr<VRScene> VRSceneWeakPtr;

OSG_END_NAMESPACE;

#endif // VRSCENE_H_INCLUDED
