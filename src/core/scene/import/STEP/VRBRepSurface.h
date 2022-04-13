#ifndef VRBREPSURFACE_H_INCLUDED
#define VRBREPSURFACE_H_INCLUDED

#include "VRBRepUtils.h"
#include "VRBRepBound.h"

#include "core/math/VRMathFwd.h"
#include "core/math/field.h"
#include "core/objects/VRObjectFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class VRBRepSurface : public VRBRepUtils {
    public:
        vector<VRBRepBound> bounds;
        string etype;
        PosePtr trans;
        double R = 1;
        double R2 = 1; // for toroidal and conical surfaces
        double h0 = 0; // for conic surfaces

        field<Vec3d> cpoints;
        field<double> weights;
        vector<double> knotsu;
        vector<double> knotsv;
        int degu = 0;
        int degv = 0;

    public:
        VRBRepSurface();

        VRGeometryPtr build(string type, bool same_sense);
};

OSG_END_NAMESPACE;

#endif // VRBREPSURFACE_H_INCLUDED
