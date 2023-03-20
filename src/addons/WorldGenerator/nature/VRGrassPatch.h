#ifndef VRGRASSPATCH_H_INCLUDED
#define VRGRASSPATCH_H_INCLUDED

#include "core/math/OSGMathFwd.h"
#include "core/objects/material/VRMaterialFwd.h"
#include <OpenSG/OSGColor.h>
#include "core/objects/VRTransform.h"
#include "addons/WorldGenerator/VRWorldGeneratorFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class VRGeoData;

class VRGrassPatch : public VRTransform {
    private:
        VRPolygonPtr area;
        vector<VRPolygonPtr> chunks;
        VRLodPtr lod;
        map<int, VRGeometryPtr> lods;
        float bladeHeight = 0.3;
        static VRTextureRendererPtr texRenderer;
        static VRPlantMaterialPtr matGrassSide;
        static VRPlantMaterialPtr matGrass;
        static VRPlantMaterialPtr matGrassUnlit;

        static void setupGrassMaterials();
        void setupGrassStage();

        void initLOD();
        void addGrassBlade(VRGeoData& data, Vec3d pos, float a, float dh, int lvl, Color3f c);
        void createPatch(VRGeoData& data, VRPolygonPtr area, int lvl = 0, float density = 100);
        void createSpriteLOD(VRGeoData& data, VRPolygonPtr area, int lvl, float density);

    public:
        VRGrassPatch();
        ~VRGrassPatch();
        static VRGrassPatchPtr create();

        void setArea(VRPolygonPtr p);

        void createLod(VRGeoData& geo, int lvl, Vec3d offset, int ID);

        static VRMaterialPtr getGrassMaterial();
        static VRMaterialPtr getGrassSideMaterial();
};

OSG_END_NAMESPACE;

#endif // VRGRASSPATCH_H_INCLUDED
