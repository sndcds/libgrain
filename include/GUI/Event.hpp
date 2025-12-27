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

#if defined(__APPLE__) && defined(__MACH__)
#else
    #include <SDL2/SDL.h>
#endif


namespace Grain {

    class Component;

    /**
     *  @brief
     *  GrainLib internally uses macOS key codes for keyboard input.
     *
     *  On other platforms, keyboard event codes must be converted or mapped
     *  to the corresponding macOS key codes in the event loop.
     */
    enum KeyCode {
        Enter = 0x0003,                 // NSEnterCharacter
        Backspace = 0x0008,             // NSBackspaceCharacter
        Tab = 0x0009,                   // NSTabCharacter
        NewLine = 0x000a,               // NSNewlineCharacter
        FormFeed = 0x000c,              // NSFormFeedCharacter
        CarriageReturn = 0x000d,        // NSCarriageReturnCharacter
        BackTab = 0x0019,               // NSBackTabCharacter
        Delete = 0x007f,                // NSDeleteCharacter
        LineSeparator = 0x2028,         // NSLineSeparatorCharacter
        ParagraphSeparator = 0x2029,    // NSParagraphSeparatorCharacter

        FunctionUpArrow = 0xF700,       // NSUpArrowFunctionKey
        FunctionDownArrow = 0xF701,     // NSDownArrowFunctionKey
        FunctionLeftArrow = 0xF702,     // NSLeftArrowFunctionKey
        FunctionRightArrow = 0xF703,    // NSRightArrowFunctionKey

        FunctionF1 = 0xF704,            // NSF1FunctionKey
        FunctionF2 = 0xF705,            // NSF2FunctionKey
        FunctionF3 = 0xF706,            // NSF3FunctionKey
        FunctionF4 = 0xF707,            // NSF4FunctionKey
        FunctionF5 = 0xF708,            // NSF5FunctionKey
        FunctionF6 = 0xF709,            // NSF6FunctionKey
        FunctionF7 = 0xF70A,            // NSF7FunctionKey
        FunctionF8 = 0xF70B,            // NSF8FunctionKey
        FunctionF9 = 0xF70C,            // NSF9FunctionKey
        FunctionF10 = 0xF70D,           // NSF10FunctionKey
        FunctionF11 = 0xF70E,           // NSF11FunctionKey
        FunctionF12 = 0xF70F,           // NSF12FunctionKey
        FunctionF13 = 0xF710,           // NSF13FunctionKey
        FunctionF14 = 0xF711,           // NSF14FunctionKey
        FunctionF15 = 0xF712,           // NSF15FunctionKey
        FunctionF16 = 0xF713,           // NSF16FunctionKey
        FunctionF17 = 0xF714,           // NSF17FunctionKey
        FunctionF18 = 0xF715,           // NSF18FunctionKey
        FunctionF19 = 0xF716,           // NSF19FunctionKey
        FunctionF20 = 0xF717,           // NSF20FunctionKey
        FunctionF21 = 0xF718,           // NSF21FunctionKey
        FunctionF22 = 0xF719,           // NSF22FunctionKey
        FunctionF23 = 0xF71A,           // NSF23FunctionKey
        FunctionF24 = 0xF71B,           // NSF24FunctionKey
        FunctionF25 = 0xF71C,           // NSF25FunctionKey
        FunctionF26 = 0xF71D,           // NSF26FunctionKey
        FunctionF27 = 0xF71E,           // NSF27FunctionKey
        FunctionF28 = 0xF71F,           // NSF28FunctionKey
        FunctionF29 = 0xF720,           // NSF29FunctionKey
        FunctionF30 = 0xF721,           // NSF30FunctionKey
        FunctionF31 = 0xF722,           // NSF31FunctionKey
        FunctionF32 = 0xF723,           // NSF32FunctionKey
        FunctionF33 = 0xF724,           // NSF33FunctionKey
        FunctionF34 = 0xF725,           // NSF34FunctionKey
        FunctionF35 = 0xF726,           // NSF35FunctionKey

        FunctionInsert = 0xF727,        // NSInsertFunctionKey

