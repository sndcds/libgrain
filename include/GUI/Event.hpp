//
//  Event.hpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 27.07.2025
//

#ifndef GrainEvent_hpp
#define GrainEvent_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Time/Timestamp.hpp"


namespace Grain {

    class Component;

    class Event {
    public:
        enum class EventType {
            Undefined = 0,
            MouseDown = 1,
            MouseDrag,
            MouseUp,
            MouseEntered,
            MouseExited,
            MouseMoved,
            RightMouseDown,
            RightMouseDrag,
            RightMouseUp,
            ScrollWheel,
            Magnification,
            SmartMagnification,
            Rotation,
            KeyDown,

            Last = KeyDown
            // Changes here must reflect in `typeName()` method
        };

        enum {
            KeyMask_CapsLock = 0x1 << 0,
            KeyMask_Shift = 0x1 << 1,
            KeyMask_Control = 0x1 << 2,
            KeyMask_Alternate = 0x1 << 3,
            KeyMask_Command = 0x1 << 4,
            KeyMask_NumericPad = 0x1 << 5,
            KeyMask_Help = 0x1 << 6,
            KeyMask_Function = 0x1 << 7,

            KeyMask_ModifierKeys = (KeyMask_Shift | KeyMask_Control | KeyMask_Alternate | KeyMask_Command)
        };

        enum class DragDirection {
            Free = 0,
            Horizontal,
            Vertical
        };

    protected:
        EventType m_event_type = EventType::Undefined;  ///< The event type

        Vec2d m_mouse_pos{};            ///< Mouse position whenthe event was fired
        double m_value = 0.0;
        Vec3d m_delta{};
        uint32_t m_key_mask = 0x0;
        int32_t m_key_unichar_count = 0;
        uint16_t m_key_unichar = 0;     ///< 16 bit Unicode character
        uint16_t m_key_code = 0;        ///< 16 bit key code

        bool m_has_precise_scrolling_deltas = false;
        bool m_mouse_double_clicked = false;
        bool m_ignore = false;

        void* m_ns_event = nullptr;     ///< Pointer to corresponding macOS NSEvent

    public:
        static bool g_mouse_pressed;
        static bool g_right_mouse_pressed;
        static Component* g_component;
        static Component* g_previous_component;
        static Timestamp g_ts_last_mouse_click;
        static Vec2d g_mouse_down_pos;
        static int32_t g_mouse_drag_count;

    public:
        Event() noexcept = default;
        ~Event() noexcept;

        [[nodiscard]] virtual const char* className() const noexcept { return "Event"; }

        friend std::ostream& operator << (std::ostream& os, const Event* o) {
            o == nullptr ? os << "Event nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Event& o) {
            os << o.typeName() << std::endl;
            return os;
        }

        void log(Log& l) const;


        [[nodiscard]] bool shouldBeIgnored() const noexcept { return m_ignore; }
        [[nodiscard]] bool shouldBeHandled() const noexcept { return !m_ignore; }

        EventType type() const noexcept { return m_event_type; }
        const char* typeName() const noexcept {
            static const char* _names[] = {
                    "Undefined",
                    "MouseDown",
                    "MouseDrag",
                    "MouseUp",
                    "MouseEntered",
                    "MouseExited",
                    "MouseMoved",
                    "RightMouseDown",
                    "RightMouseDrag",
                    "RightMouseUp",
                    "ScrollWheel",
                    "Magnification",
                    "SmartMagnification",
                    "Rotation",
                    "KeyDown",
            };

            if (m_event_type >= EventType::Undefined && m_event_type <= EventType::Last) {
                return _names[static_cast<int32_t>(m_event_type)];
            }
            else {
                return _names[0];
            }
        }


        [[nodiscard]] void* nsEvent() const noexcept { return m_ns_event; }

