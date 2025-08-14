//
//  Gradient.hpp
//
//  Created by Roald Christesen on from 18.01.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainGradient_hpp
#define GrainGradient_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "Color/RGB.hpp"
#include "Color/RGBA.hpp"
#include "2d/Rect.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreGraphics/CoreGraphics.h>
#endif


namespace Grain {

    class RGBLUT1;
    class GraphicContext;


    /**
     *  @brief A single stop in a gradient definition.
     *
     *  A GradientStop represents a specific point within a gradient and defines
     *  the color (or colors) and behavior at that position. Gradient stops are
     *  used by the Gradient class to construct complex color transitions.
     *
     *  Each stop includes its relative position in the [0, 1] range, a color (or two,
     *  if `m_two_colored` is true), optional level curve parameters for non-linear
     *  interpolation, and support for stepped color transitions.
     */
    class GradientStop : public Object {

        friend class Gradient;

    public:
        enum {
            kBlendRGB = 0,
            kBlendMixbox
        };

    protected:
        float m_pos = 0;  ///< Position of the stop within the gradient range [0, 1].

        RGBA m_colors[2] = { { 0, 0, 0, 1 }, { 0, 0, 0, 1 } };
        //!< Colors at the stop. If `m_two_colored` is false, only m_colors[0] is used.
        //!< If `m_two_colored` is true, m_colors[0] and m_colors[1] represent the left and right sides.

        int32_t m_level_curve_values[6] = { 0, 5000, 0, 5000, 10000, 10000 };
        //!< Parameters for the level curve; values are scaled integers (scale: 1:10000).

        int32_t m_step_count = 0;       ///< Number of steps for discrete (stepped) gradient segments.
        bool m_two_colored = false;     ///< If true, the stop uses two colors with a hard transition at `m_pos`.
        int32_t blend_mode = kBlendRGB; ///< Blend mode for this spot in which it blends into the next color

        // GUI-related properties
        bool m_selected = false;       ///< Indicates if the stop is selected in the UI.
        bool m_second_active = false;  ///< Indicates if the second color is selected in the UI.
        float m_remembered_pos = 0.0f; ///< Used to remember the position during drag/move interactions.

    public:
        GradientStop() noexcept;
        GradientStop(float pos, const RGBA& color) noexcept;
        GradientStop(float pos, const RGBA& color1, const RGBA& color2) noexcept;
        GradientStop(const GradientStop* stop) noexcept;
        ~GradientStop() noexcept;

        const char* className() const noexcept override { return "GradientStop"; }

        friend std::ostream& operator << (std::ostream& os, const GradientStop* o) {
            o == nullptr ? os << "GradientStop nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GradientStop& o) {
            os << o.m_pos << ", " << o.m_colors[0] << ", " << o.m_colors[1];
            for (int32_t i = 0; i < 6; i++) { os << ", " << o.m_level_curve_values[i]; }
            os << ", " << o.m_step_count << ", " << o.m_two_colored;
            return os;
        }


        // Compare operators
        bool operator < (const GradientStop& other) const { return m_pos < other.m_pos; }
        bool operator == (const GradientStop& other) const { return m_pos == other.m_pos; }
        bool operator != (const GradientStop& other) const { return m_pos != other.m_pos; }
        bool operator > (const GradientStop& other) const { return m_pos > other.m_pos; }
        bool operator >= (const GradientStop& other) const { return m_pos >= other.m_pos; }
        bool operator <= (const GradientStop& other) const { return m_pos <= other.m_pos; }


        void init() noexcept {
            m_colors[0] = RGBA::kBlack;
            m_colors[1] = RGBA::kBlack;
            m_level_curve_values[0] = 0;
            m_level_curve_values[1] = 5000;
            m_level_curve_values[2] = 0;
            m_level_curve_values[3] = 5000;
            m_level_curve_values[4] = 10000;
            m_level_curve_values[5] = 10000;
            m_step_count = 0;
            m_two_colored = false;
            m_selected = false;
        }

