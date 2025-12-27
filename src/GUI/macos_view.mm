#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "GUI/macos_view.h"
#include "GUI/macos_window.h"

#include "Grain.hpp"
#include "App/App.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Components/Component.hpp"
#include "GUI/Event.hpp"
#include "2d/Rect.hpp"
#include "String/StringList.hpp"
#include "Graphic/AppleCGContext.hpp"


namespace Grain {

    void _macosEvent_set(Event* event, Component* component, const NSView* ns_view, const NSEvent* ns_event);


    void _macosView_initForUI(Component* component, const Rectd& rect) {
        component->setNSView([[GrainNSView alloc] initForUI:component rect:rect]);
    }

    void _macosView_releaseView(Component* component) {
        [(NSView*)component->nsView() removeFromSuperview];
        [(NSView*)component->nsView() release];
    }

    void _macosView_addComponent(Component* parent, Component* component, Component::AddFlags flags) {
        auto parent_ns_view = (NSView*)parent->nsView();
        auto component_ns_view = (NSView*)component->nsView();
        component_ns_view.wantsLayer = (flags & Component::AddFlags::kWantsLayer) ? YES : NO;
        [parent_ns_view addSubview:component_ns_view];
    }

    void _macosView_removeFromSuperview(Component* component) {
        [(NSView*)component->nsView() removeFromSuperview];
    }

    void _macosView_setNeedsDisplay(const Component* component) {
        const auto& rect = component->boundsRect();
        NSRect ns_rect = NSMakeRect(rect.x_, rect.y_, rect.width_, rect.height_);
        [(NSView*)component->nsView() setNeedsDisplayInRect:ns_rect];
        // [(NSView*)component->nsView() setNeedsDisplay:YES];
    }

    void _macosView_forcedDisplay(const Component* component) {
        const auto& rect = component->boundsRect();
        NSRect ns_rect = NSMakeRect(rect.x_, rect.y_, rect.width_, rect.height_);
        [(NSView*)component->nsView() setNeedsDisplayInRect:ns_rect];
        [(NSView*)component->nsView() displayIfNeeded];
    }

    void _macosView_selectNextKeyView(Component* component) {
        if (!component->isEnabled() && component->isKeyComponent()) {
            auto ns_window = (NSWindow*)[(GrainNSView*)component->nsView() window];
            [ns_window selectNextKeyView:(NSView*)component->nsView()];
        }
    }

    void _macosView_interpretKeyEvent(Component* component, const Event& event) {
        [(GrainNSView*)component->nsView() interpretKeyEvents:[NSArray arrayWithObject:(NSEvent*)(event.nsEvent())]];
    }

    void _macosView_setOpacity(Component* component, float opacity) {
        [(NSView*)component->nsView() setAlphaValue:opacity];
    }

    void _macosView_setHidden(Component* component, bool hidden) {
        [(NSView*)component->nsView() setHidden:hidden];
    }

    bool _macosView_isKeyView(const Component* component) {
        auto ns_view = (GrainNSView*)component->nsView();
        auto ns_window = (GrainNSWindow*)[ns_view window];
        auto window = [ns_window window];
        return [ns_window firstResponder] == ns_view && window->isKeyWindow();
    }

    bool _macosView_gotoView(Component* component) {
        if (component) {
            BOOL success = NO;
            auto key_window = App::keyWindow();
            if (key_window) {
                success = [(GrainNSWindow*)key_window->nsWindow() makeFirstResponder:(GrainNSView*)component->nsView()];
            }
            return success;
        }
        return false;
    }

    void _macosView_setFrame(Component* component, Rectd& rect) {
        NSRect frame = NSMakeRect(rect.x_, rect.y_, rect.width_, rect.height_);
        [(NSView*)component->nsView() setFrame:frame];
    }

    void _macosView_setFrameOrigin(Component* component, double x, double y) {
        [(NSView*)component->nsView() setFrameOrigin:NSMakePoint(x, y)];
    }

    void _macosView_setFrameSize(Component* component, double width, double height) {
        [(NSView*)component->nsView() setFrameSize:NSMakeSize(width, height)];
    }

    void _macosView_setContextByComponent(AppleCGContext* gc, Component* component) {
        gc->setCGContextByComponent([[NSGraphicsContext currentContext] CGContext], component);
    }

    void _macosView_updateCGContext(Component* component) noexcept {
        if (!component) return;

        dispatch_async(dispatch_get_main_queue(), ^{
            auto ns_view = static_cast<NSView*>(component->nsView());
            if (!ns_view) return;

            CGContextRef cgContext = [[NSGraphicsContext currentContext] CGContext];
            if (!cgContext) return;

            auto agc = dynamic_cast<AppleCGContext*>(component->graphicContextPtr());
            if (agc) {
                agc->setCGContext(cgContext);
            }
        });
    }
}