        void _setNSEvent(void* ns_event) noexcept { m_ns_event = ns_event; }
        void _setType(EventType type) noexcept { m_event_type = type; }
        void _setMousePos(const Vec2d& pos) noexcept { m_mouse_pos = pos; }
        void _setMouseDoubleClicked(bool double_clicked) noexcept { m_mouse_double_clicked = double_clicked; }
        void _setValue(double value) noexcept { m_value = value; }
        void _setDelta(const Vec3d& delta) noexcept { m_delta = delta; }
        void _setKeyMask(uint32_t mask) noexcept { m_key_mask = mask; }
        void _setKeyCharCount(int32_t count) noexcept { m_key_unichar_count = count; }
        void _setKeyChar(uint16_t unicode_c) noexcept { m_key_unichar = unicode_c; }
        void _setKeyCode(uint16_t key_code) noexcept { m_key_code = key_code; }
        void _setIgnore(bool ignore) noexcept { m_ignore = ignore; }
        void _setHasPreciseScrollingDeltas(bool has_precise_scrolling_deltas) noexcept { m_has_precise_scrolling_deltas = has_precise_scrolling_deltas; }

        [[nodiscard]] Component* component() const noexcept { return g_component; }
        [[nodiscard]] Component* previousComponent() const noexcept { return g_previous_component; }
        [[nodiscard]] bool isMousePressed() const noexcept { return Event::g_mouse_pressed; }
        [[nodiscard]] bool isMouseDoubleClicked() const noexcept { return m_mouse_double_clicked; }
        [[nodiscard]] bool hasPreciseScrollingDeltas() const noexcept { return m_has_precise_scrolling_deltas; }
        [[nodiscard]] bool isFromTrackpad() const noexcept { return m_has_precise_scrolling_deltas; }
        [[nodiscard]] bool isRightMousePressed() const noexcept { return Event::g_right_mouse_pressed; }
        [[nodiscard]] Timestamp timeOfLastMouseClick() const noexcept { return g_ts_last_mouse_click; }
        [[nodiscard]] Vec2d mousePos() const noexcept { return m_mouse_pos; }
        [[nodiscard]] double mouseX() const noexcept { return m_mouse_pos.m_x; }
        [[nodiscard]] double mouseY() const noexcept { return m_mouse_pos.m_y; }
        [[nodiscard]] Vec2d mouseDownPos() const noexcept { return Event::g_mouse_down_pos; }
        [[nodiscard]] double mouseDownX() const noexcept { return Event::g_mouse_down_pos.m_x; }
        [[nodiscard]] double mouseDownY() const noexcept { return Event::g_mouse_down_pos.m_y; }
        [[nodiscard]] int32_t mouseDragCount() const noexcept { return Event::g_mouse_drag_count; }
        [[nodiscard]] double dragZoomX(double step) const noexcept;
        [[nodiscard]] double dragZoomY(double step, bool flipped = false) const noexcept;

        [[nodiscard]] double mouseDragDeltaX() const noexcept { return m_mouse_pos.m_x - Event::g_mouse_down_pos.m_x; }
        [[nodiscard]] double mouseDragDeltaY() const noexcept { return m_mouse_pos.m_y - Event::g_mouse_down_pos.m_y; }
        [[nodiscard]] Vec2d mouseDragDelta() const noexcept { return Vec2d(mouseDragDeltaX(), mouseDragDeltaY()); }
        [[nodiscard]] double mouseDragDistance() const noexcept { return m_mouse_pos.distance(Event::g_mouse_down_pos); }
        [[nodiscard]] DragDirection dragDirection() const noexcept;

        [[nodiscard]] double value() const noexcept { return m_value; }
        [[nodiscard]] Vec3d delta() const noexcept { return m_delta; }
        [[nodiscard]] double deltaX() const noexcept { return m_delta.m_x; }
        [[nodiscard]] double deltaY() const noexcept { return m_delta.m_y; }
        [[nodiscard]] double deltaZ() const noexcept { return m_delta.m_z; }
        [[nodiscard]] uint32_t keyMask() const noexcept { return m_key_mask; }
        [[nodiscard]] int32_t keyCharCount() const noexcept { return m_key_unichar_count; }
        [[nodiscard]] uint16_t keyChar() const noexcept { return m_key_unichar; }

        [[nodiscard]] bool isSingleKeyChar() const noexcept { return keyCharCount() == 1; }

        [[nodiscard]] bool isMouseDown() const noexcept { return m_event_type == EventType::MouseDown; }
        [[nodiscard]] bool isMouseUp() const noexcept { return m_event_type == EventType::MouseUp; }
        [[nodiscard]] bool isMouseDrag() const noexcept { return m_event_type == EventType::MouseDrag; }