        float pos() const noexcept { return m_pos; }
        RGBA color(int32_t part) const noexcept { return part >= 0 && part <= 1 ? m_colors[part] : RGBA::kBlack; }
        RGBA leftColor() const noexcept { return m_colors[0]; }
        RGBA rightColor() const noexcept { return m_two_colored ? m_colors[1] : m_colors[0]; }
        bool isSelected() const noexcept { return m_selected; }
        bool isTwoColored() const noexcept { return m_two_colored; }
        bool isSecondActive() const noexcept { return m_second_active; }
        int32_t stepCount() const noexcept { return m_step_count; }
        float rememberedPos() const noexcept { return m_remembered_pos; }

        bool setPos(float pos) noexcept {
            if (pos != m_pos) {
                m_pos = pos;
                return true;
            }
            return false;
        }

        bool setColor(int32_t part, const RGB& color) noexcept {
            if (part >= 0 && part <= 1 && color != m_colors[part]) {
                m_colors[part] = color;
                return true;
            }
            return false;
        }

        bool setColor(int32_t part, const RGBA& color) noexcept {
            if (part >= 0 && part <= 1 && color != m_colors[part]) {
                m_colors[part] = color;
                return true;
            }
            return false;
        }

        bool setStepCount(int32_t step_count) noexcept {
            if (step_count != m_step_count) {
                m_step_count = step_count;
                return true;
            }
            return false;
        }

        void select() noexcept { m_selected = true; }
        void deselect() noexcept { m_selected = false; }
        void toggleSelection() noexcept { m_selected = !m_selected; }

        void remember() noexcept { m_remembered_pos = m_pos; }
    };


    /**
     *  @brief Color gradient.
     *
     *  Color gradients are versatile tools in computer programs, allowing designers, developers, and
     *  artists to create visually engaging and dynamic content. They serve to enhance aesthetics,
     *  improve user experience, and convey information effectively. Gradients are an essential element
     *  of modern graphic design and are used in a wide range of applications, from web design to
     *  digital art and beyond.
     */
    class Gradient : public Object {

        friend class GradientStop;
        friend class GradientControl;

    public:
        enum {
            kErrNSGradientCreationFailed = 0
        };

        enum class ColorSpace {
            sRGB = 0,
            LinearRGB
        };

        enum class Preset {
            Fast = 0,
            Blackbody,
            Viridis,
            Inferno,
            Magma,
            Plasma,

            First = Fast,
            Last = Plasma
        };


    protected:
        ColorSpace m_color_space = ColorSpace::sRGB;

        List<GradientStop> m_stops;             ///< An array of gradient stops
        int32_t m_lut_resolution = 512;         ///< Number of color samples for representing the gradient as a lookup table (LUT)
        RGBLUT1* m_lut = nullptr;               ///< The LUT, will be created, as soon it is needed
        bool m_must_sort = false;               ///< Indicates, if stops must be sorted befor usage
        bool m_lut_must_update = false;         ///< Indicates, if LUT must be updated before usage

        #if defined(__APPLE__) && defined(__MACH__)
            CGGradientRef m_cg_gradient = nullptr;  ///< CoreGraphics representation for internal use
            int32_t m_cg_resolution = 20;           ///< Define resolution for smoothing, when using CoreGraphics
            bool m_cg_gradient_must_update = true;  ///< Indicates, that CoreGraphics representation must be updated
        #endif


    public:
        Gradient() noexcept;
        Gradient(const Gradient* gradient) noexcept;
        ~Gradient() noexcept;

        const char* className() const noexcept override { return "Gradient"; }

        friend std::ostream& operator << (std::ostream& os, const Gradient* o) {
            o == nullptr ? os << "Gradient nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Gradient& o) {
            os << (int32_t)o.m_color_space << ", " << o.m_stops.size() << ", " << o.m_lut_resolution;
            os << ", " << o.m_must_sort << ", " << o.m_lut_must_update;
            return os;
        }

        static Gradient* createByPreset(Preset preset, bool flip = false) noexcept;

