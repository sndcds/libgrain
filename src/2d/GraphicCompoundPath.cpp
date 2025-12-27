//
//  GraphicCompoundPath.cpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 23.08.2025
//

#include "2d/GraphicCompoundPath.hpp"
#include "2d/RangeRect.hpp"
#include "2d/GraphicPath.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "String/String.hpp"
#include "Geo/WKBParser.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/Font.hpp"
#include "Core/Log.hpp"


namespace Grain {

GraphicCompoundPath::GraphicCompoundPath() noexcept {
    paths_.reserve(4);
    offs_.zero();
}


GraphicCompoundPath::~GraphicCompoundPath() noexcept {
    clear();
}


GraphicPath* GraphicCompoundPath::pathPtrAtIndex(int32_t index) noexcept {
    return paths_.elementAtIndex(index);
}


GraphicPath* GraphicCompoundPath::lastPathPtr() noexcept {
    return paths_.lastElement();
}


bool GraphicCompoundPath::boundsRect(Rectd& out_bounds_rect) const noexcept {
    #pragma message("GraphicCompoundPath::boundsRect() must be implemented!")
    return false;
}


double GraphicCompoundPath::polygonCentroid(Vec2d& out_centroid) const noexcept {
    double total_area = 0;
    Vec2d compound_c;

    for (auto& path : paths_) {
        Vec2d c;
        double area = path->polygonCentroid(c);
        if (area != 0.0) {
            compound_c += c * area;
            total_area += area;
        }
    }

    if (total_area > std::numeric_limits<double>::epsilon()) {
        out_centroid = compound_c * (1.0 / total_area);
    }

    return total_area;
}


void GraphicCompoundPath::clear() noexcept {
    paths_.clear();
}


ErrorCode GraphicCompoundPath::addEmptyPath(int32_t point_capacity) noexcept {
    auto result = ErrorCode::None;

    try {
        auto path = new (std::nothrow) GraphicPath(point_capacity);
        paths_.push(path);
        must_add_path_ = false;
    }
    catch (const std::exception& e) {
        result = ErrorCode::StdCppException;
    }

    return result;
}


void _CGPathEnumerationCallback(void* info, const CGPathElement* element) {
    if (info) {
        ((GraphicCompoundPath*)info)->_addCGPathElement(element);
    }
}


/*
Rectd GraphicCompoundPath::buildFromText(const Font& font, const char* text) noexcept {
    #pragma message("GraphicCompoundPath::buildFromText() must be implemented!")
    return { 0, 0, 0, 0 };
}
*/

