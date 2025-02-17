#include "VRAnnotationEngine.h"

#include "core/objects/material/VRMaterial.h"
#include "core/objects/material/VRMaterialT.h"
#include "core/objects/material/VRTexture.h"
#include "core/objects/geometry/VRGeoData.h"
#include "core/objects/geometry/OSGGeometry.h"
#include "core/objects/VRCamera.h"
#include "core/tools/VRText.h"
#include "core/math/pose.h"
#include "core/utils/toString.h"
#include "core/scene/VRScene.h"

#include <OpenSG/OSGIntersectAction.h>

#define GLSL(shader) #shader

//#define OSG_OGL_ES2

using namespace OSG;

bool hasGS = false;

VRAnnotationEngine::VRAnnotationEngine(string name, bool init) : VRGeometry(name) {
    type = "AnnotationEngine";
    hasGS = VRScene::getCurrent()->hasGeomShader();
    if (init) initialize();
}

VRAnnotationEnginePtr VRAnnotationEngine::create(string name, bool init) { return shared_ptr<VRAnnotationEngine>(new VRAnnotationEngine(name, init) ); }
VRAnnotationEnginePtr VRAnnotationEngine::ptr() { return static_pointer_cast<VRAnnotationEngine>( shared_from_this() ); }

void VRAnnotationEngine::initialize() {
    mat = VRMaterial::create("AnnEngMat");
#ifndef OSG_OGL_ES2
    if (hasGS) {
        mat->setVertexShader(vp, "annotationVS");
        mat->setFragmentShader(fp, "annotationFS");
        mat->setFragmentShader(dfp, "annotationDFS", true);
        mat->setGeometryShader(gp, "annotationGS");
    } else {
#endif
        mat->setVertexShader(vp_es2, "annotationVSes2");
        mat->setFragmentShader(fp_es2, "annotationFSes2");
#ifndef OSG_OGL_ES2
    }
#endif
    mat->setPointSize(5);
    if (!hasGS) mat->setMagMinFilter(GL_LINEAR, GL_LINEAR); // TODO: implement test for mipmaps!
    setMaterial(mat);
    updateTexture();

    setSize(0.2);
    setBillboard(false);
    setScreensize(false);
    data = VRGeoData::create();

    setOrientation(Vec3d(0,0,1), Vec3d(0,1,0));
}

void VRAnnotationEngine::clear() {
    labels.clear();
    data->reset();
}

VRObjectPtr VRAnnotationEngine::copy(vector<VRObjectPtr> children) {
    VRAnnotationEnginePtr ann = VRAnnotationEngine::create(getBaseName(), false);
    //ann->setMesh(mesh);
    ann->setMaterial(mat);
    ann->setReference(getReference());
    ann->setVisible(isVisible());
    ann->setPickable(isPickable());
    ann->setEntity(entity);
    ann->setMatrix(getMatrix());

    ann->mat = mat;
    ann->fg = fg;
    ann->bg = bg;
    ann->oc = oc;
    ann->oradius = oradius;
    ann->size = size;
    ann->texPadding = texPadding;
    ann->charTexSize = charTexSize;
    ann->orientationUp = orientationUp;
    ann->orientationDir = orientationDir;
    ann->characterIDs = characterIDs;
    ann->data = VRGeoData::create();
    return ann;
}

void VRAnnotationEngine::setColor(Color4f c) { fg = c; updateTexture(); }
void VRAnnotationEngine::setBackground(Color4f c) { bg = c; updateTexture(); }
void VRAnnotationEngine::setOutline(int r, Color4f c) { oradius = r; oc = c; updateTexture(); }

string VRAnnotationEngine::getLabel(int i) {
    return labels[i].str;
}

map<int, string> VRAnnotationEngine::getLabels() {
    map<int, string> res;
    for (size_t i=0; i<labels.size(); i++) res[i] = labels[i].str;
    return res;
}

