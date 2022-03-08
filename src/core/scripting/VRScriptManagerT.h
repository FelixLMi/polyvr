#ifndef VRSCRIPTMANAGERT_H_INCLUDED
#define VRSCRIPTMANAGERT_H_INCLUDED

#include "VRScriptManager.h"

OSG_BEGIN_NAMESPACE;

template<class T>
void VRScriptManager::registerModule(string mod, PyObject* parent, PyTypeObject* base, string mod_parent) {
    vector<PyTypeObject*> bases;
    if (base) bases.push_back(base);
    T::registerModule(mod, parent, bases );
    moduleTypes[mod_parent][mod] = T::typeRef;
}

template<class T>
void VRScriptManager::registerModule(string mod, PyObject* parent, vector<PyTypeObject*> bases, string mod_parent) {
    T::registerModule(mod, parent, bases);
    moduleTypes[mod_parent][mod] = T::typeRef;
}

OSG_END_NAMESPACE;

#endif // VRSCRIPTMANAGERT_H_INCLUDED