@implementation GrainNSView

- (id)initForUI:(Grain::Component*)component rect:(Grain::Rectd)rect {
    m_component = component;
    self = [super initWithFrame: NSMakeRect(rect.x_, rect.y_, rect.width_, rect.height_)];
    [self setClipsToBounds: TRUE];

    return self;
}


- (void)dealloc {
    NSArray *tracking_areas = [self trackingAreas];
    for (NSTrackingArea *a in tracking_areas) {
        [self removeTrackingArea:a];
    }

    [super dealloc];
}


- (Grain::Component*)component {
    return m_component;
}


- (BOOL)acceptsFirstMouse:(NSEvent*)ns_event { return m_component->acceptsFirstMouse(); }
- (BOOL)acceptsFirstResponder { return m_component->canGetFocus(); }
- (BOOL)canBecomeKeyView { return m_component->canGetFocus(); }
- (BOOL)isFlipped { return m_component->isFlippedView(); }
- (BOOL)isOpaque { return m_component->isOpaque(); }


- (void)cursorUpdate:(NSEvent*)ns_event {
/*
TODO:!!!
{
    static int n = 0;
    std::cout << "NSEventTypeCursorUpdate: " << n << std::endl;
    n++;
}

static int i = 0;

switch (i) {
    case 0: [[NSCursor arrowCursor] set]; break;
    case 1: [[NSCursor IBeamCursor] set]; break;
    case 2: [[NSCursor pointingHandCursor] set]; break;
    case 3: [[NSCursor closedHandCursor] set]; break;
    case 4: [[NSCursor openHandCursor] set]; break;
    case 5: [[NSCursor resizeLeftCursor] set]; break;
    case 6: [[NSCursor resizeRightCursor] set]; break;
    case 7: [[NSCursor resizeUpCursor] set]; break;
    case 8: [[NSCursor resizeDownCursor] set]; break;
    case 9: [[NSCursor resizeUpDownCursor] set]; break;
    case 10: [[NSCursor crosshairCursor] set]; break;
}
i++;
if (i > 10) {
    i = 0;
}
*/
}


- (void)resetCursorRects {
/*
TODO:!!!
static int i = 0;

// [super resetCursorRects];

auto rect = NSMakeRect(0, 0, 2000, 2000);
switch (i) {
    case 0: [self addCursorRect:rect cursor:[NSCursor arrowCursor]]; break;
    case 1: [self addCursorRect:rect cursor:[NSCursor IBeamCursor]]; break;
    case 2: [self addCursorRect:rect cursor:[NSCursor pointingHandCursor]]; break;
    case 3: [self addCursorRect:rect cursor:[NSCursor closedHandCursor]]; break;
    case 4: [self addCursorRect:rect cursor:[NSCursor openHandCursor]]; break;
    case 5: [self addCursorRect:rect cursor:[NSCursor resizeLeftCursor]]; break;
    case 6: [self addCursorRect:rect cursor:[NSCursor resizeRightCursor]]; break;
    case 7: [self addCursorRect:rect cursor:[NSCursor resizeUpCursor]]; break;
    case 8: [self addCursorRect:rect cursor:[NSCursor resizeDownCursor]]; break;
    case 9: [self addCursorRect:rect cursor:[NSCursor resizeUpDownCursor]]; break;
    case 10: [self addCursorRect:rect cursor:[NSCursor crosshairCursor]]; break;
}
i++;
if (i > 10) {
    i = 0;
}
*/
}


- (void) viewWillMoveToWindow:(NSWindow*)new_window {
    [self updateTrackingAreas];
}


- (void)resizeWithOldSuperviewSize:(NSSize)old_size {
    [super resizeWithOldSuperviewSize:old_size];
    [self setFrame:self.superview.bounds];
    Grain::Rectd rect(self.superview.bounds.origin.x, self.superview.bounds.origin.y, self.superview.bounds.size.width, self.superview.bounds.size.height);
    m_component->setRect(rect);
}


- (BOOL)becomeFirstResponder {
    BOOL result = [super becomeFirstResponder];

    if (result == YES) {
        m_component->setFocusFlag(true);
        m_component->becomeFirstResponder();
    }

    return result;
}


- (BOOL)canResignFirstResponder {
    return YES;
}


- (BOOL)resignFirstResponder {
    [super resignFirstResponder];
    m_component->setFocusFlag(false);

    if (m_component->isKeyComponent()) {
        m_component->resignFirstResponder();
        return YES;
    }

    return NO;
}


- (void)insertText:(id)string {
    m_component->insertText([string UTF8String]);
}


