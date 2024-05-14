#include "VRCocoaWindow.h"

#include <OpenSG/OSGCocoaWindow.h>
#import <Cocoa/Cocoa.h>

// This prevents warnings that "NSApplication might not
// respond to setAppleMenu" on OS X 10.4
@interface NSApplication(OpenSG)
- (void)setAppleMenu:(NSMenu *)menu;
@end

using namespace OSG;

VRCocoaWindow::VRCocoaWindow() { init(); }
VRCocoaWindow::~VRCocoaWindow() { cleanup(); }

VRCocoaWindowPtr VRCocoaWindow::create() { return VRCocoaWindowPtr( new VRCocoaWindow() ); }
VRCocoaWindowPtr VRCocoaWindow::ptr() { return static_pointer_cast<VRCocoaWindow>(shared_from_this()); }

void VRCocoaWindow::save(XMLElementPtr node) { VRWindow::save(node); }
void VRCocoaWindow::load(XMLElementPtr node) { VRWindow::load(node); }

void VRCocoaWindow::onDisplay() {}
void VRCocoaWindow::onMouse(int b, int s, int x, int y) {}
void VRCocoaWindow::onMotion(int x, int y) {}
void VRCocoaWindow::onKeyboard(int k, int s, int x, int y) {}
void VRCocoaWindow::onKeyboard_special(int k, int s, int x, int y) {}

CocoaWindowUnrecPtr cwin;
VRCocoaWindow* vrCocoaWin = 0;


@interface MyOpenGLView: NSOpenGLView
{
}
- (BOOL) acceptsFirstResponder;

- (void) handleMouseEvent: (NSEvent*) event;

- (void) mouseDown: (NSEvent*) event;
- (void) mouseDragged: (NSEvent*) event;
- (void) mouseUp: (NSEvent*) event;
- (void) rightMouseDown: (NSEvent*) event;
- (void) rightMouseDragged: (NSEvent*) event;
- (void) rightMouseUp: (NSEvent*) event;
- (void) otherMouseDown: (NSEvent*) event;
- (void) otherMouseDragged: (NSEvent*) event;
- (void) otherMouseUp: (NSEvent*) event;

- (void) keyDown: (NSEvent*) event;

- (void) reshape;
- (void) drawRect: (NSRect) bounds;
@end

@implementation MyOpenGLView

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) handleMouseEvent: (NSEvent*) event
{
    Real32 a,b,c,d;

    int buttonNumber = [event buttonNumber];
    unsigned int modifierFlags = [event modifierFlags];

    // Traditionally, Apple mice just have one button. It is common practice to simulate
    // the middle and the right button by pressing the option or the control key.
    if (buttonNumber == 0)
    {
        if (modifierFlags & NSAlternateKeyMask)
            buttonNumber = 2;
        if (modifierFlags & NSControlKeyMask)
            buttonNumber = 1;
    }

    NSPoint location = [event locationInWindow];

    switch ([event type])
    {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
        //onMouseDown(buttonNumber, location.x, location.y); // TODO
        break;

    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
        //onMouseUp(buttonNumber); // TODO
        break;

    case NSLeftMouseDragged:
    case NSRightMouseDragged:
    case NSOtherMouseDragged:
        //onMouseDragged(buttonNumber, location.x, location.y); // TODO
        break;

    default:
        break;
    }
}

- (void) mouseDown: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) mouseDragged: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) mouseUp: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) rightMouseDown: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) rightMouseDragged: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) rightMouseUp: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) otherMouseDown: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) otherMouseDragged: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) otherMouseUp: (NSEvent*) event
{
    [self handleMouseEvent: event];
}

- (void) keyDown: (NSEvent*) event
{
    if ([[event characters] length] != 1) return;
    //onKeyDown( [[event characters] characterAtIndex: 0] ); // TODO
}

- (void) reshape
{
    [self update];
    NSWindow *window = [self window];
    NSRect frame = [self bounds];
    float scaleFactor = [window backingScaleFactor];
    int W = static_cast<int>(frame.size.width*scaleFactor);
    int H = static_cast<int>(frame.size.height*scaleFactor);
    cwin->resize( W, H );
}

- (void) drawRect: (NSRect) bounds
{
    //redraw();
}

@end

@interface MyDelegate : NSObject

{
    NSWindow *window;
    MyOpenGLView *glView;
}

- (void) applicationWillFinishLaunching: (NSNotification*) notification;

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*) application;

@end


@implementation MyDelegate

- (void) dealloc
{
    [window release];
    [super dealloc];
}

- (void) applicationWillFinishLaunching: (NSNotification*) notification
{
    window = [NSWindow alloc];
    NSRect rect = { { 0, 0 }, { 300, 600 } };
    [window initWithContentRect: rect styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask) backing: NSBackingStoreBuffered defer: YES];
    [window setTitle: @"PolyVR"];
    [window setReleasedWhenClosed: NO];

    glView = [[MyOpenGLView alloc] autorelease];
    [glView initWithFrame: rect];
    [glView setAutoresizingMask: NSViewMaxXMargin | NSViewWidthSizable | NSViewMaxYMargin | NSViewHeightSizable];
    [[window contentView] addSubview: glView];

    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
        NSOpenGLPixelFormatAttribute(0)
    };
    NSOpenGLPixelFormat *pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
    [glView setPixelFormat: pixFmt];

    // Create OpenSG window
    cwin->setContext ( [glView openGLContext] );
    cwin->init();
    cwin->resize( 300, 300 );

    cwin->activate();

    // do some OpenGL init. Will move into State Chunks later.

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

	// Show the window
    [window makeKeyAndOrderFront: nil];
    [window makeFirstResponder:glView];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*) application
{
    return YES;
}

@end

NSAutoreleasePool *pool;
MyDelegate *delegate;

void VRCocoaWindow::init() {
    vrCocoaWin = this;
    cwin = CocoaWindow::create();

    // A magic method that allows applications to react to events even
    // when they are not organized in a bundle
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcess(&psn);

    // Create application
    [NSApplication sharedApplication];
    pool = [[NSAutoreleasePool alloc] init];
    delegate =  [[MyDelegate new] autorelease];
    [NSApp setDelegate: delegate];
    [delegate applicationWillFinishLaunching:nil];

    _win = cwin;
}

void VRCocoaWindow::cleanup() {
    cwin = 0;
    [pool release];
}

void VRCocoaWindow::render(bool fromThread) {
  if (fromThread) return;

  NSEvent* event = 0;
  do {
      event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
      [NSApp sendEvent: event];
  } while(event != nil);

  VRWindow::render();
}