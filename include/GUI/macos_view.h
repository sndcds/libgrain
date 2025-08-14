#ifndef GrainNSView_h
#define GrainNSView_h

#import <Cocoa/Cocoa.h>
#include "GUI/Component.hpp"

@interface GrainNSView : NSView <NSDraggingDestination> {

@protected
    Grain::Component* m_component;
    NSTrackingArea* m_tracking_area;
    bool m_remember_accepting_mouse_events;
}

- (id)initForUI:(Grain::Component*)component rect:(Grain::Rectd)rect;
- (void)handleEvent:(NSEvent*)ns_event;

- (Grain::Component*)component;

@end

#endif // GrainNSView_h