- (void)drawRect:(NSRect)dirty_rect {
    // [super drawRect:dirty_rect]; // TODO: Check!
    if (m_component) {
        Grain::Rectd rect(dirty_rect);
        m_component->updateBeforeDrawing(rect);

        if (m_component->isRectUsable()) {
            m_component->draw(rect);
        }

        /** TODO: Debug!
        if (m_component->canHaveChildren()) {
            NSRect trackingRect = [m_tracking_area rect];
            [[NSColor redColor] setStroke];
            NSBezierPath *path = [NSBezierPath bezierPathWithRect:trackingRect];
            [path setLineWidth:2.0];
            [path stroke];
        }
         */
    }
}


- (void)mouseEntered:(NSEvent*)ns_event {
    if (m_component->isHandlingMouseMoved()) {
        m_remember_accepting_mouse_events = [[self window] acceptsMouseMovedEvents];
        [[self window] setAcceptsMouseMovedEvents:YES];
    }

    [self handleEvent: ns_event];
}


- (void)mouseExited:(NSEvent*)ns_event {
    if (m_component->isHandlingMouseMoved()) {
        [[self window] setAcceptsMouseMovedEvents:m_remember_accepting_mouse_events];
    }

    [self handleEvent: ns_event];
}


- (void)mouseDown:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)mouseDragged:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)mouseUp:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)rightMouseDown:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)rightMouseDragged:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)rightMouseUp:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)mouseMoved:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)scrollWheel:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)magnifyWithEvent:(NSEvent*)ns_event { [self handleEvent: ns_event]; }
- (void)rotateWithEvent:(NSEvent*)ns_event { [self handleEvent: ns_event]; }


- (void)handleEvent:(NSEvent*)ns_event {
    Grain::Event event;
    _macosEvent_set(&event, m_component, self, ns_event);
    if (event.shouldBeHandled()) {
        m_component->handleEvent(event);
    }
}


- (void)keyDown:(NSEvent*)ns_event {
    NSString *key_characters = [ns_event charactersIgnoringModifiers];
    auto key_char_count = static_cast<int32_t>([key_characters length]);

    if (key_char_count > 0) {
        auto key_char = static_cast<int32_t>([key_characters characterAtIndex:0]);

        auto ns_window = (GrainNSWindow*)[self window];
        Grain::Window *window = [ns_window window];

        if (window) {
            if (key_char_count == 1) {
                switch (key_char) {
                    case NSTabCharacter:
                        m_component->gotoNextKeyComponent();
                        return;
                    case NSBackTabCharacter:
                        m_component->gotoPreviousKeyComponent();
                        return;
                }
            }
        }
    }

    [self handleEvent: ns_event];
}


- (void)updateTrackingAreas {
    // TODO: Maybe this is not beeing called correct?
    [super updateTrackingAreas];

    if (m_tracking_area != nil) {
        [self removeTrackingArea:m_tracking_area];
        m_tracking_area = nil;
    }

    NSTrackingAreaOptions tracking_area_options =
            NSTrackingInVisibleRect |
            NSTrackingCursorUpdate |
            NSTrackingMouseEnteredAndExited |
            NSTrackingMouseMoved |
            NSTrackingActiveAlways;

    m_tracking_area = [[NSTrackingArea alloc] initWithRect:self.bounds options:tracking_area_options owner:self userInfo:nil];
    [self addTrackingArea:m_tracking_area];
}


- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
    if (m_component->isEnabled()) {
        m_component->setDragEntered(true);
        m_component->handleDraggingEntered();
        return NSDragOperationCopy;
    }
    else {
        return NSDragOperationNone;
    }
}


- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender {
    if (m_component->isEnabled()) {
        m_component->handleDraggingUpdated();
        return NSDragOperationCopy;
    }
    else {
        return NSDragOperationNone;
    }
}


- (void)draggingExited:(id <NSDraggingInfo>)sender {
    if (m_component->isEnabled()) {
        m_component->setDragEntered(false);
        m_component->handleDraggingExited();
    }
}


- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
    m_component->setDragEntered(false);
    m_component->needsDisplay();
    return YES;
}


- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
    if ([sender draggingSource] != self) {

        // NSDragOperation source_drag_mask = [sender draggingSourceOperationMask];    TODO: ?
        NSPasteboard *pasteboard = [sender draggingPasteboard];

        if ([[pasteboard types] containsObject:NSPasteboardTypeFileURL]) {

            NSArray<Class> *classes = @[[NSURL class]];
            NSDictionary *options = @{};
            NSArray<NSURL*> *urls = [pasteboard readObjectsForClasses:classes options:options];
            auto n = static_cast<int32_t>([urls count]);
            if (n > 0) {

                Grain::StringList result_file_paths(n);
                for (int32_t i = 0; i < n; i++) {
                    result_file_paths.pushString([[[urls objectAtIndex:i] path] UTF8String]);
                }

                m_component->filesDropped(result_file_paths);
            }

            return YES;
        }
    }

    return NO;
}


@end