    /*
Rectd GraphicCompoundPath::buildFromText(const Font& font, const char* text) noexcept {

    Rectd bounds_rect;

    NSString* ns_string = nullptr;

    try {
        if (!text) {
            throw ErrorCode::NullData;
        }

        ns_string = [NSString stringWithUTF8String:text]; // Autoreleased!
        if (!ns_string) {
            throw ErrorCode::MemCantAllocate;
        }

        int32_t char_count = (int32_t)[ns_string length];
        if (char_count < 1) {
            throw ErrorCode::NoData;
        }

        // TODO: Maybe this a a problem with long text.
        unichar chars[char_count];
        CGGlyph glyphs[char_count];
        CGSize advances[char_count];
        CGRect bounding_rects[char_count];
        CTFontRef ct_font = font.ctFont();

        [ns_string getCharacters:chars range:NSMakeRange(0, char_count)];

        CTFontGetGlyphsForCharacters(ct_font, chars, glyphs, char_count);
        CTFontGetAdvancesForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, advances, char_count);

        // Unused:
        // CGRect bounding_box = CTFontGetBoundingRectsForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, bounding_rects, char_count);

        double min_x = bounding_rects[0].origin.x;
        double max_x = min_x;
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();

        bounds_rect.width_ = 0.0;
        must_add_path_ = true;
        for (int32_t i = 0; i < char_count; i++) {

            CGPathRef path = CTFontCreatePathForGlyph(ct_font, glyphs[i], NULL);
            if (path) {
                CGPathApply(path, this, _CGPathEnumerationCallback);
                CGPathRelease(path);
            }

            offs_.translate(advances[i].width, advances[i].height);

            if (bounding_rects[i].origin.y < min_x) {
                min_x = bounding_rects[i].origin.x;
            }

            if (bounding_rects[i].origin.y < min_y) {
                min_y = bounding_rects[i].origin.y;
            }

            if (i < char_count - 1) {
                max_x += advances[i].width;
            }
            else {
                max_x += bounding_rects[i].size.width;
            }

            if (bounding_rects[i].origin.y + bounding_rects[i].size.height > max_y) {
                max_y = bounding_rects[i].origin.y + bounding_rects[i].size.height;
            }
        }

        bounds_rect.x_ = min_x;
        bounds_rect.m_y = min_y - max_y - min_y;
        bounds_rect.width_ = max_x - min_x;
        bounds_rect.height_ = max_y - min_y;

        finish();
    }
    catch (ErrorCode err) {
    }

    return bounds_rect;
}
*/

Rectd GraphicCompoundPath::buildFromText(const Font& font, const char* text) noexcept {
    Rectd bounds_rect;

    try {
        if (!text) {
            throw ErrorCode::NullData;
        }

        // Create CFString directly from UTF-8 C-string
        CFStringRef cfString = CFStringCreateWithCString(
            kCFAllocatorDefault,
            text,
            kCFStringEncodingUTF8
        );
        if (!cfString) {
            throw ErrorCode::MemCantAllocate;
        }

        // Get character count
        CFIndex char_count = CFStringGetLength(cfString);
        if (char_count < 1) {
            CFRelease(cfString);
            throw ErrorCode::NoData;
        }

        std::vector<UniChar> chars(char_count);
        CFStringGetCharacters(cfString, CFRangeMake(0, char_count), chars.data());
        CFRelease(cfString);

        std::vector<CGGlyph> glyphs(char_count);
        std::vector<CGSize> advances(char_count);
        std::vector<CGRect> bounding_rects(char_count);

        CTFontRef ct_font = font.ctFont();
        CTFontGetGlyphsForCharacters(ct_font, chars.data(), glyphs.data(), char_count);
        CTFontGetAdvancesForGlyphs(ct_font, kCTFontOrientationDefault, glyphs.data(), advances.data(), char_count);
        CTFontGetBoundingRectsForGlyphs(ct_font, kCTFontOrientationDefault, glyphs.data(), bounding_rects.data(), char_count);

        double min_x = bounding_rects[0].origin.x;
        double max_x = min_x;
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();

        bounds_rect.width_ = 0.0;
        must_add_path_ = true;

        for (CFIndex i = 0; i < char_count; ++i) {
            CGPathRef path = CTFontCreatePathForGlyph(ct_font, glyphs[i], nullptr);
            if (path) {
                CGPathApply(path, this, _CGPathEnumerationCallback);
                CGPathRelease(path);
            }

            offs_.translate(advances[i].width, advances[i].height);

            if (bounding_rects[i].origin.x < min_x)
                min_x = bounding_rects[i].origin.x;
            if (bounding_rects[i].origin.y < min_y)
                min_y = bounding_rects[i].origin.y;

            if (i < char_count - 1)
                max_x += advances[i].width;
            else
                max_x += bounding_rects[i].size.width;

            if (bounding_rects[i].origin.y + bounding_rects[i].size.height > max_y)
                max_y = bounding_rects[i].origin.y + bounding_rects[i].size.height;
        }

        bounds_rect.x_ = min_x;
        bounds_rect.y_ = min_y;
        bounds_rect.width_ = max_x - min_x;
        bounds_rect.height_ = max_y - min_y;

        finish();
    } catch (ErrorCode err) {
        // You might log or set an error state here
    }

    return bounds_rect;
}


Rectd GraphicCompoundPath::buildFromText(const Font& font, const String& text) noexcept {
    return buildFromText(font, text.utf8());
}


Rectd GraphicCompoundPath::buildFromWKB(WKBParser& wkb_parser, RemapRectd& remap_rect) noexcept {
    Rectd result_rect;
    result_rect.zero();

    RangeRectd range_rect;
    range_rect.initForMinMaxSearch();

    try {
        if (wkb_parser.isPolygon() || wkb_parser.isMultiPolygon()) {
            bool multi_polygon_flag = wkb_parser.isMultiPolygon();
            uint32_t num_polygons = 1;

            if (multi_polygon_flag) {
                num_polygons = wkb_parser.readInt();
            }

            for (int polygon_index = 0; polygon_index < num_polygons; polygon_index++) {
                if (multi_polygon_flag == true) {
                    // In a multi polygon, each polygon starts with byte order and geometry type
                    wkb_parser.skipBytes(5);

                    /*
                     *  When byte order or geometry type are needed, use this instead of
                     *  wkb_parser.skipBytes(5) above!
                     *
                     *  wkb_parser.readByte(); // Byte order
                     *  wkb_parser.readInt(); // Geometry type
                     */
                }

                uint32_t num_rings = wkb_parser.readInt();

                for (int32_t ring_index = 0; ring_index < num_rings; ring_index++) {
                    uint32_t num_points = wkb_parser.readInt();

                    auto err = addEmptyPath();
                    Exception::throwStandard(err);

                    auto* graphic_path = lastPathPtr();
                    if (!graphic_path) {
                        throw ErrorCode::NullData;
                    }

                    for (int32_t point_index = 0; point_index < num_points; point_index++) {
                        Vec2d point;
                        wkb_parser.readVec2(point);
                        remap_rect.mapVec2(point);
                        graphic_path->addPoint(point);
                        range_rect += point;
                    }
                    graphic_path->close();
                }
            }

            finish();
        }
        else if (wkb_parser.isLineString() || wkb_parser.isMultiLineString()) {
            bool multi_line_flag = wkb_parser.isMultiLineString();
            uint32_t num_lines = 1;

            if (multi_line_flag) {
                num_lines = wkb_parser.readInt(); // Read number of line strings
            }

            for (int line_index = 0; line_index < num_lines; ++line_index) {
                if (multi_line_flag) {
                    // In a multi polygon, each polygon starts with byte order and geometry type
                    wkb_parser.skipBytes(5);

                    /*
                     *  When byte order or geometry type are needed, use this instead of
                     *  wkb_parser.skipBytes(5) above!
                     *
                     *  wkb_parser.readByte(); // Byte order
                     *  wkb_parser.readInt(); // Geometry type
                     */
                }

                int32_t num_points = wkb_parser.readInt();

                auto err = addEmptyPath();
                Exception::throwStandard(err);

                auto* graphic_path = lastPathPtr();
                if (!graphic_path) {
                    throw ErrorCode::NullData;
                }

                for (int32_t point_index = 0; point_index < num_points; ++point_index) {
                    Vec2d point;
                    wkb_parser.readVec2(point);
                    remap_rect.mapVec2(point);
                    graphic_path->addPoint(point);
                    range_rect += point;
                }

                finish();
            }
        }
        else {
            throw ErrorCode::Unknown;
            // TODO: Message!
        }

        result_rect.set(range_rect.minX(), range_rect.minY(), range_rect.width(), range_rect.height());
    }
    catch (ErrorCode err) {
        std::cout << "Exception in GraphicCompoundPath::buildFromWKB: " << (int32_t)err << std::endl;
        result_rect.zero();
    }

    return result_rect;
}


#if defined(__APPLE__) && defined(__MACH__)
void GraphicCompoundPath::_addCGPathElement(const CGPathElement* element) noexcept {
    if (must_add_path_) {
        addEmptyPath();
    }

    auto p = lastPathPtr();
    if (!p) {
        return;
    }

    switch (element->type) {
        case kCGPathElementMoveToPoint:
        case kCGPathElementAddLineToPoint:
            p->addPoint(offs_.x_ + element->points[0].x, offs_.y_ - element->points[0].y);
            break;

        case kCGPathElementAddQuadCurveToPoint: {
            double cx = offs_.x_ + element->points[0].x;
            double cy = offs_.y_ - element->points[0].y;
            double x = offs_.x_ + element->points[1].x;
            double y = offs_.y_ - element->points[1].y;
            auto lp = (GraphicPathPoint*)p->lastPointPtr();
            double c1x = lp->anchor_.x_ + 2.0 / 3.0 * (cx - lp->anchor_.x_);
            double c2x = x + 2.0 / 3.0 * (cx - x);
            double c1y = lp->anchor_.y_ + 2.0 / 3.0 * (cy - lp->anchor_.y_);
            double c2y = y + 2.0 / 3.0 * (cy - y);
            lp->setRight(c1x, c1y);
            p->addPointLeft(x, y, c2x, c2y);
            break;
        }

        case kCGPathElementAddCurveToPoint: {
            auto lp = (GraphicPathPoint*)p->lastPointPtr();
            lp->setRight(offs_.x_ + element->points[0].x, offs_.y_ - element->points[0].y);
            p->addPointLeft(
                offs_.x_ + element->points[2].x, offs_.y_ - element->points[2].y,
                offs_.x_ + element->points[1].x, offs_.y_ - element->points[1].y);
            break;
        }

        case kCGPathElementCloseSubpath:
            p->close();
            must_add_path_ = true;
            break;

        default:
            std::cout << "Error in GraphicCompoundPath::_addCGPathElement\n";   // TODO: !!!!
            break;
    }
}
#endif


void GraphicCompoundPath::finish() noexcept {
    // TODO: ???
}


void GraphicCompoundPath::projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix) noexcept {
    for (auto& path : paths_) {
        path->projectToQuadrilateral(quadrilateral, matrix);
    }
}


void GraphicCompoundPath::addAllPaths(GraphicContext* gc) noexcept {
    for (auto& path : paths_) {
        gc->addPath(path);
    }
}


void GraphicCompoundPath::fill(GraphicContext* gc) noexcept {
    addAllPaths(gc);
    gc->fillPath();
}


void GraphicCompoundPath::fillOuter(GraphicContext* gc) noexcept {
    // TODO: Check!
    if (paths_.size() > 1) {
        gc->addPath(paths_.elementAtIndex(0));
        gc->fillPath();
    }
}


void GraphicCompoundPath::fillEvenOdd(GraphicContext* gc) noexcept {
    addAllPaths(gc);
    gc->fillPathEvenOdd();
}


void GraphicCompoundPath::stroke(GraphicContext* gc, StrokeStyle* stroke_style) noexcept {
    // TODO: Use `stroke_style`!
    addAllPaths(gc);
    gc->strokePath();
}


void GraphicCompoundPath::addClip(GraphicContext* gc) noexcept {
    addAllPaths(gc);
    gc->clipPath();
}


} // End of namespace Grain
