#ifndef GrainNSWindow_h
#define GrainNSWindow_h

#import <Cocoa/Cocoa.h>
#include "GUI/Window.hpp"

@interface GrainNSWindow : NSWindow {

    @protected
        Grain::Window* m_window;
}

- (id)initWithContentRect:
        (NSRect)content_rect
        styleMask:(NSUInteger)a_style
        backing:(NSBackingStoreType)buffering_type
        defer:(BOOL)flag
        window:(Grain::Window*)window
        screen:(NSScreen*)screen;

- (Grain::Window*)window;

@end

#endif // GrainNSWindow_h
