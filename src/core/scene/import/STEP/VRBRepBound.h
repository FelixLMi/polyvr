#ifndef VRBREPBOUND_H_INCLUDED
#define VRBREPBOUND_H_INCLUDED

#include "VRBRepUtils.h"
#include "VRBRepEdge.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class VRBRepBound : public VRBRepUtils {
    public:
        vector<VRBRepEdge> edges;
        vector<Vec3d> points;
        vector<float> angles;
        bool outer = true;
        string BRepType;

    public:
        VRBRepBound();

        bool isClosed();
        string edgeEndsToString();

        bool containsNan();
        void shiftEdges(int i0);
};

OSG_END_NAMESPACE;

#endif // VRBREPBOUND_H_INCLUDED
