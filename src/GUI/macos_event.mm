#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "GUI/macos_event.h"
#include "GUI/Event.hpp"
#include "App/App.hpp"


namespace Grain {

    void _macosEvent_release(Event* event) {
        [(NSEvent*)event->nsEvent() release];
    }

    void _macosEvent_set(Event* event, Component* component, const NSView* ns_view, const NSEvent* ns_event) {

        Event::EventType event_type = Event::EventType::Undefined;

        event->_setNSEvent((void*)ns_event);
        [ns_event retain];

        Event::g_previous_component = Event::g_component;
        Event::g_component = component;

        switch ([ns_event type]) {
            case NSEventTypeLeftMouseDown: event_type = Event::EventType::MouseDown; break;
            case NSEventTypeLeftMouseUp: event_type = Event::EventType::MouseUp; break;
            case NSEventTypeRightMouseDown: event_type = Event::EventType::RightMouseDown; break;
            case NSEventTypeRightMouseUp: event_type = Event::EventType::RightMouseUp; break;
            case NSEventTypeMouseMoved: event_type = Event::EventType::MouseMoved; break;
            case NSEventTypeLeftMouseDragged: event_type = Event::EventType::MouseDrag; break;
            case NSEventTypeRightMouseDragged: event_type = Event::EventType::RightMouseDrag; break;
            case NSEventTypeMouseEntered: event_type = Event::EventType::MouseEntered; break;
            case NSEventTypeMouseExited: event_type = Event::EventType::MouseExited; break;
            case NSEventTypeKeyDown: event_type = Event::EventType::KeyDown; break;
            case NSEventTypeScrollWheel: event_type = Event::EventType::ScrollWheel; break;
            case NSEventTypeMagnify: event_type = Event::EventType::Magnification; break;
            case NSEventTypeSmartMagnify: event_type = Event::EventType::SmartMagnification; break;
            case NSEventTypeRotate: event_type = Event::EventType::Rotation; break;

            // TODO: ...
            case NSEventTypeKeyUp:
            case NSEventTypeFlagsChanged:
            case NSEventTypeAppKitDefined:
            case NSEventTypeSystemDefined:
            case NSEventTypeApplicationDefined:
            case NSEventTypePeriodic:
            case NSEventTypeCursorUpdate: {
                static int n = 0;
                std::cout << "NSEventTypeCursorUpdate: " << n << std::endl; // TODO: !!!
                n++;
                break;
            }

            case NSEventTypeTabletPoint:
            case NSEventTypeTabletProximity:
            case NSEventTypeOtherMouseDown:
            case NSEventTypeOtherMouseUp:
            case NSEventTypeOtherMouseDragged:
            case NSEventTypeGesture:

            case NSEventTypeBeginGesture:
            case NSEventTypeEndGesture:
            case NSEventTypeQuickLook:
            case NSEventTypePressure:

            default:
                break;
        }

        event->_setType(event_type);

        NSPoint point = [ns_view convertPoint:[ns_event locationInWindow] fromView:nil];
        event->setMousePos(Vec2d(point.x, point.y));

        uint32_t key_mask = 0x0;
        NSEventModifierFlags modifierFlags = [ns_event modifierFlags];
        if (modifierFlags & NSEventModifierFlagCapsLock) key_mask |= Event::KeyMask_CapsLock;
        if (modifierFlags & NSEventModifierFlagShift) key_mask |= Event::KeyMask_Shift;
        if (modifierFlags & NSEventModifierFlagControl) key_mask |= Event::KeyMask_Control;
        if (modifierFlags & NSEventModifierFlagOption) key_mask |= Event::KeyMask_Alternate;
        if (modifierFlags & NSEventModifierFlagCommand) key_mask |= Event::KeyMask_Command;
        if (modifierFlags & NSEventModifierFlagNumericPad) key_mask |= Event::KeyMask_NumericPad;
        if (modifierFlags & NSEventModifierFlagHelp) key_mask |= Event::KeyMask_Help;
        if (modifierFlags & NSEventModifierFlagFunction) key_mask |= Event::KeyMask_Function;
        event->setKeyMask(key_mask);


        bool ignore = false;
        bool double_clicked = false;

        switch (event_type) {

            case Event::EventType::MouseDown:
                if (!Event::g_mouse_pressed && !Event::g_right_mouse_pressed) {
                    Event::g_mouse_pressed = true;
                    Event::g_mouse_down_pos = event->mousePos();
                    Event::g_mouse_drag_count = 0;

                    // Recognize double clicks
                    Timestamp t = Event::g_ts_last_mouse_click;
                    Event::g_ts_last_mouse_click.now();

                    if (Event::g_component == Event::g_previous_component &&
                        (Event::g_ts_last_mouse_click - t).milliseconds() < App::doubleClickMillis()) {
                        double_clicked = true;
                    }
                    else {
                        double_clicked = false;
                    }
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::RightMouseDown:
                if (Event::g_mouse_pressed == false && Event::g_right_mouse_pressed == false) {
                    Event::g_right_mouse_pressed = true;
                    Event::g_mouse_down_pos = event->mousePos();
                    Event::g_mouse_drag_count = 0;
                    double_clicked = false;
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::MouseDrag:
                if (Event::g_mouse_pressed) {
                    Event::g_mouse_drag_count++;
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::RightMouseDrag:
                if (Event::g_right_mouse_pressed) {
                    Event::g_mouse_drag_count++;
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::MouseUp:
                if (Event::g_mouse_pressed) {
                    Event::g_mouse_pressed = false;
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::RightMouseUp:
                if (Event::g_right_mouse_pressed) {
                    Event::g_right_mouse_pressed = false;
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::ScrollWheel:
                event->_setHasPreciseScrollingDeltas(ns_event.hasPreciseScrollingDeltas);
                if (Event::g_mouse_pressed == false && Event::g_right_mouse_pressed == false) {
                    event->_setDelta(Vec3d([ns_event deltaX], [ns_event deltaY], [ns_event deltaZ]));
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::Magnification:
                if (Event::g_mouse_pressed == false && Event::g_right_mouse_pressed == false) {
                    event->_setValue(1.0 + [ns_event magnification]);
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::Rotation:
                if (Event::g_mouse_pressed == false && Event::g_right_mouse_pressed == false) {
                    event->_setValue([ns_event rotation]);
                }
                else {
                    ignore = true;
                }
                break;

            case Event::EventType::KeyDown:

                if (Event::g_mouse_pressed == false && Event::g_right_mouse_pressed == false) {
                    NSPoint mouse_location = [NSEvent mouseLocation];
                    NSRect rect = [[ns_view window] convertRectFromScreen:NSMakeRect(mouse_location.x, mouse_location.y, 1, 1)];
                    [ns_view convertPoint:rect.origin fromView:nil];
                    NSPoint pos = [ns_view convertPoint:rect.origin fromView:nil];
                    event->_setMousePos(Vec2d(pos.x, pos.y));
                    event->_setKeyCode([ns_event keyCode]);
                    NSString *key_characters = [ns_event charactersIgnoringModifiers];
                    int32_t key_char_count = (int32_t)[key_characters length];
                    if (key_char_count > 0) {
                        event->_setKeyChar([key_characters characterAtIndex:0]);
                    }
                    event->_setKeyCharCount(key_char_count);
                }
                else {
                    ignore = true;
                }
                break;

            default:
                break;
        }

        event->_setIgnore(ignore);
        event->_setMouseDoubleClicked(double_clicked);

        // The Event is now set and ready to be handled
    }
}
