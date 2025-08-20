//
//  Component.hpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 26.07.2025
//

#ifndef GrainComponent_hpp
#define GrainComponent_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "2d/Rect.hpp"
#include "2d/Dimension.hpp"
#include "GUI/GUIStyle.hpp"


namespace Grain {

    class Gradient;
    class RGBA;
    class Component;
    class View;
    class TextField;
    class Event;
    class Font;
    class Text;
    class GraphicContext;


    typedef void (*ComponentAction)(Component* component);
    typedef void (*ComponentDrawFunc)(GraphicContext& gc, Component* component, void* ref);
    typedef bool (*ComponentHandleEventFunc)(Component* component, const Event& event, void* ref);
    typedef bool (*ComponentHandleMessageFunc)(Component* component, const char* message, void* ref, void* data);


    class Component : Object {

    public:
        enum class ComponentType {
            Undefined = -1,
            View = 0,
            MetalView,
            SplitView,
            Viewport,
            ScrollView,
            ScrollAreaView,
            ScrollBar,
            Separator,
            Label,
            Button,
            PopUpButton,
            IconButton,
            SymbolButton,
            TransportButton,
            CheckBox,
            Toggle,
            TextField,
            TextEditor,
            Knob,
            Slider,
            ProgressBar,
            ColorWell,
            ColorWheel,
            ColorPaletteControl,
            GradientControl,
            SignalView,
            SignalOverview,
            PartialsView,
            ImageView,
            TableView,
            TableScrollAreaView,
            AudioLocationControl,
            SpatSysControl,
            LevelCurveControl,
            Custom
        };

        enum AddFlags {
            kNone,
            kWantsLayer = 0x1
        };

        enum class ActionType {
            None = 0,
            StateChanged,
            ViewportChanged
        };

    public:
        explicit Component(int32_t tag = 0) noexcept : Component(Rectd(0, 0, 1, 1), tag) {}
        explicit Component(const Rectd& rect, int32_t tag = 0) noexcept;

        ~Component() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "Component"; }

        friend std::ostream& operator << (std::ostream& os, const Component* o) {
            o == nullptr ? os << "Component nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Component& o) {
            os << (int32_t)o.m_type << std::endl; // TODO: Implement!
            return os;
        }



        #if defined(__APPLE__) && defined(__MACH__)
            [[nodiscard]] virtual void* nsView() const noexcept { return m_ns_view; }
            virtual void setNSView(void* ns_view) noexcept { m_ns_view = ns_view; }
        #endif

        [[nodiscard]] ComponentType componentType() const noexcept { return m_type; }

        [[nodiscard]] int32_t tag() const noexcept { return m_tag; }
        void setTag(int32_t tag) noexcept { m_tag = tag; }

        [[nodiscard]] double x() const noexcept { return m_rect.m_x; }
        [[nodiscard]] double y() const noexcept { return m_rect.m_y; }
        [[nodiscard]] double width() const noexcept { return m_rect.m_width; }
        [[nodiscard]] double height() const noexcept { return m_rect.m_height; }
        [[nodiscard]] Dimensiond dimension() const noexcept { return Dimensiond(m_rect.m_width, m_rect.m_height); }
        [[nodiscard]] double size(bool vertical) const noexcept { return vertical ? m_rect.m_height : m_rect.m_width; }
        [[nodiscard]] double aspectRatio() const noexcept { return m_rect.m_width != 0.0 ? m_rect.m_height / m_rect.m_width : 1.0; }
        [[nodiscard]] double shortSide() const noexcept { return m_rect.shortSide(); }
        [[nodiscard]] double longSide() const noexcept { return m_rect.longSide(); }
        [[nodiscard]] virtual Vec2d center() const noexcept { return Vec2d(m_rect.m_width * 0.5, m_rect.m_height * 0.5); }
        [[nodiscard]] double centerX() const noexcept { return m_rect.m_width * 0.5; }
        [[nodiscard]] double centerY() const noexcept { return m_rect.m_height * 0.5; }


        // Flags
        [[nodiscard]] bool isEnabled() const noexcept { return m_is_enabled; }
        static bool setEnabled(Component* component, bool enabled) noexcept;
        virtual bool setEnabled(bool enabled) noexcept;
        bool enable() noexcept { return setEnabled(true); }
        bool disable() noexcept { return setEnabled(false); }
        void toggleEnabled() noexcept{ setEnabled(!m_is_enabled); }
        void setVisibility(bool visibility) noexcept;