        FunctionDelete = 0xF728,        // NSDeleteFunctionKey
        FunctionHome = 0xF729,          // NSHomeFunctionKey
        FunctionBegin = 0xF72A,         // NSBeginFunctionKey
        FunctionEnd = 0xF72B,           // NSEndFunctionKey
        FunctionPageUp = 0xF72C,        // NSPageUpFunctionKey
        FunctionPageDown = 0xF72D,      // NSPageDownFunctionKey
        FunctionPrintScreen = 0xF72E,   // NSPrintScreenFunctionKey
        FunctionScrollLock = 0xF72F,    // NSScrollLockFunctionKey
        FunctionPause = 0xF730,         // NSPauseFunctionKey

        FunctionSysReq = 0xF731,        // NSSysReqFunctionKey
        FunctionBreak = 0xF732,         // NSBreakFunctionKey
        FunctionReset = 0xF733,         // NSResetFunctionKey
        FunctionStop = 0xF734,          // NSStopFunctionKey
        FunctionMenu = 0xF735,          // NSMenuFunctionKey
        FunctionUser = 0xF736,          // NSUserFunctionKey
        FunctionSystem = 0xF737,        // NSSystemFunctionKey
        FunctionPrint = 0xF738,         // NSPrintFunctionKey
        FunctionClearLine = 0xF739,     // NSClearLineFunctionKey

        FunctionClearDisplay = 0xF73A,  // NSClearDisplayFunctionKey
        FunctionInsertLine = 0xF73B,    // NSInsertLineFunctionKey
        FunctionDeleteLine = 0xF73C,    // NSDeleteLineFunctionKey
        FunctionInsertChar = 0xF73D,    // NSInsertCharFunctionKey
        FunctionDeleteChar = 0xF73E,    // NSDeleteCharFunctionKey
        FunctionPrev = 0xF73F,          // NSPrevFunctionKey
        FunctionNext = 0xF740,          // NSNextFunctionKey
        FunctionSelect = 0xF741,        // NSSelectFunctionKey
        FunctionExecute = 0xF742,       // NSExecuteFunctionKey
        FunctionUndo = 0xF743,          // NSUndoFunctionKey
        FunctionRedo = 0xF744,          // NSRedoFunctionKey
        FunctionFind = 0xF745,          // NSFindFunctionKey
        FunctionHelp = 0xF746,          // NSHelpFunctionKey
        FunctionModeSwitch = 0xF747,    // NSModeSwitchFunctionKey
    };

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
        [[nodiscard]] double mouseX() const noexcept { return m_mouse_pos.x_; }
        [[nodiscard]] double mouseY() const noexcept { return m_mouse_pos.y_; }
        [[nodiscard]] Vec2d mouseDownPos() const noexcept { return Event::g_mouse_down_pos; }
        [[nodiscard]] double mouseDownX() const noexcept { return Event::g_mouse_down_pos.x_; }
        [[nodiscard]] double mouseDownY() const noexcept { return Event::g_mouse_down_pos.y_; }
        [[nodiscard]] int32_t mouseDragCount() const noexcept { return Event::g_mouse_drag_count; }
        [[nodiscard]] double dragZoomX(double step) const noexcept;
        [[nodiscard]] double dragZoomY(double step, bool flipped = false) const noexcept;

        [[nodiscard]] double mouseDragDeltaX() const noexcept { return m_mouse_pos.x_ - Event::g_mouse_down_pos.x_; }
        [[nodiscard]] double mouseDragDeltaY() const noexcept { return m_mouse_pos.y_ - Event::g_mouse_down_pos.y_; }
        [[nodiscard]] Vec2d mouseDragDelta() const noexcept { return Vec2d(mouseDragDeltaX(), mouseDragDeltaY()); }
        [[nodiscard]] double mouseDragDistance() const noexcept { return m_mouse_pos.distance(Event::g_mouse_down_pos); }
        [[nodiscard]] DragDirection dragDirection() const noexcept;

        [[nodiscard]] double value() const noexcept { return m_value; }
        [[nodiscard]] Vec3d delta() const noexcept { return m_delta; }
        [[nodiscard]] double deltaX() const noexcept { return m_delta.x_; }
        [[nodiscard]] double deltaY() const noexcept { return m_delta.y_; }
        [[nodiscard]] double deltaZ() const noexcept { return m_delta.z_; }
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
    };


} // End of namespace Grain

#endif // GrainEvent_hpp