bool VRAnnotationEngine::applyIntersectionAction(Action* action) {
    if (!mesh || !mesh->geo) return false;


    IntersectAction* ia = dynamic_cast<IntersectAction*>(action);

    auto ia_line = ia->getLine();
    Pnt3d l0 = Pnt3d(ia_line.getPosition());
    Vec3d ld = Vec3d(ia_line.getDirection());
    Pnt3d lh = Pnt3d(ia->getHitPoint());

    // TODO: convert ray in engine coord system!
    //cout << "VRAnnotationEngine::applyIntersectionAction l0: " << l0 << ", ld: " << ld << ", lh: " << lh << endl;

    double h = size;
    double w = size;
    bool didHit = false;
    Real32 T = 1e6;

    PosePtr camPose;
    if (doScreensize || doBillboard) {
        auto cam = VRScene::getCurrent()->getActiveCamera();
        camPose = getPoseTo(cam);
    }

    for (auto& l : labels) {
        int N = l.entries.size();
#ifdef OSG_OGL_ES2
        N = ceil(N/12.0)+4;
#endif
        if (N == 0) continue;

        Pnt3d P0 = data->getPosition(l.entries[0]);

        if (doScreensize) {
            Vec3d D = Vec3d(P0) - camPose->pos();
            D.normalize();
            P0 = Pnt3d(camPose->pos() + D); // essentially move the point to be at 1 from camera, TODO: might not work with head tracking!
        }

        Vec3d Pp;
        Pnt3d lh;
        double X, Y, d;
        if (doBillboard) { // labels are facing camera
            Vec3d n = -camPose->dir();
            d = Vec3d(P0-l0).dot(n)/ld.dot(n);
            lh = l0 + ld*d;
            Pp = lh - P0;
            X = Pp.dot(camPose->x());
            Y = Pp.dot(camPose->up());
            //cout << "applyIntersectionAction n:" << n << "  d:" << d << "  P0:" << P0 << " lh:" << lh << " Pp:" << Pp << "  " << Vec2d(X/(w*((N-4)*3-0.5)),Y/(h*0.5)) << endl;
            //cout << " applyIntersectionAction Nw: " << ((N-4)*3-0.5) << "  W:" << w*((N-4)*3-0.5) << " w:" << w << endl;
        } else {
            d = (P0[2]-l0[2])/ld[2]; // all labels are in z+k=0 plane
            lh = l0 + ld*d;
            Pp = lh - P0;
            X = Pp[0];
            Y = Pp[1];
        }

        //cout << " VRAnnotationEngine::applyIntersectionAction P0 " << P0 << ", d " << d << ", X " << X << ", lh " << lh << endl;
        //cout << " VRAnnotationEngine::applyIntersectionAction Y " << Y << ", X " << X << ", w " << w << ", h " << h << ",  " << endl;
        if (Y < -h*0.5) continue;
        if (Y >  h*0.5) continue;

        if (X < -w*0.5) continue;
        if (X >  w*(l.Ngraphemes-0.5) ) continue;
        didHit = true;
        //cout << " VRAnnotationEngine::applyIntersectionAction " << N << " -> " << (N-4)*3 << " -> " << ((N-4)*3-0.5) << " " << Pp[0] << " " << w << " -> " << w*((N-4)*3-0.5) << endl;

        // label hit!
        Real32 t = l0.dist( lh );
        if (t < T) {
            T = t;
            Vec3f norm(0,1,0);
            ia->setHit(t, ia->getActNode(), 0, norm, l.ID);
            //cout << " VRAnnotationEngine::applyIntersectionAction ID: " << l.ID << " " << l.str << endl;
        }
    }

    //cout << "  VRAnnotationEngine::applyIntersectionAction T: " << T << ", didHit: " << didHit << endl;
    if (didHit) return true;
	else return VRGeometry::applyIntersectionAction(action); // fallback
}

bool VRAnnotationEngine::checkUIn(int i) {
    if (i < 0 || i > (int)data->size()) return true;
    return false;
}

void VRAnnotationEngine::resize(Label& l, Vec3d p, int N) {
    int eN = l.entries.size();
    if (N == eN) return;

#ifndef OSG_OGL_ES2
    if (hasGS) {
        for (int i=0; i<eN; i++) data->setVert(l.entries[i], Vec3d(), Vec3d(), Vec2d()); // clear old buffer

        if (N <= eN) return;
        l.entries.resize(N, 0);
        int pN = data->size();

        for (int i=0; i<N-eN; i++) {
            data->pushVert(p, Vec3d(), Vec2d());
            data->pushPoint();
            l.entries[eN+i] = pN+i;
        }
    }
    else {
#endif
        for (int i=0; i<eN; i++) data->setVert(l.entries[i], Vec3d(), Vec3d(), Vec2d()); // clear old buffer

        if (N <= eN) return;
        l.entries.resize(N, 0);
        int pN = data->size();

        for (int i=eN; i<N; i++) {
            data->pushVert(p, Vec3d(), Vec2d());
            l.entries[i] = pN+i-eN;
            if (i%4 == 3) {
                data->pushTri(-4,-3,-2);
                data->pushTri(-4,-2,-1);
            }
        }
#ifndef OSG_OGL_ES2
    }
#endif

    data->apply( ptr() );
}

