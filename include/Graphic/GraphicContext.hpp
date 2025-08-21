//
//  GraphicContext.hpp
//
//  Created by Roald Christesen on from 03.05.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

/*
 *  TODO:
 *  Eventually not needed beginDrawing and endDrawing.
 */

#ifndef GrainGraphicContext_hpp
#define GrainGraphicContext_hpp

#include "Grain.hpp"
#include "Graphic/Graphic.hpp"
#include "Color/RGB.hpp"
#include "Color/RGBA.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat3.hpp"
#include "2d/Line.hpp"
#include "2d/Circle.hpp"
#include "2d/Rect.hpp"
#include "2d/Triangle.hpp"
#include "2d/Quadrilateral.hpp"
#include "2d/Dimension.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreGraphics/CoreGraphics.h>
#endif


namespace Grain {

    // Forward declarations
    class Bezier;
    class CatmullRomCurve;
    class Font;
    class Icon;
    class Gradient;
    class Polygon;
    class GraphicPath;
    class GraphicPathSplitParam;
    class View;
    class Image;
    class Component;
    class String;
    class PDFWriter;


    // Class GraphicContext
    class GraphicContext {
        friend class Image;

    public:
        enum class Engine {
            Undefined = -1,
            Cairo = 0,
            CoreGraphics = 1,
            Default = Cairo
        };

        enum class BlendMode {
            Undefined = -1,
            Normal = 0,
            Multiply = 1,
            Screen = 2,
            Overlay = 3,
            Darken = 4,
            Lighten = 5,
            ColorDodge = 6,
            ColorBurn = 7,
            SoftLight = 8,
            HardLight = 9,
            Difference = 10,
            Exclusion = 11,
            Hue = 12,
            Saturation = 13,
            Color = 14,
            Luminosity = 15,
            Clear = 16,
            Copy = 17,
            SourceIn = 18,
            SourceOut = 19,
            SourceAtop = 20,
            DestinationOver = 21,
            DestinationIn = 22,
            DestinationOut = 23,
            DestinationAtop = 24,
            XOR = 25,
            PlusDarker = 26,
            PlusLighter = 27,

            First = 0,
            Last = PlusLighter
        };

    protected:
        bool m_flipped_y = true;        ///< true, if vertical axis is flipped
        double m_width = 0.0;           ///< Pixel width
        double m_height = 0.0;          ///< Pixel height
        int32_t m_state_depth = 0;      ///< Depth of context state savings

        Vec2d m_last_pos;               ///< Last, current positionfor drawing methods

        RGBA m_fill_color;              ///< Color to use for fill operations
        RGBA m_stroke_color;            ///< Color to use for stroke operations

        RGBA m_debug_bg_color = { 0, 0, 0, 1 };     ///< Color to use for debug information
        RGBA m_debug_fg_color = { 1, 1, 1, 1 };     ///< Color to use for debug information

        Image* m_image = nullptr;           ///< If context was constructed for drawing into an image, then here is a pointer to it
        Component* m_component = nullptr;   ///< If context was constructed for drawing into a component, then here is a pointer to it

        #if defined(__APPLE__) && defined(__MACH__)
            CGContextRef m_cg_context = nullptr;    ///< Core Graphics Context
            CGColorSpaceRef m_cg_color_space = nullptr;
        #endif

        void* _m_cairo_surface = nullptr;   ///< Cairo cairo_surface_t*
        void* _m_cairo_cr = nullptr;        ///< Cairo cairo_t*

    public:
        explicit GraphicContext(Component* component) noexcept;
        explicit GraphicContext(Image* image) noexcept;
        explicit GraphicContext(PDFWriter* pdf_writer) noexcept;
        ~GraphicContext() noexcept;

        void _init() noexcept;
        void _freeResources() noexcept;

        virtual void _setImage(Image* image) noexcept;
        #if defined(__APPLE__) && defined(__MACH__)
            void _macos_setImage(Image* image) noexcept;
        #endif

