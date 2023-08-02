#include "VRGlutExtensions.h"
#include "core/gui/VRGuiManager.h"
#include "core/utils/system/VRSystem.h"
#include "core/objects/material/VRTexture.h"

#include <GL/glx.h>
#include <thread>

Display* xdisplay = 0;
XID xwindow = 0;
static bool doGrabShiftTab = true;
string backend;

void initGlutExtensions() {
    string XDG_SESSION_TYPE = getSystemVariable("XDG_SESSION_TYPE");
    string DISPLAY = getSystemVariable("DISPLAY");
    string WAYLAND_DISPLAY = getSystemVariable("WAYLAND_DISPLAY");

    if (WAYLAND_DISPLAY != "" || XDG_SESSION_TYPE == "wayland") backend = "wayland";
    else if (DISPLAY != "" || XDG_SESSION_TYPE == "x11") backend = "x11";

    // TODO: use backend variable to guard x11 calls

    xdisplay = glXGetCurrentDisplay();
    xwindow = glXGetCurrentDrawable();
}

void cleanupGlutExtensions() {
    doGrabShiftTab = false;
}

void set_x11_icon_list(IconList& iconList) {

    Atom net_wm_icon = XInternAtom(xdisplay, "_NET_WM_ICON", False);
    Atom cardinal = XInternAtom(xdisplay, "CARDINAL", False);

    XChangeProperty (xdisplay, xwindow, net_wm_icon, cardinal, 32, PropModeReplace, (uint8_t*) iconList.data, iconList.size);
}






Icon::Icon(int w, int h) : w(w), h(h) {
    data = (uint64_t*)malloc ((w*h) * sizeof(uint64_t));
}

uint64_t* IconList::add(int w, int h) {
    images.push_back(Icon(w,h));
    return images[images.size()-1].data;
}

void IconList::compile() {
    size = 0;
    for (auto& img : images) size += img.w*img.h + 2;
    if (data) free(data);
    data = (uint64_t*)malloc (size * sizeof(uint64_t));

    int k=0;
    for (auto& img : images) {
        data[k+0] = img.w;
        data[k+1] = img.h;
        for (int i=0; i<img.w*img.h; i++) data[k+i+2] = img.data[i];
        k += img.w*img.h+2;
    }
}


void IconList::load(string path) {
    auto img = OSG::VRTexture::create();
    if (img->read(path)) {
        auto size = img->getSize();
        auto pixels = img->getPixels(true);
        uint64_t* p = add(size[0], size[1]);
        for (auto& pi : pixels) {
            uint8_t r = pi[0] * 255.0;
            uint8_t g = pi[1] * 255.0;
            uint8_t b = pi[2] * 255.0;
            uint8_t a = pi[3] * 255.0;

            *p++ = a << 24 | r << 16 | g << 8 | b ;
        }
    } else cout << "Warning! could not load icon " << path << endl;
}

void IconList::addTest() {
    int width = 32;
    int height = 32;
    uint64_t* p = add(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t r = x*255.0/(width-1);
            uint8_t g = y*255.0/(height-1);
            uint8_t b = 0;
            uint8_t a = 255;

            *p++ = a << 24 | r << 16 | g << 8 | b ;
        }
    }
}

void IconList::apply() {
    compile();
    if (backend == "x11") set_x11_icon_list(*this);
    //else // TODO, implement wayland here
}

void listenForKey(XID grab_window) { // TODO: add windows and wayland versions
    Display* dpy = XOpenDisplay(0);
    unsigned int modifiers1 = ShiftMask;
    unsigned int modifiers2 = ShiftMask | Mod2Mask;
    KeyCode keycode = XKeysymToKeycode(dpy, XK_Tab);

    XGrabKey(dpy, keycode, modifiers1, grab_window, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers2, grab_window, False, GrabModeAsync, GrabModeAsync);
    XSelectInput(dpy, grab_window, KeyPressMask | KeyReleaseMask);
    XEvent ev;
    while(doGrabShiftTab) {
         XNextEvent(dpy, &ev);
         if (ev.type == KeyPress) uiSignal("shiftTab", {{"state","1"}});
         if (ev.type == KeyRelease) uiSignal("shiftTab", {{"state","0"}});
    }
    XUngrabKey(dpy, keycode, modifiers1, grab_window);
    XUngrabKey(dpy, keycode, modifiers2, grab_window);
    XCloseDisplay(dpy);
}

void startGrabShiftTab() {
    static thread listener(listenForKey, xwindow);
}