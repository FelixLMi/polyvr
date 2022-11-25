#include "VRBRepUtils.h"
#include "core/utils/toString.h"
#include "core/math/pose.h"
#include <OpenSG/OSGVector.h>

using namespace OSG;

const float VRBRepUtils::Dangle = 2*Pi/(VRBRepUtils::Ncurv-1);
vector<float> VRBRepUtils::Adict;

VRBRepUtils::VRBRepUtils() {
    for (int i=0; i<Ncurv; i++) Adict.push_back(Dangle*i);
}

bool VRBRepUtils::sameVec(const Vec2d& v1, const Vec2d& v2, float d) {
    Vec2d dv = v2-v1;
    return ( abs(dv[0]) < d && abs(dv[1]) < d);
}

bool VRBRepUtils::sameVec(const Vec3d& v1, const Vec3d& v2, float d) {
    Vec3d dv = v2-v1;
    return ( abs(dv[0]) < d && abs(dv[1]) < d && abs(dv[2]) < d );
}

int VRBRepUtils::getSideN(double a) {
    return round( a/Dangle - 0.5 );
}

int VRBRepUtils::getSideN(float a) {
    return round( a/Dangle - 0.5 );
}

Vec2d VRBRepUtils::getSide(double a) {
    int i = getSideN(a);
    return getSide(i);
}

Vec2d VRBRepUtils::getSide(float a) {
    int i = getSideN(a);
    return getSide(i);
}

Vec2d VRBRepUtils::getSide(int i) {
    return Vec2d(i*Dangle, (i+1)*Dangle);
}

vector<Vec2d> VRBRepUtils::angleFrame(Vec2d p1, Vec2d p2) {
    float A = (p2-p1).length();

    vector<Vec2d> angles;
    angles.push_back(p1);
    for(float a = 0; a < A-1e-6; a += Dangle) angles.push_back(p1 + (p2-p1)*a/A);
    angles.push_back(p2);
    return angles;
}

vector<float> VRBRepUtils::angleFrame(float a1, float a2) {
    if (a1 == a2) { // full circle
        cout << "Full circle detected! " << a1;
        a1 = -Pi;
        a2 = Pi;
        cout << " change to " << a1 << " " << a2 << endl;
    }

    if (a1 > a2) a2 += 2*Pi;

    /*vector<float> angles;
    angles.push_back(a1);
    float a = a1;
    for(; a < a2-1e-6; a += Dangle) angles.push_back(a);
    angles.push_back(a2);
    return;*/

    vector<float> angles;
    //angles.push_back(a1);
    //int s1 = getSideN(a1);
    float a = a1;


    int splitN = max(1.0f, floor((a2-a1)/Dangle));
    float res = (a2-a1)/splitN;
    /*float a = s1*Dangle;
    while (a <= a1) a += Dangle;*/
    for(; a < a2-1e-6; a += res) angles.push_back(a);
    angles.push_back(a2);
    return angles;
}

float VRBRepUtils::Bik(float t, int i, int k, const vector<double>& knots, string indent, bool verbose) {
    if (verbose) indent += "  ";
    float ti = knots[i];
    float ti1 = knots[i+1];
    float tik = knots[i+k];
    float tik1 = knots[i+k+1];
    float t0 = knots[0];
    float tL = knots[knots.size()-1];
    if (verbose) cout << indent << "Bik t: " << t << " i: " << i << " k: " << k << "  -->  ti: " << ti << " ti1: " << ti1 << " tik: " << tik << " tik1: " << tik1 << endl;
    if (k == 0) {
        if (t >= ti && t < ti1) { if (verbose) cout << indent << " Bik goes first 1" << endl; return 1; }
        if (t <= t0 && ti <= t0) { if (verbose) cout << indent << " Bik goes second 1" << endl; return 1; }
        if (t >= tL && ti1 >= tL) { if (verbose) cout << indent << " Bik goes third 1" << endl; return 1; }
        if (verbose) cout << indent << " Bik goes 0, i: " << i << ", t: " << t << ", t0: " << t0 << ", tL: " << tL << endl;
        return 0;
    }
    float A = tik == ti ? 0 : Bik(t, i, k-1, knots, indent, verbose)*(t-ti)/(tik-ti);
    float B = tik1 == ti1 ? 0 : Bik(t, i+1, k-1, knots, indent, verbose)*(tik1 - t)/(tik1 - ti1);
    if (verbose) cout << indent << " Bik return A " << A << "(" << (tik == ti) << ")" << " B " << B << "(" << (tik1 == ti1) << ")" << endl;
    return A + B;
}

Vec3d VRBRepUtils::BSpline(float t, int deg, const vector<Vec3d>& cpoints, const vector<double>& knots, bool verbose) {
    Vec3d p;
    for (uint i=0; i<cpoints.size(); i++) p += cpoints[i]*Bik(t, i, deg, knots, "", verbose);
    return p;
}

Vec3d VRBRepUtils::BSplineW(float t, int deg, const vector<Vec3d>& cpoints, const vector<double>& knots, const vector<double>& weights, bool verbose) {
    Vec3d p;
    float W = 0;
    for (uint i=0; i<cpoints.size(); i++) W += Bik(t, i, deg, knots, "", verbose)*weights[i];
    if (abs(W) > 1e-4) {
        for (uint i=0; i<cpoints.size(); i++) p += cpoints[i]*Bik(t, i, deg, knots, "", verbose)*weights[i]/W;
    } else {
        for (uint i=0; i<cpoints.size(); i++) {
            cout << " i: " << i << ", Bik: " << Bik(t, i, deg, knots, "", verbose) << ", t: " << t << ", i: " << i << ", deg: " << toString(deg) << ", knots: " << toString(knots) << endl;
            Bik(t, i, deg, knots, "", true); // verbose
        }

        for (uint i=0; i<cpoints.size(); i++) p += cpoints[i]*Bik(t, i, deg, knots, "", verbose);
    }
    return p;
}

