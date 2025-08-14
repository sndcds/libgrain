//
//  App.hpp
//
//  Created by Roald Christesen on 26.07.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 26.07.2025
//

#ifndef GrainApp_hpp
#define GrainApp_hpp

#include "GUI/Screen.hpp"
#include "GUI/Window.hpp"
#include "GUI/Style.hpp"
#include "String/String.hpp"
#include "Type/List.hpp"
#include "Core/Log.hpp"


namespace Grain {

    class Font;


    enum {
        kMaxScreenCount = 32    ///< Maximum number of screens
    };

    enum class ConfirmButton {
        Undefined = 1,
        Cancel = 0,
        OK = 1
    };

    enum class MouseCursor {
        Undefined = -1,
        Arrow,
        IBeam,
        PointingHand,
        ClosedHand,
        OpenHand,
        ResizeLeft, ResizeRight, ResizeLeftRight,
        ResizeUp, ResizeDown, ResizeUpDown,
        Crosshair,
        DisappearingItem,
        OperationNotAllowed,
        DragLink,
        DragCopy,
        ContextualMenu,
        IBeamVertical
    };

    class App {
    public:
        static App* g_instance;     ///< Global App singleton instance
        void* m_ns_app = nullptr;   ///< macOS NSApplication
        bool m_verbose = false;     ///< Verbose flag

        String m_conf_file_path;    ///< Path to configuration file

        // File System
        static String g_app_path;
        static String g_home_dir_path;
        static String g_desktop_dir_path;
        static String g_document_dir_path;
        static String g_app_support_dir_path;
        static String g_app_data_dir_path;
        static String g_app_test_data_dir_path;

        Timestamp m_start_time;
        bool m_big_endian = false;

        int32_t m_physical_core_count = 0;
        int32_t m_logical_core_count = 0;
        size_t m_mem_size = 0;

        // Screens
        ObjectList<Screen*> m_screens;          ///< Pointers to screens
        int32_t m_smallest_screen_index = -1;   ///< Index of smalles screen
        int32_t m_largest_screen_index = -1;    ///< Index of larfest screen
        int32_t m_total_screen_pixel_count = 0; ///< Total amount of pixels on all screens

        // Windows
        ObjectList<Window*> m_windows;          ///< List of all windows
        Window* m_confirm_window = nullptr;
        Window* m_key_window = nullptr;         ///< Pointer to current key window

        // Fonts
        Font* m_ui_font = nullptr;
        Font* m_small_ui_font = nullptr;
        Font* m_title_ui_font = nullptr;
        Font* m_mono_font = nullptr;

        // Style
        StyleSet m_style_set;

        // GUI
        float m_default_corner_radius = 5.0f;   // !!!!!!!!!!

        timestamp_t m_double_click_ms = 250;    ///< Maximum time for detecting double clicks, default 250 msec ~ 1/4 sec
        float m_scroll_wheel_speed = 4;

    public:
        App();
        ~App();

        [[nodiscard]] const char* className() const noexcept { return "App"; }

        void log(Log& l) const {
            l.header(className());
            l << "start_time: " << m_start_time << Log::endl;
            l << "big_endian: " << l.boolValue(m_big_endian) << Log::endl;
            l << "physical_core_count: " << m_physical_core_count << Log::endl;
            l << "logical_core_count: " << m_logical_core_count << Log::endl;
            l << "mem_size: " << (m_mem_size / 1024 / 1024) << " MB, " << (m_mem_size / 1024 / 1024 / 1024) << " GB" << Log::endl;
            l << "screen_count: " << screenCount() << Log::endl;
            l << "smallest_screen_index: " << m_smallest_screen_index << Log::endl;
            l << "largest_screen_index: " << m_largest_screen_index << Log::endl;
            l << "total_screen_pixel_count: " << m_total_screen_pixel_count << Log::endl;
            l--;
        }


        static void addMenu();
        static void start();

        static App* instance() noexcept { return g_instance; }
        static void beep() noexcept;


        static String& confFilePath() {
            return g_instance->m_conf_file_path;
        }


        // Screen
        void updateScreenInfos() noexcept;
        static int32_t screenCount() noexcept {
            return static_cast<int32_t>(g_instance->m_screens.size());
        }
        static Screen* mainScreen() noexcept {
            return App::screenAtIndex(0);
        }
        static Screen* smallestScreen() noexcept {
            return App::screenAtIndex(g_instance->m_smallest_screen_index);
        }
        static Screen* largestScreen() noexcept {
            return App::screenAtIndex(g_instance->m_largest_screen_index);
        }
        static Screen* screenAtIndex(int32_t index) noexcept {
            return g_instance->m_screens.elementAtIndex(index);
        }
        static int32_t totalScreenPixelCount() noexcept {
            return g_instance->m_total_screen_pixel_count;
        }
        static Rectd mainScreenRect() noexcept;

        // Window
        Window* addWindow(const char* title, const Rectd& rect, Window::Style window_style, Screen* screen) noexcept;
        [[nodiscard]] int32_t windowCount() const noexcept { return static_cast<int32_t>(m_windows.size()); }
        static Window *keyWindow() noexcept { return g_instance->m_key_window; }

        // Font
        static Font* uiFont() noexcept { return g_instance->m_ui_font; }
        static Font* uiSmallFont() noexcept { return g_instance->m_small_ui_font; }
        static Font* uiTitleFont() noexcept { return g_instance->m_title_ui_font; }
        static Font* monoFont() noexcept { return g_instance->m_mono_font; }

        // GUI
        static float defaultCornerRadius() noexcept { return g_instance->m_default_corner_radius; }

        // Style
        static StyleSet* styleSet() noexcept { return &g_instance->m_style_set; }

        static void setKeyWindow(Window* window) noexcept;
        static void allWindowsNeedsDisplay() noexcept;

        static timestamp_t doubleClickMillis() noexcept { return g_instance->m_double_click_ms; }
    };

} // End of namespace Grain

#endif