        void set(const Gradient* gradient) noexcept;
        void setColorSpace(ColorSpace color_space) noexcept {
            if (color_space != m_color_space) {
                m_color_space = color_space;
                needsUpdate();
            }
        }

        void buildKelvinGradient(float k0, float k1, float s, float v, int32_t resolution = 16) noexcept;
        void buildHueGradient(float s, float v, int32_t resolution = 16) noexcept;
        void buildHSVGradient(const HSV& hsv1, const HSV& hsv2) noexcept;

        bool hasStops() const noexcept { return m_stops.size() > 0; }
        bool canAccessStop(int32_t index) const noexcept { return index >= 0 || index < m_stops.size(); }

        int32_t stopCount() const noexcept { return (int32_t)m_stops.size(); }
        int32_t lastStopIndex() const noexcept { return (int32_t)m_stops.size() - 1; }
        int32_t selectedStopCount() const noexcept;

        int32_t firstSelectedStopIndex() const noexcept;
        int32_t lastSelectedStopIndex() const noexcept;
        int32_t selectedStopIndices(int32_t* out_first_index, int32_t* out_last_index) const noexcept;

        void selectAllStops() noexcept;
        void deselectAllStops() noexcept;
        void revertStopSelection() noexcept;
        void selectStopEach(int32_t step) noexcept;
        bool stepSelectedStops(int32_t step_count) noexcept;
        void removeAllStops() noexcept;
        void removeStop(int32_t index) noexcept;
        int32_t removeSelectedStops() noexcept;

        void addStop(float pos, const RGB& rgb) noexcept;
        void addStop(float pos, const RGBA& color) noexcept;
        void addStop(float pos, const RGBA& color1, const RGBA& color2) noexcept;
        void addStop(GradientStop& stop) noexcept;

        GradientStop* mutStopPtrAtIndex(int32_t index) noexcept;
        const GradientStop* stopPtrAtIndex(int32_t index) const noexcept;

        void colorAtIndex(int32_t index, int32_t part, RGBA& out_color) const noexcept;
        void leftColorAtIndex(int32_t index, RGBA& out_color) const noexcept;
        void rightColorAtIndex(int32_t index, RGBA& out_color) const noexcept;
        bool lookupColor(float pos, RGB& out_color) noexcept;
        bool lookupColor(float pos, RGBA& out_color) noexcept;
        bool lookupColorValues(float pos, float* out_values) noexcept;
        bool setColorAtIndex(int32_t index, int32_t part, const RGBA& color) noexcept;

        void rememberSelectedStops() noexcept;
        bool moveSelectedStops(float delta) noexcept;
        bool setColorOfSelectedStops(const RGB& color) noexcept;
        bool setColorOfSelectedStops(const RGBA& color) noexcept;
        bool setStepCountOfSelectedStops(int32_t step_count) noexcept;
        bool setColorModeOfSetectedStops(bool two_colored) noexcept;

        void needsUpdate(bool flag = true) noexcept;
        void update(GraphicContext& gc) noexcept;
        bool reset() noexcept;
        bool flip() noexcept;
        bool distribute() noexcept;
        bool stretch() noexcept;

        #if defined(__APPLE__) && defined(__MACH__)
            CGGradientRef macos_cgGradient(GraphicContext& gc) noexcept;
            int32_t macos_cgColorCount() noexcept;
        #endif

        void sortStops() noexcept;

        RGBLUT1* lutPtr() noexcept { updateLUT(); return m_lut; }
        int32_t lutResolution() const noexcept { return m_lut_resolution; }
        void setLUTResolution(int32_t resolution) noexcept;
        bool updateLUT() noexcept;
        bool lookupFromLUT(float pos, RGB& out_color) noexcept;

        ErrorCode saveDataFile(const String& file_path, bool can_overwrite = false) const noexcept;
        ErrorCode setFromDataFile(const String& file_path) noexcept;