        [[nodiscard]] void* cairoSurface() { return _m_cairo_surface; }
        [[nodiscard]] void* cairoCr() { return _m_cairo_cr; }

        [[nodiscard]] double width() const noexcept { return m_width; }
        [[nodiscard]] double height() const noexcept { return m_height; }

        [[nodiscard]] Image* image() noexcept { return m_image; }
        [[nodiscard]] Component* component() noexcept { return m_component; }

        bool isValid() noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                return m_cg_context != nullptr;
            #else
                return false; // TODO: Implement linux version
            #endif
        }

        void save() noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSaveGState(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif

            m_state_depth++;
        }

        void restore() noexcept {
            if (m_state_depth > 0) {
                #if defined(__APPLE__) && defined(__MACH__)
                    CGContextRestoreGState(m_cg_context);
                #else
                    // TODO: Implement linux version
                #endif
                m_state_depth--;
            }
        }

        #if defined(__APPLE__) && defined(__MACH__)
            [[nodiscard]] CGContextRef macos_cgContext() const noexcept { return m_cg_context; }
            void macos_setCGContextByComponent(CGContextRef context, Component* component) noexcept;
            [[nodiscard]] CGColorSpaceRef macos_cgColorSpace() const noexcept { return m_cg_color_space; }
        #endif