int VRAnnotationEngine::add(Vec3d p, string s) {
    int i = labels.size();
    set(i,p,s);
    return i;
}

VRAnnotationEngine::Label::Label(int id) : ID(id) {}

void VRAnnotationEngine::setMask(int i, float a, float b) {
    if (i < 0 || i >= labels.size()) return;
    Label& label = labels[i];
    int Ng = label.Ngraphemes;
    label.maskA = round(a*Ng);
    label.maskB = round(b*Ng);

    int Nc = 2; // number of points, 2 chars per vertex
    int Nv = ceil(Ng/double(Nc));
    for (int j=0; j<Nv; j++) {
        int k = label.entries[j];
        Vec2d mask(1,1);
        if (label.maskA != -1 && j*2   < label.maskA || label.maskB != -1 && j*2   >= label.maskB) mask[0] = 0;
        if (label.maskA != -1 && j*2+1 < label.maskA || label.maskB != -1 && j*2+1 >= label.maskB) mask[1] = 0;
        data->setTexCoord(k, mask);
    }
}

void VRAnnotationEngine::setLine(int i, Vec3d p, string str, bool ascii, bool force) {
    //cout << "VRAnnotationEngine::setLine i: " << i << ", p: " << p << ", str: '" << str << "', ascii: " << ascii << endl;
    while (i >= (int)labels.size()) labels.push_back(Label(labels.size()));
    auto& l = labels[i];
    if (l.str == str && (l.pos-p).length() < 1e-6 && !force) return;
    l.pos = p;
		l.ascii = ascii;
    vector<string> graphemes;
    if (!ascii) {
        l.Ngraphemes = VRText::countGraphemes(str);
        graphemes = VRText::splitGraphemes(str);
    } else {
        l.Ngraphemes = str.size();
        graphemes = vector<string>(l.Ngraphemes);
        for (int i=0; i<l.Ngraphemes; i++) graphemes[i] = str[i];
    }

#ifndef OSG_OGL_ES2
    if (hasGS) {
        int Nc = 2; // number of points, 2 chars per vertex
        int N = ceil(l.Ngraphemes/double(Nc));
        resize(l,p,N + 4); // plus 4 bounding points

        for (int j=0; j<N; j++) {
            int c[] = {0,0};

            for (int k = 0; k<Nc; k++) {
                if (j*Nc+k < (int)graphemes.size()) {
                    string grapheme = graphemes[j*Nc+k];
                    if (!characterIDs.count(grapheme)) addGrapheme(grapheme);
                    c[k] = characterIDs[grapheme];
                    //cout << "char: '" << grapheme << "', ID: " << characterIDs[grapheme] << endl;
                }
            }

            int k = l.entries[j];
            Vec2d mask(1,1);
            if (l.maskA != -1 && j*2   < l.maskA || l.maskB != -1 && j*2   >= l.maskB) mask[0] = 0;
            if (l.maskA != -1 && j*2+1 < l.maskA || l.maskB != -1 && j*2+1 >= l.maskB) mask[1] = 0;
            data->setVert(k, p, Vec3d(c[0],c[1],j), mask);
        }

        // bounding points to avoid word clipping
        data->setVert(l.entries[N], p+Vec3d(-0.25*size, -0.5*size, 0), Vec3d(0,0,-1), Vec2d(1,1));
        data->setVert(l.entries[N+1], p+Vec3d(-0.25*size,  0.5*size, 0), Vec3d(0,0,-1), Vec2d(1,1));
        data->setVert(l.entries[N+2], p+Vec3d((l.Ngraphemes-0.25)*size, -0.5*size, 0), Vec3d(0,0,-1), Vec2d(1,1));
        data->setVert(l.entries[N+3], p+Vec3d((l.Ngraphemes-0.25)*size,  0.5*size, 0), Vec3d(0,0,-1), Vec2d(1,1));
    }
    else {
#endif
        int N = l.Ngraphemes;
        for (int j=0; j<N; j++) {
            string grapheme = graphemes[j];
						if (!characterIDs.count(grapheme)) {
								addGrapheme(grapheme);
								updateLines();
						}
				}

        //resize(l,p,N*4+4); // plus 4 bounding points
				resize(l,p,N*4);
        float H = size*0.5;
        float D = charTexSize*0.5;
        float P = texPadding;


        Vec3d orientationX = -orientationDir.cross(orientationUp);

        for (int j=0; j<N; j++) {
            string grapheme = graphemes[j];
            char c = characterIDs[grapheme] - 1;
            float u1 = P+c*D*2;
            float u2 = P+(c+1)*D*2;
            float X = H*2*j;

            Vec3d n1 = orientationX*(-H+X) + orientationUp*(H*2);
            Vec3d n2 = orientationX*( H+X) + orientationUp*(H*2);
            Vec3d n3 = orientationX*( H+X) - orientationUp*(H*2);
            Vec3d n4 = orientationX*(-H+X) - orientationUp*(H*2);

            data->setVert(l.entries[j*4+0], p, n1, Vec2d(u1,1));
            data->setVert(l.entries[j*4+1], p, n2, Vec2d(u2,1));
            data->setVert(l.entries[j*4+2], p, n3, Vec2d(u2,0));
            data->setVert(l.entries[j*4+3], p, n4, Vec2d(u1,0));
        }

        // bounding points to avoid word clipping
        /*data->setVert(l.entries[N*4]  , p+Vec3d(-0.25*size, -0.5*size, 0), Vec3d(0,0,0));
        data->setVert(l.entries[N*4+1], p+Vec3d(-0.25*size,  0.5*size, 0), Vec3d(0,0,0));
        data->setVert(l.entries[N*4+2], p+Vec3d((l.Ngraphemes-0.25)*size, -0.5*size, 0), Vec3d(0,0,0));
        data->setVert(l.entries[N*4+3], p+Vec3d((l.Ngraphemes-0.25)*size,  0.5*size, 0), Vec3d(0,0,0));*/
#ifndef OSG_OGL_ES2
    }
#endif

    l.str = str;
}

