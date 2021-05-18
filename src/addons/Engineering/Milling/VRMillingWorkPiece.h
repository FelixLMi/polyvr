#ifndef VRMILLINGWORKPIECE_H_INCLUDED
#define VRMILLINGWORKPIECE_H_INCLUDED

#include <string>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGGeoProperties.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include "core/objects/geometry/VRGeometry.h"
#include "core/math/partitioning/Octree.h"
#include "VRMillingCuttingToolProfile.h"
#include "addons/Engineering/VREngineeringFwd.h"

OSG_BEGIN_NAMESPACE;

class VRWorkpieceElement {
private:
    VRMillingWorkPiece& workpiece;
    VRWorkpieceElement* children[2];
    VRWorkpieceElement* parent;

    Vec3i blocks;
    Vec3d size;
    Vec3d offset;
    Vec3d position;
    const int level;

    bool deleted;
    bool updateIssued;

    VRGeometryPtr geometry;
    GeoPnt3fPropertyRecPtr positions;
    GeoVec3fPropertyRecPtr normals;
    GeoUInt32PropertyRecPtr indices;

    static Vec3d mulVec3f(Vec3d lhs, Vec3d rhs);

public:
    VRWorkpieceElement(VRMillingWorkPiece& workpiece, VRWorkpieceElement* parent,
                       Vec3i blocks, Vec3d size, Vec3d offset, Vec3d position, const int level);

    // 6 sides with 4 vertices each
    static const int verticesPerElement = 6 * 4;
    static const Vec3d planeOffsetMasks[2][3];
    static const Vec3d vertexOffsetMasks[2][3][4];
    static const float sign[2];

    /*
     * deletes all children with their geometries
     */
    ~VRWorkpieceElement();
    void deleteGeometry();
    void deleteChildren();
    void issueGeometryUpdate();
    void split();
    bool collides(Vec3d position);
    bool collide(Vec3d position);
    bool doesCollide(Vec3d position); //Add of Marie
    void deleteElement();
    bool isDeleted() const;
    void build();
    void build(GeoPnt3fPropertyRecPtr positions,
               GeoVec3fPropertyRecPtr normals,
               GeoUInt32PropertyRecPtr indices,
               uint32_t& index);
    void buildGeometry(GeoPnt3fPropertyRecPtr positions,
                       GeoVec3fPropertyRecPtr normals,
                       GeoUInt32PropertyRecPtr indices,
                       uint32_t& index);

};

class VRMillingWorkPiece : public VRGeometry {
    private:
        Vec3i gridSize;
        int updateCount = 0;
        PosePtr toolPose;
        VRTransformWeakPtr tool;
        VRUpdateCbPtr uFkt;
        int levelsPerGeometry = 12; // can be overridden
        int geometryUpdateWait = 1;
        int maxTreeLevel = 0;
        void update();
        VRWorkpieceElement* rootElement;

    public:
        float blockSize = 0.01;
        int geometryCreateLevel = 0;
        shared_ptr<VRMillingCuttingToolProfile> cuttingProfile;

    public:
        VRMillingWorkPiece(string name);
        VRMillingWorkPiecePtr ptr();
        static VRMillingWorkPiecePtr create(string name = "millingWorkPiece");
        void init(Vec3i gSize, float bSize = 0.01);
        void reset();

        void setCuttingTool(VRTransformPtr geo);
        void setCuttingProfile(shared_ptr<VRMillingCuttingToolProfile> profile);

        void updateGeometry();
        void setLevelsPerGeometry(int levels);  // this will take effect after the next reset
        void setRefreshWait(int updatesToWait); // this will take effect immediately

};

OSG_END_NAMESPACE;

#endif // VRMILLINGWORKPIECE_H_INCLUDED