        void draw(GraphicContext& gc, const Vec2d& start_pos, const Vec2d& end_pos) noexcept;
        void draw(GraphicContext& gc, const Vec2d& start_pos, const Vec2d& end_pos, bool draw_before, bool draw_after) noexcept;
        void drawInRect(GraphicContext& gc, const Rectd& rect, Direction direction) noexcept;
        void drawInRect(GraphicContext& gc, const Rectd& rect, Direction direction, bool draw_before, bool draw_after) noexcept;

        void drawRadial(GraphicContext& gc, const Vec2d& pos, double radius, bool draw_before, bool draw_after) noexcept;

        static int _spotSortCompareFunc(const GradientStop* a, const GradientStop* b) {
            if (a == nullptr || b == nullptr) {
                return 0;
            }
            else {
                float pos_a = a->pos();
                float pos_b = b->pos();
                return pos_a < pos_b ? -1 : pos_a > pos_b ? 1 : 0;
            }
        }

    private:
        void _init() noexcept;
    };



    #if defined(__APPLE__) && defined(__MACH__)
        typedef void (*GradientColorFunc)(void* info_ptr, const CGFloat* in, CGFloat* out);
    #endif


    class GradientFunction {
    public:
        enum {
            kMaxVars = 16,
            kMaxColors = 8
        };

        enum {
            kVar_Kelvin1 = 0,
            kVar_Kelvin2,
        };

        enum class StandardFunctionType {
            Gradient = 0,
            GradientAlpha,
            LUT1,
            RGBLUT1,
            OKLChHueRamp,
            Kelvin,
            Count
        };

    protected:
        void* m_info_ptr = nullptr;
        double _m_vars[kMaxVars]{};
        RGBA m_colors[kMaxColors];
        #if defined(__APPLE__) && defined(__MACH__)
            GradientColorFunc _m_color_func = nullptr;
        #endif

    public:
        GradientFunction(StandardFunctionType function_type) noexcept;
        #if defined(__APPLE__) && defined(__MACH__)
            GradientFunction(GradientColorFunc func) noexcept;
        #endif

        ~GradientFunction() noexcept;

        void setInfoPtr(void* info_ptr) noexcept { m_info_ptr = info_ptr; }

        void setVar(int32_t index, double value) noexcept {
            if (index >= 0 && index < kMaxVars) {
                _m_vars[index] = value;
            }
        }

        void setColor(int32_t index, const RGB& color) noexcept {
            if (index >= 0 && index < kMaxColors) {
                m_colors[index] = RGBA(color, 1.0f);
            }
        }

        void setColor(int32_t index, const RGBA& color) noexcept {
            if (index >= 0 && index < kMaxColors) {
                m_colors[index] = color;
            }
        }

        void drawAxial(GraphicContext& gc, const Vec2d& start_point, const Vec2d& end_point, bool extend_start = false, bool extend_end = false) noexcept {
            _draw(gc, start_point, -1.0, end_point, -1.0, extend_start, extend_end);
        }
        void drawRadial(GraphicContext& gc, const Vec2d& start_point, double start_radius, const Vec2d& end_point, double end_radius, bool extend_start = false, bool extend_end = false) noexcept {
            _draw(gc, start_point, start_radius, end_point, end_radius, extend_start, extend_end);
        }
        void _draw(GraphicContext& gc, const Vec2d& start_point, double start_radius, const Vec2d& end_point, double end_radius, bool extend_start = false, bool extend_end = false) noexcept;

        #if defined(__APPLE__) && defined(__MACH__)
            static void _standardFunc_gradient(void* info_ptr, const CGFloat* in, CGFloat* out);
            static void _standardFunc_gradientAlpha(void* info_ptr, const CGFloat* in, CGFloat* out);
            static void _standardFunc_LUT1(void* info_ptr, const CGFloat* in, CGFloat* out);
            static void _standardFunc_RGBLUT1(void* info_ptr, const CGFloat* in, CGFloat* out);
            static void _standardFunc_oklchHue(void* info_ptr, const CGFloat* in, CGFloat* out);
            static void _standardFunc_kelvin(void* info_ptr, const CGFloat* in, CGFloat* out);
        #endif
    };


} // End of namespace Grain

#endif // GrainGradient_hpp
