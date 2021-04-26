#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <vector>
#include "core/math/OSGMathFwd.h"
#include "core/math/VRMathFwd.h"
#include "core/math/boundingbox.h"
#include "core/math/pose.h"
#include "core/utils/VRStorage.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class Graph : public VRStorage {
    public:
        enum CONNECTION {
            SIMPLE = 0,
            HIERARCHY = 1,
            DEPENDENCY = 2,
            SIBLING = 3
        };

        struct node {
            Pose p;
            Boundingbox box;
            int ID = -1;

            vector<int> inEdges;
            vector<int> outEdges;
        };

        struct edge {
            int from = 0;
            int to = 0;
            CONNECTION connection = SIMPLE;
            int ID = -1;

            vector<int> relations;
            edge(int i = 0, int j = 0, CONNECTION c = SIMPLE, int ID = 0);
        };

        struct position {
            int node = -1;
            int edge = -1;
            float pos = 0;

            position();
            position(int n);
            position(int e, float p);
        };

    protected:
        map<int, edge> edges;
        map<int, node> nodes;
        edge nullEdge;

    public:
        Graph();
        ~Graph();

        static shared_ptr< Graph > create() { return shared_ptr< Graph >(new Graph()); }

        int connect(int i, int j, int c = SIMPLE);
        void disconnect(int i, int j);
        int split(int i, int e);
        node& getNode(int i);
        edge& getEdge(int e);
        edge& getEdge(int n1, int n2);
        edge getEdgeCopy(int n1, int n2);
        edge getEdgeCopyByID(int e);
        int getEdgeID(int n1, int n2);

        map< int, node >& getNodes();
        vector< node > getNodesCopy();
        vector< node > getNextNodes(int i);
        vector< node > getPreviousNodes(int i);
        vector< node > getNeighbors(int i);

        map< int, edge>& getEdges();
        vector< edge > getEdgesCopy();
        vector< edge > getInEdges(int n);
        vector< edge > getOutEdges(int n);

        vector< int > getRelations(int e);
        int getNEdges();
        int size();
        bool connected(int i1, int i2);
        void setPosition(int i, PosePtr v);
        void addRelation(int e1, int e2);
        float getEdgeLength(int e);
        PosePtr getPosition(int i);

        bool hasNode(int i);
        bool hasEdge(int i);
        bool hasRelation(int e1, int e2);

        vector<edge> getConnectedEdges(node& n);
        vector<edge> getPrevEdges(edge& e);
        vector<edge> getNextEdges(edge& e);

        virtual int addNode(PosePtr p = 0, BoundingboxPtr b = 0);
        virtual void remNode(int i);
        virtual void update(int i, bool changed);
        virtual void clear();
};

OSG_END_NAMESPACE;

#endif // GRAPH_H_INCLUDED
