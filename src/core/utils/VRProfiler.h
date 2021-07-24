#ifndef VRPROFILER_H_INCLUDED
#define VRPROFILER_H_INCLUDED

#include <list>
#include <map>

#include "core/utils/VRMutex.h"

using namespace std;

class VRProfiler {
    public:
        struct Call {
            string name;
            int t0 = 0;
            int t1 = 0;
            int thread = 0;
        };

        struct Frame {
            int t0 = 0;
            int t1 = 0;
            bool running = true;
            map<int, Call> calls;
            unsigned long int fID = 0;
            unsigned long int Nchanged = 0;
            unsigned long int Ncreated = 0;
        };

    private:
        list<Frame> frames;
        Frame* current = 0;
        int ID = 0;
        int history = 100;
        bool active = true;
        map<unsigned long, int> threadIDs;

        OSG::VRMutex mutex;

        VRProfiler();

    public:
        static VRProfiler* get();

        void setActive(bool b);
        bool isActive();

        int regStart(string name);
        void regStop(int ID);

        list<Frame> getFrames();
        Frame getFrame(int i);

        void setHistoryLength(int N);
        int getHistoryLength();

        void swap();
};

#endif // VRPROFILER_H_INCLUDED
