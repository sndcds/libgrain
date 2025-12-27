//
//  Component.cpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include <utility>

#include "GUI/Components/Component.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Event.hpp"
#include "GUI/Components/TextField.hpp"
#include "App/App.hpp"
#include "String/String.hpp"
#include "Color/Gradient.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/AppleCGContext.hpp"
#include "Graphic/Font.hpp"


namespace Grain {
    void Component::fireActionAndDisplay(ActionType action_type, const Component* excluded_component) noexcept {
        fireAction(action_type, excluded_component);
        needsDisplay();
    }
#if defined(__APPLE__) && defined(__MACH__)
    void _macosView_initForUI(Component* component, const Grain::Rectd& rect);
    void _macosView_releaseView(Component* component);
    void _macosView_setNeedsDisplay(const Component* component);
    void _macosView_forcedDisplay(const Component* component);
    void _macosView_selectNextKeyView(Component* component);
    void _macosView_interpretKeyEvent(Component* component, const Event& event);
    void _macosView_setOpacity(Component* component, float opacity);
    void _macosView_setHidden(Component* component, bool hidden);
    bool _macosView_isKeyView(const Component* component);
    bool _macosView_gotoView(Component* component);
    void _macosView_setFrame(Component* component, Rectd& rect);
    void _macosView_setFrameOrigin(Component* component, double x, double y);
    void _macosView_setFrameSize(Component* component, double width, double height);
    void _macosView_updateCGContext(Component* component) noexcept;
#endif


    Component::Component(const Rectd& rect, int32_t tag) noexcept : tag_(tag), rect_(rect) {
#if defined(__APPLE__) && defined(__MACH__)
        _macosView_initForUI(this, rect);
#endif
    }


    Component::~Component() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosView_releaseView(this);
#endif

        delete text_;
    }


    Rectd Component::contentRect() const noexcept {
        Rectd rect = boundsRect();
        auto style = guiStyle();
        if (style) {
            rect.inset(style->paddingTop(), style->paddingRight(), style->paddingBottom(), style->paddingLeft());
        }
        return rect;
    }


    bool Component::setEnabled(Component* component, bool enabled) noexcept {
        return component != nullptr ? component->setEnabled(enabled) : false;
    }


    bool Component::setEnabled(bool enabled) noexcept {
        bool result = false;

        if (enabled != is_enabled_) {
            is_enabled_ = enabled;

#if defined(__APPLE__) && defined(__MACH__)
            _macosView_selectNextKeyView(this);
#endif

            if (componentType() == ComponentType::PopUpButton) {
                /* TODO: macOS...
                [((GrNSPopUpButton*)mNSView) setEnabled:mEnabled];
                 */
            }

            result = true;
            needsDisplay();
        }

        if (textfield_) {
            textfield_->setEnabled(enabled);
        }
        /*
                if (m_label) {
                    m_label->setEnabled(enabled);
                }

                if (m_color_well) {
                    // m_color_well->setEnabled(enabled);  TODO: Uncomment!
                }
                */

        return result;
    }

    void Component::setVisibility(bool visibility) noexcept {
        if (textfield_) {
            textfield_->setVisibility(visibility);
        }

        is_visible_ = visibility;
#if defined(__APPLE__) && defined(__MACH__)
        _macosView_setHidden(this, !is_visible_);
#endif
    }


    GUIStyle* Component::guiStyle() const noexcept {
        return App::guiStyleAtIndex(style_index_);
    }


    void Component::setText(const char* text_str) noexcept {
        if (!text_str) {
            text_str = "";
        }

        if (!text_) {
            text_ = new(std::nothrow) String();
        }

        if (text_) {
            text_->set(text_str);
        }

        needsDisplay();
        textChangedAction();
    }


    void Component::setText(const String& text) noexcept {
        setText(text.utf8());
    }


    void Component::setNextKeyComponent(Component* component) noexcept {
        next_key_component_ = component;
        if (component) {
            component->previous_key_component_ = this;
        }
    }


    bool Component::isKeyComponent() const noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        return _macosView_isKeyView(this);
#else
        // TODO: Implement linux version
        return false;
