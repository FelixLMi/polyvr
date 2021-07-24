#ifndef VRPARTICLEST_H_INCLUDED
#define VRPARTICLEST_H_INCLUDED

#include "core/utils/VRFunctionFwd.h"
#include "core/objects/material/VRMaterial.h"

#include "core/objects/geometry/OSGGeometry.h"
#include <OpenSG/OSGGeometry.h>
#include "core/utils/VRMutex.h"



OSG_BEGIN_NAMESPACE;

template<class P>
void VRParticles::resetParticles(int amount) {
    VRScenePtr scene = VRScene::getCurrent();
    if (scene) world = scene->bltWorld();

    this->disableFunctions();

    {
        VRLock lock(mtx());
        this->N = amount;
        for (uint i=0; i<particles.size(); i++) delete particles[i];
        particles.resize(N, 0);
        for (uint i=0; i<particles.size(); i++) particles[i] = new P(world);


        // material
        mat = VRMaterial::create("particles");
        mat->setDiffuse(Color3f(0,0,1));
        mat->setPointSize(5);
        mat->setLit(false);

        // geometry
        GeoUInt32PropertyRecPtr Length = GeoUInt32Property::create();
        GeoUInt32PropertyRecPtr inds = GeoUInt32Property::create();
        pos = GeoPnt3fProperty::create();
        normals = GeoVec3fProperty::create();
        colors = GeoVec4fProperty::create();
        Length->addValue(N);

        for(int i=0;i<N;i++) pos->addValue(Pnt3d(0,300,0));
        for(int i=0;i<N;i++) inds->addValue(i);
        normals->resize(N);
        colors->resize(N);

        setType(GL_POINTS);
        setLengths(Length);
        setPositions(pos);
        setNormals(normals);
        setColors(colors);
        setIndices(inds);
        setMaterial(mat);

        getMesh()->geo->setDlistCache(false);
    }

    printf("VRParticles::resetParticles(amount=%i) -- done\n", amount);
}

OSG_END_NAMESPACE;

#endif // VRPARTICLEST_H_INCLUDED
