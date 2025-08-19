//
//  Window.cpp
//
//  Created by Roald Christesen on from 02.06.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Window.hpp"
#include "GUI/Screen.hpp"
#include "GUI/View.hpp"
#include "App/App.hpp"
#include "Math/Vec2.hpp"
#include "2d/Rect.hpp"


namespace Grain {

    #if defined(__APPLE__) && defined(__MACH__)
        void _macosWindow_init(Window* window, const char* title, const Rectd& rect, Window::Style style, Screen* screen);
        void _macosWindow_setFrame(const Window* window, const Rectd& rect);
        void _macosWindow_setTitle(const Window* window, const String& title);
        void _macosWindow_release(Window* window);
        void _macosWindow_orderFront(Window* window);
        void _macosWindow_makeKeyWindow(Window* window);
        void _macosWindow_makeKeyAndOrderFront(Window* window);
        void _macosWindow_setFirstResponder(Window* window, Component* component);
        void _macosWindow_makeFirstResponder(Window* window, Component* component);
        void _macosWindow_close(Window* window);
        Rectd _macosWindow_bounds(const Window* window);
    #endif


    Window::Window(const char* title, const Rectd& rect, Window::Style style, Screen* screen) noexcept : Object() {
        if (screen != nullptr) {
            screen = App::mainScreen();
        }
        _init(title, rect, style, screen);
    }


    Window::~Window() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_release(this);
        #endif
    }


    void Window::_setRootView(View *view) noexcept {
        m_root_view = view;
    }


    void Window::_init(const char* title, const Rectd& rect, Window::Style style, Screen* screen) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_init(this, title, rect, style, screen);
        #endif
    }


    Rectd Window::boundsRect() const noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            return _macosWindow_bounds(this);
        #else
            return Rectd();
        #endif
    }


    void Window::setBoundsRect(const Rectd& bounds_rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_setFrame(this, bounds_rect);
        #endif
    }


    void Window::setTitle(const String& title) noexcept {
        m_title = title;
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_setTitle(this, title);
        #endif
    }


    void Window::orderFront() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_orderFront(this);
        #endif
    }


    void Window::makeKeyWindow() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_makeKeyWindow(this);
        #endif
    }


    void Window::makeKeyAndOrderFront() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_makeKeyAndOrderFront(this);
        #endif
    }


    void Window::close() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_close(this);
        #endif
    }


    Rectd Window::rootViewBoundsRect() const noexcept {
        if (m_root_view != nullptr) {
            return m_root_view->boundsRect();
        }
        else {
            return Rectd();
        }
    }


    View* Window::setView(const Rectd& rect) noexcept {
        return m_root_view->addView(rect);
    }


    bool Window::hasComponent(const Component* component) const noexcept {
        if (m_root_view != nullptr) {
            if (m_root_view->hasDescendant(component)) {
                return true;
            }
        }
        return false;
    }


    void Window::setFirstResponder(Component* component) noexcept {
        if (component != nullptr) {
            #if defined(__APPLE__) && defined(__MACH__)
                _macosWindow_setFirstResponder(this, component);
            #endif
        }
    }


    void Window::makeFirstResponder(Component* component) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosWindow_makeFirstResponder(this, component);
        #endif
    }


    void Window::becomeKeyWindow() noexcept {
        App::setKeyWindow(this);
        m_is_key_window = true;
    }


    void Window::resignFirstResponder() noexcept {
        makeFirstResponder(nullptr);
        needsDisplay();
    }


    void Window::needsDisplay() const noexcept {
        if (m_root_view != nullptr) {
            m_root_view->needsDisplay();
        }
    }


    void Window::firstResponderNeedsDisplay() const noexcept {
        if (m_root_view != nullptr) {
            m_root_view->needsDisplay();
        }
    }


} // End of namespace Grain
