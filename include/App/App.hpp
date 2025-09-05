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
#include "GUI/GUIStyle.hpp"
#include "String/String.hpp"
#include "Type/List.hpp"
#include "Core/Log.hpp"


namespace Grain {

    class Font;
    class GraphicContext;

    enum {
        kAppFlag_SDL2 = 0x1,        // Windowing and Events by SDL2, https://www.libsdl.org/
        kAppFlag_Cairo = 0x1 << 1,  // Render with Cairo, https://cairographics.org/
        kAppFlag_FFTW = 0x1 << 2,   // Use FFTW, https://www.fftw.org/
    };

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

        bool m_use_sdl2 = false;
        bool m_use_cairo = false;
        bool m_use_fftw = false;

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

        // GraphicContext
        GraphicContextType m_gc_type = GraphicContextType::AppleMac;

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
        GUIStyleSet m_gui_styles_set;

        // GUI
        float m_default_corner_radius = 5.0f;   // TODO: !!!!!


        timestamp_t m_double_click_ms = 250;    ///< Maximum time for detecting double clicks, default 250 msec ~ 1/4 sec
        float m_scroll_wheel_speed = 4;

    public:
        explicit App(uint32_t flags = 0x0);
        ~App();

        [[nodiscard]] static const char* className() noexcept { return "App"; }

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
            l << "use DSL2: " << l.boolValue(m_use_sdl2) << l.endl;
            l << "use Cairo: " << l.boolValue(m_use_cairo) << l.endl;
            l << "use FFTW: " << l.boolValue(m_use_fftw) << l.endl;
            l--;
        }

        void _initGUIStyle();


        static void addMenu();
        static void start();

        [[nodiscard]] static App* instance() noexcept { return g_instance; }
        static void beep() noexcept;


        [[nodiscard]] static const String& confFilePath() {
            return g_instance->m_conf_file_path;
        }


        // GraphicContext
        static GraphicContextType graphicContextType() noexcept { return g_instance->m_gc_type; }
        [[nodiscard]] static GraphicContext* createGUIGraphicContext() noexcept;


        // Screen
        void updateScreenInfos() noexcept;
        static int32_t screenCount() noexcept {
            return static_cast<int32_t>(g_instance->m_screens.size());
        }
        [[nodiscard]] static Screen* mainScreen() noexcept {
            return App::screenAtIndex(0);
        }
        [[nodiscard]] static Screen* smallestScreen() noexcept {
            return App::screenAtIndex(g_instance->m_smallest_screen_index);
        }
        [[nodiscard]] static Screen* largestScreen() noexcept {
            return App::screenAtIndex(g_instance->m_largest_screen_index);
        }
        [[nodiscard]] static Screen* screenAtIndex(int32_t index) noexcept {
            return g_instance->m_screens.elementAtIndex(index);
        }
        [[nodiscard]] static int32_t totalScreenPixelCount() noexcept {
            return g_instance->m_total_screen_pixel_count;
        }
        [[nodiscard]] static Rectd mainScreenRect() noexcept;

        // Window
        Window* addWindow(const char* title, const Rectd& rect, Window::Style window_style = Window::Style::Default, Screen* screen = nullptr) noexcept;
        [[nodiscard]] int32_t windowCount() const noexcept { return static_cast<int32_t>(m_windows.size()); }
        [[nodiscard]] static Window *keyWindow() noexcept { return g_instance->m_key_window; }

        // Font
        [[nodiscard]] static Font* uiFont() noexcept { return g_instance->m_ui_font; }
        [[nodiscard]] static Font* uiSmallFont() noexcept { return g_instance->m_small_ui_font; }
        [[nodiscard]] static Font* uiTitleFont() noexcept { return g_instance->m_title_ui_font; }
        [[nodiscard]] static Font* monoFont() noexcept { return g_instance->m_mono_font; }

        // GUI
        [[nodiscard]] static float defaultCornerRadius() noexcept { return g_instance->m_default_corner_radius; }
        [[nodiscard]] static timestamp_t doubleClickMillis() noexcept { return g_instance->m_double_click_ms; }


        // Hardware
        [[nodiscard]] static float scrollWheelSpeed() { return 1.0f; }

        // Style
        [[nodiscard]] static GUIStyle* guiStyleAtIndex(int32_t index) noexcept {
            return g_instance->m_gui_styles_set.styleAtIndex(index);
        }
        static int32_t addGUIStyle() {
            return g_instance->m_gui_styles_set.addStyle();
        }
        static GUIStyle* addGUIStyle(int32_t& out_index) {
            out_index = g_instance->m_gui_styles_set.addStyle();
            return guiStyleAtIndex(out_index);
        }

        static void setKeyWindow(Window* window) noexcept;
        static void allWindowsNeedsDisplay() noexcept;
    };

} // End of namespace Grain

#endif