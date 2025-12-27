#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "GUI/macos_window.h"
#include "GUI/macos_view.h"

#include "Grain.hpp"
#include "App/App.hpp"
#include "GUI/Window.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Screen.hpp"
#include "2d/Rect.hpp"


namespace Grain {

    void _macosWindow_init(
            Window* window,
            const char* title,
            const Rectd& rect,
            Window::Style style,
            Screen* screen) {

        auto style_bits = static_cast<uint32_t>(style);
        BOOL ns_defer_flag = NO;
        NSScreen* ns_screen = screen ? (__bridge NSScreen*)screen->nsScreen() : nil;
        NSUInteger ns_style_mask = NSWindowStyleMaskBorderless;

        if (style_bits & static_cast<uint32_t>(Window::Style::Titled)) {
            ns_style_mask |= NSWindowStyleMaskTitled;
        }
        if (style_bits &  static_cast<uint32_t>(Window::Style::Closable)) {
            ns_style_mask |= NSWindowStyleMaskClosable;
        }
        if (style_bits & static_cast<uint32_t>(Window::Style::Miniaturizable)) {
            ns_style_mask |= NSWindowStyleMaskMiniaturizable;
        }
        if (style_bits & static_cast<uint32_t>(Window::Style::Resizable)) {
            ns_style_mask |= NSWindowStyleMaskResizable;
        }
        if (style_bits & static_cast<uint32_t>(Window::Style::UnifiedTitleAndToolbar)) {
            ns_style_mask |= NSWindowStyleMaskUnifiedTitleAndToolbar;
        }
        if (style_bits & static_cast<uint32_t>(Window::Style::FullScreen)) {
            ns_style_mask |= NSWindowStyleMaskFullScreen;
        }
        if (style_bits & static_cast<uint32_t>(Window::Style::FullSizeContentView)) {
            ns_style_mask |= NSWindowStyleMaskFullSizeContentView;
        }


        GrainNSWindow* ns_window = [[GrainNSWindow alloc]
                initWithContentRect:NSMakeRect(rect.x_, rect.y_, rect.width_, rect.height_)
                styleMask:ns_style_mask
                backing:NSBackingStoreBuffered
                defer:ns_defer_flag
                window:window
                screen:ns_screen];

        [ns_window setTitle:[NSString stringWithUTF8String:title]];
        [ns_window setAutorecalculatesKeyViewLoop:NO];

        window->setNSWindow((__bridge void*)ns_window);
        window->setTitle(title);


        // Root View
        Rectd bounds_rect = window->boundsRect();
        auto view = new (std::nothrow) View(bounds_rect);
        if (view) {
            window->_setRootView(view);
            auto ns_view = (GrainNSView*)view->nsView();
            [ns_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
            [ns_window setContentView:ns_view];
        }
    }

    void _macosWindow_setFrame(const Window* window, const Rectd& rect) {
        NSRect frame = NSMakeRect(rect.x_, rect.y_, rect.width_, rect.height_);
        [(GrainNSWindow*)window->nsWindow() setFrame:frame display:YES];
    }

    void _macosWindow_setTitle(const Window* window, const String& title) {
        [(GrainNSWindow*)window->nsWindow() setTitle:[NSString stringWithUTF8String:title.utf8()]];
    }

    void _macosWindow_release(Window* window) {
        [(NSWindow*)window->nsWindow() release];
    }

    Rectd _macosWindow_bounds(const Window* window) {
        NSRect ns_bounds = [[(NSWindow*)window->nsWindow() contentView] bounds];
        return Rectd(ns_bounds.origin.x, ns_bounds.origin.y, ns_bounds.size.width, ns_bounds.size.height);
    }

    void _macosWindow_orderFront(Window* window) {
        [(NSWindow*)window->nsWindow() orderFront:nil];
    }

    void _macosWindow_makeKeyWindow(Window* window) {
        [(NSWindow*)window->nsWindow() makeKeyWindow];
    }

    void _macosWindow_makeKeyAndOrderFront(Window* window) {
        [(NSWindow*)window->nsWindow() makeKeyAndOrderFront:nil];
    }

    void _macosWindow_setFirstResponder(Window* window, Component* component) {
        [(NSWindow*)window->nsWindow() setInitialFirstResponder:(NSView*)component->nsView()];
    }

    void _macosWindow_makeFirstResponder(Window* window, Component* component) {
        if (component) {
            [(GrainNSWindow*)window->nsWindow() makeFirstResponder:(NSView*)component->nsView()];
        }
        else {
            [(GrainNSWindow*)window->nsWindow() makeFirstResponder:nil];
        }
    }

    void _macosWindow_close(Window* window) {
        [(NSWindow*)window->nsWindow() close];
    }
}


@implementation GrainNSWindow


- (id)initWithContentRect:(NSRect)content_rect styleMask:(NSUInteger)a_style backing:(NSBackingStoreType)buffering_type defer:(BOOL)flag window:(Grain::Window*)window screen:(NSScreen*)screen {

    GrainNSWindow* ns_window = [super initWithContentRect:content_rect
                                                styleMask:a_style
                                                  backing:buffering_type
                                                    defer:flag screen:screen];

    [ns_window setDelegate:(id)ns_window];
    [ns_window setReleasedWhenClosed:FALSE];
    [ns_window setCollectionBehavior:NSWindowCollectionBehaviorParticipatesInCycle];

    m_window = window;

    return (id)ns_window;
}


- (void)dealloc {

    [super dealloc];
}


- (BOOL)canBecomeKeyWindow {
    return m_window->canBecomeKeyWindow();
}


- (void)becomeKeyWindow {
   // TODO: Possibe optimization, only draw Component, which lost the focus!
    // TODO: Not all windows must be redraw!

    Grain::App::setKeyWindow(m_window);
    m_window->becomeKeyWindow();

    Grain::App::allWindowsNeedsDisplay();
}


- (void)windowDidResize:(NSNotification*)notification {
    double width = ((NSView*)self.contentView).frame.size.width;
    double height = ((NSView*)self.contentView).frame.size.height;
    Grain::View* root_view = m_window->rootView();
    if (root_view) {
        root_view->setRect(Grain::Rectd(0, 0, width, height));
    }
}


- (void)windowDidBecomeMain:(NSNotification*)notification {
    // TODO: Your code to handle the window becoming the main window.
}


- (void)windowDidResignMain:(NSNotification*)notification {

    // TODO: Your code to handle the window resigning as the main window.
}


- (void)windowWillClose:(NSNotification*)notification {

    // TODO: Your code to handle window closing.
}


- (void)windowDidMove:(NSNotification*)notification {

    // TODO: Your code to handle window resizing.
}


- (void)windowDidChangeScreen:(NSNotification*)notification {

    // TODO: Your code to handle window changing screen.
}


- (void)windowDidMiniaturize:(NSNotification*)notification {

    // TODO: Your code to handle window minimization.
}


- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize {

    // TODO: Your code to handle window resizing dynamically.
    // You can modify the frameSize if needed.

    return frameSize;
}


- (void)windowDidEndLiveResize:(NSNotification*)notification {

    // TODO: Your code to handle the completion of a live resize.

}


- (Grain::Window*)window {

    return m_window;
}


@end