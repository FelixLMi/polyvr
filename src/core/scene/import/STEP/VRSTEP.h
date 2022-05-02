#ifndef VRSTEP_H_INCLUDED
#define VRSTEP_H_INCLUDED

class STEPfile;
class Registry;
class InstMgr;
class SDAI_Application_instance;
typedef SDAI_Application_instance STEPentity;
class STEPaggregate;
class SDAI_Select;
class STEPattribute;
class STEPcomplex;
class SingleLinkNode;

#include <memory>
#include <string>
#include <map>
#include <tuple>
#include "core/math/OSGMathFwd.h"
#include <OpenSG/OSGLine.h>
#include "core/objects/VRObjectFwd.h"
#include "core/utils/VRFunctionFwd.h"
#include "core/utils/VRUtilsFwd.h"
#include "core/math/field.h"
#include "core/math/VRMathFwd.h"
#include "../VRImportFwd.h"
#include "core/gui/VRGuiFwd.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRGuiTreeExplorer;

class VRSTEP : public std::enable_shared_from_this<VRSTEP> {
    public:
        typedef shared_ptr<Registry> RegistryPtr;
        typedef shared_ptr<InstMgr> InstMgrPtr;
        typedef shared_ptr<STEPfile> STEPfilePtr;

        struct Type {
            bool print = false;
            string path; // args
            string cpath; // complex entity args
            shared_ptr< VRFunction<STEPentity*> > cb;
        };

        struct Node {
            bool traversed = 0;
            string type = "ENTITY";
            string a_val = "NONE";
            string a_name = "NONE";
            STEPentity* entity = 0;
            STEPaggregate* aggregate = 0;
            SDAI_Select* select = 0;
            map<STEPentity*, Node*> parents;
            vector<Node*> childrenV;
            map<STEPentity*, Node*> children;

            void addChild(Node*);
            STEPentity* key();
        };

        map<STEPentity*, Node*> nodes;
        VRSTEPExplorerPtr explorer;

        struct Instance {
            string type;
            STEPentity* entity = 0;
            int ID = -1;
            void* data = 0;
            template<size_t i, class... Args>
            typename std::tuple_element<i, tuple<Args...> >::type get() {
                auto t = (tuple<Args...>*)data;
                return std::get<i>(*t);
            }

            operator bool() const { return data != 0; }
        };

        struct Edge;
        struct Bound;
        struct Surface;

        static vector<STEPentity*> unfoldComplex(STEPentity* e);

    public:
        RegistryPtr registry;
        InstMgrPtr instMgr;
        STEPfilePtr sfile;

        map<string, bool> blacklist;
        int blacklisted = 0;
        map<string,string> options;
        bool scaleDefined = false;
        float scale = 1.0;
        string filePath;

        string redBeg  = "\033[0;38;2;255;150;150m";
        string greenBeg  = "\033[0;38;2;150;255;150m";
        string blueBeg  = "\033[0;38;2;150;150;255m";
        string colEnd = "\033[0m";

        map<STEPentity*, int> explRowIds;
        map<STEPentity*, Instance> instances;
        map<STEPentity*, VRMaterialPtr> materials;
        map<int, Instance> instancesById;
        map<string, vector<Instance> > instancesByType;
        map<STEPentity*, VRTransformPtr> resGeos;
        VRTransformPtr resRoot;

        VRProgressPtr progress;
        bool threaded = false;

        map<string, Type> types;
        Instance& getInstance(STEPentity* e);
        template<class T> void addType(string type, string path, string cpath, bool print = false);
        template<class T> void parse(STEPentity* e, string path, string cpath, string type);

        template<size_t i, class... Args>
        Instance& getChild(Instance& instance, string type) {
            auto& child = getInstance( instance.get<i, Args...>() );
            if (!child) cout << "VRSTEP::getChild Error, " << instance.ID << " has no valid data!";
            if (child.type != type) cout << "VRSTEP::getChild Error, type mismatch, child " << i << " (#" << child.ID << ") has type '" << child.type << "', expected type: '" << type << "'" << endl;
            return child;
        }

        bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, string& t, char c, string& type);
        bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, int& t, char c, string& type);
        bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, STEPentity*& t, char c, string& type);
        bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, SDAI_Select*& t, char c, string& type);
        bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, double& t, char c, string& type);
        bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, bool& t, char c, string& type);
        template<typename T> bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, vector<T>& vec, char t, string& type);
        template<typename T> bool getValue(STEPentity* e, STEPattribute* a, SingleLinkNode* an, field<T>& f, char t, string& type);
        void fieldPush(field<double>& f, string v);
        void fieldPush(field<STEPentity*>& f, string v);

        template<typename T> bool query(STEPentity* e, string path, T& t, string type);
        STEPentity* getSelectEntity(SDAI_Select* s);

        void loadT(string file, STEPfilePtr sfile, bool* done);
        void open(string file);

        void registerEntity(STEPentity* se, bool complexPass = 0);
        void parseEntity(STEPentity* se, bool complexPass = 0);
        void traverseEntity(STEPentity* se, int lvl, VRSTEP::Node* parent, bool complexPass = 0);
        void traverseSelect(SDAI_Select* s, int lvl, VRSTEP::Node* parent);
        void traverseAggregate(STEPaggregate* sa, int atype, STEPattribute* attr, int lvl, VRSTEP::Node* parent);

        void buildGeometries();
        void buildScenegraph();
        void buildMaterials();
        void build();

        void exploreEntity(VRSTEP::Node* n, bool doFilter = true);

    public:
        VRSTEP();
        ~VRSTEP();

		static VRSTEPPtr create();
		VRSTEPPtr ptr();

        void load(string file, VRTransformPtr res, map<string,string> options, VRProgressPtr p = 0, bool thread = false);
};

OSG_END_NAMESPACE;


#endif // VRSTEP_H_INCLUDED
