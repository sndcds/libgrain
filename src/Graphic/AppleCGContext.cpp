//
//  AppleCGContext.cpp
//
//  Created by Roald Christesen on from 24.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#include "Graphic/AppleCGContext.hpp"
#include "Image/Image.hpp"
#include "Core/Log.hpp"
#include "Graphic/Font.hpp"
#include "Color/Gradient.hpp"
#include "GUI/Components/Component.hpp"

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>


namespace Grain {

    void _macosView_setContextByComponent(AppleCGContext* gc, Component* component);


    AppleCGContext::AppleCGContext() noexcept : GraphicContext() {
        _macGCInit();
    }


    AppleCGContext::AppleCGContext(Component* component) noexcept {
        _macGCInit();
        _macosView_setContextByComponent(this, component);
    }


    AppleCGContext::~AppleCGContext() noexcept {
        _macCGFreeResources();
    }


    void AppleCGContext::setCGContextByComponent(CGContextRef context, Component* component) noexcept {
        m_cg_context = context;
        m_cg_color_space = CGColorSpaceCreateDeviceRGB();
        // m_cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);  // TODO: An alternative?

        if (component != nullptr) {
            m_flipped_y = component->isFlippedView();
            m_width = component->width();
            m_height = component->height();
            m_component = component;
        }
    }


    void AppleCGContext::log(Log& l) const noexcept {
    }


    void AppleCGContext::_macGCInit() noexcept {
        m_magic = Type::fourcc('m', 'a', 'c', ' ');
    }


    void AppleCGContext::_macCGFreeResources() noexcept {
        CGColorSpaceRelease(m_cg_color_space);
        m_cg_color_space = nullptr;
    }


    void AppleCGContext::setComponent(Component* component) noexcept {
        _macosView_setContextByComponent(this, component);
    }


    void AppleCGContext::setImage(Image* image) noexcept {
        GraphicContext::setImage(image);

        if (image != nullptr) {
            GRAIN_RETAIN(image);
            m_image = image;
            image->graphicContext(this);

            m_width = image->width();
            m_height = image->height();

            switch (image->colorModel()) {
                case Color::Model::Lumina:
                case Color::Model::LuminaAlpha:
                    m_cg_color_space = CGColorSpaceCreateDeviceGray();
                    break;

                case Color::Model::CMYK:
                    m_cg_color_space = CGColorSpaceCreateDeviceCMYK();

                case Color::Model::RGB:
                case Color::Model::RGBA:
                default:
                    // m_cg_color_space = CGColorSpaceCreateDeviceRGB();
                    m_cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
                    break;
            }

            if (m_cg_context) {
                CGContextSetAllowsAntialiasing(m_cg_context, true);
                CGContextSetShouldAntialias(m_cg_context, true);

                CGContextSetAllowsFontSmoothing(m_cg_context, true);
                CGContextSetShouldSmoothFonts(m_cg_context, true);

                CGContextSetAllowsFontSubpixelPositioning(m_cg_context, true);
                CGContextSetShouldSubpixelPositionFonts(m_cg_context, true);

                CGContextSetAllowsFontSubpixelQuantization(m_cg_context, false);
                CGContextSetShouldSubpixelQuantizeFonts(m_cg_context, false);

                CGContextSetFillColorSpace(m_cg_context, m_cg_color_space);
                CGContextSetStrokeColorSpace(m_cg_context, m_cg_color_space);
            }
        }
    }


    bool AppleCGContext::isValid() noexcept {
        return m_cg_context != nullptr;
    }


    void AppleCGContext::save() noexcept {
        CGContextSaveGState(m_cg_context);
        m_state_depth++;
    }


    void AppleCGContext::restore() noexcept {
        if (m_state_depth > 0) {
            CGContextRestoreGState(m_cg_context);
            m_state_depth--;
        }
    }

    void AppleCGContext::setAlpha(float alpha) noexcept {
        m_alpha = alpha;
        CGContextSetAlpha(m_cg_context, alpha);
    }


    void AppleCGContext::setFillColor(float r, float g, float b, float alpha) noexcept {
        m_fill_color.setRGBA(r, g, b, alpha);
        CGContextSetRGBFillColor(m_cg_context, r, g, b, alpha);
    }


