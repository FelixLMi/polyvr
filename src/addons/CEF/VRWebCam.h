#ifndef VRWEBCAM_H_INCLUDED
#define VRWEBCAM_H_INCLUDED

#include "core/objects/geometry/sprite/VRSprite.h"
#include <string>

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRWebCam : public VRSprite {
    protected:
        string uri;
        string site;

    public:
        VRWebCam(string name = "webCam");
        ~VRWebCam();

        static shared_ptr<VRWebCam> create(string name = "webCam");

        void connect(string uri, int res, float ratio);
        void test();
};

typedef shared_ptr<VRWebCam> VRWebCamPtr;

OSG_END_NAMESPACE;

#endif // VRWEBCAM_H_INCLUDED
