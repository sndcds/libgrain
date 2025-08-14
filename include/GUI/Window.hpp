//
//  Window.hpp
//
//  Created by Roald Christesen on from 02.06.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 26.07.2025
//

#ifndef GrainWindow_hpp
#define GrainWindow_hpp

#include "Grain.hpp"
#include "GUI/Style.hpp"
#include "Type/Object.hpp"
#include "2d/Rect.hpp"
#include "String/String.hpp"


namespace Grain {

    class View;
    class Component;
    class Screen;
    class Window;


    class Window : public Object {
        friend class Component;

    public:
        enum class Style {
            Borderless = 0,
            Titled = 1 << 0,
            Closable = 1 << 1,
            Miniaturizable = 1 << 2,
            Resizable = 1 << 3,
            UnifiedTitleAndToolbar = 1 << 4,
            FullScreen = 1 << 5,
            FullSizeContentView = 1 << 6,

            Default = Titled | Closable | Miniaturizable | Resizable
        };

        enum BackingStoreType {
            Retained = 0,
            Nonretained = 1,
            Buffered = 2
        };

    public:
        Window(const char* title, const Rectd& rect, Style style, Screen* screen = nullptr) noexcept;

        virtual ~Window() noexcept;

        const char* className() const noexcept override { return "Window"; }

        friend std::ostream& operator << (std::ostream& os, const Window* o) {
            if (o == nullptr) {
                return os << "Window nullptr";
            }
            else {
                return os << *o;
            }
        }

        friend std::ostream& operator << (std::ostream& os, const Window& o) {
            return os << "Window: " << o.title();
        }

        void* nsWindow() const noexcept { return m_ns_window; }
        void setNSWindow(void* ns_window) noexcept { m_ns_window = ns_window; }

        Rectd boundsRect() const noexcept;
        void setBoundsRect(const Rectd& bounds_rect) noexcept;

        const String title() const noexcept { return m_title; }
        void setTitle(const String& title) noexcept;

        int32_t tag() const noexcept { return m_tag; }
        void setTag(int32_t tag) noexcept { m_tag = tag; }

        void show() noexcept { makeKeyAndOrderFront(); }
        void orderFront() noexcept;
        void makeKeyWindow() noexcept;
        void makeKeyAndOrderFront() noexcept;

        void close() noexcept;


        View* rootView() const noexcept { return m_root_view; }
        Rectd rootViewBoundsRect() const noexcept;
        View* setView(const Rectd& rect) noexcept;
        bool hasComponent(const Component* component) const noexcept;


        void setCanBecomeKeyWindow(bool can_become_key_window) noexcept { m_can_become_key_window = can_become_key_window; }
        bool canBecomeKeyWindow() const noexcept { return m_can_become_key_window; }
        virtual void becomeKeyWindow() noexcept;

        bool isKeyWindow() const noexcept { return m_is_key_window; }
        bool setIsKeyWindow(bool is_key_window) noexcept {
            bool result = m_is_key_window != is_key_window;
            m_is_key_window = is_key_window;
            return result;
        }

        bool handleKeyEvent (uint16_t keyCode, uint32_t keyMask) noexcept;

        void setFirstResponder(Component* component) noexcept;
        void makeFirstResponder(Component* component) noexcept;
        void resignFirstResponder() noexcept;

        void needsDisplay() const noexcept;
        void firstResponderNeedsDisplay() const noexcept;

        void _setRootView(View* view) noexcept;

    private:
        void _init(const char* title, const Rectd& rect, Window::Style style, Screen* screen) noexcept;

    protected:
        String m_title = "Untitled";
        int32_t m_tag = -1;

        void* m_ns_window = nullptr;    ///< Pointer to macOS NSWindow
        View* m_root_view = nullptr;

        bool m_can_become_key_window = true;
        bool m_is_key_window = false;

        // Style
        StyleSet m_style_set;
    };


} // End of namespace Grain

#endif // GrainWindow_hpp