Vec3d VRBRepUtils::BSpline(float u, float v, int degu, int degv, const field<Vec3d>& cpoints, const vector<double>& knotsu, const vector<double>& knotsv) {
    Vec3d p;
    for (int i=0; i<cpoints.width; i++) {
        for (int j=0; j<cpoints.height; j++) {
            Vec3d cp = cpoints.get(i,j);
            p += cp*Bik(u, j, degu, knotsu)*Bik(v, i, degv, knotsv);
            //cout << " VRBRepUtils::BSpline Bik(u) " << Bik(u, j, degu, knotsu,1) << endl;
        }
    }
    //cout << "VRBRepUtils::BSpline u,v " << Vec2d(u, v) << "\t p " << p << endl;
    return p;
}

Vec3d VRBRepUtils::BSplineNorm(float u, float v, int degu, int degv, const field<Vec3d>& cpoints, const vector<double>& knotsu, const vector<double>& knotsv) {
    Vec3d du, dv;

    // derivative in u
    for (int i=0; i<cpoints.width; i++) {
        for (int j=0; j<cpoints.height-1; j++) {
            float ti1 = knotsu[j+1];
            float tik1 = knotsu[j+degu+1];
            Vec3d cp = (cpoints.get(i,j+1) - cpoints.get(i,j))*degu/(tik1-ti1);
            du += cp*Bik(u, j+1, degu-1, knotsu)*Bik(v, i, degv, knotsv);
        }
    }

    // derivative in v
    for (int i=0; i<cpoints.width-1; i++) {
        for (int j=0; j<cpoints.height; j++) {
            float ti1 = knotsv[i+1];
            float tik1 = knotsv[i+degv+1];
            Vec3d cp = (cpoints.get(i+1,j) - cpoints.get(i,j))*degu/(tik1-ti1);
            dv += cp*Bik(u, j, degu, knotsu)*Bik(v, i+1, degv-1, knotsv);
        }
    }

    return dv.cross(du);
}

Vec3d VRBRepUtils::BSpline(float u, float v, int degu, int degv, const field<Vec3d>& cpoints, const vector<double>& knotsu, const vector<double>& knotsv, const field<double>& weights) {
    float W = 0;
    for (int i=0; i<cpoints.width; i++) {
        for (int j=0; j<cpoints.height; j++) {
            W += Bik(u, j, degu, knotsu)*Bik(v, i, degv, knotsv)*weights.get(i,j);
        }
    }

    Vec3d p;
    for (int i=0; i<cpoints.width; i++) {
        for (int j=0; j<cpoints.height; j++) {
            Vec3d cp = cpoints.get(i,j)*weights.get(i,j)/W;
            p += cp*Bik(u, j, degu, knotsu)*Bik(v, i, degv, knotsv);
        }
    }
    return p;
}

Vec3d VRBRepUtils::BSplineNorm(float u, float v, int degu, int degv, const field<Vec3d>& cpoints, const vector<double>& knotsu, const vector<double>& knotsv, const field<double>& weights) {
    float W = 0;
    for (int i=0; i<cpoints.width; i++) {
        for (int j=0; j<cpoints.height; j++) {
            W += Bik(u, j, degu, knotsu)*Bik(v, i, degv, knotsv)*weights.get(i,j);
        }
    }

    Vec3d du, dv;

    // derivative in u
    for (int i=0; i<cpoints.width; i++) {
        for (int j=0; j<cpoints.height-1; j++) {
            float ti1 = knotsu[j+1];
            float tik1 = knotsu[j+degu+1];
            Vec3d cp = (cpoints.get(i,j+1)/*weights.get(i,j+1)/W*/ - cpoints.get(i,j)/*weights.get(i,j)/W*/)*degu/(tik1-ti1);
            du += cp*Bik(u, j+1, degu-1, knotsu)*Bik(v, i, degv, knotsv);
        }
    }

    // derivative in v
    for (int i=0; i<cpoints.width-1; i++) {
        for (int j=0; j<cpoints.height; j++) {
            float ti1 = knotsv[i+1];
            float tik1 = knotsv[i+degv+1];
            Vec3d cp = (cpoints.get(i+1,j)/*weights.get(i+1,j)/W*/ - cpoints.get(i,j)/*weights.get(i,j)/W*/)*degu/(tik1-ti1);
            dv += cp*Bik(u, j, degu, knotsu)*Bik(v, i+1, degv-1, knotsv);
        }
    }

    return dv.cross(du);
}

void VRBRepUtils::compareCoordSystems(PosePtr p1, PosePtr p2, double eps) {
    double ddd = p1->dir().dot(p2->dir());
    double udu = p1->up().dot(p2->up());
    bool coPlanar = bool(abs(ddd) > 1.0-eps);
    double uangle = acos(udu);

    cout << "compare coord systems: ";
    if (coPlanar && ddd<0) cout << " inverse";
    if (coPlanar) cout << " coplanar";
    if (uangle > eps) cout << " rotated " << int(uangle*180/3.14159) << " deg";
    cout << endl;
}





