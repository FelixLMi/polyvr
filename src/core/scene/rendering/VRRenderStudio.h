#ifndef VRRENDERSTUDIO_H_INCLUDED
#define VRRENDERSTUDIO_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGBackground.h>
#include "core/math/OSGMathFwd.h"
#include "core/objects/VRObjectFwd.h"
#include "core/scene/VRSceneFwd.h"
#include "core/setup/windows/VRView.h"
#include "core/utils/VRStorage.h"
#include "VRDeferredRenderStage.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRDefShading;
class VRSSAO;
class VRFXAA;
class VRHMDDistortion;
class VRDeferredRenderStage;

class VRRenderStudio : public VRStorage {
    public:
        enum EYE {
            LEFT = 0,
            RIGHT
        };

    private:
        bool deferredRendering = false;
        bool do_ssao = false;
        bool calib = false;
        bool do_hmdd = false;
        bool do_marker = false;
        bool do_fxaa = false;
        bool do_stencil = false;
        int ssao_kernel = 4;
        int ssao_noise = 4;
        float ssao_radius = 0.02;
        Color4f fogParams = Color4f(0, 0, 100, 0.1);
        Color4f fogColor = Color4f(0.5, 0.5, 0.5, 1);
        EYE eye = LEFT;
        OSGCameraPtr cam;

        map<string, std::shared_ptr<VRDeferredRenderStage>> stages;

        void initDSProxy(VRMaterialPtr mat);
        void initCalib(VRMaterialPtr mat);
        void initMarker(VRMaterialPtr mat);
        void initStencilViewer(VRMaterialPtr mat);

    protected:
        shared_ptr<VRSSAO> ssao;
        shared_ptr<VRHMDDistortion> hmdd;
        shared_ptr<VRFXAA> fxaa;
        VRObjectPtr root_system;
        VRObjectPtr root_scene;
        map<int, VRLightWeakPtr> light_map;

    public:
        VRRenderStudio(EYE e);
        ~VRRenderStudio();

        static VRRenderStudioPtr create(EYE e = LEFT);

        void init(VRObjectPtr root = 0);
        void reset();

        VRObjectPtr getRoot();
        VRObjectPtr getSceneRoot();

        void addStage(string name, string parent = "");
        void setStageActive(string name, bool da, bool la);
        void setStageShader(string name, string VPpath, string FPpath, bool doDeferred);
        int addStageBuffer(string name, int pformat, int ptype);
        void setStageParameter(string name, string var, int val);
        void setStageTexture(string name, VRTexturePtr tex, int unit, int mag, int min);

        void addLight(VRLightPtr l);
        void updateLight(VRLightPtr l);
        VRLightPtr getLight(int ID);
        void subLight(int ID);
        void clearLights();

        void updateSceneLink();
        void setScene(VRObjectPtr root);
        void setCamera(OSGCameraPtr cam);
        void setBackground(BackgroundMTRecPtr bg);
        void resize(Vec2i s);
        void setEye(EYE e);
        void setFogParams(Color4f fogParams, Color4f fogColor);

        void setDefferedShading(bool b);
        void setDeferredChannel(int c);
        bool getDefferedShading();
        void setSSAO(bool b);
        bool getSSAO();
        void setSSAOradius(float r);
        void setSSAOkernel(int k);
        void setSSAOnoise(int n);
        void setCalib(bool b);
        void setHMDD(bool b);
        bool getHMDD();
        void setMarker(bool b);
        bool getMarker();
        void setStencil(bool b);
        bool getStencil();
        void setFXAA(bool b);
        bool getFXAA();

        void setHMDDeye(float e);

        void update();
        void reloadStageShaders();
};

OSG_END_NAMESPACE;

#endif // VRRENDERSTUDIO_H_INCLUDED