void VRAnnotationEngine::set(int i0, Vec3d p0, string txt) {
    auto strings = splitString(txt, '\n');
    if (txt == "") strings = { "" };
    int Nlines = strings.size();
    for (int y = 0; y<Nlines; y++) {
        string str = strings[y];
        Vec3d p = p0;
        p[1] -= y*size;
        int i = i0+y;
        if (i < 0) return;
        setLine(i, p, str);
    }
}

void VRAnnotationEngine::updateLines() {
		for (int i=0; i<labels.size(); i++) {
				auto& l = labels[i];
				setLine(i, l.pos, l.str, l.ascii, true);
		}
}

void VRAnnotationEngine::setSize(float f) { size = f; mat->setShaderParameter("size", Real32(f)); }
void VRAnnotationEngine::setBillboard(bool b) { doBillboard = b; mat->setShaderParameter("doBillboard", Real32(b)); }
void VRAnnotationEngine::setScreensize(bool b) { doScreensize= b; mat->setShaderParameter("screen_size", Real32(b)); }

void VRAnnotationEngine::setOrientation(Vec3d d, Vec3d u) {
    orientationUp = u;
    orientationDir = d;
    mat->setShaderParameter("orientationDir", Vec3f(d));
    mat->setShaderParameter("orientationUp", Vec3f(u));
}

void VRAnnotationEngine::addGrapheme(string g) {
    characters += g;
    updateTexture();
}

void VRAnnotationEngine::updateTexture() {
    int cN = VRText::countGraphemes(characters);
		if (cN == 0) return;
    int padding = 3;
    int spread = 6;
		//cout << "VRAnnotationEngine::updateTexture " << characters << ", " << cN << endl;
    auto img = VRText::get()->create(characters, "Mono.ttf", 48, padding, fg, bg, oradius, oc, spread);

    float tW = img->getSize()[0];
    float lW = VRText::get()->layoutWidth;
    texPadding = (padding+oradius) / tW;
    charTexSize = lW/tW / cN;

    mat->setTexture(img);
    mat->setShaderParameter("texPadding", Real32(texPadding)); // tested
    mat->setShaderParameter("charTexSize", Real32(charTexSize));
    //img->write(getName()+"-annChars.png");

    int i=1; // 0 is used for invalid/no char
    for (auto c : VRText::splitGraphemes(characters)) {
        characterIDs[c] = i;
        i++;
    }
}

