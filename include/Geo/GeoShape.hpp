//
//  GeoShape.hpp
//
//  Created by Roald Christesen on from 13.09.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 23.08.2025
//

// @see // http://switchfromshapefile.org/#geopackage

#ifndef GrainGeoShape_hpp
#define GrainGeoShape_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "File/File.hpp"
#include "2d/Rect.hpp"
#include "2d/RangeRect.hpp"
#include "2d/GraphicCompoundPath.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Color/RGBA.hpp"


namespace Grain {

    class GeoProj;
    class GeoShapePoly;
    class Log;


    /**
     *  @brief Geo Shape support.
     */
    class GeoShape : public Object {

        friend class GeoShapeFile;
        friend class GeoShapePoly;

    public:
        enum class ShapeType {
            Undefined = -1,
            Null = 0,
            Point = 1,
            PolyLine = 3,
            Polygon = 5,
            MultiPoint = 8,
            PointZ = 11,
            PolyLineZ = 13,
            PolygonZ = 15,
            MultiPointZ = 18,
            PointM = 21,
            PolyLineM = 23,
            PolygonM = 25,
            MultiPointM = 28,
            MultiPatch = 31
        };

        enum {
            kErrUnsupportedShapeType = 0,
            kErrPolygonLimitExceeded    ///< Error code indicating that the maximum limit for polygons has been exceeded

        };


    protected:
        String m_dst_crs;   ///< Destination CRS, i.e. EPSG:4236

        ShapeType m_shape_type = ShapeType::Undefined;
        double m_shape_bbox[8]{};
        int64_t m_record_start_pos = -1;

        int32_t m_point_count = 0;
        int32_t m_part_count = 0;
        int32_t m_poly_count = 0;

        // List<Vec2d> m_points
        std::vector<Vec2d> m_points;        // TODO: Use List instead of std::vector, maybe use Point2d instead of Vec2d
        std::vector<int32_t> m_parts;       // TODO: Use List instead of std::vector
        std::vector<GeoShapePoly> m_polys;  // TODO: Use List instead of std::vector

        bool _m_closed_path_drawing = false;

        RangeRectd _m_range;
        RGBA m_fill_color;
        RGBA m_stroke_color;
        DrawMode m_draw_mode = DrawMode::Undefined;
        double m_stroke_width = 1.0;
        double m_point_radius = 1.0;
        StrokeJoinStyle m_stroke_join_style = StrokeJoinStyle::Bevel;
        StrokeCapStyle m_stroke_cap_style = StrokeCapStyle::Square;

        double mPointTolerance = 0.000001;

    public:
        GeoShape() noexcept;
        ~GeoShape() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "GeoShape"; }

        friend std::ostream& operator << (std::ostream& os, const GeoShape* o) {
            o == nullptr ? os << "GeoShape nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GeoShape& o) {
            os << "m_dst_crs: " << o.m_dst_crs;
            os << ", m_shape_type: " << o.shapeTypeName();
            os << ", m_point_count: " << o.m_point_count;
            os << ", m_part_count: " << o.m_part_count;
            os << ", m_poly_count: " << o.m_poly_count;
            return os;
        }

        void log(Log& l) const;

        [[nodiscard]] String desCrs() const noexcept { return m_dst_crs; }

        [[nodiscard]] bool isPointType() const noexcept { return m_shape_type == ShapeType::Point; }
        [[nodiscard]] bool isPolyLine() const noexcept { return m_shape_type == ShapeType::PolyLine; }
        [[nodiscard]] bool isPolygonType() const noexcept { return m_shape_type == ShapeType::Polygon; }

        [[nodiscard]] int32_t pointCount() const noexcept { return m_point_count; }
        [[nodiscard]] int32_t partCount() const noexcept { return m_part_count; }
        [[nodiscard]] int32_t polyCount() const noexcept { return m_poly_count; }

        void setDstCrs(const String& dst_crs) noexcept {
            m_dst_crs = dst_crs;
        }

        void setSRID(const String& srid) noexcept {
            // TODO: Support other systems than EPSG
            m_dst_crs = "EPSG:";
            m_dst_crs += srid;
        }

        void setShapeType(ShapeType shape_type) noexcept {
            m_shape_type = shape_type;
            updateClosedPathDrawing();
        }

        ShapeType shapeType() const noexcept { return m_shape_type; }

        Vec2d* pointPtrAtIndex(int32_t index) noexcept {
            return index >= 0 && index < m_points.size() ? &m_points[index] : nullptr;
        }

        GeoShapePoly* polyPtrAtIndex(int32_t index) noexcept;

        ErrorCode initWithShapeAndProjection(const String& file_path, int32_t dst_srid) noexcept;


        bool shouldDrawAsLines() const noexcept {
            switch (m_shape_type) {
                case ShapeType::PolyLine:
                case ShapeType::PolyLineZ:
                case ShapeType::PolyLineM:
                    return true;
                default:
                    return false;
            }
        }

