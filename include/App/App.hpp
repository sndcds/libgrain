//
//  App.hpp
//
//  Created by Roald Christesen on 26.07.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
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

    void* ns_app_ = nullptr;    ///< macOS NSApplication
    bool verbose_ = false;      ///< Verbose flag

    String conf_file_path_;     ///< Path to configuration file

    bool use_sdl2_ = false;
    bool use_cairo_ = false;
    bool use_fftw_ = false;

    Timestamp start_time_;
    bool big_endian_ = false;

    int32_t physical_core_count_ = 0;
    int32_t logical_core_count_ = 0;
    size_t mem_size_ = 0;

    // GraphicContext
    GraphicContextType gc_type_ = GraphicContextType::AppleMac;

    // Screens
    ObjectList<Screen*> screens_;           ///< Pointers to screens
    int32_t smallest_screen_index_ = -1;    ///< Index of smalles screen
    int32_t largest_screen_index_ = -1;     ///< Index of larfest screen
    int32_t total_screen_pixel_count_ = 0;  ///< Total amount of pixels on all screens

    // Windows
    ObjectList<Window*> windows_;           ///< List of all windows
    Window* confirm_window_ = nullptr;
    Window* key_window_ = nullptr;          ///< Pointer to current key window

    // Fonts
    Font* ui_font_ = nullptr;
    Font* small_ui_font_ = nullptr;
    Font* title_ui_font_ = nullptr;
    Font* mono_font_ = nullptr;

    // Style
    GUIStyleSet gui_styles_set_;

    // GUI
    float default_corner_radius_ = 5.0f;    // TODO: !!!!!


    timestamp_t double_click_ms_ = 250;     ///< Maximum time for detecting double clicks, default 250 msec ~ 1/4 sec
    float scroll_wheel_speed_ = 4;

public:
    explicit App(uint32_t flags = 0x0);
    ~App();

    [[nodiscard]] static const char* className() noexcept { return "App"; }

    void log(Log& l) const {
        l.header(className());
        l << "start_time: " << start_time_ << Log::endl;
        l << "big_endian: " << l.boolValue(big_endian_) << Log::endl;
        l << "physical_core_count: " << physical_core_count_ << Log::endl;
        l << "logical_core_count: " << logical_core_count_ << Log::endl;
        l << "mem_size: " << (mem_size_ / 1024 / 1024) << " MB, " << (mem_size_ / 1024 / 1024 / 1024) << " GB" << Log::endl;
        l << "screen_count: " << screenCount() << Log::endl;
        l << "smallest_screen_index: " << smallest_screen_index_ << Log::endl;
        l << "largest_screen_index: " << largest_screen_index_ << Log::endl;
        l << "total_screen_pixel_count: " << total_screen_pixel_count_ << Log::endl;
        l << "use DSL2: " << l.boolValue(use_sdl2_) << l.endl;
        l << "use Cairo: " << l.boolValue(use_cairo_) << l.endl;
        l << "use FFTW: " << l.boolValue(use_fftw_) << l.endl;
        l--;
    }

    void _initGUIStyle();


    static void addMenu();
    static void start();

    [[nodiscard]] static App* instance() noexcept { return g_instance; }
    static void beep() noexcept;


    [[nodiscard]] static const String& confFilePath() {
        return g_instance->conf_file_path_;
    }


    // GraphicContext
    static GraphicContextType graphicContextType() noexcept { return g_instance->gc_type_; }
    [[nodiscard]] static GraphicContext* createGUIGraphicContext() noexcept;


    // Screen
    void updateScreenInfos() noexcept;
    static int32_t screenCount() noexcept {
        return static_cast<int32_t>(g_instance->screens_.size());
    }
    [[nodiscard]] static Screen* mainScreen() noexcept {
        return App::screenAtIndex(0);
    }
    [[nodiscard]] static Screen* smallestScreen() noexcept {
        return App::screenAtIndex(g_instance->smallest_screen_index_);
    }
    [[nodiscard]] static Screen* largestScreen() noexcept {
        return App::screenAtIndex(g_instance->largest_screen_index_);
    }
    [[nodiscard]] static Screen* screenAtIndex(int32_t index) noexcept {
        return g_instance->screens_.elementAtIndex(index);
    }
    [[nodiscard]] static int32_t totalScreenPixelCount() noexcept {
        return g_instance->total_screen_pixel_count_;
    }
    [[nodiscard]] static Rectd mainScreenRect() noexcept;

    // Window
    Window* addWindow(const char* title, const Rectd& rect, Window::Style window_style = Window::Style::Default, Screen* screen = nullptr) noexcept;
    [[nodiscard]] int32_t windowCount() const noexcept { return static_cast<int32_t>(windows_.size()); }
    [[nodiscard]] static Window *keyWindow() noexcept { return g_instance->key_window_; }

    // Font
    [[nodiscard]] static Font* uiFont() noexcept { return g_instance->ui_font_; }
    [[nodiscard]] static Font* uiSmallFont() noexcept { return g_instance->small_ui_font_; }
    [[nodiscard]] static Font* uiTitleFont() noexcept { return g_instance->title_ui_font_; }
    [[nodiscard]] static Font* monoFont() noexcept { return g_instance->mono_font_; }

    // GUI
    [[nodiscard]] static float defaultCornerRadius() noexcept { return g_instance->default_corner_radius_; }
    [[nodiscard]] static timestamp_t doubleClickMillis() noexcept { return g_instance->double_click_ms_; }


    // Hardware
    [[nodiscard]] static float scrollWheelSpeed() { return 1.0f; }

    // Style
    [[nodiscard]] static GUIStyle* guiStyleAtIndex(int32_t index) noexcept {
        return g_instance->gui_styles_set_.styleAtIndex(index);
    }
    static int32_t addGUIStyle() {
        return g_instance->gui_styles_set_.addStyle();
    }
    static GUIStyle* addGUIStyle(int32_t& out_index) {
        out_index = g_instance->gui_styles_set_.addStyle();
        return guiStyleAtIndex(out_index);
    }

    static void setKeyWindow(Window* window) noexcept;
    static void allWindowsNeedsDisplay() noexcept;
};


} // End of namespace Grain

#endif