    void AppleCGContext::setStrokeColor(float r, float g, float b, float alpha) noexcept {
        m_stroke_color.setRGBA(r, g, b, alpha);
        CGContextSetRGBStrokeColor(m_cg_context, r, g, b, alpha);
    }


    void AppleCGContext::setStrokeWidth(double width) noexcept {
        CGContextSetLineWidth(m_cg_context, width);
    }


    void AppleCGContext::setStrokeMiterLimit(double limit) noexcept {
        CGContextSetMiterLimit(m_cg_context, limit);
    }

    void AppleCGContext::setStrokeJoinStyle(StrokeJoinStyle join) noexcept {
        CGContextSetLineJoin(m_cg_context, (CGLineJoin)join);
    }

    void AppleCGContext::setStrokeCapStyle(StrokeCapStyle cap) noexcept {
        CGContextSetLineCap(m_cg_context, (CGLineCap)cap);
    }

    void AppleCGContext::setStrokeDash(double dash_length, double gap_length) noexcept {
        CGFloat l[2] = { dash_length, gap_length };
        CGContextSetLineDash(m_cg_context, 0.0, l, 2);
    }

    void AppleCGContext::setStrokeDash(int32_t array_length, const double* array, double scale) noexcept {
        if (array_length < 0 || !array) {
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
    }

    void AppleCGContext::setStrokeSolid() noexcept {
        CGContextSetLineDash(m_cg_context, 0, nullptr, 0);
    }

    void AppleCGContext::setBlendMode(BlendMode blend_mode) noexcept {
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
    }


    void AppleCGContext::setBlendModeNormal() noexcept {
        CGContextSetBlendMode(m_cg_context, kCGBlendModeNormal);
    }


    void AppleCGContext::setBlendModeMultiply() noexcept {
        CGContextSetBlendMode(m_cg_context, kCGBlendModeMultiply);
    }


    void AppleCGContext::enableAliasing() noexcept {
        CGContextSetAllowsAntialiasing(m_cg_context, true);
    }


    void AppleCGContext::disableAliasing() noexcept {
        CGContextSetAllowsAntialiasing(m_cg_context, false);
    }


    void AppleCGContext::enableFontSmoothing() noexcept {
        CGContextSetShouldSmoothFonts(m_cg_context, true);
    }


    void AppleCGContext::disableFontSmoothing() noexcept {
        CGContextSetShouldSmoothFonts(m_cg_context, false);
    }


    void AppleCGContext::enableFontSubpixelQuantization() noexcept {
        CGContextSetShouldSubpixelQuantizeFonts(m_cg_context, true);
    }


    void AppleCGContext::disableFontSubpixelQuantization() noexcept {
        CGContextSetShouldSubpixelQuantizeFonts(m_cg_context, false);
    }


    void AppleCGContext::setTextMatrix(double a, double b, double c, double d, double tx, double ty) noexcept {
        if (!m_flipped_y) {
            d = -d;
        }
        CGContextSetTextMatrix(m_cg_context, CGAffineTransformMake(a, b, c, d, tx, ty));
    }


    void AppleCGContext::beginPath() noexcept {
        CGContextBeginPath(m_cg_context);
    }


    void AppleCGContext::moveTo(double x, double y) noexcept {
        CGContextMoveToPoint(m_cg_context, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void AppleCGContext::moveTo(const Vec2d& point) noexcept {
        CGContextMoveToPoint(m_cg_context, point.m_x, point.m_y);
        m_last_pos = point;
    }


    void AppleCGContext::lineTo(double x, double y) noexcept {
        CGContextAddLineToPoint(m_cg_context, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void AppleCGContext::lineTo(double x, double y, bool start_flag) noexcept {
        if (start_flag) {
            CGContextMoveToPoint(m_cg_context, x, y);
        }
        else {
            CGContextAddLineToPoint(m_cg_context, x, y);
        }
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void AppleCGContext::lineTo(const Vec2d& point) noexcept {
        CGContextAddLineToPoint(m_cg_context, point.m_x, point.m_y);
        m_last_pos = point;
    }

    void AppleCGContext::lineTo(const Vec2d& point, bool start_flag) noexcept {
        if (start_flag) {
            CGContextMoveToPoint(m_cg_context, point.m_x, point.m_y);
        }
        else {
            CGContextAddLineToPoint(m_cg_context, point.m_x, point.m_y);
        }
        m_last_pos = point;
    }


    void AppleCGContext::curveTo(double c1x, double c1y, double c2x, double c2y, double x, double y) noexcept {
        CGContextAddCurveToPoint(m_cg_context, c1x, c1y, c2x, c2y, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void AppleCGContext::curveTo(const Vec2d& control1, const Vec2d& control2, const Vec2d& point) noexcept {
        CGContextAddCurveToPoint(m_cg_context, control1.m_x, control1.m_y, control2.m_x, control2.m_y, point.m_x, point.m_y);
        m_last_pos = point;
    }


    void AppleCGContext::curveTo(double cx, double cy, double x, double y) noexcept {
        double c1x = m_last_pos.m_x + 2.0 / 3.0 * (cx - m_last_pos.m_x);
        double c2x = x + 2.0 / 3.0 * (cx - x);
        double c1y = m_last_pos.m_y + 2.0 / 3.0 * (cy - m_last_pos.m_y);
        double c2y = y + 2.0 / 3.0 * (cy - y);
        CGContextAddCurveToPoint(m_cg_context, c1x, c1y, c2x, c2y, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void AppleCGContext::closePath() noexcept {
        CGContextClosePath(m_cg_context);
    }


    void AppleCGContext::fillPath() noexcept {
        CGContextFillPath(m_cg_context);
    }


    void AppleCGContext::fillPathEvenOdd() noexcept {
        CGContextEOFillPath(m_cg_context);
    }


    void AppleCGContext::strokePath() noexcept {
        CGContextStrokePath(m_cg_context);
    }


    void AppleCGContext::drawPath() noexcept {
        CGContextDrawPath(m_cg_context, kCGPathFillStroke);
    }


    void AppleCGContext::addRectPath(double x, double y, double width, double height) noexcept {
        CGContextAddRect(m_cg_context, CGRectMake(x, y, width, height));
    }


    void AppleCGContext::addEllipsePath(const Rectd& rect) noexcept {
        CGContextAddEllipseInRect(m_cg_context, rect.cgRect());
    }


    void AppleCGContext::addCirclePath(double x, double y, double radius) noexcept {
        double d = radius + radius;
        CGContextAddEllipseInRect(m_cg_context, CGRectMake(x - radius, y - radius, d, d));
    }


    void AppleCGContext::addRingPath(
            const Vec2d& center,
            double inner_radius,
            double outer_radius,
            double angle,
            double span) noexcept {

        if (span > 0.0) {
            angle = Math::degtorad(angle);
            span = Math::degtorad(span);
            double endAngle = angle + span;
            CGContextAddArc(m_cg_context, center.m_x, center.m_y, outer_radius, angle, endAngle, false);
            CGContextAddArc(m_cg_context, center.m_x, center.m_y, inner_radius, endAngle, angle, true);
            closePath();
        }
    }


    void AppleCGContext::fillRect(double x, double y, double width, double height) noexcept {
        if (width <= 0.0 && height <= 0.0) return;
        CGContextFillRect(m_cg_context, CGRectMake(x, y, width, height));
    }


    void AppleCGContext::strokeRect(double x, double y, double width, double height) noexcept {
        CGContextStrokeRect(m_cg_context, CGRectMake(x, y, width, height));
    }


    void AppleCGContext::fillEllipse(double x, double y, double rh, double rv) noexcept {
        CGContextFillEllipseInRect(m_cg_context, CGRectMake(x - rh, y - rv, rh * 2, rv * 2));
    }


    void AppleCGContext::strokeEllipse(double x, double y, double rh, double rv) noexcept {
        CGContextStrokeEllipseInRect(m_cg_context, CGRectMake(x - rh, y - rv, rh * 2, rv * 2));
    }


    void AppleCGContext::fillCircle(double x, double y, double radius) noexcept {
        if (radius <= std::numeric_limits<float>::epsilon()) return;
        double diameter = radius + radius;
        CGContextFillEllipseInRect(m_cg_context, CGRectMake(x - radius, y - radius, diameter, diameter));
    }


    void AppleCGContext::strokeCircle(double x, double y, double radius) noexcept {
        if (radius <= std::numeric_limits<float>::epsilon()) return;
        double diameter = radius + radius;
        CGContextStrokeEllipseInRect(m_cg_context, CGRectMake(x - radius, y - radius, diameter, diameter));
    }


    void AppleCGContext::drawGradient(
            Gradient* gradient,
            const Vec2d& start_pos,
            const Vec2d& end_pos,
            bool draw_before,
            bool draw_after) noexcept {

        if (!gradient || gradient->stopCount() < 2)  return;

        gradient->update(this);
        auto cg_gradient = gradient->macos_cgGradient(this);
        if (cg_gradient) {
            CGGradientDrawingOptions options = 0x0;

            if (draw_before) {
                options |= kCGGradientDrawsBeforeStartLocation;
            }
            if (draw_after) {
                options |= kCGGradientDrawsAfterEndLocation;
            }

            CGContextDrawLinearGradient(
                    cgContext(),
                    cg_gradient,
                    start_pos.cgPoint(),
                    end_pos.cgPoint(),
                    options);
        }
    }


    void AppleCGContext::drawRadialGradient(
            Gradient* gradient,
            const Vec2d& pos,
            double radius,
            bool draw_before,
            bool draw_after) noexcept {

        if (!gradient || gradient->stopCount() < 2)  return;

        gradient->updateLUT();
        gradient->update(this);

        auto cg_gradient = gradient->macos_cgGradient(this);
        if (cg_gradient) {
            CGGradientDrawingOptions options = 0x0;

            if (draw_before) {
                options |= kCGGradientDrawsBeforeStartLocation;
            }
            if (draw_after) {
                options |= kCGGradientDrawsAfterEndLocation;
            }

            CGContextDrawRadialGradient(cgContext(), cg_gradient, pos.cgPoint(), 0, pos.cgPoint(), radius, options);
        }
    }


    void AppleCGContext::drawImage(Image* image, const Rectd& rect, float alpha) noexcept {
        if (!image || !image->hasPixel()) return;
        if (CGImageRef cg_image = image->macos_cgImageRef()) {
            CGContextSaveGState(m_cg_context);
            if (m_flipped_y) {
                CGContextTranslateCTM(m_cg_context, 0, rect.centerY());
                CGContextScaleCTM(m_cg_context, 1, -1);
                CGContextTranslateCTM(m_cg_context, 0, -rect.centerY());
            }
            CGContextSetAlpha(m_cg_context, alpha);
            CGContextSetInterpolationQuality(m_cg_context, kCGInterpolationMedium);
            CGContextDrawImage(m_cg_context, rect.cgRect(), cg_image);
            CGContextRestoreGState(m_cg_context);
        }
    }


    ErrorCode AppleCGContext::drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral) noexcept {
        /* TODO !!!!!!
        auto result = ErrorCode::None;

        @autoreleasepool {

        try {

            CGImageRef cg_image = image->macos_cgImageRef();
            CIImage* ci_image = [CIImage imageWithCGImage:cg_image];
            if (!ci_image) {
                throw ErrorCode::Fatal;
            }

            // Create the perspective transform filter.
            CIFilter* perspective_filter = [CIFilter filterWithName:@"CIPerspectiveTransform"];
            if (!perspective_filter) {
                throw ErrorCode::Fatal;
            }

            [perspective_filter setValue:ci_image forKey:kCIInputImageKey];

            auto points = quadrilateral.pointsPtr();

            // Define the quadrilateral points for transformation
            [perspective_filter setValue:[CIVector vectorWithX:points[0].m_x Y:points[0].m_y] forKey:@"inputTopLeft"];
            [perspective_filter setValue:[CIVector vectorWithX:points[1].m_x Y:points[1].m_y] forKey:@"inputTopRight"];
            [perspective_filter setValue:[CIVector vectorWithX:points[2].m_x Y:points[2].m_y] forKey:@"inputBottomRight"];
            [perspective_filter setValue:[CIVector vectorWithX:points[3].m_x Y:points[3].m_y] forKey:@"inputBottomLeft"];

            // Get the output CIImage from the filter.
            CIImage* output_ci_image = [perspective_filter outputImage];
            if (!output_ci_image) {
                throw ErrorCode::Fatal;
            }

            // Create a CIContext for rendering.
            CIContext* ci_context = [CIContext contextWithCGContext:m_cg_context options:nil];

            CGRect target_rect = output_ci_image.extent;

            // Render the CIImage to the CGContextRef within the target rectangle
            [ci_context drawImage:output_ci_image inRect:target_rect fromRect:[output_ci_image extent]];

            // Manual memory cleanup.   // TODO: Check what needs do be released!
            // [ci_image release]; // Not necessary, momory is autoreleased!
            // [perspective_filter release]; // Not necessary, momory is autoreleased!
            // [output_ci_image release]; // Not necessary, momory is autoreleased!
            // [ci_context release]; // Not necessary, momory is autoreleased!

        }
        catch (ErrorCode err) {
            result = err;
        }

        } // End of @autoreleasepool

        */
        return ErrorCode::None;
    }


    ErrorCode AppleCGContext::drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral, float alpha) noexcept {
        /* TODO !!!!!!
        // TODO: Combine the two methods GraphicContext::drawQuadrilateralImage into one.

        // Get CGImage reference and convert it to CIImage
        CGImageRef cg_image = image->macos_cgImageRef();
        CIImage* ci_image = [CIImage imageWithCGImage:cg_image];
        CGImageRelease(cg_image);
        if (!ci_image) {
            return ErrorCode::Fatal;
        }

        // Create the perspective transform filter
        CIFilter* perspective_filter = [CIFilter filterWithName:@"CIPerspectiveTransform"];
        if (!perspective_filter) {
            return ErrorCode::Fatal;
        }
        [perspective_filter setValue:ci_image forKey:kCIInputImageKey];

        // Define the quadrilateral points for transformation
        auto points = quadrilateral.pointsPtr();
        [perspective_filter setValue:[CIVector vectorWithX:points[0].m_x Y:points[0].m_y] forKey:@"inputTopLeft"];
        [perspective_filter setValue:[CIVector vectorWithX:points[1].m_x Y:points[1].m_y] forKey:@"inputTopRight"];
        [perspective_filter setValue:[CIVector vectorWithX:points[2].m_x Y:points[2].m_y] forKey:@"inputBottomRight"];
        [perspective_filter setValue:[CIVector vectorWithX:points[3].m_x Y:points[3].m_y] forKey:@"inputBottomLeft"];

        // Get the output CIImage from the perspective filter
        CIImage* output_ci_image = [perspective_filter outputImage];
        if (!output_ci_image) {
            return ErrorCode::Fatal;
        }

        // Apply alpha transparency using a color generator and compositing filter
        CIFilter* alphaFilter = [CIFilter filterWithName:@"CIConstantColorGenerator"];
        CIColor* transparentColor = [CIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:alpha];
        [alphaFilter setValue:transparentColor forKey:kCIInputColorKey];

        CIFilter* compositeFilter = [CIFilter filterWithName:@"CIMultiplyCompositing"];
        [compositeFilter setValue:alphaFilter.outputImage forKey:kCIInputBackgroundImageKey];
        [compositeFilter setValue:output_ci_image forKey:kCIInputImageKey];

        CIImage* transparent_image = compositeFilter.outputImage;

        // Create a CIContext for rendering
        CIContext* ci_context = [CIContext contextWithCGContext:m_cg_context options:nil];

        CGRect target_rect = transparent_image.extent;

        // Render the transparent CIImage to the CGContextRef within the target rectangle
        [ci_context drawImage:transparent_image inRect:target_rect fromRect:[output_ci_image extent]];

        // Manual memory cleanup.   // TODO: Check what needs do be released!
        [ci_image release];
        // [perspective_filter release];
        // [compositeFilter release];
        // [output_ci_image release];
        [ci_context release];

         */
        return ErrorCode::None;
    }


    void AppleCGContext::drawIcon(const Icon* icon, const Rectd& rect, float alpha) noexcept {
        /* TODO !!!!!!
        if (icon != nullptr) {

        CGContextSaveGState(m_cg_context);

        // CGContextTranslateCTM(m_cg_context, 0.0, m_height);
        CGContextScaleCTM(m_cg_context, 1.0, -1.0);
        CGContextTranslateCTM(m_cg_context, 0.0, - rect.m_y * 2 - rect.m_height);

        setAlpha(alpha);
        CGContextDrawImage(m_cg_context, rect.cgRect(), icon->_m_cg_image);

        CGContextRestoreGState(m_cg_context);
        }
         */
    }


    void AppleCGContext::drawIcon(const Icon* icon, const Rectd& rect, const RGB& color, float alpha) noexcept {
        /* TODO !!!!!!
        if (icon != nullptr) {

        CGContextSaveGState(m_cg_context);

        // CGContextTranslateCTM(m_cg_context, 0.0, m_height);
        CGContextScaleCTM(m_cg_context, 1, -1);
        CGContextTranslateCTM(m_cg_context, 0, -rect.m_height - rect.m_y * 2 );

        // CGContextDrawImage(m_cg_context, rect.cgRect(), icon->_m_cg_image);
        CGContextClipToMask(m_cg_context, rect.cgRect(), icon->_m_cg_image);
        setFillColor(color, alpha);
        fillRect(rect);

        CGContextRestoreGState(m_cg_context);
        }
         */
    }


    void AppleCGContext::drawIconInCircle(const Icon* icon, const Vec2d& center, double radius, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept {
        /* TODO: ...

        Rectd rect(center.m_x - radius, center.m_y - radius, radius * 2, radius * 2);

        if (bg_alpha > 0) {

        setFillColor(bg_color, bg_alpha);
        fillEllipse(rect);

        if (border_width > 0) {
        rect.inset(border_width * 0.5f);
        setStrokeWidth(border_width);
        setStrokeColor(border_color, border_alpha);
        strokeEllipse(rect);
        }
        }

        if (icon != nullptr) {

        drawIcon(icon, icon->getCenteredRect(rect), icon_color, icon_alpha);
        }

        */
    }


    Rectd AppleCGContext::textRect(const char* text, const Font* font) noexcept {
        Rectd text_rect;
        text_rect.zero();

        if (text != nullptr && font != nullptr) {
            setTextMatrix(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

            CTFontRef ct_font = font->ctFont();
            CFDictionaryRef cf_str_attr = CFDictionaryCreate(
                    kCFAllocatorDefault,
                    (const void**)&kCTFontAttributeName,
                    (const void**)&ct_font,
                    1,
                    &kCFTypeDictionaryKeyCallBacks,
                    &kCFTypeDictionaryValueCallBacks);

            CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
            CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

            CGRect line_bounds = CTLineGetImageBounds(line, m_cg_context);
            text_rect.set(0, 0, line_bounds.size.width, line_bounds.size.height);

            CFRelease(cf_str);
            CFRelease(cf_str_attr);
            CFRelease(cf_attr_str);
            CFRelease(line);
        }

        return text_rect;
    }


    void AppleCGContext::drawText(
            const char* text,
            const Vec2d& pos,
            const Font* font,
            const RGB& color,
            float alpha) noexcept
    {
        if (!font || !text) return;

        setTextMatrix(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

        CGColorRef cg_text_color = color.createCGColor(alpha);

        CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
        CFTypeRef values[] = { font->ctFont(), cg_text_color };

        CFDictionaryRef cf_str_attr =
                CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
                                   (const void**)&values, sizeof(keys) / sizeof(keys[0]),
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);


        CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
        CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);

        CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

        CGContextSetTextPosition(m_cg_context, pos.m_x, pos.m_y);
        CTLineDraw(line, m_cg_context);

        CFRelease(cf_str);
        CFRelease(cf_str_attr);
        CFRelease(cf_attr_str);
        CFRelease(line);
        CGColorRelease(cg_text_color);
    }


    double AppleCGContext::drawTextInRect(
            const char* text,
            const Rectd& rect,
            Alignment alignment,
            const Font* font,
            const RGB& color,
            float alpha) noexcept
    {

        if (!font || !text) return 0.0;

        setTextMatrix(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

        CGColorRef cg_text_color = color.createCGColor(alpha);

        CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
        CFTypeRef values[] = { font->ctFont(), cg_text_color };

        CFDictionaryRef cf_str_attr =
                CFDictionaryCreate(
                        kCFAllocatorDefault,
                        (const void**)&keys,
                        (const void**)&values,
                        2, // sizeof(keys) / sizeof(keys[0]),
                        &kCFTypeDictionaryKeyCallBacks,
                        &kCFTypeDictionaryValueCallBacks);

        CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
        CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);

        CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

        CGFloat ascent, descent, leading;
        double text_width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
        double text_x, text_y;

        // Horizontal alignment
        switch (alignment) {
            case Alignment::Center:
            case Alignment::Top:
            case Alignment::Bottom:
                text_x = rect.m_x + rect.m_width / 2 - text_width / 2;
                break;

            case Alignment::Right:
            case Alignment::TopRight:
            case Alignment::BottomRight:
                text_x = rect.m_x + rect.m_width - text_width;
                break;

            default:
                text_x = rect.m_x;
                break;
        }

        // Vertical alignment
        switch (alignment) {
            case Alignment::Center:
            case Alignment::Left:
            case Alignment::Right:
                if (m_flipped_y == true) {
                    text_y = rect.m_y + rect.m_height / 2 + (ascent - descent) / 2;
                }
                else {
                    text_y = rect.m_y + rect.m_height / 2 - (ascent - descent) / 2;
                }
                break;

            case Alignment::BottomLeft:
            case Alignment::Bottom:
            case Alignment::BottomRight:
                if (m_flipped_y == true) {
                    text_y = rect.m_y + rect.m_height - descent;
                }
                else {
                    text_y = rect.m_y + descent;
                }
                break;

            default:
                if (m_flipped_y == true) {
                    text_y = rect.m_y + ascent;
                }
                else {
                    text_y = rect.m_y + rect.m_height - ascent;
                }
                break;
        }

        CGContextSetTextPosition(m_cg_context, text_x, text_y);
        CTLineDraw(line, m_cg_context);

        CGRect line_bounds = CTLineGetImageBounds(line, m_cg_context);

        CFRelease(cf_str);
        CFRelease(cf_str_attr);
        CFRelease(cf_attr_str);
        CFRelease(line);
        CGColorRelease(cg_text_color);

        return line_bounds.size.width;
    }


    double AppleCGContext::drawWrappedText(
            const char* text,
            const Rectd& bounds_rect,
            const Rectd& rect,
            TextAlignment alignment,
            double line_gap,
            const Font* font,
            const RGB& color,
            float alpha) noexcept
    {
        double total_height = 0.0;

        // Convert our custom alignment enum to CoreText alignment
        CTTextAlignment ct_alignment = kCTTextAlignmentLeft;
        switch (alignment) {
            case TextAlignment::Center:
                ct_alignment = kCTTextAlignmentCenter;
                break;
            case TextAlignment::Right:
                ct_alignment = kCTTextAlignmentRight;
                break;
            case TextAlignment::Justified:
                ct_alignment = kCTTextAlignmentJustified;
                break;
            case TextAlignment::Left:
            default:
                ct_alignment = kCTTextAlignmentLeft;
                break;
        }

        // Build paragraph style (alignment + line spacing)
        CGFloat cg_line_gap = line_gap;
        CTParagraphStyleSetting settings[] = {
                { kCTParagraphStyleSpecifierAlignment, sizeof(ct_alignment), &ct_alignment },
                { kCTParagraphStyleSpecifierLineSpacingAdjustment, sizeof(cg_line_gap), &cg_line_gap }
        };
        CTParagraphStyleRef paragraph_style =
                CTParagraphStyleCreate(settings, sizeof(settings) / sizeof(settings[0]));

        // Text attributes
        CGColorRef cg_text_color = color.createCGColor(alpha);
        CFStringRef keys[]   = { kCTFontAttributeName, kCTForegroundColorAttributeName, kCTParagraphStyleAttributeName };
        CFTypeRef   values[] = { font->ctFont(), cg_text_color, paragraph_style };

        CFDictionaryRef attrs = CFDictionaryCreate(
                kCFAllocatorDefault,
                (const void**)&keys,
                (const void**)&values,
                3,
                &kCFTypeDictionaryKeyCallBacks,
                &kCFTypeDictionaryValueCallBacks);

        CFStringRef cf_str = CFStringCreateWithCString(nullptr, text, kCFStringEncodingUTF8);
        CFAttributedStringRef attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, attrs);

        // Create framesetter
        CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString(attr_str);

        CGRect cg_rect = rect.cgRect();

        // Create a local path at origin (0, 0, width, height)
        CGRect localRect = CGRectMake(0, 0, cg_rect.size.width, cg_rect.size.height);
        CGPathRef path = CGPathCreateWithRect(localRect, NULL);
        CTFrameRef frame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), path, NULL);

        // Draw only if visible
        if (rect.overlaps(bounds_rect)) {
            CGContextSaveGState(m_cg_context);
            CGContextSetTextMatrix(m_cg_context, CGAffineTransformIdentity);

            // Move origin to bottom-left of the target rect and flip Y
            CGContextTranslateCTM(m_cg_context, cg_rect.origin.x, cg_rect.origin.y + cg_rect.size.height);
            CGContextScaleCTM(m_cg_context, 1.0, -1.0);

            // Let CoreText draw the frame (alignment, soft-hyphens, etc. work here)
            CTFrameDraw(frame, m_cg_context);

            CGContextRestoreGState(m_cg_context);
        }

        // Measure total height (for layout logic)
        CGSize suggestedSize = CTFramesetterSuggestFrameSizeWithConstraints(
                framesetter,
                CFRangeMake(0, 0),
                nullptr,
                cg_rect.size,
                nullptr);

        total_height = suggestedSize.height;

        // Cleanup
        CFRelease(cf_str);
        CFRelease(attrs);
        CFRelease(attr_str);
        CGColorRelease(cg_text_color);
        CFRelease(paragraph_style);
        CFRelease(framesetter);
        CFRelease(path);
        CFRelease(frame);

        return total_height;
    }


    void AppleCGContext::addTextPath(const char* text, const Font* font) noexcept {
        /* TODO: !!!!!! Implement!
        if (!text || !font) {
            return;
        }

        NSString* ns_string = [NSString stringWithUTF8String:text];
        int32_t length = (int32_t)ns_string.length;
        if (length < 1) {
            return;
        }

        unichar chars[length];
        CGGlyph glyphs[length];
        CGPoint positions[length];
        CGSize advances[length];
//    Rectd textBounds = getTextBounds(text, font);

        CGRect text_rect = CGRectMake(0, 0, 1000, 1000);

        [ns_string getCharacters:chars range:NSMakeRange(0, length)];
        CTFontRef ct_font = font->ctFont();

        CTFontGetGlyphsForCharacters(ct_font, chars, glyphs, length);
        CTFontGetAdvancesForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, advances, length);

        CGPoint position = CGPointMake(0, 0);
        for (int32_t i = 0; i < length; i++) {
            positions[i] = CGPointMake(position.x, position.y);
            CGSize advance = advances[i];
            position.x += advance.width;
            position.y += advance.height;
        }

        CGRect bounding_box = CTFontGetBoundingRectsForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, NULL, length);

        CGContextTranslateCTM(m_cg_context, 0.0, text_rect.origin.y);
        CGContextTranslateCTM(m_cg_context, 0.0, bounding_box.size.height);
        CGContextScaleCTM(m_cg_context, 1.0, -1.0);
        CGContextTranslateCTM(m_cg_context, 0.0, -text_rect.origin.y);

        for (int32_t i = 0; i < length; i++) {
            CGPoint position = positions[i];
            CGAffineTransform tt = CGAffineTransformMakeTranslation(position.x, position.y);
            CGPathRef path = CTFontCreatePathForGlyph(ct_font, glyphs[i], &tt);
            CGContextAddPath(m_cg_context, path);
            CGPathRelease(path);
        }
         */
    }


    void AppleCGContext::clipPath() noexcept {
        CGContextClip(m_cg_context);
    }


    void AppleCGContext:: clipPathEvenOdd() noexcept {
        CGContextEOClip(m_cg_context);
    }


    Rectd AppleCGContext::clipBoundsRect() noexcept {
        return Rectd(CGContextGetClipBoundingBox(m_cg_context));
    }


    void AppleCGContext::resetClip() noexcept {
        CGContextResetClip(m_cg_context);
    }


    void AppleCGContext::translate(double tx, double ty) noexcept {
        CGContextTranslateCTM(m_cg_context, tx, ty);
    }


    void AppleCGContext::scale(double sx, double sy) noexcept {
        CGContextScaleCTM(m_cg_context, sx, sy);
    }


    void AppleCGContext::rotate(double angle) noexcept {
        CGContextRotateCTM(m_cg_context, angle * std::numbers::pi / 180.0);
    }


    void AppleCGContext::affineTransform(const Mat3d& matrix) noexcept {
        CGAffineTransform m;
        const double* p = matrix.dataPtr();
        m.a = p[0];
        m.b = p[1];
        m.c = p[3];
        m.d = p[4];
        m.tx = p[6];
        m.ty = p[7];
        CGContextConcatCTM(m_cg_context, m);
    }

} // End of namespace Grain