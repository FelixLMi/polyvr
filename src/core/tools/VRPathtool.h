#ifndef VRPATHTOOL_H_INCLUDED
#define VRPATHTOOL_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include "core/math/OSGMathFwd.h"
#include <OpenSG/OSGColor.h>
#include <vector>
#include <map>

#include "core/objects/VRObjectFwd.h"
#include "core/objects/material/VRMaterialFwd.h"
#include "core/utils/VRFunctionFwd.h"
#include "core/tools/VRToolsFwd.h"
#include "core/objects/VRTransform.h"
#include "core/math/partitioning/graph.h"
#include "core/math/VRMathFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE

class VRDevice;

class VRManipulator {
    private:
        VRObjectPtr anchor;
        VRGeometryPtr gTX, gTY, gTZ;
        VRGeometryPtr gRX, gRY, gRZ;
        VRTransformPtr sel = 0;

        void setup();

    public:
        VRManipulator();

        void handle(VRGeometryPtr g);
        void manipulate(VRTransformPtr t);
};

class VRExtruder {
    private:
        void update();

    public:
        VRExtruder();

        // update fkt
        //  check if any device beacon changed
        //  intersect with invisible cones
        //  on intersection
        //   make cone visible

        //  check if any device drags a cone of ours
        //   trigger events -> start drag/ update drag/ stop drag
};

class VRPathtool : public VRTransform {
//class VRPathtool : public VRObject {
    public:
        struct entry {
            int edge = 0;
            bool doUpdate = 1;
            PathPtr p = 0;
            map<VRGeometry*, int> points;
            vector<VRGeometryWeakPtr> handles;
            VRStrokeWeakPtr line;
            VRObjectWeakPtr anchor;
            VRGeometryWeakPtr arrow;

            entry(VRObjectPtr anchor);
            void addHandle(VRGeometryPtr h, int i);
        };

        struct knot {
            int ID;
            vector<int> in;
            vector<int> out;
            VRGeometryWeakPtr handle;
        };

        struct option {
            int resolution = 10;
            bool useControlHandles = false;
            bool doSmoothGraphNodes = true;
            bool isVisible = true;

            Color3f color1;
            Color3f color2;
            bool useColors = false;

            Vec3d bulge;

            option(int r = 10, bool uch = false);
        };

        typedef shared_ptr<entry> entryPtr;

        void updateHandlePose(knot& knot, map<int, Vec3d>& hPositions, bool doUpdateEntry = true);
        void updateBezierVisuals();

    private:
        GraphPtr graph;
        map<int, PathPtr> paths;
        map<int, option > options;
        vector<VRGeometryWeakPtr> handles;
        vector<VRGeometryWeakPtr> controlHandles;
        VRGeometryWeakPtr controlPoints;
        VRGeometryWeakPtr bezierHulls;
        VRGeometryPtr arrowTemplate;

        bool showBControlPoints = false;
        bool showBHulls = false;
        float arrowScale = 1;

        map<Path*, entryPtr> pathToEntry;
        map<VRGeometry*, vector<entryPtr> > handleToEntries; // map handle geometries to the entries
        map<VRGeometry*, int> handleToNode;
        map<int, knot> knots; // maps graph node ids to pathtool knots
        PathPtr selectedPath = 0;

        VRMaterialPtr lmat;
        VRMaterialPtr lsmat;
        VRMaterialPtr amat;

        VRUpdateCbPtr updatePtr;
        VRManipulator* manip = 0;
        VRExtruder* ext = 0;
        VRObjectPtr projObj;

        VRGeometryPtr customHandle;
        VRGeometryPtr newControlHandle(VRGeometryPtr handle, Vec3d n);
        VRGeometryPtr newHandle();
        entryPtr newEntry(PathPtr p, option o, int eID, VRObjectPtr anchor = 0);
        void setupHandles(entryPtr p, VRGeometryPtr ha, VRGeometryPtr he);
        void updateHandle(VRGeometryPtr handle);
        void updateEntry(entryPtr e);
        void updateDevs();

        VRGeometryPtr setGraphNode(int i);
        void setGraphEdge(Graph::edge& e, bool handles = false, bool doArrow = false);
        void setGraphEdge(Graph::edge& e, bool handles, bool doArrow, Vec3d n1, Vec3d n2);
        void projectHandle(VRGeometryPtr handle, VRDevicePtr dev);

    public:
        VRPathtool();
        ~VRPathtool();

        static VRPathtoolPtr create();
        void setup(VRStorageContextPtr context);
        void setupBefore(VRStorageContextPtr context);

        void setProjectionGeometry(VRObjectPtr obj);

        void setGraph(GraphPtr g, bool doClear = true, bool handles = false, bool doArrows = false);
        GraphPtr getGraph();
        int addNode(PosePtr p);
        void removeNode(int i);
        int getNodeID(VRObjectPtr o);
        VRGeometryPtr addHandle(int nID, PosePtr p);
        void setHandlePose(int nID, PosePtr p);
        void connect(int i1, int i2, Vec3d n1, Vec3d n2, bool handles = true, bool doArrow = false);
        void disconnect(int i1, int i2);

        PathPtr newPath(VRDevicePtr dev, VRObjectPtr anchor, int resolution = 10, bool doCHandles = false);
        VRGeometryPtr extrude(VRDevicePtr dev, PathPtr p);
        void remPath(PathPtr p);

        void addPath(PathPtr p, VRObjectPtr anchor = 0, VRGeometryPtr ha = 0, VRGeometryPtr he = 0, bool handles = true);
        void setVisuals(bool handles, bool lines);
        void setBezierVisuals(bool controlPoints, bool hulls);
        void setHandleGeometry(VRGeometryPtr geo);
        void clear(PathPtr p = 0);

        vector<PathPtr> getPaths(VRGeometryPtr h = 0);
        PathPtr getPath(VRGeometryPtr h1, VRGeometryPtr h2);
        VRGeometryPtr getHandle(int ID);
        vector<VRGeometryPtr> getHandles(PathPtr p = 0);
        VRStrokePtr getStroke(PathPtr p);

        VRMaterialPtr getPathMaterial();
        VRMaterialPtr getArrowMaterial();
        void setArrowSize(float s);

        // edge options
        void setEdgeResolution(int eID, int resolution);
        void setEdgeColor(int eID, Color3f color1, Color3f color2);
        void setEdgeBulge(int eID, Vec3d bulge);
        void setEdgeSmoothGraphNodes(int eID, bool b);
        void setEdgeVisibility(int eID, bool b);

        void select(VRGeometryPtr handle);
        void selectPath(PathPtr p);
        void deselect();
        void update();
};

OSG_END_NAMESPACE

#endif // VRPATHTOOL_H_INCLUDED
