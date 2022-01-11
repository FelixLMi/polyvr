#include "VRPyLight.h"
#include "VRPyLightBeacon.h"
#include "VRPyBaseT.h"
#include "VRPyBaseFactory.h"

using namespace OSG;

simpleVRPyType(Light, New_VRObjects_ptr);

PyMethodDef VRPyLight::methods[] = {
    {"setOn", PyWrap(Light, setOn, "Set light state", void, bool) },
    {"setBeacon", PyWrap(Light, setBeacon, "Set the light beacon", void, VRLightBeaconPtr) },
    {"getBeacon", PyWrap(Light, getBeacon, "Get the light beacon", VRLightBeaconPtr ) },
    {"addBeacon", PyWrap(Light, addBeacon, "Create and return a light beacon", VRLightBeaconPtr ) },
    {"setDiffuse", PyWrap(Light, setDiffuse, "Set diffuse light color", void, Color4f) },
    {"setAmbient", PyWrap(Light, setAmbient, "Set ambient light color", void, Color4f) },
    {"setSpecular", PyWrap(Light, setSpecular, "Set specular light color", void, Color4f) },
    {"setAttenuation", PyWrap(Light, setAttenuation, "Set light attenuation parameters, [constant, linear, quadratic]", void, Vec3d) },
    {"getAttenuation", PyWrap(Light, getAttenuation, "Get light attenuation parameters, [constant, linear, quadratic]", Vec3d) },
    {"setType", PyWrap(Light, setType, "Set light type: 'point', 'directional', 'spot', 'photometric'", void, string) },
    {"setShadowParams", PyWrapOpt(Light, setShadowParams, "Set shadow parameters", "0 0", void, bool, int, Color4f, Vec2d) },
    {"setPhotometricMap", PyWrap(Light, setPhotometricMap, "Set map for photometric light, path to .ies file", void, VRTexturePtr) },
    {"loadPhotometricMap", PyWrap(Light, loadPhotometricMap, "Set map for photometric light, path to .ies file", void, string) },
    {"getPhotometricMap", PyWrapOpt(Light, getPhotometricMap, "Return photometric map as texture", "0", VRTexturePtr, bool) },
    {"isOn", PyWrap(Light, isOn, "Return the state of the light", bool) },
    {"reloadDeferredSystem", PyWrap(Light, reloadDeferredSystem, "Reload parameters and shaders for the deferred system", void) },
    {"toggleShadows", PyWrap(Light, toggleShadows, "Toggle shadows efficiently", void, bool) },
    {"setShadowNearFar", PyWrap(Light, setShadowNearFar, "Set near and far of the light pass", void, Vec2d) },
    {"setShadowVolume", PyWrap(Light, setShadowVolume, "Set shadowed scene volume", void, Boundingbox) },
    //{"setShadowBlendFactors", PyWrap(Light, setShadowBlendFactors, "Test function to tune shadow blending", void, int, int, int, float) },
    {NULL}
};



