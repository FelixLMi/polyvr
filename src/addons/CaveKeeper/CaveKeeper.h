#ifndef CAVEKEEPER_H_INCLUDED
#define CAVEKEEPER_H_INCLUDED

#include "core/math/OSGMathFwd.h"
#include <map>
#include <string>
#include <vector>
#include "CKOctree.h"
#include "core/utils/VRFunctionFwd.h"
#include "core/objects/VRObjectFwd.h"
#include "core/objects/material/VRMaterialFwd.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

// -------------- TODO --------
// check if element exists when adding to octree
// when changing the world, only change the minimum data, do not rebuild the full mesh!
// bumpmap

class BlockWorld {
    public:
		CKOctree* tree = 0;

        VRObjectPtr getAnchor();

    private:
        VRObjectPtr anchor;

        map<string, VRMaterialPtr> materials;
        map<int, VRGeometryPtr> chunks;
        VRUpdateCbPtr updatePtr;

        // octree population algorithm

        void createPlane(int w);
        void createSphere(int r, Vec3i p0);
        // mesh methods

        VRMaterialPtr initMaterial(string texture);
		VRGeometryPtr createChunk(vector<CKOctree::element*>& elements);
        VRGeometryPtr initChunk();

		void appendToVector(vector<CKOctree::element*>* elements, CKOctree::element* e);

        // update methods

        void updateShaderCamPos();

    protected:
        BlockWorld();
        ~BlockWorld();

        void initWorld();

        void redraw(int chunk_id = 0);
};

class CaveKeeper : public BlockWorld {
    private:
        void placeLight(Vec3d p);

    public:
        CaveKeeper();
        ~CaveKeeper();

        void init(VRObjectPtr anchor);

        static shared_ptr<CaveKeeper> create();

        int intersect(VRDevicePtr dev);
        int addBlock(Vec3i p);
        void remBlock(int i);

        void place(VRDevicePtr dev, string s, VRTransformPtr geo);
};

typedef shared_ptr<CaveKeeper> CaveKeeperPtr;

OSG_END_NAMESPACE

#endif // CAVEKEEPER_H_INCLUDED
