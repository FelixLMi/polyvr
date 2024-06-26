#ifndef VRCODECOMPLETION_H_INCLUDED
#define VRCODECOMPLETION_H_INCLUDED

#include <map>
#include <vector>
#include "VRPyBase.h"
#include "core/scripting/VRScriptFwd.h"

using namespace std;

struct VRPyScript : VRPyBaseT<OSG::VRScript> {
    static PyMethodDef methods[];
};

class VRPyCodeCompletion {
    private:
        map<PyObject*, vector<string>> members;
        map<string, PyObject*> objects;
        PyObject* jediWrap = 0;

        bool startsWith(const string& a, const string& b);

        PyObject* getObject(string, PyObject* parent = 0);
        PyObject* resolvePath(vector<string>& path);
        map<string, PyObject*> getMembers(PyObject* obj);

	public:
        VRPyCodeCompletion();
        ~VRPyCodeCompletion();

        vector<string> getSuggestions(string s);

        // jedi wrapper
        vector<string> getJediSuggestions(OSG::VRScriptPtr script, int line, int column);
};

#endif // VRCODECOMPLETION_H_INCLUDED
