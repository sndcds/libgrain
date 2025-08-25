//
//  MacCGContext.hpp
//
//  Created by Roald Christesen on from 24.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#ifndef GrainMacCGContext_hpp
#define GrainMacCGContext_hpp

#include "Graphic/GraphicContext.hpp"


namespace Grain {

    class MacCGContext : public GraphicContext {

    protected:
        CGContextRef m_cg_context = nullptr;            ///< Core Graphics Context
        CGColorSpaceRef m_cg_color_space = nullptr;     ///< Core Graphics Color Space

    public:
        // Re-expose base class overloads hidden by name hiding in derived class
        using GraphicContext::fillRect;
        using GraphicContext::fillRoundRect;
        using GraphicContext::fillEllipse;
        using GraphicContext::fillCircle;
        using GraphicContext::drawTextInRect;

        MacCGContext() noexcept;
        explicit MacCGContext(Component* component) noexcept;
        explicit MacCGContext(PDFWriter* pdf_writer) noexcept;

        virtual ~MacCGContext() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "MacCGContext"; }
        void log(Log& l) const noexcept override;


        void _macGCInit() noexcept;
        void _macCGFreeResources() noexcept;

        [[nodiscard]] CGContextRef cgContext() const noexcept { return m_cg_context; }
        [[nodiscard]] CGColorSpaceRef cgColorSpace() const noexcept { return m_cg_color_space; }

        void setCGContext(CGContextRef cg_context) { m_cg_context = cg_context; }
        void setCGColorSpace(CGColorSpaceRef cg_color_space) { m_cg_color_space = cg_color_space; }

        void setCGContextByComponent(CGContextRef context, Component* component) noexcept;

        void setImage(Image* image) noexcept override;

        bool isValid() noexcept override;
        void save() noexcept override;
        void restore() noexcept override;
        void setAlpha(float alpha) noexcept override;

        void setFillColor(float r, float g, float b, float alpha) noexcept override;
        void setStrokeColor(float r, float g, float b, float alpha) noexcept override;

        void setStrokeWidth(double width) noexcept override;
        void setStrokeMiterLimit(double limit) noexcept override;
        void setStrokeJoinStyle(StrokeJoinStyle join) noexcept override;
        void setStrokeCapStyle(StrokeCapStyle cap) noexcept override;
        void setStrokeDash(double dash_length, double gap_length) noexcept override;
        void setStrokeDash(int32_t array_length, const double* array, double scale) noexcept override;
        void setStrokeSolid() noexcept override;

        void setBlendMode(BlendMode blend_mode) noexcept override;
        void setBlendModeNormal() noexcept override;
        void setBlendModeMultiply() noexcept override;

        void enableAliasing() noexcept override;
        void disableAliasing() noexcept override;
        void enableFontSmoothing() noexcept override;
        void disableFontSmoothing() noexcept override;
        void enableFontSubpixelQuantization() noexcept override;
        void disableFontSubpixelQuantization() noexcept override;

        void setTextMatrix(double a, double b, double c, double d, double tx, double ty) noexcept override;

        void beginPath() noexcept override;
        void moveTo(double x, double y) noexcept override;
        void moveTo(const Vec2d& point) noexcept override;
        void lineTo(double x, double y) noexcept override;
        void lineTo(double x, double y, bool start_flag) noexcept override;
        void lineTo(const Vec2d& point) noexcept override;
        void lineTo(const Vec2d& point, bool start_flag) noexcept override;
        void curveTo(double c1x, double c1y, double c2x, double c2y, double x, double y) noexcept override;
        void curveTo(const Vec2d& control1, const Vec2d& control2, const Vec2d& point) noexcept override;
        void curveTo(double cx, double cy, double x, double y) noexcept override;
        void closePath() noexcept override;
        void fillPath() noexcept override;
        void fillPathEvenOdd() noexcept override;
        void strokePath() noexcept override;
        void drawPath() noexcept override;

        void addRectPath(double x, double y, double width, double height) noexcept override;
        void addEllipsePath(const Rectd& rect) noexcept override;
        void addCirclePath(double x, double y, double radius) noexcept override;
        void addRingPath(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept override;

        void fillRect(double x, double y, double width, double height) noexcept override;

        void strokeRect(double x, double y, double width, double height) noexcept override;

        void fillEllipse(double x, double y, double rh, double rv) noexcept override;
        void strokeEllipse(double x, double y, double rh, double rv) noexcept override;

        void fillCircle(double x, double y, double radius) noexcept override;
        void strokeCircle(double x, double y, double radius) noexcept override;

        void drawGradient(Gradient* gradient, const Vec2d& start_pos, const Vec2d& end_pos, bool draw_before, bool draw_after) noexcept override;
        void drawRadialGradient(Gradient* gradient, const Vec2d& pos, double radius, bool draw_before, bool draw_after) noexcept override;

        void drawImage(Image* image, const Rectd& rect, float alpha = 1.0f) noexcept override;
        ErrorCode drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral) noexcept override;
        ErrorCode drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral, float alpha) noexcept override;

        void drawIcon(const Icon* icon, const Rectd& rect, float alpha) noexcept override;
        void drawIcon(const Icon* icon, const Rectd& rect, const RGB& color, float alpha) noexcept override;
        void drawIconInCircle(const Icon* icon, const Vec2d& center, double radius, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept override;

        Rectd textRect(const char* text, const Font* font) noexcept override;
        void drawText(const char* text, const Vec2d& pos, const Font* font, const RGB& color, float alpha = 1.0f) noexcept override;
        double drawTextLineByLine(const char* text, const Rectd& bounds_rect, const Rectd& rect, double line_gap, const Font* font, const RGB& color, float alpha = 1.0f) noexcept override;
        void addTextPath(const char* text, const Font* font) noexcept override;

        void clipPath() noexcept override;
        void clipPathEvenOdd() noexcept override;
        Rectd clipBoundsRect() noexcept override;
        void resetClip() noexcept override;

        void translate(double tx, double ty) noexcept override;
        void scale(double sx, double sy) noexcept override;
        void rotate(double angle) noexcept override;
        void affineTransform(const Mat3d& matrix) noexcept override;
    };


} // End of namespace Grain

#endif // GrainMacCGContext_hpp