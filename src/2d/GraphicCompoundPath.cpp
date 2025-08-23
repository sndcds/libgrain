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
// #include "WKBParser.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/Font.hpp"
#include "Core/Log.hpp"


namespace Grain {

    GraphicCompoundPath::GraphicCompoundPath() noexcept {
        m_paths.reserve(4);
        m_offset.zero();
    }


    GraphicCompoundPath::~GraphicCompoundPath() noexcept {
        clear();
    }


    void GraphicCompoundPath::log(Log& l) const {

        l << "GraphicCompoundPath:" << l.endl;
        l << "pathCount(): " << pathCount() << l.endl;
        l++;
        for (auto& path : m_paths) {
            // path->log(); TODO:!!!!
        }
        l--;
    }


    GraphicPath *GraphicCompoundPath::pathPtrAtIndex(int32_t index) noexcept {
        return m_paths.elementAtIndex(index);
    }


    GraphicPath *GraphicCompoundPath::lastPathPtr() noexcept {
        return m_paths.lastElement();
    }


    bool GraphicCompoundPath::boundsRect(Rectd &out_bounds_rect) const noexcept {
        #pragma message("GraphicCompoundPath::boundsRect() nust be implemented!")
        return false;
    }


    double GraphicCompoundPath::polygonCentroid(Vec2d &out_centroid) const noexcept {
        double total_area = 0;
        Vec2d compound_c;

        for (auto& path : m_paths) {
            Vec2d c;
            double area = path->polygonCentroid(c);
            if (area != 0.0) {
                compound_c += c * area;
                total_area += area;
            }
        }

        if (total_area != 0.0) {
            out_centroid = compound_c / total_area;
        }

        return total_area;
    }


    void GraphicCompoundPath::clear() noexcept {
        m_paths.clear();
    }


    ErrorCode GraphicCompoundPath::addEmptyPath(int32_t point_capacity) noexcept {
        auto result = ErrorCode::None;

        try {
            auto path = new (std::nothrow) GraphicPath(point_capacity);
            m_paths.push(path);
            m_must_add_path = false;
        }
        catch (const std::exception &e) {
            result = ErrorCode::StdCppException;
        }

        return result;
    }


    void _CGPathEnumerationCallback(void *info, const CGPathElement *element) {
        if (info) {
            ((GraphicCompoundPath*)info)->_addCGPathElement(element);
        }
    }

/* TODO: Implement!
    Rectd GraphicCompoundPath::buildFromText(const Font &font, const char *text) noexcept {

        Rectd bounds_rect;

        NSString *ns_string = nullptr;

        try {
            if (text == nullptr) {
                throw ErrorCode::NullData;
            }

            ns_string = [NSString stringWithUTF8String:text]; // Autoreleased!
            if (ns_string == nullptr) {
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

            bounds_rect.m_width = 0.0;
            m_must_add_path = true;
            for (int32_t i = 0; i < char_count; i++) {

                CGPathRef path = CTFontCreatePathForGlyph(ct_font, glyphs[i], NULL);
                if (path != nullptr) {
                    CGPathApply(path, this, _CGPathEnumerationCallback);
                    CGPathRelease(path);
                }

                m_offset.translate(advances[i].width, advances[i].height);

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

            bounds_rect.m_x = min_x;
            bounds_rect.m_y = min_y - max_y - min_y;
            bounds_rect.m_width = max_x - min_x;
            bounds_rect.m_height = max_y - min_y;

            finish();
        }
        catch (ErrorCode err) {
        }

        return bounds_rect;
    }
*/