        void setAlpha(float alpha) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetAlpha(m_cg_context, alpha);
            #else
                // TODO: Implement linux version
            #endif
        }

        virtual void setOpaque() const noexcept { setAlpha(1.0f); }

        virtual void setFillClearColor() noexcept { setFillColor(1.0f, 1.0f, 1.0f, 0.0f); }
        virtual void setFillColor(float r, float g, float b, float alpha) noexcept;
        virtual void setFillGray(float grey) noexcept { setFillColor(grey, grey, grey, 1.0f); }
        virtual void setFillGrayAndAlpha(float grey, float alpha) noexcept { setFillColor(grey, grey, grey, alpha); }
        virtual void setFillRGB(const RGB& rgb) noexcept;
        virtual void setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept;
        virtual void setFillRGBA(const RGBA& rgba) noexcept;

        virtual void setStrokeColor(float r, float g, float b, float alpha) noexcept;
        virtual void setStrokeGray(float grey) noexcept { setStrokeColor(grey, grey, grey, 1.0f); }
        virtual void setStrokeGrayAndAlpha(float grey, float alpha) noexcept { setStrokeColor(grey, grey, grey, alpha); }
        virtual void setStrokeRGB(const RGB& rgb) noexcept;
        virtual void setStrokeRGBAndAlpha(const RGB& rgb, float alpha) noexcept;
        virtual void setStrokeRGBA(const RGBA& rgba) noexcept;

        void setDebugFgColor(const RGBA& color) noexcept { m_debug_fg_color = color; }
        void setDebugBgColor(const RGBA& color) noexcept { m_debug_bg_color = color; }

        void setStrokeWidth(double width) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetLineWidth(m_cg_context, width);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setStrokeMiterLimit(double limit) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetMiterLimit(m_cg_context, limit);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setStrokeJoinStyle(StrokeJoinStyle join) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetLineJoin(m_cg_context, (CGLineJoin)join);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setStrokeCapStyle(StrokeCapStyle cap) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetLineCap(m_cg_context, (CGLineCap)cap);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setStrokeDash(double dash_length, double gap_length) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGFloat l[2] = { dash_length, gap_length };
                CGContextSetLineDash(m_cg_context, 0.0, l, 2);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setStrokeDash(int32_t array_length, const double* array, double scale) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                if (array_length < 0 || array == nullptr) {
                    CGContextSetLineDash(m_cg_context, 0, nullptr, 0);
                }
                else {
                    if (array_length > 32) {
                        array_length = 32;
                    }
                    CGFloat l[32];
                    for (int32_t i = 0; i < array_length; i++) {
                        l[i] = array[i] * scale;
                    }
                    CGContextSetLineDash(m_cg_context, 0.0, l, array_length);
                }
            #else
                // TODO: Implement linux version
            #endif
        }

        void setStrokeSolid() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetLineDash(m_cg_context, 0, nullptr, 0);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setBlendMode(BlendMode blend_mode) const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                static CGBlendMode cg_blend_modes[] = {
                        kCGBlendModeNormal,
                        kCGBlendModeMultiply,
                        kCGBlendModeScreen,
                        kCGBlendModeOverlay,
                        kCGBlendModeDarken,
                        kCGBlendModeLighten,
                        kCGBlendModeColorDodge,
                        kCGBlendModeColorBurn,
                        kCGBlendModeSoftLight,
                        kCGBlendModeHardLight,
                        kCGBlendModeDifference,
                        kCGBlendModeExclusion,
                        kCGBlendModeHue,
                        kCGBlendModeSaturation,
                        kCGBlendModeColor,
                        kCGBlendModeLuminosity,
                        kCGBlendModeClear,
                        kCGBlendModeCopy,
                        kCGBlendModeSourceIn,
                        kCGBlendModeSourceOut,
                        kCGBlendModeSourceAtop,
                        kCGBlendModeDestinationOver,
                        kCGBlendModeDestinationIn,
                        kCGBlendModeDestinationOut,
                        kCGBlendModeDestinationAtop,
                        kCGBlendModeXOR,
                        kCGBlendModePlusDarker,
                        kCGBlendModePlusLighter
                };

                if (blend_mode < BlendMode::Normal || blend_mode > BlendMode::Last) {
                    blend_mode = BlendMode::Normal;
                }

                CGContextSetBlendMode(m_cg_context, cg_blend_modes[static_cast<int32_t>(blend_mode)]);
            #else
                // TODO: Implement linux version
            #endif
        }

        static BlendMode blendModeByName(const char* blend_mode_name) noexcept {
            static const char* cg_blend_mode_names[] = {
                    "Normal", "Multiply", "Screen", "Overlay", "Darken", "Lighten",
                    "ColorDodge", "ColorBurn", "SoftLight", "HardLight", "Difference",
                    "Exclusion", "Hue", "Saturation", "Color", "Luminosity", "Clear",
                    "Copy", "SourceIn", "SourceOut", "SourceAtop", "DestinationOver",
                    "DestinationIn", "DestinationOut", "DestinationAtop", "XOR",
                    "PlusDarker", "PlusLighter", nullptr
            };


            int32_t i = 0;
            while (cg_blend_mode_names[i] != nullptr) {
                if (strcasecmp(cg_blend_mode_names[i], blend_mode_name) == 0) {
                    return (BlendMode)i;
                }
                i++;
            }

            return BlendMode::Undefined;
        }

        void setBlendModeNormal() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetBlendMode(m_cg_context, kCGBlendModeNormal);
            #else
                // TODO: Implement linux version
            #endif
        }

        void setBlendModeMultiply() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetBlendMode(m_cg_context, kCGBlendModeMultiply);
            #else
                // TODO: Implement linux version
            #endif
        }

        void enableAliasing() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetAllowsAntialiasing(m_cg_context, true);
            #else
                // TODO: Implement linux version
            #endif
        }

        void disableAliasing() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetAllowsAntialiasing(m_cg_context, false);
            #else
                // TODO: Implement linux version
            #endif
        }

        void enableFontSmoothing() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetShouldSmoothFonts(m_cg_context, true);
            #else
                // TODO: Implement linux version
            #endif
        }

        void disableFontSmoothing() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetShouldSmoothFonts(m_cg_context, false);
            #else
                // TODO: Implement linux version
            #endif
        }

        void enableFontSubpixelQuantization() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetShouldSubpixelQuantizeFonts(m_cg_context, true);
            #else
                // TODO: Implement linux version
            #endif
        }

        void disableFontSubpixelQuantization() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextSetShouldSubpixelQuantizeFonts(m_cg_context, false);
            #else
                // TODO: Implement linux version
            #endif
        }

        //

        void setTextMatrix(double a, double b, double c, double d, double tx, double ty) const noexcept;

        // Path
        void beginPath() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextBeginPath(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }

        void moveTo(double x, double y) noexcept;
        void moveTo(const Vec2d& point) noexcept;
        void lineTo(double x, double y) noexcept;
        void lineTo(double x, double y, bool start_flag) noexcept;
        void lineTo(const Vec2d& point) noexcept;
        void lineTo(const Vec2d& point, bool start_flag) noexcept;
        void curveTo(double c1x, double c1y, double c2x, double c2y, double x, double y) noexcept;
        void curveTo(const Vec2d& control1, const Vec2d& control2, const Vec2d& point) noexcept;
        void curveTo(double cx, double cy, double x, double y) noexcept;

        void closePath() noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextClosePath(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }

        void fillPath() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextFillPath(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }

        void fillPathEvenOdd() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextEOFillPath(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }

        void strokePath() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextStrokePath(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }

        void drawPath() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextDrawPath(m_cg_context, kCGPathFillStroke);
            #else
                // TODO: Implement linux version
            #endif
        }

        void addPolygon(Polygon* polygon) noexcept;

        void addPath(GraphicPath* path) noexcept;
        void addPath(GraphicPath* path, const GraphicPathSplitParam& split_param) noexcept;

        void addRectPath(double x, double y, double width, double height) noexcept;
        void addRectPath(const Rectd& rect) noexcept;

        bool addFramePath(const Rectd& rect, double top, double right, double bottom, double left) noexcept;

        void addEllipsePath(const Rectd& rect) noexcept;
        void addCirclePath(double x, double y, double radius) noexcept;
        void addCirclePath(const Vec2d& center, double radius) noexcept;

        void addRoundBarPath(double x, double y, double width, double height) noexcept;
        void addRoundBarPath(const Rectd& rect) noexcept;

        void addRoundRectPath(double x, double y, double width, double height, double radius) noexcept;
        void addRoundRectPath(const Rectd& rect, double radius) noexcept;

        void addRoundRectPath(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept;
        void addRoundRectPath(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept;

        void addRingPath(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept;

        void addTrianglePath(const Triangled& triangle) noexcept;
        void addTrianglePath(const Vec2d& point1, const Vec2d& point2, const Vec2d& point3) noexcept;
        void addTrianglePath(double x, double y, double width, double height, Direction direction) noexcept;
        void addTrianglePath(const Rectd& rect, Direction direction) noexcept;

        void addPolygonPath(int32_t point_count, const Vec2d* points) noexcept;

        void addDropPath() noexcept;
        void addRightHalfDropPath() noexcept;
        void addLeftHalfDropPath() noexcept;

        // Line
        void strokeLine(double x1, double y1, double x2, double y2) noexcept;
        void strokeLine(const Vec2d& point1, const Vec2d& point2) noexcept;
        void strokeLineXZ(const Vec3d& point1, const Vec3d& point2) noexcept;
        void strokeLineXY(const Vec3d& point1, const Vec3d& point2) noexcept;
        void strokeLine(const Lined& line) noexcept;
        void strokeHorizontalLine(double x1, double x2, double y) noexcept;
        void strokeVerticalLine(double x, double y1, double y2) noexcept;
        void strokeHorizontalConnection(const Vec2d& start_point, const Vec2d& end_point) noexcept;

        // Bezier
        void strokeBezier(const Bezier& bezier) noexcept;

        // CutmullRomCurve
        void strokeCatmullRomCurve(const CatmullRomCurve& catmull_rom_curve, int32_t resolution = -1) noexcept;
        void strokeCatmullRomCurve(const CatmullRomCurve& catmull_rom_curve, float t_beg, float t_end, int32_t resolution = -1) noexcept;

        // Rect
        virtual void fillRect(double x, double y, double width, double height) noexcept;
        virtual void fillRect(const Rectd& rect) noexcept;
        void fillRect(const Rectd& rect, double radius) noexcept;
        void fillRoundBar(double x, double y, double width, double height) noexcept;
        void fillRoundBar(const Rectd& rect) noexcept;
        void fillRoundRect(double x, double y, double width, double height, double radius) noexcept;
        void fillRoundRect(const Rectd& rect, double radius) noexcept;
        void fillRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept;
        void fillRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept;

        void fillFrame(const Rectd& rect, double size) noexcept;
        void fillFrame(const Rectd& rect, double width, double height) noexcept;
        void fillFrame(const Rectd& rect, double top, double right, double bottom, double left) noexcept;

        void strokeRoundBar(double x, double y, double width, double height) noexcept;
        void strokeRoundBar(const Rectd& rect) noexcept;

        void strokeRect(double x, double y, double width, double height) noexcept;
        void strokeRect(const Rectd& rect) noexcept;
        void strokeRect(const Rectd& rect, double offset) noexcept;
        void strokeRoundRect(double x, double y, double width, double height, double radius) noexcept;
        void strokeRoundRect(const Rectd& rect, double radius) noexcept;
        void strokeRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept;
        void strokeRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept;

        // Quadrilateral
        void addQuadrilateralPath(const Vec2d* points) noexcept;
        void addQuadrilateralPath(const Quadrilateral& quadrilateral) noexcept;
        void fillQuadrilateral(const Vec2d* points) noexcept;
        void fillQuadrilateral(const Quadrilateral& quadrilateral) noexcept;
        void strokeQuadrilateral(const Vec2d* points) noexcept;
        void strokeQuadrilateral(const Quadrilateral& quadrilateral) noexcept;

        // Triangle
        void fillTriangle(const Rectd& rect, Direction direction) noexcept;

        // Polygon
        void fillPolygon(int32_t point_count, const Vec2d* points) noexcept;

        // Ellipse/Circle
        void fillEllipse(const Rectd& rect) noexcept;
        void fillEllipse(double x, double y, double rh, double rv) noexcept;
        void fillEllipse(const Vec2d& center, double rh, double rv) noexcept;
        void strokeEllipse(const Rectd& rect) noexcept;
        void strokeEllipse(double x, double y, double rh, double rv) noexcept;
        void strokeEllipse(const Vec2d& center, double rh, double rv) noexcept;

        void fillCircle(const Circled& circle) noexcept;
        void fillCircle(const Rectd& rect) noexcept;
        void fillCircle(const Rectd& rect, double min_radius, double max_radius) noexcept;
        void fillCircle(double x, double y, double radius) noexcept;
        void fillCircle(const Vec2d& center, double radius) noexcept;
        void strokeCircle(const Circled& circle) noexcept;
        void strokeCircle(double x, double y, double radius) noexcept;
        void strokeCircle(const Vec2d& center, double radius) noexcept;
        void strokeCircle(const Rectd& rect) noexcept;
        void strokeCircle(const Rectd& rect, double min_radius, double max_radius) noexcept;

        void fillRing(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept;

        void fillColorWheel(const Vec2d& center, double outer_radius, double inner_radius) noexcept;
        void fillAudioLocationControl(const Vec2d& center, double radius) noexcept;

        // Image
        void drawImage(Image* image, const Rectd& rect, float alpha = 1.0f) noexcept;
        ErrorCode drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral) noexcept;
        ErrorCode drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral, float alpha) noexcept;

        // Icon
        void drawIcon(const Icon* icon, const Rectd& rect, float alpha) noexcept;
        void drawIcon(const Icon* icon, const Rectd& rect, const RGB& color, float alpha) noexcept;
        void drawIconInCircle(const Icon* icon, const Vec2d& center, double radius, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept;
        void drawIconInRoundRect(const Icon* icon, const Rectd& rect, double radius1, double radius2, double radius3, double radius4, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept;

        // Text
        Rectd textRect(const String& string, const Font* font) const noexcept;
        Rectd textRect(const char* text, const Font* font) const noexcept;

        void drawText(const String& string, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;
        void drawText(const char* text, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;
        void drawTextInt(int64_t value, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;

        double drawTextInRect(const String& string, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;
        double drawTextInRect(const char* text, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;
        double drawTextIntInRect(int64_t value, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;

        double drawTextLineByLine(const char* text, const Rectd& bounds_rect, const Rectd& rect, double line_gap, const Font* font, const RGB& color, float alpha = 1.0f) const noexcept;

        void drawDebugText(const char* text, Vec2d& pos, int32_t spacing = 2) noexcept;
        void drawDebugBool(const char* label, bool value, Vec2d& pos, int32_t spacing = 2) noexcept;
        void drawDebugInt64(const char* label, int64_t value, Vec2d& pos, int32_t spacing = 2) noexcept;
        void drawDebugDouble(const char* label, double value, Vec2d& pos, int32_t spacing = 2) noexcept;

        void addTextPath(const char* text, const Font* font) const noexcept;


        // Clip
        void clipPath() noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextClip(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif

        }

        void clipPathEvenOdd() noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextEOClip(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }

        void clipRect(const Rectd& rect) noexcept;
        void clipRoundRect(const Rectd& rect, double radius) noexcept;
        void clipEllipse(const Rectd& rect) noexcept;
        void clipCircle(double x, double y, double radius) noexcept;
        void clipCircle(const Vec2d& center, double radius) noexcept;
        Rectd clipBoundsRect() noexcept;

        void resetClip() noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextResetClip(m_cg_context);
            #else
                // TODO: Implement linux version
            #endif
        }


        // Transformation
        void translateX(double tx) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextTranslateCTM(m_cg_context, tx, 0.0);
            #else
                // TODO: Implement linux version
            #endif
        }

        void translateY(double ty) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextTranslateCTM(m_cg_context, 0.0, ty);
            #else
                // TODO: Implement linux version
            #endif
        }

        void translate(double tx, double ty) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextTranslateCTM(m_cg_context, tx, ty);
            #else
                // TODO: Implement linux version
            #endif
        }

        template<typename T>
        void translate(const Vec2<T>& tv) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextTranslateCTM(m_cg_context, static_cast<double>(tv.m_x), static_cast<double>(tv.m_y));
            #else
                // TODO: Implement linux version
            #endif
        }

        void rotate(double angle) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextRotateCTM(m_cg_context, angle * std::numbers::pi / 180.0);
            #else
                // TODO: Implement linux version
            #endif
        }

        void rotateAroundPivot(const Vec2d& pivot, double angle) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                translate(pivot);
                CGContextRotateCTM(m_cg_context, angle * std::numbers::pi / 180.0);
                translate(-pivot);
            #else
                // TODO: Implement linux version
            #endif
        }

        void scale(double s) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextScaleCTM(m_cg_context, s, s);
            #else
                // TODO: Implement linux version
            #endif
        }

        void scale(double sx, double sy) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextScaleCTM(m_cg_context, sx, sy);
            #else
                // TODO: Implement linux version
            #endif
        }

        void scale(const Vec2d& sv) noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextScaleCTM(m_cg_context, sv.m_x, sv.m_y);
            #else
                // TODO: Implement linux version
            #endif
        }

        void scaleFromPivot(const Vec2d& pivot, double s) noexcept { translate(pivot); scale(s); translate(-pivot); }
        void scaleFromPivot(const Vec2d& pivot, double sx, double sy) noexcept { translate(pivot); scale(sx, sy); translate(-pivot); }

        void affineTransform(const Mat3d& matrix) noexcept;

        void transformToFitRectProportionally(const Rectd& src_rect, const Rectd& dst_rect) noexcept;

        //

        void drawHorizontalKeyboard(int32_t low_pitch, int32_t high_pitch, int32_t marked_pitch, double begin_freq, double end_freq, double min_x, double max_x, double y0, double y1, const RGB& light_color, const RGB& dark_color, const RGB& bg_color, const RGB& mark_color, float alpha) noexcept;
    };


} // End of namespace Grain

#endif // GrainGraphicContext_hpp