        bool shouldDrawAsPoints() const noexcept {
            switch (m_shape_type) {
                case ShapeType::Point:
                case ShapeType::MultiPoint:
                case ShapeType::PointZ:
                case ShapeType::PointM:
                case ShapeType::MultiPointM:
                    return true;
                default:
                    return false;
            }
        }

        DrawMode defaultDrawMode() const noexcept {
            if (shouldDrawAsLines()) {
                return DrawMode::Stroke;
            }
            else {
                return DrawMode::Fill;
            }
        }

        DrawMode drawMode() const noexcept { return m_draw_mode; }
        RGBA fillColor(const RGBA& color) const noexcept { return m_fill_color; }
        RGBA strokeColor(const RGBA& color) const noexcept { return m_stroke_color; }
        double strokeWidth() const noexcept { return m_stroke_width; }
        double pointRadius() const noexcept { return m_point_radius; }
        StrokeJoinStyle strokeJoinStyle() const noexcept { return m_stroke_join_style; }
        StrokeCapStyle strokeCapStyle() const noexcept { return m_stroke_cap_style; }

        void setDrawMode(DrawMode draw_mode) { m_draw_mode = draw_mode; }
        void setDrawModeFill() { m_draw_mode = DrawMode::Fill; }
        void setDrawModeStroke() { m_draw_mode = DrawMode::Stroke; }
        void setDrawModeFillStroke() { m_draw_mode = DrawMode::FillStroke; }
        void setDrawModeStrokeFill() { m_draw_mode = DrawMode::StrokeFill; }
        void setFillColor(const RGB& color) noexcept { m_fill_color = RGBA(color, 1.0f); }
        void setStrokeColor(const RGB& color) noexcept { m_stroke_color = RGBA(color, 1.0f); }
        void setFillColor(const RGBA& color) noexcept { m_fill_color = color; }
        void setStrokeColor(const RGBA& color) noexcept { m_stroke_color = color; }
        void setStrokeWidth(double width) noexcept { m_stroke_width = width; }
        void setPointRadius(double radius) noexcept { m_point_radius = radius; }
        void setStrokeJoinStyle(StrokeJoinStyle join_style) noexcept { m_stroke_join_style = join_style; }
        void setStrokeCapStyle(StrokeCapStyle cap_style) noexcept { m_stroke_cap_style = cap_style; }

        const char* shapeTypeName() const { return shapeTypeName(m_shape_type); }

        static const char* shapeTypeName(ShapeType shape_type) noexcept {
            switch (shape_type) {
                case ShapeType::Null: return "Null Shape";
                case ShapeType::Point: return "Point";
                case ShapeType::PolyLine: return "PolyLine";
                case ShapeType::Polygon: return "Polygon";
                case ShapeType::MultiPoint: return "MultiPoint";
                case ShapeType::PointZ: return "PointZ";
                case ShapeType::PolyLineZ: return "PolyLineZ";
                case ShapeType::PolygonZ: return "PolygonZ";
                case ShapeType::MultiPointZ: return "MultiPointZ";
                case ShapeType::PointM: return "PointM";
                case ShapeType::PolyLineM: return "PolyLineM";
                case ShapeType::PolygonM: return "PolygonM";
                case ShapeType::MultiPointM: return "MultiPointM";
                case ShapeType::MultiPatch: return "MultiPatch";
                default: return "Undefined";
            }
        }

        void updateClosedPathDrawing() noexcept {
            switch (m_shape_type) {
                case ShapeType::Polygon:
                case ShapeType::PolygonZ:
                case ShapeType::PolygonM:
                    _m_closed_path_drawing = true;
                    break;
                default:
                    _m_closed_path_drawing = false;
            }
        }

        bool closedPathDrawing() const noexcept { return _m_closed_path_drawing; }

        static void _projectFunc(GeoProj& proj, GeoShape* shape, Vec2d* p) noexcept;

        bool project(GeoProj& proj) noexcept;
        bool project(GeoProj* proj) noexcept { return proj != nullptr ? project(*proj) : false; }

        RangeRectd range() const noexcept { return _m_range; }
        void clearRange() noexcept { _m_range.set(100000000.0, 100000000.0, -100000000.0, -100000000.0); }
        bool addPointToRange(Vec2d* point) noexcept { return point ? _m_range.add(point) : false; }
        bool addPointToRange(Vec2d& point) noexcept { return addPointToRange(&point); }

        RangeRectd polyBbox(int32_t index) noexcept;

        void buildPolyCompoundPath(GraphicContext& gc, int32_t index, const RemapRectd& remap_rect, GraphicCompoundPath& out_path) noexcept;
        void buildPolyGCPath(GraphicContext& gc, int32_t index, const RemapRectd& remap_rect) noexcept;