        [[nodiscard]] bool isSelected() const noexcept { return m_is_selected; }
        virtual void setSelected(bool selected) noexcept { m_is_selected = selected; needsDisplay(); }
        void select() noexcept { setSelected(true); }
        void deselect() noexcept { setSelected(false); }
        void deselectWithoutChecking() noexcept { m_is_selected = false; needsDisplay(); }

        [[nodiscard]] bool isToggleMode() const noexcept { return m_is_toggle_mode; }
        void toggleSelection() noexcept { setSelected(!m_is_selected); }
        void setToggleMode(bool toggle_mode) noexcept { m_is_toggle_mode = toggle_mode; }

        [[nodiscard]] bool isFlippedView() const noexcept { return m_view_is_flipped; }
        void setFlippedView(bool flipped_view ) noexcept { m_view_is_flipped = flipped_view; }

        [[nodiscard]] bool isNumberMode() const noexcept { return m_is_number_mode; }
        void virtual setNumberMode(bool mode) noexcept {};
        virtual void stepNumber(bool use_big_step, bool negative) noexcept {};

        [[nodiscard]] bool canGetFocus() const noexcept {
            return m_is_visible && m_can_get_focus && m_is_enabled && m_rect.m_width > 0.0 && m_rect.m_height > 0.0;
        }
        void setFocusFlag(bool focus_flag) noexcept {
            if (m_focus_flag != focus_flag) {
                m_focus_flag = focus_flag;
                needsDisplay();
            }
        }
        [[nodiscard]] bool hasFocusFlag() const noexcept { return m_focus_flag; }


        [[nodiscard]] bool isHorizontal() const noexcept { return m_rect.isHorizontal(); }
        [[nodiscard]] bool isVertical() const noexcept { return m_rect.isVertical(); }
        [[nodiscard]] bool acceptsFirstMouse() const noexcept { return m_is_visible ? m_accepts_first_mouse : false; }
        void setAcceptsFirstMouse(bool accepts_first_mouse ) noexcept { m_accepts_first_mouse = accepts_first_mouse; }
        [[nodiscard]] bool isHandlingMouseMoved() const noexcept { return m_handles_mouse_moved; }
        void setHandlesMouseMoved(bool handles_mouse_moved) noexcept { m_handles_mouse_moved = handles_mouse_moved; }
        [[nodiscard]] bool isMouseInView() const noexcept { return m_mouse_is_in_view; }

        [[nodiscard]] bool isDragEntered() const noexcept { return m_drag_entered_flag; }
        void setDragEntered(bool drag_entered) noexcept { m_drag_entered_flag = drag_entered; }

        // Rect, Bounds
        [[nodiscard]] Rectd rect() const noexcept { return m_rect; }
        [[nodiscard]] bool isRectUsable() const noexcept { return m_rect.usable(); }
        [[nodiscard]] Rectd boundsRect() const noexcept { return Rectd(m_rect.m_width, m_rect.m_height); }
        [[nodiscard]] Rectd contentRect() const noexcept;


        // Style
        [[nodiscard]] bool isOpaque() const noexcept { return true; }
        void setStyleIndex(int32_t index) { m_style_index = index; }
        [[nodiscard]] GUIStyle* guiStyle() const noexcept;


        // Text
        [[nodiscard]] bool hasText() const noexcept { return m_text ? m_text->length() > 0 : false; }
        void setText(const char* text_str) noexcept;
        void setText(const String& text) noexcept;
        [[nodiscard]] virtual int32_t textLength() const noexcept { return m_text ? m_text->length() : 0; }

        //
        [[nodiscard]] virtual bool hasDescendant(const Component* component) noexcept { return false; }
        void setNextKeyComponent(Component* component) noexcept;
        [[nodiscard]] bool isKeyComponent() const noexcept;
        bool gotoComponent(Component* component) noexcept;
        bool gotoNextKeyComponent() noexcept;
        bool gotoPreviousKeyComponent() noexcept;

        // Action
        virtual void _fireAction() noexcept { fireAction(ActionType::None, nullptr); }
        virtual void fireAction(ActionType action_type, const Component* excluded_component) noexcept;
        virtual void fireActionAndDisplay(ActionType action_type, const Component* excluded_component) noexcept;
        virtual void updateRepresentations(const Component* excluded_component) noexcept;
        virtual void transmit() noexcept {
            if (m_receiver_component) {
                m_receiver_component->setByComponent(this);
            }
        }
        virtual void setByComponent(Component* component) noexcept {}
        virtual void setReceiverComponent(Component* component) noexcept { m_receiver_component = component; }
        virtual void setTextField(TextField* textfield) noexcept { m_textfield = textfield; }

