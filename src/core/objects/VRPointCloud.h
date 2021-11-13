#ifndef VRPOINTCLOUD_H_INCLUDED
#define VRPOINTCLOUD_H_INCLUDED

#include "core/objects/VRTransform.h"
#include <OpenSG/OSGColor.h>

OSG_BEGIN_NAMESPACE;

class VRPointCloud : public VRTransform {
    private:
        VRMaterialPtr mat;
        OctreePtr octree;
        int levels = 1;
        bool keepOctree = 0;
        vector<int> downsamplingRate = {1};
        vector<float> lodDistances;

    public:
        VRPointCloud(string name = "pointcloud");
        ~VRPointCloud();

        static VRPointCloudPtr create(string name = "pointcloud");
        void applySettings(map<string, string> options);

        void addLevel(float distance, int downsampling);
        void setupLODs();

        void setupMaterial(bool lit, int pointsize);
        VRMaterialPtr getMaterial();

        void addPoint(Vec3d p, Color3f c);

        OctreePtr getOctree();
};

OSG_END_NAMESPACE;

#endif // VRPOINTCLOUD_H_INCLUDED
