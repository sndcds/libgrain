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


namespace Grain {

    #if defined(__APPLE__) && defined(__MACH__)
        void _macosApp_start();
        void _macosApp_addMenu();
        void _macosApp_beep();
        void _macosApp_updateScreenInfos(Grain::App* app);
    #endif


    App* App::g_instance = nullptr;

    String App::g_app_path;
    String App::g_home_dir_path;
    String App::g_desktop_dir_path;
    String App::g_document_dir_path;
    String App::g_app_support_dir_path;
    String App::g_app_data_dir_path;
    String App::g_app_test_data_dir_path;


    App::App() {
        if (App::g_instance) { // Prevent multiple instantiation
            return;
        }

        g_instance = this;

        m_start_time.now();

        // Collect information about the computer
        m_big_endian = std::endian::native == std::endian::big;
        m_physical_core_count = Hardware::physicalCores();
        m_logical_core_count = Hardware::logicalCores();
        m_mem_size = Hardware::memSize();

        updateScreenInfos();

        // Fonts
        m_ui_font = new (std::nothrow) Font(16);
        m_small_ui_font = new (std::nothrow) Font(12);
        m_title_ui_font = new (std::nothrow) Font(22);
        m_mono_font = new (std::nothrow) Font("SF Mono", 11); // TODO: Fallback!

        g_instance->_initGUIStyle();
    }


    App::~App() {
        // Release all Screens
        for (int32_t i = 0; i < kMaxScreenCount; i++) {
            delete m_screens[i];
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


    void App::updateScreenInfos() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            _macosApp_updateScreenInfos(g_instance);
        #endif
    }


    Rectd App::mainScreenRect() noexcept {
        auto screen = g_instance->screenAtIndex(0);
        if (screen != nullptr) {
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

            if (!g_instance->m_windows.push(window)) {
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
            for (auto& w : g_instance->m_windows) {
                if (w->setIsKeyWindow(w == window)) {
                    g_instance->m_key_window = window;
                    w->needsDisplay();
                }
            }
        }
    }


    void App::allWindowsNeedsDisplay() noexcept {
        for (auto& window : g_instance->m_windows) {
            window->needsDisplay();
        }
    }


} // End of namespace Grain