        void setAction(ComponentAction action) noexcept { setAction(action, nullptr); }
        void setAction(ComponentAction action, void* action_ref) noexcept {
            m_action = action;
            m_action_ref = action_ref;
        }
        void* actionRef() const noexcept { return m_action_ref; }

        void setTextChangedAction(ComponentAction action) noexcept { setTextChangedAction(action, nullptr); }
        void setTextChangedAction(ComponentAction action, void* action_ref) noexcept {
            m_text_changed_action = action;
            m_text_changed_action_ref = action_ref;
        }
        void textChangedAction() noexcept {
            if (m_text_changed_action != nullptr) {
                m_text_changed_action(this);
            }
        }



        // FirstResponder
        virtual void becomeFirstResponder() noexcept {}
        virtual void resignFirstResponder() noexcept {}

        virtual void insertText(const char* text) noexcept {}

        // Geometry
        void setRect(const Rectd& rect) noexcept;
        void setPosition(double x, double y) noexcept;
        void setDimension(double width, double height) noexcept;
        void setEdgeAligned() noexcept;
        void setEdgeAligned(Alignment alignment, float top, float right, float bottom, float left) noexcept;
        virtual void geometryChanged() noexcept {}
        virtual void parentGeometryChanged() noexcept;

        // Radio Group
        [[nodiscard]] virtual int32_t radioGroup() const noexcept { return std::numeric_limits<int32_t>::max(); }
        [[nodiscard]] virtual int32_t radioValue() const noexcept { return std::numeric_limits<int32_t>::max(); }
        virtual void setRadioGroup(int32_t radioGroup) noexcept {}
        virtual void setRadioValue(int32_t radioValue) noexcept {}
        virtual void deselectRadioGroup(int32_t radio_group) noexcept {}

        // Highlight
        [[nodiscard]] bool isHighlighted() const noexcept { return m_is_highlighted; }
        virtual void setHighlighted(bool highlighted) noexcept;
        void highlight() noexcept { setHighlighted(true); }
        void deHighlight() noexcept { setHighlighted(false); }

        // Delayed
        [[nodiscard]] bool isDelayed() const noexcept { return m_is_delayed; }
        void setDelayed(bool delayed) noexcept { m_is_delayed = delayed; }

        // Value
        [[nodiscard]] virtual Fix value() const noexcept { return Fix{}; }
        virtual bool setValue(const Fix & value) noexcept { return false; }
        virtual void setValueRange(const Fix& min, const Fix& max) noexcept {}
        [[nodiscard]] virtual int32_t valueAsInt32() const noexcept { return 0; }
        [[nodiscard]] virtual double valueAsDouble() const noexcept { return 0; }
        virtual bool setValueInt(int32_t value) noexcept { return setValue(Fix(value)); }
        virtual bool setValueDouble(double value) noexcept { return setValue(Fix(value)); }


        // Event
        virtual void handleEvent(const Event& event) noexcept;
        [[nodiscard]] bool hasHandleEventFunction() const noexcept { return _m_handle_event_func != nullptr; }
        void setHandleEventFunction(ComponentHandleEventFunc func, void* ref = nullptr) noexcept;
        bool callHandleEventFunction(const Event& event) noexcept;
        void _interpretKeyEvents(const Event& event) noexcept;

        // Drawing
        virtual void draw(const Rectd& dirty_rect) noexcept {}
        virtual void updateBeforeDrawing(const Rectd& dirty_rect) noexcept {}

        // Custom draw function
        [[nodiscard]] bool hasDrawFunction() const noexcept { return _m_draw_func != nullptr; }
        void setDrawFunction(ComponentDrawFunc func, void* ref = nullptr) noexcept;
        void callDrawFunction(GraphicContext& gc) noexcept;

        // Dragging
        virtual void handleDraggingEntered() noexcept {}
        virtual void handleDraggingExited() noexcept {}
        virtual void handleDraggingUpdated() noexcept {}

        // Dropping
        virtual void filesDropped(const StringList& path_list) noexcept {}

        // Update
        void needsDisplay() const noexcept;
        virtual void forcedDisplay() const noexcept;

