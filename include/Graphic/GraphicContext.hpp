//
//  GraphicContext.hpp
//
//  Created by Roald Christesen on from 03.05.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
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
        fourcc_t m_magic = Type::fourcc('b', 'a', 's', 'e');
        bool m_flipped_y = true;        ///< true, if vertical axis is flipped
        double m_width = 0.0;           ///< Pixel width
        double m_height = 0.0;          ///< Pixel height
        int32_t m_state_depth = 0;      ///< Depth of context state savings
        float m_alpha = 1.0f;           ///< Current alpha value

        Vec2d m_last_pos;               ///< Last, current positionfor drawing methods

        RGBA m_fill_color;              ///< Color to use for fill operations
        RGBA m_stroke_color;            ///< Color to use for stroke operations

        RGBA m_debug_bg_color = { 0, 0, 0, 1 };     ///< Color to use for debug information
        RGBA m_debug_fg_color = { 1, 1, 1, 1 };     ///< Color to use for debug information

        Image* m_image = nullptr;           ///< If context was constructed for drawing into an image, then here is a pointer to it
        Component* m_component = nullptr;   ///< If context was constructed for drawing into a component, then here is a pointer to it

    public:
        GraphicContext() noexcept;
        explicit GraphicContext(Component* component) noexcept;
        explicit GraphicContext(PDFWriter* pdf_writer) noexcept;

        virtual ~GraphicContext() noexcept;

        [[nodiscard]] virtual const char* className() const noexcept { return "GraphicContext"; }

        virtual void log(Log& l) const noexcept {}


        void _init() noexcept;
        void _freeResources() noexcept;
        void _freeImage() noexcept;

        [[nodiscard]] fourcc_t magic() const noexcept { return m_magic; }

        [[nodiscard]] double width() const noexcept { return m_width; }
        [[nodiscard]] double height() const noexcept { return m_height; }

        [[nodiscard]] Image* image() noexcept { return m_image; }
        [[nodiscard]] Component* component() noexcept { return m_component; }

        void setDebugFgColor(const RGBA& color) noexcept { m_debug_fg_color = color; }
        void setDebugBgColor(const RGBA& color) noexcept { m_debug_bg_color = color; }

        virtual void setComponent(Component* component) noexcept {}
        virtual void setImage(Image* image) noexcept;

        virtual bool isValid() noexcept { return false; }
        virtual void save() noexcept {}
        virtual void restore() noexcept {}
        virtual void setAlpha(float alpha) noexcept {}

        void setOpaque() noexcept { setAlpha(1.0f); }

        virtual void setFillColor(float r, float g, float b, float alpha) noexcept {}
        virtual void setFillClearColor() noexcept {
            setFillColor(1.0f, 1.0f, 1.0f, 0.0f);
        }
        virtual void setFillGray(float grey) noexcept {
            setFillColor(grey, grey, grey, 1.0f);
        }
        virtual void setFillGrayAndAlpha(float grey, float alpha) noexcept {
            setFillColor(grey, grey, grey, alpha);
        }
        virtual void setFillRGB(const RGB& rgb) noexcept {
            setFillColor(rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], 1.0f);
        }
        virtual void setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept {
            setFillColor(rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], alpha);
        }
        virtual void setFillRGBA(const RGBA& rgba) noexcept {
            setFillColor(rgba.m_data[0], rgba.m_data[1], rgba.m_data[2], rgba.m_alpha);
        }

        virtual void setStrokeColor(float r, float g, float b, float alpha) noexcept {}
        virtual void setStrokeGray(float grey) noexcept {
            setStrokeColor(grey, grey, grey, 1.0f);
        }
        virtual void setStrokeGrayAndAlpha(float grey, float alpha) noexcept {
            setStrokeColor(grey, grey, grey, alpha);
        }
        virtual void setStrokeRGB(const RGB& rgb) noexcept {
            setStrokeColor(rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], 1.0f);
        }
        virtual void setStrokeRGBAndAlpha(const RGB& rgb, float alpha) noexcept {
            setStrokeColor(rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], alpha);
        }
        virtual void setStrokeRGBA(const RGBA& rgba) noexcept {
            setStrokeColor(rgba.m_data[0], rgba.m_data[1], rgba.m_data[2], rgba.m_alpha);
        }

        virtual void setStrokeWidth(double width) noexcept {}
        virtual void setStrokeMiterLimit(double limit) noexcept {}
        virtual void setStrokeJoinStyle(StrokeJoinStyle join) noexcept {}
        virtual void setStrokeCapStyle(StrokeCapStyle cap) noexcept {}
        virtual void setStrokeDash(double dash_length, double gap_length) noexcept {}
        virtual void setStrokeDash(int32_t array_length, const double* array, double scale) noexcept {}
        virtual void setStrokeSolid() noexcept {}

        virtual void setBlendMode(BlendMode blend_mode) noexcept {}
        virtual void setBlendModeNormal() noexcept {}
        virtual void setBlendModeMultiply() noexcept {}

        virtual void enableAliasing() noexcept {}
        virtual void disableAliasing() noexcept {}
        virtual void enableFontSmoothing() noexcept {}
        virtual void disableFontSmoothing() noexcept {}
        virtual void enableFontSubpixelQuantization() noexcept {}
        virtual void disableFontSubpixelQuantization() noexcept {}

        virtual void setTextMatrix(double a, double b, double c, double d, double tx, double ty) noexcept {}

        virtual void beginPath() noexcept {}
        virtual void moveTo(double x, double y) noexcept {}
        virtual void moveTo(const Vec2d& point) noexcept { moveTo(point.m_x, point.m_y); }
        virtual void lineTo(double x, double y) noexcept {}
        virtual void lineTo(double x, double y, bool start_flag) noexcept {}
        virtual void lineTo(const Vec2d& point) noexcept { lineTo(point.m_x, point.m_y); }
        virtual void lineTo(const Vec2d& point, bool start_flag) noexcept { lineTo(point.m_x, point.m_y, start_flag); }
        virtual void curveTo(double c1x, double c1y, double c2x, double c2y, double x, double y) noexcept {}
        virtual void curveTo(const Vec2d& control1, const Vec2d& control2, const Vec2d& point) noexcept {}
        virtual void curveTo(double cx, double cy, double x, double y) noexcept {}

        virtual void closePath() noexcept {}
        virtual void fillPath() noexcept {}
        virtual void fillPathEvenOdd() noexcept {}
        virtual void strokePath() noexcept {}
        virtual void drawPath() noexcept {}

        virtual void addPolygon(Polygon* polygon) noexcept;
        virtual void addPath(GraphicPath* path) noexcept;
        virtual void addPath(GraphicPath* path, const GraphicPathSplitParam& split_param) noexcept;

        virtual void addRectPath(double x, double y, double width, double height) noexcept;
        virtual void addRectPath(const Rectd& rect) noexcept {
            addRectPath(rect.m_x, rect.m_y, rect.m_width, rect.m_height);
        };

        virtual bool addFramePath(const Rectd& rect, double top, double right, double bottom, double left) noexcept;

        virtual void addEllipsePath(const Rectd& rect) noexcept {}
        virtual void addCirclePath(double x, double y, double radius) noexcept {}
        virtual void addCirclePath(const Vec2d& center, double radius) noexcept {
            addCirclePath(center.m_x, center.m_y, radius);
        }

        virtual void addRoundBarPath(double x, double y, double width, double height) noexcept;
        virtual void addRoundBarPath(const Rectd& rect) noexcept {
            addRoundBarPath(rect.m_x, rect.m_y, rect.m_width, rect.m_height);
        }

        virtual void addRoundRectPath(double x, double y, double width, double height, double radius) noexcept;
        virtual void addRoundRectPath(const Rectd& rect, double radius) noexcept {
            addRoundRectPath(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius);
        }

        virtual void addRoundRectPath(
                double x, double y, double width, double height,
                double radius1, double radius2, double radius3, double radius4) noexcept;
        virtual void addRoundRectPath(
                const Rectd& rect,
                double radius1, double radius2, double radius3, double radius4) noexcept {
            addRoundRectPath(
                    rect.m_x, rect.m_y, rect.m_width, rect.m_height,
                    radius1, radius2, radius3, radius4);

        }

        virtual void addRingPath(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept {}

        virtual void addTrianglePath(const Triangled& triangle) noexcept;
        virtual void addTrianglePath(const Vec2d& point1, const Vec2d& point2, const Vec2d& point3) noexcept;
        virtual void addTrianglePath(double x, double y, double width, double height, Direction direction) noexcept;
        virtual void addTrianglePath(const Rectd& rect, Direction direction) noexcept;

        virtual void addPolygonPath(int32_t point_count, const Vec2d* points) noexcept;

        virtual void addDropPath() noexcept;
        virtual void addRightHalfDropPath() noexcept;
        virtual void addLeftHalfDropPath() noexcept;

        virtual void strokeLine(double x1, double y1, double x2, double y2) noexcept;
        virtual void strokeLine(const Vec2d& point1, const Vec2d& point2) noexcept;
        virtual void strokeLineXZ(const Vec3d& point1, const Vec3d& point2) noexcept;
        virtual void strokeLineXY(const Vec3d& point1, const Vec3d& point2) noexcept;
        virtual void strokeLine(const Lined& line) noexcept;
        virtual void strokeHorizontalLine(double x1, double x2, double y) noexcept;
        virtual void strokeVerticalLine(double x, double y1, double y2) noexcept;
        virtual void strokeHorizontalConnection(const Vec2d& start_point, const Vec2d& end_point) noexcept;

        virtual void strokeBezier(const Bezier& bezier) noexcept;

        virtual void strokeCatmullRomCurve(const CatmullRomCurve& catmull_rom_curve, int32_t resolution = -1) noexcept;
        virtual void strokeCatmullRomCurve(const CatmullRomCurve& catmull_rom_curve, float t_beg, float t_end, int32_t resolution = -1) noexcept;

        virtual void fillRect(double x, double y, double width, double height) noexcept {}
        void fillRect(const Rectd& rect) noexcept;
        void fillRect(const Rectd& rect, double radius) noexcept;

        virtual void fillRoundBar(double x, double y, double width, double height) noexcept;
        virtual void fillRoundBar(const Rectd& rect) noexcept;
        virtual void fillRoundRect(double x, double y, double width, double height, double radius) noexcept;
        virtual void fillRoundRect(const Rectd& rect, double radius) noexcept;
        virtual void fillRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept;
        virtual void fillRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept;

        virtual void fillFrame(const Rectd& rect, double size) noexcept;
        virtual void fillFrame(const Rectd& rect, double width, double height) noexcept;
        virtual void fillFrame(const Rectd& rect, double top, double right, double bottom, double left) noexcept;

        virtual void strokeRect(double x, double y, double width, double height) noexcept {}
        virtual void strokeRect(const Rectd& rect) noexcept {
            strokeRect(rect.m_x, rect.m_y, rect.m_width, rect.m_height);
        }
        virtual void strokeRect(const Rectd& rect, double offset) noexcept {
            strokeRect(rect.m_x - offset, rect.m_y - offset, rect.m_width + offset * 2, rect.m_height + offset * 2);
        }
        virtual void strokeRoundBar(double x, double y, double width, double height) noexcept;
        virtual void strokeRoundBar(const Rectd& rect) noexcept;
        virtual void strokeRoundRect(double x, double y, double width, double height, double radius) noexcept;
        virtual void strokeRoundRect(const Rectd& rect, double radius) noexcept {
            strokeRoundRect(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius);
        }
        virtual void strokeRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept;
        virtual void strokeRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept;

        virtual void addQuadrilateralPath(const Vec2d* points) noexcept;
        virtual void addQuadrilateralPath(const Quadrilateral& quadrilateral) noexcept;
        virtual void fillQuadrilateral(const Vec2d* points) noexcept;
        virtual void fillQuadrilateral(const Quadrilateral& quadrilateral) noexcept;
        virtual void strokeQuadrilateral(const Vec2d* points) noexcept;
        virtual void strokeQuadrilateral(const Quadrilateral& quadrilateral) noexcept;

        virtual void fillTriangle(const Rectd& rect, Direction direction) noexcept;
        virtual void fillPolygon(int32_t point_count, const Vec2d* points) noexcept;

        virtual void fillEllipse(double x, double y, double rh, double rv) noexcept {}
        void fillEllipse(const Rectd& rect) noexcept;
        void fillEllipse(const Vec2d& center, double rh, double rv) noexcept;

        virtual void strokeEllipse(double x, double y, double rh, double rv) noexcept {}
        void strokeEllipse(const Rectd& rect) noexcept;
        void strokeEllipse(const Vec2d& center, double rh, double rv) noexcept;

        virtual void fillCircle(double x, double y, double radius) noexcept {}
        void fillCircle(const Circled& circle) noexcept;
        void fillCircle(const Rectd& rect) noexcept;
        void fillCircle(const Rectd& rect, double min_radius, double max_radius) noexcept;
        void fillCircle(const Vec2d& center, double radius) noexcept;

        virtual void strokeCircle(double x, double y, double radius) noexcept {}
        void strokeCircle(const Circled& circle) noexcept;
        void strokeCircle(const Rectd& rect) noexcept;
        void strokeCircle(const Rectd& rect, double min_radius, double max_radius) noexcept;
        void strokeCircle(const Vec2d& center, double radius) noexcept;

        virtual void fillRing(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept;
        virtual void fillColorWheel(const Vec2d& center, double outer_radius, double inner_radius) noexcept {}
        virtual void fillAudioLocationControl(const Vec2d& center, double radius) noexcept {}

        virtual void drawGradient(Gradient* gradient, const Vec2d& start_pos, const Vec2d& end_pos, bool draw_before, bool draw_after) noexcept {}
        void drawGradient(Gradient* gradient, const Vec2d& start_pos, const Vec2d& end_pos) noexcept {
            drawGradient(gradient, start_pos, end_pos, true, true);
        }
        virtual void drawRadialGradient(Gradient* gradient, const Vec2d& pos, double radius, bool draw_before, bool draw_after) noexcept {}

        virtual void drawImage(Image* image, const Rectd& rect, float alpha = 1.0f) noexcept {}
        virtual ErrorCode drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral) noexcept { return ErrorCode::Unknown; }
        virtual ErrorCode drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral, float alpha) noexcept { return ErrorCode::Unknown; }

        virtual void drawIcon(const Icon* icon, const Rectd& rect, float alpha) noexcept {}
        virtual void drawIcon(const Icon* icon, const Rectd& rect, const RGB& color, float alpha) noexcept {}
        virtual void drawIconInCircle(const Icon* icon, const Vec2d& center, double radius, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept {}
        virtual void drawIconInRoundRect(const Icon* icon, const Rectd& rect, double radius1, double radius2, double radius3, double radius4, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept;

        virtual Rectd textRect(const String& string, const Font* font) noexcept;
        virtual Rectd textRect(const char* text, const Font* font) noexcept { return Rectd(); }

        virtual void drawText(const String& string, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) noexcept;
        virtual void drawText(const char* text, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) noexcept {}
        virtual void drawTextInt(int64_t value, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) noexcept;

        virtual double drawTextInRect(const String& string, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha = 1.0f) noexcept;
        virtual double drawTextInRect(const char* text, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha = 1.0f) noexcept { return 0.0; }
        virtual double drawTextIntInRect(int64_t value, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha = 1.0f) noexcept;
        virtual double drawWrappedText(const char* text, const Rectd& bounds_rect, const Rectd& rect, TextAlignment alignment, double line_gap, const Font* font, const RGB& color, float alpha = 1.0f) noexcept { return 0.0; }

        virtual void drawDebugText(const char* text, Vec2d& pos, int32_t spacing = 2) noexcept;
        virtual void drawDebugBool(const char* label, bool value, Vec2d& pos, int32_t spacing = 2) noexcept;
        virtual void drawDebugInt64(const char* label, int64_t value, Vec2d& pos, int32_t spacing = 2) noexcept;
        virtual void drawDebugDouble(const char* label, double value, Vec2d& pos, int32_t spacing = 2) noexcept;

        virtual void addTextPath(const char* text, const Font* font) noexcept {}

        virtual void clipPath() noexcept {}
        virtual void clipPathEvenOdd() noexcept {}
        virtual void clipRect(const Rectd& rect) noexcept;
        virtual void clipRoundRect(const Rectd& rect, double radius) noexcept;
        virtual void clipEllipse(const Rectd& rect) noexcept;
        virtual void clipCircle(double x, double y, double radius) noexcept;
        virtual void clipCircle(const Vec2d& center, double radius) noexcept;
        virtual Rectd clipBoundsRect() noexcept { return Rectd(); }
        virtual void resetClip() noexcept {}

        virtual void translate(double tx, double ty) noexcept {}
        void translateX(double tx) noexcept { translate(tx, 0.0); }
        void translateY(double ty) noexcept { translate(0.0, ty); }
        template<typename T>
        void translate(const Vec2<T>& tv) noexcept {
            translate(static_cast<double>(tv.m_x), static_cast<double>(tv.m_y));
        }
        virtual void scale(double sx, double sy) noexcept {}
        void scale(double s) noexcept { scale(s, s); }
        void scale(const Vec2d& sv) noexcept { scale(sv.m_x, sv.m_y); }
        void scaleFromPivot(const Vec2d& pivot, double s) noexcept { translate(pivot); scale(s); translate(-pivot); }
        void scaleFromPivot(const Vec2d& pivot, double sx, double sy) noexcept { translate(pivot); scale(sx, sy); translate(-pivot); }
        virtual void rotate(double angle) noexcept {}
        void rotateAroundPivot(const Vec2d& pivot, double angle) noexcept;
        virtual void affineTransform(const Mat3d& matrix) noexcept {}
        void transformToFitRectProportionally(const Rectd& src_rect, const Rectd& dst_rect) noexcept;



        //

        void drawHorizontalKeyboard(int32_t low_pitch, int32_t high_pitch, int32_t marked_pitch, double begin_freq, double end_freq, double min_x, double max_x, double y0, double y1, const RGB& light_color, const RGB& dark_color, const RGB& bg_color, const RGB& mark_color, float alpha) noexcept;

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
            while (cg_blend_mode_names[i]) {
                if (strcasecmp(cg_blend_mode_names[i], blend_mode_name) == 0) {
                    return (BlendMode)i;
                }
                i++;
            }

            return BlendMode::Undefined;
        }
    };


} // End of namespace Grain

#endif // GrainGraphicContext_hpp
