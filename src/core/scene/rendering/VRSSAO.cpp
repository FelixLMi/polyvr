#include "VRSSAO.h"

#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGNode.h>

#include "core/objects/material/VRTexture.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/geometry/VRGeometry.h"
#include "core/scene/VRSceneManager.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

VRSSAO::VRSSAO() {}
VRSSAO::~VRSSAO() {}

float random(float f1, float f2) {
    float r = float(rand())/RAND_MAX;
    r *= (f2-f1);
    r += f1;
    return r;
}

float lerp(float f1, float f2, float f3) {
    return (f2-f1)*f3;
}

void VRSSAO::setSSAOparams(float r, int k, int n) {
    radius = r;
    kernelSize = k;
    noiseSize = n;

    if (ssao_mat) {
        int kernelSize2 = kernelSize*kernelSize;
        int noiseSize2 = noiseSize*noiseSize;

        ssao_mat->setShaderParameter<int>("KernelSize", kernelSize);
        ssao_mat->setShaderParameter<float>("uRadius", radius);
        ssao_mat->setShaderParameter<float>("texScale", 1.0/noiseSize);
        ssao_mat->setShaderParameter<int>("NoiseSize", noiseSize);

        // kernel
        vector<float> kernel(3*kernelSize2);
        vector<float> noise(3*noiseSize2);
        srand(0);

        for (int i = 0; i < kernelSize2; i++) {
            Vec3d k(random(-1,1), random(-1,1), random(0,1));
            k.normalize();
            k *= random(0,1);
            float scale = float(i) / float(kernelSize2);
            k *= lerp(0.1, 1, scale * scale);

            kernel[i*3+0] = k[0];
            kernel[i*3+1] = k[1];
            kernel[i*3+2] = k[2];
        }

        // noise texture
        for (int i = 0; i < noiseSize2; i++) {
            Vec3d n(random(-1,1), random(-1,1), 0);
            n.normalize();

            noise[i*3+0] = n[0];
            noise[i*3+1] = n[1];
            noise[i*3+2] = n[2];
        }

        // kernel texture
        VRTexturePtr img = VRTexture::create();
        img->getImage()->set(OSG::Image::OSG_RGB_PF, kernelSize, kernelSize, 1, 0, 1, 0.0, (const uint8_t*)&kernel[0], OSG::Image::OSG_FLOAT32_IMAGEDATA);
        ssao_mat->setTextureAndUnit(img, 3);
        ssao_mat->setMagMinFilter(GL_NEAREST, GL_NEAREST, 3);

        // noise texture
        VRTexturePtr imgN = VRTexture::create();
        imgN->getImage()->set(OSG::Image::OSG_RGB_PF, noiseSize, noiseSize, 1, 0, 1, 0.0, (const uint8_t*)&noise[0], OSG::Image::OSG_FLOAT32_IMAGEDATA);
        ssao_mat->setTextureAndUnit(imgN, 4);
        ssao_mat->setMagMinFilter(GL_NEAREST, GL_NEAREST, 4);
    }

    // blur size
    if (blur_matX) blur_matX->setShaderParameter<int>("uBlurSize", noiseSize);
    if (blur_matY) blur_matY->setShaderParameter<int>("uBlurSize", noiseSize);
}

void VRSSAO::initSSAO(VRMaterialPtr mat) {
    string shdrDir = VRSceneManager::get()->getOriginalWorkdir() + "/shader/DeferredShading/";
    ssao_mat = mat;

    // ssao material pass
    ssao_mat->setLit(false);
    ssao_mat->readVertexShader(shdrDir + "SSAOAmbient.vp.glsl");
    ssao_mat->readFragmentShader(shdrDir + "SSAOAmbient.fp.glsl", true);
    ssao_mat->setShaderParameter<int>("texBufPos", 0);
    ssao_mat->setShaderParameter<int>("texBufNorm", 1);
    ssao_mat->setShaderParameter<int>("texBufDiff", 2);
    ssao_mat->setShaderParameter<int>("uTexKernel", 3); // kernel texture
    ssao_mat->setShaderParameter<int>("uTexNoise", 4); // noise texture
    setSSAOparams(0.02, 6, 6);
}

void VRSSAO::initBlur(VRMaterialPtr matX, VRMaterialPtr matY) {
    string shdrDir = VRSceneManager::get()->getOriginalWorkdir() + "/shader/DeferredShading/";
    blur_matX = matX;
    blur_matY = matY;

    // ssao blur material
    blur_matX->readVertexShader(shdrDir + "blur.vp.glsl");
    blur_matX->readFragmentShader(shdrDir + "blur.fp.glsl", true);
    blur_matX->setShaderParameter<int>("texBufPos", 0);
    blur_matX->setShaderParameter<int>("texBufNorm", 1);
    blur_matX->setShaderParameter<int>("texBufDiff", 2);
    blur_matX->setShaderParameter<Vec2f>("direction", Vec2f(1,0));

    blur_matY->readVertexShader(shdrDir + "blur.vp.glsl");
    blur_matY->readFragmentShader(shdrDir + "blur.fp.glsl", true);
    blur_matY->setShaderParameter<int>("texBufPos", 0);
    blur_matY->setShaderParameter<int>("texBufNorm", 1);
    blur_matY->setShaderParameter<int>("texBufDiff", 2);
    blur_matY->setShaderParameter<Vec2f>("direction", Vec2f(0,1));
    setSSAOparams(0.02, 6, 6);
}

void VRSSAO::updateShader() {
    string shdrDir = VRSceneManager::get()->getOriginalWorkdir() + "/shader/DeferredShading/";

    // ssao material pass
    if (ssao_mat) {
        ssao_mat->setLit(false);
        ssao_mat->readVertexShader(shdrDir + "SSAOAmbient.vp.glsl");
        ssao_mat->readFragmentShader(shdrDir + "SSAOAmbient.fp.glsl", true);
        ssao_mat->setShaderParameter<int>("texBufPos", 0);
        ssao_mat->setShaderParameter<int>("texBufNorm", 1);
        ssao_mat->setShaderParameter<int>("texBufDiff", 2);
        ssao_mat->setShaderParameter<int>("uTexKernel", 3); // kernel texture
        ssao_mat->setShaderParameter<int>("uTexNoise", 4); // noise texture
    }

    // ssao blur material
    if (blur_matX) {
        blur_matX->readVertexShader(shdrDir + "blur.vp.glsl");
        blur_matX->readFragmentShader(shdrDir + "blur.fp.glsl", true);
        blur_matX->setShaderParameter<int>("texBufPos", 0);
        blur_matX->setShaderParameter<int>("texBufNorm", 1);
        blur_matX->setShaderParameter<int>("texBufDiff", 2);
        blur_matX->setShaderParameter<Vec2f>("direction", Vec2f(1,0));
    }

    if (blur_matY) {
        blur_matY->readVertexShader(shdrDir + "blur.vp.glsl");
        blur_matY->readFragmentShader(shdrDir + "blur.fp.glsl", true);
        blur_matY->setShaderParameter<int>("texBufPos", 0);
        blur_matY->setShaderParameter<int>("texBufNorm", 1);
        blur_matY->setShaderParameter<int>("texBufDiff", 2);
        blur_matY->setShaderParameter<Vec2f>("direction", Vec2f(0,1));
    }

    // parameters
    setSSAOparams(radius, kernelSize, noiseSize);
}

OSG_END_NAMESPACE;