#endif
    }


    bool Component::gotoComponent(Component* component) noexcept {
        if (component) {
#if defined(__APPLE__) && defined(__MACH__)
            return _macosView_gotoView(component);
#else
            // TODO: Implement linux version
            return false;
#endif
        }
        else {
            return false;
        }
    }


    bool Component::gotoNextKeyComponent() noexcept {
        return gotoComponent(next_key_component_);
    }


    bool Component::gotoPreviousKeyComponent() noexcept {
        std::cout << (long) previous_key_component_ << std::endl;
        return gotoComponent(previous_key_component_);
    }


    void Component::fireAction(ActionType action_type, const Component* excluded_component) noexcept {
        action_type_ = action_type;
        updateRepresentations(excluded_component);
        transmit();
        if (action_) {
            action_(this);
        }
    }


    void Component::updateRepresentations(const Component* excluded_component) noexcept {
        /* !!!!!
        if (m_textfield && m_textfield != excluded_component) {
            m_textfield->setValue(value());
        }
        if (m_color_well && m_color_well != excluded_component) {
            m_color_well->setByComponent(this);
        }
         */
    }


    void Component::setRect(const Rectd& rect) noexcept {
        Rectd new_rect = rect;
        new_rect.avoidNegativeSize();

        if (new_rect != rect_) {
            rect_ = new_rect;
            geometryChanged();

#if defined(__APPLE__) && defined(__MACH__)
            _macosView_setFrame(this, rect_);
#endif

            needsDisplay();
        }
    }

    void Component::setPosition(double x, double y) noexcept {
        if (x != rect_.x_ || y != rect_.y_) {
            rect_.x_ = x;
            rect_.y_ = y;

#if defined(__APPLE__) && defined(__MACH__)
            _macosView_setFrameOrigin(this, x, y);
#endif

            needsDisplay();
        }
    }


    void Component::setDimension(double width, double height) noexcept {
        if (width < 0) {
            width = 0;
        }
        if (height < 0) {
            height = 0;
        }
        if (width != rect_.width_ || height != rect_.height_) {
            rect_.width_ = width;
            rect_.height_ = height;

#if defined(__APPLE__) && defined(__MACH__)
            _macosView_setFrameSize(this, width, height);
#endif

            geometryChanged();
            needsDisplay();
        }
    }

    void Component::setEdgeAligned() noexcept {
        setEdgeAligned(Alignment::Center, 0.0f, 0.0f, 0.0f, 0.0f);
    }

    void Component::setEdgeAligned(Alignment alignment, float top, float right, float bottom, float left) noexcept {
        edge_alignment_ = alignment;
        margin_.top_ = top;
        margin_.right_ = right;
        margin_.bottom_ = bottom;
        margin_.left_ = left;

        parentGeometryChanged();
    }

    void Component::parentGeometryChanged() noexcept {
        if (parent_ && edge_alignment_ != Alignment::No) {
            Rectd rect =
                parent_->rect_.edgeAlignedRectRelative(
                    edge_alignment_, margin_.top_, margin_.right_,
                    margin_.bottom_, margin_.left_);
            if (rect != rect_) {
                setRect(rect);
            }
        }
    }

    void Component::setHighlighted(bool highlighted) noexcept {
        if (is_highlighted_ != highlighted) {
            is_highlighted_ = highlighted;
            needsDisplay();
        }
    }

    void Component::handleEvent(const Event& event) noexcept {
        if (!is_enabled_) {
            return;
        }

        if (hasHandleEventFunction()) {
            if (callHandleEventFunction(event)) {
                return;
            }
        }

        Event::EventType event_type = event.type();

        switch (event_type) {
            case Event::EventType::MouseDown:
            case Event::EventType::RightMouseDown:
                is_modified_while_mouse_drag_ = false;
                is_modified_since_mouse_down_ = false;
                mouse_precision_mode_ = event.isControlPressedOnly();
                updateAtMouseDown(event);
                if (event_type == Event::EventType::RightMouseDown) {
                    handleRightMouseDown(event);
                }
                else {
                    handleMouseDown(event);
                }
                break;

            case Event::EventType::MouseDrag:
                handleMouseDrag(event);
                break;

            case Event::EventType::RightMouseDrag:
                handleRightMouseDrag(event);
                break;

            case Event::EventType::MouseUp:
                handleMouseUp(event);
                event.mousePressedFinished();
                break;

            case Event::EventType::RightMouseUp:
                handleRightMouseUp(event);
                event.mousePressedFinished();
                break;

            case Event::EventType::MouseEntered:
                mouse_is_in_view_ = true;
                handleMouseEntered(event);
                break;

            case Event::EventType::MouseExited:
                mouse_is_in_view_ = false;
                handleMouseExited(event);
                break;

            case Event::EventType::MouseMoved:
                handleMouseMoved(event);
                break;

            case Event::EventType::ScrollWheel:
                handleScrollWheel(event);
                break;

            case Event::EventType::Magnification:
                handleMagnification(event);
                break;

            case Event::EventType::Rotation:
                handleRotation(event);
                break;

            case Event::EventType::KeyDown:
                handleKeyDown(event);
                break;

            default:
                break;
        }
    }


    void Component::setHandleEventFunction(ComponentHandleEventFunc func, void* ref) noexcept {
        handle_event_func_ = func;
        handle_event_func_ref_ = ref;
    }


    bool Component::callHandleEventFunction(const Event& event) noexcept {
        if (hasHandleEventFunction()) {
            return handle_event_func_(this, event, handle_event_func_ref_);
        }
        else {
            return false;
        }
    }

    void Component::_interpretKeyEvents(const Event& event) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosView_interpretKeyEvent(this, event);