        bool pointAtIndex(int32_t index, Vec2d& out_point) noexcept;
        void pointAtIndex(int32_t index, const RemapRectd& remap_rect, Vec2d& out_point) noexcept;

        DrawMode usedDrawMode(DrawMode draw_mode) const noexcept;
        void applyDrawStyle(GraphicContext& gc);

        void drawAll(GraphicContext& gc, const RemapRectd& remap_rect, DrawMode draw_mode = DrawMode::Undefined) noexcept;

        void drawPoly(GraphicContext& gc, int32_t index, const RemapRectd& remap_rect, DrawMode draw_mode = DrawMode::Undefined) noexcept;
        void drawPolys(GraphicContext& gc, int32_t start_index, int32_t end_index, const RemapRectd& remap_rect, DrawMode draw_mode = DrawMode::Undefined) noexcept;
        void drawPolys(GraphicContext& gc, const RemapRectd& remap_rect, DrawMode draw_mode = DrawMode::Undefined) noexcept;


        ErrorCode readFromShapeFile(const String& file_path, int32_t limit = -1) noexcept;

        // Implementation of methods for parameter handling

        ErrorCode setParam(const String& name, const String& value) noexcept override;
    };



    class GeoShapePoly {

    public:
        GeoShape* m_shape = nullptr;
        int32_t m_record_number;
        int32_t m_content_length;
        GeoShape::ShapeType m_shape_type;
        RangeRectd m_bbox;
        int32_t m_part_offset = 0;
        int32_t m_part_count = 0;
        int32_t m_point_offset = 0;
        int32_t m_point_count = 0;

    public:
        GeoShapePoly() {}

        // Copy constructor
        GeoShapePoly(const GeoShapePoly& poly) { _copyFrom(poly); }

        // Copy assignment operator
        GeoShapePoly& operator = (const GeoShapePoly& poly) {
            if (this != &poly) {
                _copyFrom(poly);
            }
            return *this;
        }

        // Move constructor
        GeoShapePoly(GeoShapePoly& poly) noexcept { _moveFrom(std::move(poly)); }

        // Move assignment operator
        GeoShapePoly& operator = (GeoShapePoly&& poly) noexcept {
            if (this != &poly) {
                _moveFrom(std::move(poly));
            }
            return *this;
        }

        // Helper function for copy semantics
        void _copyFrom(const GeoShapePoly& poly) {
            m_shape = poly.m_shape;
            m_record_number = poly.m_record_number;
            m_content_length = poly.m_content_length;
            m_shape_type = poly.m_shape_type;
            m_bbox = poly.m_bbox;
            m_part_offset = poly.m_part_offset;
            m_part_count = poly.m_part_count;
            m_point_offset = poly.m_point_offset;
            m_point_count = poly.m_point_count;
        }

        // Helper function for move semantics
        void _moveFrom(GeoShapePoly&& poly) {
            m_shape = poly.m_shape;
            m_record_number = poly.m_record_number;
            m_content_length = poly.m_content_length;
            m_shape_type = poly.m_shape_type;
            m_bbox = std::move(poly.m_bbox);
            m_part_offset = poly.m_part_offset;
            m_part_count = poly.m_part_count;
            m_point_offset = poly.m_point_offset;
            m_point_count = poly.m_point_count;

            // Reset the source object
            // Not all members musz be resetted; it depends on design
            poly.m_shape = nullptr;
            poly.m_record_number = 0;
            poly.m_content_length = 0;
            poly.m_shape_type = GeoShape::ShapeType::Undefined;
        }

        bool isPartIndex(int32_t index) noexcept { return index >= 0 && index < m_part_count; }
        bool isPointIndex(int32_t index) noexcept { return index >= 0 && index < m_point_count; }

        int32_t partAtIndex(int32_t index) noexcept {
            return isPartIndex(index) ? m_shape->m_parts[m_part_offset + index] : 0;
        }

        bool pointAtIndex(int32_t index, Vec2d& outPoint) noexcept {
            if (isPointIndex(index)) {
                outPoint = m_shape->m_points[m_point_offset + index];
                return true;
            }
            else {
                return false;
            }
        }

        Vec2d* pointPtrAtIndex(int32_t index) noexcept {
            if (isPointIndex(index)) {
                return &m_shape->m_points[m_point_offset + index];
            }
            else {
                return nullptr;
            }
        }

        int32_t pointCountOfPartAtIndex(int32_t index) noexcept {
            if (isPartIndex(index) == false) {
                return 0;
            }
            if (index == m_part_count - 1) {
                return m_point_count - m_shape->m_parts[m_part_offset + index];
            }
            else {
                return m_shape->m_parts[m_part_offset + index + 1] - m_shape->m_parts[m_part_offset + index];
            }
        }
    };


}  // End of namespace Grain

#endif  // GrGeoShape_hpp
