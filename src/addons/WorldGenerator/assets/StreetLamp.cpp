#include "StreetLamp.h"
#include "core/objects/geometry/VRGeoData.h"
#include "core/objects/geometry/VRStroke.h"
#include "core/math/path.h"
#include "core/math/pose.h"
#include "core/scene/import/VRImport.h"

#include <OpenSG/OSGMatrixUtility.h>

using namespace OSG;

void StreetLamp::make() {
	auto lamp = VRTransform::create("lamp");
	lamp->setOrientation(Vec3d(0,1,0), Vec3d(0,0,1));

	auto addPart = [&](float r, PathPtr p) {
		int N = 6;
		vector<Vec3d> prof;
		for (int i=0; i<N; i++) {
			float a = i*2*Pi/N;
			float x = r*cos(a);
			float y = r*sin(a);
			prof.push_back(Vec3d(x,y,0));
		}

        auto pole = VRStroke::create("pole");
		pole->addPath(p);
        pole->strokeProfile(prof, 1, 1);
        lamp->addChild(pole);
	};

	auto p1 = Path::create();
	p1->addPoint( Pose::create(Vec3d(0,0,0)));
	p1->addPoint( Pose::create(Vec3d(0,0,-2)));
	p1->compute(2);

	auto p2 = Path::create();
	p2->addPoint( Pose::create(Vec3d(0,0,-2)));
	p2->addPoint( Pose::create(Vec3d(0,0,-4)));
	p2->compute(2);

	auto p3 = Path::create();
	p3->addPoint( Pose::create(Vec3d(0,0,-4)));
	p3->addPoint( Pose::create(Vec3d(1,0,-5), Vec3d(2,0,-1)));
	p3->compute(5);

	addPart(0.1, p1);
	addPart(0.04, p2);
	addPart(0.04, p3);

    VRTransformPtr obj = VRImport::get()->load( "world/assets/lamps/streetlamp1.2.dae", 0, 1);
    obj->setPose(Pose::create(Vec3d(1,0,-5.08), Vec3d(1,0,2.5), Vec3d(0,0,-1)));
    obj->setScale(Vec3d(0.07,0.07,0.07));
    lamp->addChild(obj);

	auto g = merge(lamp);
    g->updateNormals();

    assets["streetlamp"] = g;
}

void StreetLamp::add(const Pose& p, VRGeoData* geo) {
    string name = "streetlamp";
    if (!assets.count(name)) make();
    if (!assets.count(name)) return;
    auto g = assets[name];

	Matrix4d mR;
	MatrixLookAt(mR, Pnt3d(), Pnt3d(0,1,0), Vec3d(1,0,0));
	Matrix4d m = p.asMatrix();
	m.mult(mR);
	geo->append(g, m);
}
