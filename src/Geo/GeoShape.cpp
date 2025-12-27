//
//  GeoShape.cpp
//
//  Created by Roald Christesen on from 13.09.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 23.08.2025
//

#include "Geo/GeoShape.hpp"
#include "Core/Log.hpp"
#include "Geo/Geo.hpp"
#include "Geo/GeoProj.hpp"
#include "Geo/GeoShapeFile.hpp"
#include "2d/GraphicCompoundPath.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    GeoShape::GeoShape() noexcept {
    }


    GeoShape::~GeoShape() noexcept {
    }


    void GeoShape::log(Log& l) const {
        l << "m_dst_crs: " << m_dst_crs;
        l << ", m_shape_type: " << shapeTypeName();
        l << ", m_point_count: " << m_point_count;
        l << ", m_part_count: " << m_part_count;
        l << ", m_poly_count: " << m_poly_count;
    }


    GeoShapePoly* GeoShape::polyPtrAtIndex(int32_t index) noexcept {
        return index >= 0 && index < m_polys.size() ? &m_polys[index] : nullptr;
    }


    ErrorCode GeoShape::initWithShapeAndProjection(const String& file_path, int32_t dst_srid) noexcept {
        auto result = ErrorCode::None;
        GeoProj* proj = nullptr;

        try {
            if (!File::fileExists(file_path)) {
                throw ErrorCode::FileNotFound;
            }

            String prj_file_path = file_path.filePathWithChangedExtension("prj");
            if (!File::fileExists(prj_file_path)) {
                throw ErrorCode::FileNotFound;
            }

            auto err = readFromShapeFile(file_path);
            Exception::throwStandard(err);

            proj = new (std::nothrow) GeoProj();
            if (!proj) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }
            else {
                err = proj->setSrcCrsByFile(prj_file_path);
                Exception::throwStandard(err);

                proj->setDstSRID(dst_srid);
                if (!project(*proj)) {
                    Exception::throwStandard(ErrorCode::Fatal);
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        // Cleanup
        delete proj;

        return result;
    }


    void GeoShape::_projectFunc(GeoProj& proj, GeoShape* shape, Vec2d* p) noexcept {
        proj.transform(p);
        shape->addPointToRange(p);
    }


    bool GeoShape::project(GeoProj& proj) noexcept {
        clearRange();

        if (shapeType() == ShapeType::Polygon || shapeType() == ShapeType::PolyLine) {
            for (int32_t poly_index = 0; poly_index < m_poly_count; poly_index++) {
                if (auto poly = polyPtrAtIndex(poly_index)) {
                    int32_t poly_point_index = 0;

                    for (int32_t part_index = 0; part_index < poly->m_part_count; part_index++) {
                        int32_t point_count = poly->pointCountOfPartAtIndex(part_index);

                        for (int32_t point_index = 0; point_index < point_count; point_index++) {
                            auto point_ptr = poly->pointPtrAtIndex(poly_point_index);

                            _projectFunc(proj, this, point_ptr);
                            if (poly_point_index == 0) {
                                poly->bbox_.set(*point_ptr);
                            }
                            else {
                                poly->bbox_.add(*point_ptr);
                            }
                            poly_point_index++;
                        }
                    }
                }
            }
        }
        else if (shapeType() == ShapeType::Point) {
            for (auto& point : points_) {
                _projectFunc(proj, this, &point);
            }
        }

        return true;
    }


    RangeRectd GeoShape::polyBbox(int32_t index) noexcept {
        if (auto poly = polyPtrAtIndex(index)) {
            return poly->bbox_;
        }
        else {
            return RangeRectd(0, 0, 0, 0);
        }
    }


    void GeoShape::buildPolyCompoundPath(GraphicContext* gc, int32_t index, const RemapRectd& remap_rect, GraphicCompoundPath& out_path) noexcept {
        #pragma message("GeoShape::buildPolyCompoundPath() must be implemented!")
    }


    void GeoShape::buildPolyGCPath(GraphicContext* gc, int32_t index, const RemapRectd& remap_rect) noexcept {
        if (auto poly = polyPtrAtIndex(index)) {
            Vec2d point;

            gc->beginPath();
            for (int32_t part_index = 0; part_index < poly->m_part_count; part_index++) {
                int32_t point_count = poly->pointCountOfPartAtIndex(part_index);

                for (int32_t point_index = 0; point_index < point_count; point_index++) {
                    poly->pointAtIndex(point_index + poly->partAtIndex(part_index), point);
                    remap_rect.mapVec2(point);

                    if (point_index == 0) {
                        gc->moveTo(point);
                    }
                    else {
                        gc->lineTo(point);
                    }
                }

                if (closedPathDrawing()) {
                    gc->closePath();
                }
            }
        }
    }


    bool GeoShape::pointAtIndex(int32_t index, Vec2d& out_point) noexcept {
        if (index >= 0 && index < points_.size()) {
            out_point = points_[index];
            return true;
        }
        else {
            return false;
        }
    }


    void GeoShape::pointAtIndex(int32_t index, const RemapRectd& remap_rect, Vec2d& out_point) noexcept {
        if (index >= 0 && index < points_.size()) {
            out_point = points_[index];
            remap_rect.mapVec2(out_point);
        }
    }


    DrawMode GeoShape::usedDrawMode(DrawMode draw_mode) const noexcept {
        if (draw_mode == DrawMode::Undefined) {
            return defaultDrawMode();
        }
        else {
            return draw_mode;
        }
    }


    /**
     *  @brief Prepare the Graphic Context with style of shape.
     *
     *  @param gc The graphic context to prepare.
     */
    void GeoShape::applyDrawStyle(GraphicContext* gc) {
        bool fill_flag = false;
        bool stroke_flag = false;

        DrawMode draw_mode = usedDrawMode(m_draw_mode);

        switch (draw_mode) {
            case DrawMode::Undefined:
            case DrawMode::Fill:
                fill_flag = true;
                break;

            case DrawMode::Stroke:
                stroke_flag = true;
                break;

            case DrawMode::FillStroke:
            case DrawMode::StrokeFill:
                fill_flag = true;
                stroke_flag = true;
                break;
        }

        if (fill_flag) {
            gc->setFillRGBA(m_fill_color);
        }

        if (stroke_flag) {
            gc->setStrokeRGBA(m_stroke_color);
            gc->setStrokeWidth(m_stroke_width);
            gc->setStrokeJoinStyle(m_stroke_join_style);
            gc->setStrokeCapStyle(m_stroke_cap_style);
        }
    }


    void GeoShape::drawAll(GraphicContext* gc, const RemapRectd& remap_rect, DrawMode draw_mode) noexcept {
        if (shouldDrawAsPoints()) {
            for (int32_t point_index = 0; point_index < points_.size(); point_index++) {
                Vec2d point;

                pointAtIndex(point_index, point);
                remap_rect.mapVec2(point);
                gc->fillCircle(point, m_point_radius);
            }
        }
        else if (shouldDrawAsLines()) {
            #pragma message("GeoShape::drawAll() draw lines must be implemented!")
        }
        else {
            for (int32_t i = 0; i < polyCount(); i++) {
                drawPoly(gc, i, remap_rect, draw_mode);
            }
        }
    }


    /**
     *  @brief Draw a single polygon from shape with remapped point coordinates.
     *
     *  @param gc The graphic context to draw in.
     *  @param index The index of the polygon to draw.
     *  @param remap_rect The information about remapping.
     *  @param draw_mode The draw mode.
     */
    void GeoShape::drawPoly(GraphicContext* gc, int32_t index, const RemapRectd& remap_rect, DrawMode draw_mode) noexcept {
        if (auto poly = polyPtrAtIndex(index)) {
            buildPolyGCPath(gc, index, remap_rect);

            if (draw_mode != DrawMode::Undefined) {
                draw_mode = usedDrawMode(draw_mode);
            }
            else {
                draw_mode = usedDrawMode(m_draw_mode);
            }

            switch (draw_mode) {
                case DrawMode::Undefined:
                case DrawMode::Fill:
                    gc->fillPath();
                    break;

                case DrawMode::Stroke:
                    gc->strokePath();
                    break;

                case DrawMode::FillStroke:
                    gc->drawPath();
                    break;

                case DrawMode::StrokeFill:
                    gc->strokePath();
                    // Note: buildPolyGCPath must be called a second time here
                    buildPolyGCPath(gc, index, remap_rect);
                    gc->fillPath();
                    break;
            }
        }
    }


    void GeoShape::drawPolys(GraphicContext* gc, int32_t startIndex, int32_t endIndex, const RemapRectd& remap_rect, DrawMode draw_mode) noexcept {
        if (startIndex < 0) {
            startIndex = 0;
        }

        int32_t lastIndex = polyCount() - 1;
        if (endIndex > lastIndex) {
            endIndex = lastIndex;
        }

        for (int32_t i = startIndex; i < endIndex; i++) {
            drawPoly(gc, i, remap_rect, draw_mode);
        }
    }


    void GeoShape::drawPolys(GraphicContext* gc, const RemapRectd& remap_rect, DrawMode draw_mode) noexcept {
        for (int32_t i = 0; i < polyCount(); i++) {
            drawPoly(gc, i, remap_rect, draw_mode);
        }
    }


    ErrorCode GeoShape::readFromShapeFile(const String& file_path, int32_t limit) noexcept {

        auto result = ErrorCode::None;

        GeoShapeFile* shape_file = nullptr;

        try {
            shape_file = new GeoShapeFile(file_path);
            if (!shape_file) {
                throw ErrorCode::MemCantAllocate;
            }

            shape_file->startRead();
            shape_file->setGeoShape(this);   // Tell the file where it should put the geometry data.

            clearRange();

            switch (shape_file->shapeType()) {
                case GeoShape::ShapeType::Point: {
                    auto err = shape_file->readAllPoints();
                    Exception::throwStandard(err);
                    break;
                }

                case GeoShape::ShapeType::PolyLine:
                case GeoShape::ShapeType::Polygon: {
                    auto err = shape_file->_countAllPolys();
                    Exception::throwStandard(err);

                    if (limit < 0 || polyCount() <= limit) {
                        err = shape_file->_readAllPolys();
                        Exception::throwStandard(err);
                    }

                    if (limit >= 0 && polyCount() > limit) {
                        Exception::throwSpecific(kErrPolygonLimitExceeded);
                    }
                    break;
                }

                case GeoShape::ShapeType::PointZ:
                case GeoShape::ShapeType::PointM:

                case GeoShape::ShapeType::MultiPoint:
                case GeoShape::ShapeType::MultiPointZ:
                case GeoShape::ShapeType::MultiPointM:

                case GeoShape::ShapeType::PolyLineZ:
                case GeoShape::ShapeType::PolyLineM:

                case GeoShape::ShapeType::PolygonZ:
                case GeoShape::ShapeType::PolygonM:

                case GeoShape::ShapeType::MultiPatch:
                    // TODO: Handle Multiline, Multipolygon... Implement!

                default:
                    Exception::throwSpecific(kErrUnsupportedShapeType);
            }

        }
        catch (ErrorCode err) {
            result = err;
        }

        delete shape_file;

        return result;
    }


    ErrorCode GeoShape::setParam(const String& name, const String& value) noexcept {
        if (name == "srid") {
            setSRID(value);
        }
        else if (name == "draw-mode") {
            if (value == "fill") {
                setDrawMode(DrawMode::Fill);
            }
            else if (value == "stroke") {
                setDrawMode(DrawMode::Stroke);
            }
            else if (value == "fill-stroke") {
                setDrawMode(DrawMode::FillStroke);
            }
            else if (value == "stroke-fill") {
                setDrawMode(DrawMode::StrokeFill);
            }
            else {
                return ErrorCode::UnknownValue;
            }
        }
        else if (name == "fill-color") {
            m_fill_color.setByCSV(value.utf8());
        }
        else if (name == "stroke-color") {
            m_stroke_color.setByCSV(value.utf8());
        }
        else if (name == "stroke-width") {
            setStrokeWidth(value.asFloat()); // TODO: Check format!
        }
        else if (name == "point-radius") {
            setPointRadius(value.asFloat()); // TODO: Check format!
        }
        else {
            return ErrorCode::UnknownParameter;
        }

        return ErrorCode::None;
    }


} // End of namespace Grain