#else
        // TODO: Implement for Linux
#endif
    }


    void Component::setDrawFunction(ComponentDrawFunc func, void* ref) noexcept {
        draw_func_ = func;
        draw_func_ref_ = ref;
        needsDisplay();
    }


    void Component::callDrawFunction(GraphicContext* gc) noexcept {
        if (gc && hasDrawFunction()) {
            draw_func_(gc, this, draw_func_ref_);
        }
    }


    void Component::needsDisplay() const noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosView_setNeedsDisplay(this);
#endif
    }


    void Component::forcedDisplay() const noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosView_forcedDisplay(this);
#endif
    }


    bool Component::hit(const Vec2d& pos) noexcept {
        return boundsRect().contains(pos);
    }


    bool Component::hit(const Event& event) noexcept {
        return hit(event.mousePos());
    }


    void Component::drawDummy(Grain::GraphicContext* gc) const noexcept {
        gc->setStrokeColor(1, 0, 0, 1);
        gc->setStrokeWidth(2);
        auto rect = boundsRect();
        rect.inset(1.0f);
        gc->strokeRect(rect);
        gc->strokeLine(rect.x(), rect.y(), rect.x2(), rect.y2());
        gc->strokeLine(rect.x(), rect.y2(), rect.x2(), rect.y());
    }


    /*
    void Component::drawRect(GraphicContext* gc, const Rectd& rect) const noexcept {
        gc->setFillColor(0, 0, 1, 1);
        gc->fillRect(rect);
    }
    */

    Component* Component::addComponentToView(Component* component, View* view, AddFlags flags) noexcept {
        if (component && view) {
            view->addComponent(component, flags);
        }
        return component;
    }

    GraphicContext* Component::graphicContextPtr() noexcept {
        auto component = this;
        while (!component->m_gc_ptr) {
            if (component->parent_) {
                component = component->parent_;
            }
            else {
                return nullptr;
            }
        }
        component->m_gc_ptr->setComponent(component);

#if defined(__APPLE__) && defined(__MACH__)
        _macosView_updateCGContext(this);
#endif

        return component->m_gc_ptr;
    }

/*
    GraphicContext* Component::graphicContextPtr() noexcept {
        auto component = this;
        while (!component->m_gc_ptr) {
            if (component->parent_) {
                component = component->parent_;
            }
            else {
                return nullptr;
            }
        }
        component->m_gc_ptr->setComponent(component);
        return component->m_gc_ptr;
    }
    */
} // End of namespace Grain
