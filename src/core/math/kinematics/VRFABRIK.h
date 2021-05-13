#ifndef VRFABRIK_H_INCLUDED
#define VRFABRIK_H_INCLUDED

#include <map>
#include "core/math/OSGMathFwd.h"

#include "core/utils/VRFwdDeclTemplate.h"
#include "core/math/pose.h"
#include "core/objects/VRObjectFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

ptrFwd(FABRIK);

class FABRIK {
    private:
        struct Joint;
        struct Chain;
        struct step;

        bool doConstraints = true;
        bool doSprings = false;

        map<int, Joint> joints;
        map<string, Chain> chains;
        vector<string> chainOrder;
        vector<step> executionQueue;

        float tolerance = 0.01;

        Vec3d computeConstraintDelta(int j);
        void applyConstraint(int j);
        void applyInverseConstraint(int j);
        void applySpring(int j, float d);
        void updateJointOrientation(int j);
        Vec3d movePointTowards(int j, Vec3d target, float t);
        Vec3d moveToDistance(int j1, int j2, float d, bool fwd);
        void updateExecutionQueue();

    public:
        FABRIK();
        ~FABRIK();

        static FABRIKPtr create();

        void addJoint(int ID, PosePtr p);
        void setJoint(int ID, PosePtr p);
        PosePtr getJointPose(int ID);

        void addChain(string name, vector<int> joints);
        vector<int> getChainJoints(string name);

        void addConstraint(int j, Vec4d angles);
        void addSpring(int j, Vec3d anchor);

        void forward(Chain& chain);
        void backward(Chain& chain);
        void chainIteration(Chain& chain);

        void setTarget(int i, PosePtr p);
        void iterate();
        void iterateChain(string chain);

        void visualize(VRGeometryPtr geo);

        void setDoConstraints(bool b);
        void setDoSprings(bool b);
};

OSG_END_NAMESPACE;

#endif // VRFABRIK_H_INCLUDED
