#ifndef VREXPORT_H_INCLUDED
#define VREXPORT_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include <string>
#include <map>

#include "core/objects/VRObjectFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class VRExport {
    private:
        VRExport();

    public:
        static VRExport* get();

        void write(VRObjectPtr obj, string path, map<string, string> options);
};

OSG_END_NAMESPACE;

#endif // VREXPORT_H_INCLUDED