string VRAnnotationEngine::vp =
"#version 120\n"
GLSL(
varying vec4 vertex;
varying vec3 normal;
varying mat4 MVP;
varying mat4 P;
varying vec2 vTexCoord;

attribute vec4 osg_Vertex;
attribute vec4 osg_Normal;
attribute vec2 osg_MultiTexCoord0;

void main( void ) {
    vertex = osg_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix*osg_Vertex;
    normal = osg_Normal.xyz;
    MVP = gl_ModelViewProjectionMatrix;
    P = gl_ProjectionMatrix;
    vTexCoord = osg_MultiTexCoord0;
}
);

string VRAnnotationEngine::fp =
"#version 120\n"
GLSL(
uniform sampler2D texture;

varying vec2 texCoord;

void main( void ) {
    //gl_FragColor = vec4(texCoord.x,texCoord.y,0.0,1.0);
    gl_FragColor = texture2D(texture, texCoord);
}
);

string VRAnnotationEngine::dfp =
"#version 120\n"
GLSL(
uniform sampler2D texture;

varying vec4 geomPos;
varying vec3 geomNorm;
varying vec2 texCoord;

void main( void ) {
    vec3 pos = geomPos.xyz / geomPos.w;
    vec3 diffCol = texture2D(texture, texCoord).rgb;
    gl_FragData[0] = vec4(pos, 1);
    gl_FragData[1] = vec4(normalize(geomNorm), 0);
    gl_FragData[2] = vec4(diffCol, 0);
}
);

string VRAnnotationEngine::gp =
"#version 150\n"
//"#extension GL_EXT_geometry_shader4 : enable\n"
GLSL(
layout (points) in;
layout (triangle_strip, max_vertices=60) out;

uniform float doBillboard;
uniform float screen_size;
uniform vec3 orientationDir;
uniform vec3 orientationUp;
uniform float size;
uniform float texPadding;
uniform float charTexSize;
uniform vec2 OSGViewportSize;
in vec4 vertex[];
in vec3 normal[];
in vec2 vTexCoord[];
in mat4 MVP[];
in mat4 P[];
out vec2 texCoord;
out vec4 geomPos;
out vec3 geomNorm;

vec3 orientationX = vec3(1,0,0);

vec4 transform(in float x, in float y) {
    vec3 p = -orientationX*x + orientationUp*y;
    return vec4(p, 0);
}

void emitVertex(in vec4 p, in vec2 tc, in vec4 v) {
    gl_Position = p;
    texCoord = tc;
    geomPos = v;
    geomNorm = vec3(0,0,1);
    EmitVertex();
}

void emitQuad(in float offset, in vec4 tc) {
    float sx = 0.5*size;
    float sy = size;
    float ox = 2*sx*offset;
    vec4 p1;
    vec4 p2;
    vec4 p3;
    vec4 p4;
    vec4 v1;
    vec4 v2;
    vec4 v3;
    vec4 v4;
    vec4 p = gl_in[0].gl_Position;

    if (screen_size > 0.5) {
        p.xyz = p.xyz/p.w;
        p.w = 1;
    }

    if (doBillboard < 0.5) {
        p1 = p+MVP[0]*transform(-sx+ox,-sy);
        p2 = p+MVP[0]*transform(-sx+ox, sy);
        p3 = p+MVP[0]*transform( sx+ox, sy);
        p4 = p+MVP[0]*transform( sx+ox,-sy);
    } else {
		mat4 M = P[0]; // this is enough for desktop, but not for cave
		// TODO: pass k as uniform parameter and use the camera fov instead of 1.05!
		float a = OSGViewportSize.y / OSGViewportSize.x;
		float k = 1.0/tan(1.05*0.5); // M[1][1]; // 1.0/tan(cam.fov*0.5);
		M = mat4(mat2( k*a, 0.0, 0.0, k ));

		p1 = p+M*transform(-sx+ox,-sy);
		p2 = p+M*transform(-sx+ox, sy);
		p3 = p+M*transform( sx+ox, sy);
		p4 = p+M*transform( sx+ox,-sy);
		v1 = vertex[0]+M*transform(-sx+ox,-sy);
		v2 = vertex[0]+M*transform(-sx+ox, sy);
		v3 = vertex[0]+M*transform( sx+ox, sy);
		v4 = vertex[0]+M*transform( sx+ox,-sy);
    }

    emitVertex(p1, vec2(tc[0], tc[2]), v1);
    emitVertex(p2, vec2(tc[0], tc[3]), v2);
    emitVertex(p3, vec2(tc[1], tc[3]), v3);
    EndPrimitive();
    emitVertex(p1, vec2(tc[0], tc[2]), v1);
    emitVertex(p3, vec2(tc[1], tc[3]), v3);
    emitVertex(p4, vec2(tc[1], tc[2]), v4);
    EndPrimitive();
}

void emitChar(in int d, in float p) {
    float padding = texPadding; // 0.001 texture padding
    float f = charTexSize; // 0.00832 character texture size
    d -= 1; // offset
    if (d >= 0) emitQuad(p, vec4(padding+d*f, padding+(d+1)*f, 0, 1));
}

void emitString(in int c0, in int c1, in float offset) {
    if (c0 > 0 && int(vTexCoord[0][0]) > 0) emitChar(c0, 2*offset);
    if (c1 > 0 && int(vTexCoord[0][1]) > 0) emitChar(c1, 2*offset + 1);
}

void main() {
    orientationX = cross(orientationDir, orientationUp);
    float c0 = normal[0][0];
    float c1 = normal[0][1];
    float offset = normal[0][2];
    if (offset >= 0) emitString(int(c0), int(c1), offset);
}
);


