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

#include "Type/Object.hpp"
#include "2d/Rect.hpp"
#include "2d/Dimension.hpp"
#include "2d/Border.hpp"
#include "GUI/GUIStyle.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CoreVideo.h>
#endif

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
    typedef void (*ComponentDrawFunc)(GraphicContext* gc, Component* component, void* ref);
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
            kNone = 0x0,
            kWantsLayer = 0x1
        };

        enum class ActionType {
            None = 0,
            StateChanged,
            ViewportChanged
        };

    protected:
        ComponentType type_ = ComponentType::Undefined;  ///< What type of component it is
        int32_t tag_ = 0;           ///< A tag, can be used to identify a component
        char* name_ = nullptr;      ///< An optional name

#if defined(__APPLE__) && defined(__MACH__)
        void* ns_view_ = nullptr;   ///< The related NSView on macOS
#endif

        GraphicContext* gc_ptr_ = nullptr;

        // Flags
        bool view_is_flipped_ = true;
        bool accepts_first_mouse_ = true;
        bool handles_mouse_moved_ = true;
        bool fills_bg_ = true;
        bool is_visible_ = true;
        bool is_enabled_ = true;
        bool is_selected_ = false;
        bool is_highlighted_ = false;
        bool is_delayed_ = true;
        bool is_editable_ = false;
        bool is_toggle_mode_ = false;
        bool is_number_mode_ = false;
        bool can_get_focus_ = false;
        bool focus_flag_ = false;
        bool continuous_update_flag_ = true;
        bool drag_entered_flag_ = false;
        bool simple_mode_flag_ = false;
        bool can_have_children_ = false;
        bool draws_as_button_ = false;
        bool shows_debug_info_ = false;

        Component* parent_ = nullptr;       ///< Target view that this component renders into
        Rectd rect_ = Rectd(100.0, 100.0);  ///< Position and size of component in view
        Alignment edge_alignment_ = Alignment::No;
        Borderf margin_{};

        // Style
        int32_t style_index_ = 0;   ///<
        int32_t controller_padding_ = 6;    ///< Inner padding fpr components like sliders, knobs ...

        // Text
        String* text_ = nullptr;    ///< Optional text

        // Mouse
        int32_t mouse_mode_ = 0;
        bool mouse_precision_mode_ = false;
        bool mouse_is_in_view_ = false;
        bool needs_redraw_at_mouse_enter_and_exit_ = false;
        bool is_modified_while_mouse_drag_ = false;
        bool is_modified_since_mouse_down_ = false;

        // Connected components
        TextField* textfield_{};
        // Label* label_{}; GUI!!!
        // ColorWell* color_well_{}; GUI!!!
        Component* receiver_component_{};
        Component* previous_key_component_{};
        Component* next_key_component_{};

        // Action
        ActionType action_type_ = ActionType::None;

        double animation_progress_{};


        // Component specific functions
        ComponentAction action_{};
        void* action_ref_{};

        ComponentAction text_changed_action_{};
        void* text_changed_action_ref_{};

        ComponentDrawFunc draw_func_{};
        void* draw_func_ref_{};

        ComponentHandleEventFunc handle_event_func_{};
        void* handle_event_func_ref_{};

        ComponentHandleMessageFunc handle_message_func_{};
        void* handle_message_func_ref_{};

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
            os << static_cast<int32_t>(o.type_) << std::endl; // TODO: Implement!
            return os;
        }



#if defined(__APPLE__) && defined(__MACH__)
        [[nodiscard]] virtual void* nsView() const noexcept { return ns_view_; }
        virtual void setNSView(void* ns_view) noexcept { ns_view_ = ns_view; }
