#ifndef VRPYTEXTUREGENERATOR_H_INCLUDED
#define VRPYTEXTUREGENERATOR_H_INCLUDED

#include "VRPyObject.h"
#include "core/objects/material/VRTextureGenerator.h"

struct VRPyTexGenLayer : VRPyBaseT<OSG::VRTexGenLayer> {
    static PyMethodDef methods[];
};

struct VRPyTextureGenerator : VRPyBaseT<OSG::VRTextureGenerator> {
    static PyMethodDef methods[];
};

#endif // VRPYTEXTUREGENERATOR_H_INCLUDED