        [[nodiscard]] bool isAltPressed() const noexcept { return m_key_mask & KeyMask_Alternate; }
        [[nodiscard]] bool isControlPressed() const noexcept { return m_key_mask & KeyMask_Control; }
        [[nodiscard]] bool isShiftPressed() const noexcept { return m_key_mask & KeyMask_Shift; }
        [[nodiscard]] bool isCommandPressed() const noexcept { return m_key_mask & KeyMask_Command; }
        [[nodiscard]] bool isCapsLock() const noexcept { return m_key_mask & KeyMask_CapsLock; }

        [[nodiscard]] bool isAltPressedOnly() const noexcept { return (m_key_mask & KeyMask_ModifierKeys) == KeyMask_Alternate; }
        [[nodiscard]] bool isControlPressedOnly() const noexcept { return (m_key_mask & KeyMask_ModifierKeys) == KeyMask_Control; }
        [[nodiscard]] bool isShiftPressedOnly() const noexcept { return (m_key_mask & KeyMask_ModifierKeys) == KeyMask_Shift; }
        [[nodiscard]] bool isCommandPressedOnly() const noexcept { return (m_key_mask & KeyMask_ModifierKeys) == KeyMask_Command; }

        void setMousePos(const Vec2d& pos) noexcept { m_mouse_pos = pos; }
        void setKeyMask(uint32_t key_mask) noexcept { m_key_mask = key_mask; }

        void mousePressedFinished() const noexcept { Event::g_mouse_pressed = false; }
        void rightMousePressedFinished() const noexcept { Event::g_right_mouse_pressed = false; }

        [[nodiscard]] double distanceFromMouse(Vec2d pos) const noexcept { return m_mouse_pos.distance(pos); }

        /* macos!!!!
        [[nodiscard]] static uint32_t _keyMaskFromNSEvent(const NSEvent* ns_event) noexcept {
            uint32_t key_mask = 0x0;
            NSEventModifierFlags modifier_flags = [ns_event modifierFlags];
            if (modifier_flags & NSEventModifierFlagCapsLock) { key_mask |= KeyMask_CapsLock; }
            if (modifier_flags & NSEventModifierFlagShift) { key_mask |= KeyMask_Shift; }
            if (modifier_flags & NSEventModifierFlagControl) { key_mask |= KeyMask_Control; }
            if (modifier_flags & NSEventModifierFlagOption) { key_mask |= KeyMask_Alternate; }
            if (modifier_flags & NSEventModifierFlagCommand) { key_mask |= KeyMask_Command; }
            if (modifier_flags & NSEventModifierFlagNumericPad) { key_mask |= KeyMask_NumericPad; }
            if (modifier_flags & NSEventModifierFlagHelp) { key_mask |= KeyMask_Help; }
            if (modifier_flags & NSEventModifierFlagFunction) { key_mask |= KeyMask_Function; }
            return key_mask;
        }
         */

        /* macos!!!!
        [[nodiscard]] static CGEventFlags _keyMaskToCGEventFlags(uint32_t key_mask) noexcept {
            CGEventFlags flags = 0x0;
            if (key_mask & KeyMask_Shift) { flags |= kCGEventFlagMaskShift; }
            if (key_mask & KeyMask_Control) { flags |= kCGEventFlagMaskControl; }
            if (key_mask & KeyMask_Alternate) { flags |= kCGEventFlagMaskAlternate; }
            if (key_mask & KeyMask_Command) { flags |= kCGEventFlagMaskCommand; }
            if (key_mask & KeyMask_NumericPad) { flags |= kCGEventFlagMaskNumericPad; }
            if (key_mask & KeyMask_Help) { flags |= kCGEventFlagMaskHelp; }
            if (key_mask & KeyMask_CapsLock) { flags |= kCGEventFlagMaskAlphaShift; }
            if (key_mask & KeyMask_Function) { flags |= kCGEventFlagMaskSecondaryFn; }

            return flags;
        }
         */
    };


} // End of namespace Grain

#endif // GrainEvent_hpp