#endif

        [[nodiscard]] ComponentType componentType() const noexcept { return type_; }

        [[nodiscard]] int32_t tag() const noexcept { return tag_; }
        void setTag(int32_t tag) noexcept { tag_ = tag; }

        [[nodiscard]] double x() const noexcept { return rect_.x_; }
        [[nodiscard]] double y() const noexcept { return rect_.y_; }
        [[nodiscard]] double width() const noexcept { return rect_.width_; }
        [[nodiscard]] double height() const noexcept { return rect_.height_; }
        [[nodiscard]] Dimensiond dimension() const noexcept { return { rect_.width_, rect_.height_ }; }
        [[nodiscard]] double size(bool vertical) const noexcept { return vertical ? rect_.height_ : rect_.width_; }
        [[nodiscard]] double aspectRatio() const noexcept { return rect_.width_ != 0.0 ? rect_.height_ / rect_.width_ : 1.0; }
        [[nodiscard]] double shortSide() const noexcept { return rect_.shortSide(); }
        [[nodiscard]] double longSide() const noexcept { return rect_.longSide(); }
        [[nodiscard]] virtual Vec2d center() const noexcept { return { rect_.width_ * 0.5, rect_.height_ * 0.5 }; }
        [[nodiscard]] double centerX() const noexcept { return rect_.width_ * 0.5; }
        [[nodiscard]] double centerY() const noexcept { return rect_.height_ * 0.5; }


        // Flags
        [[nodiscard]] bool isEnabled() const noexcept { return is_enabled_; }
        static bool setEnabled(Component* component, bool enabled) noexcept;
        virtual bool setEnabled(bool enabled) noexcept;
        bool enable() noexcept { return setEnabled(true); }
        bool disable() noexcept { return setEnabled(false); }
        void toggleEnabled() noexcept{ setEnabled(!is_enabled_); }
        void setVisibility(bool visibility) noexcept;

        [[nodiscard]] bool isSelected() const noexcept { return is_selected_; }
        virtual void setSelected(bool selected) noexcept { is_selected_ = selected; needsDisplay(); }
        void select() noexcept { setSelected(true); }
        void deselect() noexcept { setSelected(false); }
        void deselectWithoutChecking() noexcept { is_selected_ = false; needsDisplay(); }

        [[nodiscard]] bool isToggleMode() const noexcept { return is_toggle_mode_; }
        void toggleSelection() noexcept { setSelected(!is_selected_); }
        void setToggleMode(bool toggle_mode) noexcept { is_toggle_mode_ = toggle_mode; }

        [[nodiscard]] bool isFlippedView() const noexcept { return view_is_flipped_; }
        void setFlippedView(bool flipped_view ) noexcept { view_is_flipped_ = flipped_view; }

        [[nodiscard]] bool isNumberMode() const noexcept { return is_number_mode_; }
        void virtual setNumberMode(bool mode) noexcept {};
        virtual void stepNumber(bool use_big_step, bool negative) noexcept {};

        [[nodiscard]] bool canGetFocus() const noexcept {
            return is_visible_ && can_get_focus_ && is_enabled_ && rect_.width_ > 0.0 && rect_.height_ > 0.0;
        }
        void setFocusFlag(bool focus_flag) noexcept {
            if (focus_flag_ != focus_flag) {
                focus_flag_ = focus_flag;
                needsDisplay();
            }
        }
        [[nodiscard]] bool hasFocusFlag() const noexcept { return focus_flag_; }


        [[nodiscard]] virtual bool isHorizontal() const noexcept { return rect_.isHorizontal(); }
        [[nodiscard]] virtual bool isVertical() const noexcept { return rect_.isVertical(); }
        [[nodiscard]] bool acceptsFirstMouse() const noexcept { return is_visible_ ? accepts_first_mouse_ : false; }
        void setAcceptsFirstMouse(bool accepts_first_mouse ) noexcept { accepts_first_mouse_ = accepts_first_mouse; }
        [[nodiscard]] bool isHandlingMouseMoved() const noexcept { return handles_mouse_moved_; }
        void setHandlesMouseMoved(bool handles_mouse_moved) noexcept { handles_mouse_moved_ = handles_mouse_moved; }
        [[nodiscard]] bool isMouseInView() const noexcept { return mouse_is_in_view_; }

        [[nodiscard]] bool isDragEntered() const noexcept { return drag_entered_flag_; }
        void setDragEntered(bool drag_entered) noexcept { drag_entered_flag_ = drag_entered; }

        // Rect, Bounds
        [[nodiscard]] Rectd rect() const noexcept { return rect_; }
        [[nodiscard]] bool isRectUsable() const noexcept { return rect_.usable(); }
        [[nodiscard]] Rectd boundsRect() const noexcept { return Rectd(rect_.width_, rect_.height_); }
        [[nodiscard]] Rectd contentRect() const noexcept;


        // Style
        [[nodiscard]] bool isOpaque() const noexcept { return true; }
        void setStyleIndex(int32_t index) { style_index_ = index; }
        [[nodiscard]] GUIStyle* guiStyle() const noexcept;


        // Text
        [[nodiscard]] bool hasText() const noexcept { return text_ ? text_->length() > 0 : false; }
        void setText(const char* text_str) noexcept;
        void setText(const String& text) noexcept;
        [[nodiscard]] virtual int32_t textLength() const noexcept {
            return text_ ? static_cast<int32_t>(text_->length()) : 0;
        }

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
            if (receiver_component_) {
                receiver_component_->setByComponent(this);
            }
        }
        virtual void setByComponent(Component* component) noexcept {}
        virtual void setReceiverComponent(Component* component) noexcept { receiver_component_ = component; }
        virtual void setTextField(TextField* textfield) noexcept { textfield_ = textfield; }

        void setAction(ComponentAction action) noexcept { setAction(action, nullptr); }
        void setAction(ComponentAction action, void* action_ref) noexcept {
            action_ = action;
            action_ref_ = action_ref;
        }
        void* actionRef() const noexcept { return action_ref_; }

        void setTextChangedAction(ComponentAction action) noexcept { setTextChangedAction(action, nullptr); }
        void setTextChangedAction(ComponentAction action, void* action_ref) noexcept {
            text_changed_action_ = action;
            text_changed_action_ref_ = action_ref;
        }
        void textChangedAction() noexcept {
            if (text_changed_action_ != nullptr) {
                text_changed_action_(this);
            }
        }

        void setAnimationProgress(double progress) noexcept {
            animation_progress_ = progress;
        }

        [[nodiscard]] double animationProgress() const noexcept {
            return animation_progress_;
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
        [[nodiscard]] bool isHighlighted() const noexcept { return is_highlighted_; }
        virtual void setHighlighted(bool highlighted) noexcept;
        void highlight() noexcept { setHighlighted(true); }
        void deHighlight() noexcept { setHighlighted(false); }

        // Delayed
        [[nodiscard]] bool isDelayed() const noexcept { return is_delayed_; }
        void setDelayed(bool delayed) noexcept { is_delayed_ = delayed; }

        // Value
        [[nodiscard]] virtual Fix value() const noexcept { return Fix{}; }
        virtual bool setValue(const Fix& value) noexcept { return false; }
        virtual void setValueRange(const Fix& min, const Fix& max) noexcept {}
        [[nodiscard]] virtual int32_t valueAsInt32() const noexcept { return 0; }
        [[nodiscard]] virtual double valueAsDouble() const noexcept { return 0; }
        virtual bool setValueInt(int32_t value) noexcept { return setValue(Fix(value)); }
        virtual bool setValueDouble(double value) noexcept { return setValue(Fix(value)); }

        // Event
        virtual void handleEvent(const Event& event) noexcept;
        [[nodiscard]] bool hasHandleEventFunction() const noexcept { return handle_event_func_ != nullptr; }
        void setHandleEventFunction(ComponentHandleEventFunc func, void* ref = nullptr) noexcept;
        bool callHandleEventFunction(const Event& event) noexcept;
        void _interpretKeyEvents(const Event& event) noexcept;

        // Drawing
        virtual void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {}
        virtual void updateBeforeDrawing(const Rectd& dirty_rect) noexcept {}

        // Custom draw function
        [[nodiscard]] bool hasDrawFunction() const noexcept { return draw_func_ != nullptr; }
        void setDrawFunction(ComponentDrawFunc func, void* ref = nullptr) noexcept;
        void callDrawFunction(GraphicContext* gc) noexcept;

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
        virtual void handleMouseDown(const Event& event) noexcept {}
        virtual void handleMouseDrag(const Event& event) noexcept {}
        virtual void handleMouseUp(const Event& event) noexcept {}
        virtual void handleRightMouseDown(const Event& event) noexcept {}
        virtual void handleRightMouseDrag(const Event& event) noexcept {}
        virtual void handleRightMouseUp(const Event& event) noexcept {}
        virtual void handleMouseEntered(const Event& event) noexcept {}
        virtual void handleMouseExited(const Event& event) noexcept {}
        virtual void handleMouseMoved(const Event& event) noexcept {}
        virtual void handleScrollWheel(const Event& event) noexcept {
            if (parent_) {
                parent_->handleScrollWheel(event);
            }
        }
        virtual void handleMagnification(const Event& event) noexcept {
            if (parent_) {
                parent_->handleMagnification(event);
            }
        }
        virtual void handleRotation(const Event& event) noexcept {
            if (parent_) {
                parent_->handleRotation(event);
            }
        }
        virtual void handleKeyDown(const Event& event) noexcept {
            if (parent_) {
                parent_->handleKeyDown(event);
            }
        }

        [[nodiscard]] bool hasAction() const noexcept { return action_ != nullptr; }


        // Geometry
        virtual bool hit(const Vec2d& pos) noexcept;
        virtual bool hit(const Event& event) noexcept;

        // Drawing
        void drawDummy(GraphicContext* gc) const noexcept;
        // void drawRect(GraphicContext* gc, const Rectd& rect) const noexcept;

        // Utils
        static Component* addComponentToView(Component* component, View* view, AddFlags flags = AddFlags::kNone) noexcept;
        void _setParent(Component* parent) noexcept { parent_ = parent; }
        [[nodiscard]] View* parentView() const noexcept { return reinterpret_cast<View*>(parent_); }


        virtual GraphicContext* graphicContextPtr() noexcept;
        GraphicContext* gc() noexcept;
    };

}

#endif // GrainComponent_hpp