        // Mouse
        virtual void updateAtMouseDown(const Event& event) noexcept {}
        virtual void handleMouseDown(const Event& event) noexcept { needsDisplay(); }
        virtual void handleMouseDrag(const Event& event) noexcept {}
        virtual void handleMouseUp(const Event& event) noexcept {}
        virtual void handleRightMouseDown(const Event& event) noexcept {}
        virtual void handleRightMouseDrag(const Event& event) noexcept {}
        virtual void handleRightMouseUp(const Event& event) noexcept {}
        virtual void handleMouseEntered(const Event& event) noexcept {}
        virtual void handleMouseExited(const Event& event) noexcept {}
        virtual void handleMouseMoved(const Event& event) noexcept {}
        virtual void handleScrollWheel(const Event& event) noexcept {
            if (m_parent) {
                m_parent->handleScrollWheel(event);
            }
        }
        virtual void handleMagnification(const Event& event) noexcept {
            if (m_parent) {
                m_parent->handleMagnification(event);
            }
        }
        virtual void handleRotation(const Event& event) noexcept {
            if (m_parent) {
                m_parent->handleRotation(event);
            }
        }
        virtual void handleKeyDown(const Event& event) noexcept {
            if (m_parent) {
                m_parent->handleKeyDown(event);
            }
        }

        [[nodiscard]] bool hasAction() const noexcept { return m_action != nullptr; }


        // Geometry
        virtual bool hit(const Vec2d& pos) noexcept;
        virtual bool hit(const Event& event) noexcept;

        // Drawing
        void drawDummy(GraphicContext& gc) const noexcept;
        void drawRect(GraphicContext& gc, const Rectd& rect) const noexcept;

        // Utils
        static Component* addComponentToView(Component* component, View* view, AddFlags flags) noexcept;
        void _setParent(Component* parent) noexcept { m_parent = parent; }

    protected:
        ComponentType m_type = ComponentType::Undefined;  ///< What type of component it is
        int32_t m_tag = 0;           ///< A tag, can be used to identify a component
        char* m_name = nullptr;      ///< An optional name

        #if defined(__APPLE__) && defined(__MACH__)
            void* m_ns_view = nullptr;   ///< The related NSView on macOS
        #endif

        // Flags
        bool m_view_is_flipped = true;
        bool m_accepts_first_mouse = true;
        bool m_handles_mouse_moved = true;
        bool m_fills_bg = true;
        bool m_is_visible = true;
        bool m_is_enabled = true;
        bool m_is_selected = false;
        bool m_is_highlighted = false;
        bool m_is_delayed = true;
        bool m_is_editable = false;
        bool m_is_toggle_mode = false;
        bool m_is_number_mode = false;
        bool m_can_get_focus = false;
        bool m_focus_flag = false;
        bool m_continuous_update_flag = true;
        bool m_drag_entered_flag = false;
        bool m_simple_mode_flag = false;
        bool m_can_have_children = false;
        bool m_draws_as_button = false;
        bool m_shows_debug_info = false;

        Component* m_parent = nullptr;          ///< The parent component, root views don´t have a parent
        Rectd m_rect = Rectd(100.0, 100.0);     ///< Position and size of component in view
        Alignment m_edge_alignment = Alignment::No;
        RectEdgesf m_margin{};

        // Style
        int32_t m_style_index = 0;              ///<

        // Text
        String* m_text = nullptr;               ///< Optional text

        // Mouse
        int32_t m_mouse_mode = 0;
        bool m_mouse_precision_mode = false;
        bool m_mouse_is_in_view = false;
        bool m_needs_redraw_at_mouse_enter_and_exit = false;
        bool m_is_modified_while_mouse_drag = false;
        bool m_is_modified_since_mouse_down = false;

        // Connected components
        TextField* m_textfield{};
        // Label* m_label{}; GUI!!!
        // ColorWell* m_color_well{}; GUI!!!
        Component* m_receiver_component{};
        Component* m_previous_key_component{};
        Component* m_next_key_component{};

        // Action
        ActionType _m_action_type = ActionType::None;

        // Component specific functions
        ComponentAction m_action{};
        void* m_action_ref{};

        ComponentAction m_text_changed_action{};
        void* m_text_changed_action_ref{};

        ComponentDrawFunc _m_draw_func{};
        void* _m_draw_func_ref{};

        ComponentHandleEventFunc _m_handle_event_func{};
        void* _m_handle_event_func_ref{};

        ComponentHandleMessageFunc _m_handle_message_func{};
        void* _m_handle_message_func_ref{};
    };

}

#endif // GrainComponent_hpp