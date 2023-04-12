#ifndef VRGLUTEDITOR_H_INCLUDED
#define VRGLUTEDITOR_H_INCLUDED


#include "VRWindow.h"
#include <OpenSG/OSGGLUTWindow.h>

OSG_BEGIN_NAMESPACE;
using namespace std;


class VRGlutEditor: public VRWindow {
    private:
        GLUTWindowMTRecPtr win;
        int topWin = -1;
        int winGL = -1;
        int winUI = -1;

        typedef function<void(string, map<string, string>)> Signal;
        typedef function<void(string, int, int, int, int)> ResizeSignal;
        Signal signal;
        ResizeSignal resizeSignal;

    public:
        VRGlutEditor();
        ~VRGlutEditor();

        static VRGlutEditorPtr create();
        VRGlutEditorPtr ptr();

        static void initGlut();

        void render(bool fromThread = false) override;

        void save(XMLElementPtr node) override;
        void load(XMLElementPtr node) override;

        void onMouse(int b, int s, int x, int y);
        void onMotion(int x, int y);
        void onKeyboard(int k, int s, int x, int y);
        void onKeyboard_special(int k, int s, int x, int y);

        void resizeGLWindow(int x, int y, int w, int h);
        void on_resize_window(int w, int h);
        void on_ui_display();
        void on_gl_display();
        void on_ui_resize(int w, int h);
        void on_gl_resize(int w, int h);
};

OSG_END_NAMESPACE;

#endif // VRGLUTEDITOR_H_INCLUDED