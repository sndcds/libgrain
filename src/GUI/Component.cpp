//
//  Component.cpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include <utility>

#include "GUI/Component.hpp"
#include "GUI/View.hpp"
#include "GUI/Event.hpp"
#include "GUI/Components/Textfield.hpp"
#include "App/App.hpp"
#include "String/String.hpp"
#include "Color/RGBA.hpp"
#include "Color/Gradient.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/Font.hpp"


namespace Grain {

    #if defined(__APPLE__) && defined(__MACH__)
        void _macosView_initForUI(Component* component, const Grain::Rectd& rect);
        void _macosView_releaseView(Component* component);
        void _macosView_setNeedsDisplay(const Component* component);
        void _macosView_forcedDisplay(const Component* component);
        void _macosView_selectNextKeyView(Component* component);
        void _macosView_interpretKeyEvent(Component* component, const Event& event);
        void _macosView_setOpacity(Component* component, float opacity);
        bool _macosView_isKeyView(const Component* component);
        bool _macosView_gotoView(Component* component);
        void _macosView_setFrame(Component* component, Rectd& rect);
    #endif


    Component::Component(const Rectd& rect, int32_t tag) noexcept : m_rect(rect), m_tag(tag) {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosView_initForUI(this, rect);
        #endif
    }


    Component::~Component() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosView_releaseView(this);
        #endif

        delete m_text;
    }


    Rectd Component::contentRect() const noexcept {
        Rectd rect = boundsRect();
        auto style = guiStyle();
        if (style) {
            rect.inset(style->paddingTop(), style->paddingRight(), style->paddingBottom(), style->paddingLeft());
        }
        return rect;
    }


    bool Component::setEnabled(Component *component, bool enabled) noexcept {
        return component != nullptr ? component->setEnabled(enabled) : false;
    }


    bool Component::setEnabled(bool enabled) noexcept {
        bool result = false;

        if (enabled != m_is_enabled) {
            m_is_enabled = enabled;

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

        if (m_textfield) {
            m_textfield->setEnabled(enabled);
        }
/*
        if (m_label != nullptr) {
            m_label->setEnabled(enabled);
        }

        if (m_color_well != nullptr) {
            // m_color_well->setEnabled(enabled);  TODO: Uncomment!
        }
        */

        return result;
    }


    GUIStyle* Component::guiStyle() const noexcept {
        return App::guiStyleAtIndex(m_style_index);
    }


    void Component::setText(const char *text_str) noexcept {
        if (text_str == nullptr) {
            text_str = "";
        }

        if (m_text == nullptr) {
            m_text = new(std::nothrow) String();
        }

        if (m_text != nullptr) {
            m_text->set(text_str);
        }

        needsDisplay();
        textChangedAction();
    }


    void Component::setText(const String &text) noexcept {
        setText(text.utf8());
    }


    void Component::setNextKeyComponent(Component* component) noexcept {
        m_next_key_component = component;
        if (component) {
            component->m_previous_key_component = this;
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
        return gotoComponent(m_next_key_component);
    }


    bool Component::gotoPreviousKeyComponent() noexcept {
        std::cout << (long)m_previous_key_component << std::endl;
        return gotoComponent(m_previous_key_component);
    }


    void Component::fireAction(ActionType action_type, const Component *excluded_component) noexcept {
        _m_action_type = action_type;
        updateRepresentations(excluded_component);
        transmit();
        if (m_action != nullptr) {
            m_action(this);
        }
    }


    void Component::fireActionAndDisplay(ActionType action_type, const Component *excluded_component) noexcept {
        fireAction(action_type, excluded_component);
        needsDisplay();
    }


    void Component::updateRepresentations(const Component *excluded_component) noexcept {
        /* !!!!!
        if (m_textfield != nullptr && m_textfield != excluded_component) {
            m_textfield->setValue(value());
        }
        if (m_color_well != nullptr && m_color_well != excluded_component) {
            m_color_well->setByComponent(this);
        }
         */
    }


    void Component::setRect(const Rectd &rect) noexcept {
        Rectd new_rect = rect;
        new_rect.avoidNegativeSize();

        if (new_rect != m_rect) {
            m_rect = new_rect;
            geometryChanged();

            #if defined(__APPLE__) && defined(__MACH__)
                _macosView_setFrame(this, m_rect);
            #endif

            needsDisplay();
        }
    }


    void Component::setEdgeAligned() noexcept {
        setEdgeAligned(Alignment::Center, 0.0f, 0.0f, 0.0f, 0.0f);
    }


    void Component::setEdgeAligned(Alignment alignment, float top, float right, float bottom, float left) noexcept {
        m_edge_alignment = alignment;
        m_margin.m_top = top;
        m_margin.m_right = right;
        m_margin.m_bottom = bottom;
        m_margin.m_left = left;

        parentGeometryChanged();
    }

    void Component::parentGeometryChanged() noexcept {
        if (m_parent != nullptr && m_edge_alignment != Alignment::No) {
            Rectd rect = m_parent->m_rect.edgeAlignedRectRelative(m_edge_alignment, m_margin.m_top, m_margin.m_right, m_margin.m_bottom, m_margin.m_left);
            if (rect != m_rect) {
                setRect(rect);
            }
        }
    }


    void Component::setHighlighted(bool highlighted) noexcept {
        if (m_is_highlighted != highlighted) {
            m_is_highlighted = highlighted;
            needsDisplay();
        }
    }


    void Component::handleEvent(const Event &event) noexcept {
        if (!m_is_enabled) {
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
                m_is_modified_while_mouse_drag = false;
                m_is_modified_since_mouse_down = false;
                m_mouse_precision_mode = event.isControlPressedOnly();
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
                m_mouse_is_in_view = true;
                handleMouseEntered(event);
                break;

            case Event::EventType::MouseExited:
                m_mouse_is_in_view = false;
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


    void Component::setHandleEventFunction(ComponentHandleEventFunc func, void *ref) noexcept {
        _m_handle_event_func = func;
        _m_handle_event_func_ref = ref;
    }


    bool Component::callHandleEventFunction(const Event &event) noexcept {
        if (hasHandleEventFunction()) {
            return _m_handle_event_func(this, event, _m_handle_event_func_ref);
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
        _m_draw_func = func;
        _m_draw_func_ref = ref;
        needsDisplay();
    }


    void Component::callDrawFunction(GraphicContext& gc) noexcept {
        if (hasDrawFunction()) {
            _m_draw_func(gc, this, _m_draw_func_ref);
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


    void Component::drawDummy(Grain::GraphicContext &gc) const noexcept {
        gc.setStrokeColor(RGBA(1, 0, 0, 1));
        gc.setStrokeWidth(2);
        auto rect = boundsRect();
        rect.inset(1.0f);
        gc.strokeRect(rect);
        gc.strokeLine(rect.x(), rect.y(), rect.x2(), rect.y2());
        gc.strokeLine(rect.x(), rect.y2(), rect.x2(), rect.y());
    }


    void Component::drawRect(GraphicContext& gc, const Rectd& rect) const noexcept {
        gc.setFillColor({ 0, 0, 1, 1});
        gc.fillRect(rect);
    }


    Component* Component::addComponentToView(Component* component, View* view, AddFlags flags) noexcept {
        if (component != nullptr && view != nullptr) {
            view->addComponent(component, flags);
        }
        return component;
    }


} // End of namespace Grain