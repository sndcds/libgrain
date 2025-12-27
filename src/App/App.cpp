//
//  App.cpp
//
//  Created by Roald Christesen on 26.07.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "App/App.hpp"
#include "Core/Hardware.hpp"
#include "Graphic/Font.hpp"
#include "GUI/GUIStyle.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/CairoContext.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include "Graphic/AppleCGContext.hpp"
#endif

#include <SDL2/SDL.h>


namespace Grain {

#if defined(__APPLE__) && defined(__MACH__)
    void _macosApp_start();
    void _macosApp_addMenu();
    void _macosApp_beep();
    void _macosApp_updateScreenInfos(Grain::App* app);
#endif


    App* App::g_instance = nullptr;

    App::App(uint32_t flags) {
        if (App::g_instance) { // Prevent multiple instantiation
            return;
        }

        g_instance = this;

        start_time_.now();

        // Collect information about the computer
        big_endian_ = std::endian::native == std::endian::big;
        physical_core_count_ = Hardware::physicalCores();
        logical_core_count_ = Hardware::logicalCores();
        mem_size_ = Hardware::memSize();

        use_sdl2_ = flags & kAppFlag_SDL2;
        use_cairo_ = flags & kAppFlag_Cairo;
        use_fftw_ = flags & kAppFlag_FFTW;


#if defined(__APPLE__) && defined(__MACH__)
        if (use_cairo_) {
            gc_type_ = GraphicContextType::Cairo;
        }
        else {
            gc_type_ = GraphicContextType::AppleMac;
        }
#else
        gc_type_ =  GraphicContextType::Cairo;
#endif

        updateScreenInfos();

        // Init App fonts
        ui_font_ = new (std::nothrow) Font(16);
        small_ui_font_ = new (std::nothrow) Font(12);
        title_ui_font_ = new (std::nothrow) Font(22);
        mono_font_ = new (std::nothrow) Font("SF Mono", 11); // TODO: Fallback!

        g_instance->_initGUIStyle();
    }


    App::~App() {
        // Release all Screens
        for (int32_t i = 0; i < kMaxScreenCount; i++) {
            delete screens_[i];
        }
    }


    void App::_initGUIStyle() {
        int32_t style_index = App::addGUIStyle();
        if (style_index < 0) {
            std::cerr << "Fatal: could not add GUI style\n";
            exit(EXIT_FAILURE);
        }
    }


    void App::addMenu() {
#if defined(__APPLE__) && defined(__MACH__)
        _macosApp_addMenu();
#endif
    }

    void App::start() {
#if defined(__APPLE__) && defined(__MACH__)
        _macosApp_start();
#endif
    }


    void App::beep() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosApp_beep();
#endif
    }


    GraphicContext* App::createGUIGraphicContext() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        if (g_instance->gc_type_ == GraphicContextType::AppleMac) {
            return new (std::nothrow) AppleCGContext();
        }
        else {
            return new (std::nothrow) CairoContext();
        }
#else
        return new (std::nothrow) CairoContext();
#endif
    }


    void App::updateScreenInfos() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosApp_updateScreenInfos(g_instance);
#endif
    }


    Rectd App::mainScreenRect() noexcept {
        auto screen = g_instance->screenAtIndex(0);
        if (screen) {
            return screen->rect();
        }
        return Rectd();
    }


    Window* App::addWindow(const char* title, const Rectd& rect, Window::Style window_style, Screen* screen) noexcept {
        auto window = new (std::nothrow) Window(title, rect, window_style, screen);

        try {
            if (!window) {
                throw Exception(ErrorCode::NullData, "App::addWindow: Window is null.");
            }

            if (!g_instance->windows_.push(window)) {
                throw Exception(ErrorCode::MemCantGrow, "App::addWindow: Could not add window to list.");
            }
        }
        catch (const Exception& e) {
            GRAIN_RELEASE(window);
            window = nullptr;
        }

        return window;
    }


    void App::setKeyWindow(Window* window) noexcept {
        if (window->canBecomeKeyWindow()) {
            for (auto& w : g_instance->windows_) {
                if (w->setIsKeyWindow(w == window)) {
                    g_instance->key_window_ = window;
                    w->needsDisplay();
                }
            }
        }
    }


    void App::allWindowsNeedsDisplay() noexcept {
        for (auto& window : g_instance->windows_) {
            window->needsDisplay();
        }
    }


} // End of namespace Grain