    Rectd GraphicCompoundPath::buildFromText(const Font &font, const String &text) noexcept {
        return buildFromText(font, text.utf8());
    }


/* TODO: Implement WKBParser first!

    Rectd GraphicCompoundPath::buildFromWKB(WKBParser &wkb_parser, RemapRectd &remap_rect) noexcept {
        Rectd result_rect;
        result_rect.zero();

        RangeRectd range_rect;
        range_rect.initForMinMaxSearch();


        try {
            if (wkb_parser.isPolygon() || wkb_parser.isMultiPolygon()) {

                bool multi_polygon_flag = wkb_parser.isMultiPolygon();
                int32_t num_polygons = 1;

                if (multi_polygon_flag) {
                    num_polygons = wkb_parser.readInt();
                }

                for (int polygon_index = 0; polygon_index < num_polygons; polygon_index++) {

                    if (multi_polygon_flag == true) {

                        // In a multi polygon, each polygon starts with byte order and geometry type.

                        // Byte order.
                        wkb_parser.readByte();

                        // Geometry type.
                        wkb_parser.readInt();
                    }

                    int32_t num_rings = wkb_parser.readInt();

                    for (int32_t ring_index = 0; ring_index < num_rings; ring_index++) {
                        int32_t num_points = wkb_parser.readInt();

                        auto err = addEmptyPath();
                        Error::throwError(err);

                        auto *graphic_path = lastPathPtr();
                        if (graphic_path == nullptr) {
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
                int32_t num_lines = 1;

                if (multi_line_flag) {
                    num_lines = wkb_parser.readInt(); // Read number of line strings
                }

                for (int line_index = 0; line_index < num_lines; ++line_index) {

                    if (multi_line_flag) {
                        // Read byte order and geometry type for each LineString
                        wkb_parser.readByte();  // Byte order
                        wkb_parser.readInt();   // Geometry type
                    }

                    int32_t num_points = wkb_parser.readInt();

                    auto err = addEmptyPath();
                    Error::throwError(err);

                    auto *graphic_path = lastPathPtr();
                    if (graphic_path == nullptr) {
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
*/

/* TODO: Implement!
    void GraphicCompoundPath::_addCGPathElement(const CGPathElement *element) noexcept {
        if (m_must_add_path) {
            addEmptyPath();
        }

        auto p = lastPathPtr();
        if (p == nullptr) {
            return;
        }

        switch (element->type) {

            case kCGPathElementMoveToPoint:
            case kCGPathElementAddLineToPoint:
                p->addPoint(m_offset.m_x + element->points[0].x, m_offset.m_y - element->points[0].y);
                break;

            case kCGPathElementAddQuadCurveToPoint:
            {
                double cx = m_offset.m_x + element->points[0].x;
                double cy = m_offset.m_y - element->points[0].y;
                double x = m_offset.m_x + element->points[1].x;
                double y = m_offset.m_y - element->points[1].y;


                auto lp = (GraphicPathPoint*)p->lastPointPtr();

                double c1x = lp->m_anchor.m_x + 2.0 / 3.0 * (cx - lp->m_anchor.m_x);
                double c2x = x + 2.0 / 3.0 * (cx - x);
                double c1y = lp->m_anchor.m_y + 2.0 / 3.0 * (cy - lp->m_anchor.m_y);
                double c2y = y + 2.0 / 3.0 * (cy - y);

                lp->setRight(c1x, c1y);
                p->addPointLeft(x, y, c2x, c2y);
            }
                break;

            case kCGPathElementAddCurveToPoint:
            {
                auto lp = (GraphicPathPoint*)p->lastPointPtr();
                lp->setRight(m_offset.m_x + element->points[0].x, m_offset.m_y - element->points[0].y);
                p->addPointLeft(m_offset.m_x + element->points[2].x, m_offset.m_y - element->points[2].y,
                                m_offset.m_x + element->points[1].x, m_offset.m_y - element->points[1].y);
            }
                break;

            case kCGPathElementCloseSubpath:
                p->close();
                m_must_add_path = true;
                break;

            default:
                std::cout << "Error in GraphicCompoundPath::_addCGPathElement\n";   // TODO: !!!!
                break;
        }
    }
*/

    void GraphicCompoundPath::finish() noexcept {
        // TODO: ???
    }


    void GraphicCompoundPath::projectToQuadrilateral(const Quadrilateral &quadrilateral, const Mat3d *matrix) noexcept {
        for (auto& path : m_paths) {
            path->projectToQuadrilateral(quadrilateral, matrix);
        }
    }


    void GraphicCompoundPath::addAllPaths(GraphicContext &gc) noexcept {
        for (auto& path : m_paths) {
            gc.addPath((GraphicPath*)path);
        }
    }


    void GraphicCompoundPath::fill(GraphicContext &gc) noexcept {
        addAllPaths(gc);
        gc.fillPath();
    }


    void GraphicCompoundPath::fillOuter(GraphicContext &gc) noexcept {
        // TODO: Check!
        if (m_paths.size() > 1) {
            gc.addPath(m_paths.elementAtIndex(0));
            gc.fillPath();
        }
    }


    void GraphicCompoundPath::fillEvenOdd(GraphicContext &gc) noexcept {
        addAllPaths(gc);
        gc.fillPathEvenOdd();
    }


    void GraphicCompoundPath::stroke(GraphicContext &gc, StrokeStyle *stroke_style) noexcept {
        // TODO: Use `stroke_style`!
        addAllPaths(gc);
        gc.strokePath();
    }


    void GraphicCompoundPath::addClip(GraphicContext &gc) noexcept {
        addAllPaths(gc);
        gc.clipPath();
    }


} // End of namespace Grain