/// --------------- OpenGL ES 2.0 Version (no GS) ----------------------

string VRAnnotationEngine::vp_es2 =
GLSL(
varying vec2 texCoord;

attribute vec4 osg_Vertex;
attribute vec4 osg_Normal;
attribute vec2 osg_MultiTexCoord0;

uniform float doBillboard;
uniform float screen_size;
uniform vec2 OSGViewportSize;
\n
#ifdef __EMSCRIPTEN__
\n
uniform mat4 OSGModelViewProjectionMatrix;
uniform mat4 OSGModelViewMatrix;
uniform mat4 OSGProjectionMatrix;
\n
#endif
\n

void main( void ) {
    vec4 P = osg_Vertex;
    vec4 N = osg_Normal;

\n
#ifdef __EMSCRIPTEN__
\n
    mat4 M = OSGModelViewProjectionMatrix;
#else
\n
    mat4 M = gl_ModelViewProjectionMatrix;
\n
#endif
\n

    if (doBillboard < 0.5 && screen_size < 0.5) { // normal case
        vec4 p = P + N;
        p.xyz *= 2.0; // TODO: hack.. I did not find out where this factor ist missing, vertex and normal info is correct..
        P = M * p;
    } else if (doBillboard > 0.5 && screen_size < 0.5) { // only billboard
        float a = OSGViewportSize.y/OSGViewportSize.x;
        N.x = N.x*a;
        N.z = 0.0;
        N.w = 0.0;
        P = M * P + N;
    } else if (screen_size > 0.5) { // screensize
	vec4 k = M * P;
        k.xyz = k.xyz/k.w;
        k.w = 1.0;

        float a = OSGViewportSize.y/OSGViewportSize.x;
        N.x = N.x*a;
        N.z = 0.0;
        N.w = 0.0;
        P = k + N;
    }

    gl_Position = P;
    texCoord = osg_MultiTexCoord0;
}
);

string VRAnnotationEngine::fp_es2 =
GLSL(
\n
#ifdef __EMSCRIPTEN__
\n
precision mediump float;
\n
#endif
\n
uniform sampler2D texture;

varying vec2 texCoord;

void main( void ) {
    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    //gl_FragColor = vec4(texCoord.x,texCoord.y,0.0,1.0);
    //gl_FragColor = vec4(texCoord.x,0.0,0.0,1.0);
    gl_FragColor = texture2D(texture, texCoord);
}
);
