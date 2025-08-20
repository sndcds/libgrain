//
//  View.cpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Views/View.hpp"
#include "Graphic/GraphicContext.hpp"
#include "App/App.hpp"


namespace Grain {

    #if defined(__APPLE__) && defined(__MACH__)
        void _macosView_addComponent(Component* paren, Component* component, Component::AddFlags flags);
        void _macosView_removeFromSuperview(Component* component);
    #endif


    View::View(int32_t tag) noexcept : Component(tag) {
        _init(Rectd(0, 0, 100, 100));
    }


    View::View(const Rectd& rect, int32_t tag) noexcept : Component(rect, tag) {
        _init(rect);
    }


    View::~View() noexcept {
        for (auto component : m_components) {
            delete component;
        }

        /* TODO: !!!!! macOS_view!!!
        if (m_ns_view != nil) {
            [(GrNSView*)m_ns_view removeFromSuperviewWithoutNeedingDisplay];
            [(GrNSView*)m_ns_view release];
        }
         */
    }


    void View::_init(const Rectd& rect) noexcept {
        m_type = Component::ComponentType::View;
        m_can_have_children = true;
    }


    View* View::addView(const Rectd& rect) noexcept {
        View* view = new(std::nothrow) View(rect);
        addComponent(view, AddFlags::kWantsLayer);
        return view;
    }


    Component* View::addComponent(Component* component, AddFlags flags) noexcept {
        if (component != nullptr) {
            component->_setParent(this);
            m_components.push(component);

            if (component->canGetFocus()) {
                if (m_first_key_component != nullptr) {
                    m_first_key_component = component;
                }

                if (m_curr_key_component != nullptr) {
                    m_curr_key_component->setNextKeyComponent(component);
                    component->setNextKeyComponent(m_first_key_component);
                }

                m_curr_key_component = component;

                if (m_split_view_flag) {
                    component->setEdgeAligned();
                }
            }

            #if defined(__APPLE__) && defined(__MACH__)
                _macosView_addComponent(this, component, flags);
            #endif
        }

        return component;
    }


    void View::removeComponent(Component* component) noexcept {
        if (component) {
            int32_t index = 0;

            for (auto c : m_components) {
                if (c == component) {
                    // TODO: !!!!! Check mFirstKeyComponent and mCurrKeyComponent ...
                    // if (c == mFirstKeyComponent)
                    // if (c == mCurrKeyComponent)
                    m_components.removeAtIndex(index);
                    break;
                }
                index++;
            }

            #if defined(__APPLE__) && defined(__MACH__)
                _macosView_removeFromSuperview(component);
            #endif
        }
    }


    bool View::hasDescendant(const Component *component) noexcept {
        for (auto c : m_components) {
            if (c == component) {
                return true;
            }
            if (c->hasDescendant(component)) {
                return true;
            }
        }

        return false;
    }


    void View::deselectRadioGroup(int32_t radio_group) noexcept {
        if (radio_group > 0) {
            for (auto component : m_components) {
                if (component->radioGroup() == radio_group) {
                    component->deselectWithoutChecking();
                }
            }
        }
    }


    void View::geometryChanged() noexcept {
        for (auto component : m_components) {
            component->parentGeometryChanged();
        }
    }


    void View::draw(const Rectd& dirty_rect) noexcept {
        GraphicContext gc(this);

        auto style = App::guiStyleAtIndex(m_style_index);
        if (style) {
            Rectd bounds_rect = boundsRect();

            if (m_fills_bg) {
                gc.setFillRGBA(style->viewColor());
                gc.fillRect(bounds_rect);
            }
            else {
                gc.setFillClearColor();
                gc.fillRect(bounds_rect);
            }
        }


        callDrawFunction(gc);
    }


} // End of namespace Grain