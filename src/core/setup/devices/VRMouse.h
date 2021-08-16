#ifndef VRMOUSE_H_INCLUDED
#define VRMOUSE_H_INCLUDED

#include "core/setup/windows/VRView.h"
#include "VRDevice.h"
#include <OpenSG/OSGLine.h>

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRMouse : public VRDevice {
    private:

        // TODO: Camera Pointer is not updated if the camera of the view
        VRCameraWeakPtr cam;
        VRViewWeakPtr view;
        Line ray;

        VRSignalPtr on_to_edge = 0;
        VRSignalPtr on_from_edge = 0;
        int onEdge = -1;

        void multFull(Matrix _matrix, const Pnt3f &pntIn, Pnt3f  &pntOut);

        bool calcViewRay(VRCameraPtr pcam, Line &line, float x, float y, int W, int H);

    public:
        VRMouse();
        ~VRMouse();

        static VRMousePtr create();
        VRMousePtr ptr();

        void clearSignals() override;

        //3d object to emulate a hand in VRSpace
        void updatePosition(int x, int y);

        void mouse(int button, int state, int x, int y);
        void motion(int x, int y);

        void setCamera(VRCameraPtr cam) override;
        void setViewport(VRViewPtr view);

        Line getRay();
        VRSignalPtr getToEdgeSignal() override;
        VRSignalPtr getFromEdgeSignal() override;

        void setCursor(string c);

        void save(XMLElementPtr e);
        void load(XMLElementPtr e);
};

OSG_END_NAMESPACE;

#endif // VRMOUSE_H_INCLUDED
