//
//  GeoProj.cpp
//
//  Created by Roald Christesen on from 11.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  https://download.geofabrik.de/technical.html

#include "Geo/GeoProj.hpp"
#include "Geo/Geo.hpp"

#include <proj.h>


namespace Grain {

    GeoProj::GeoProj() noexcept {
        m_proj = nullptr;
        m_proj_context = proj_context_create();
    }

    GeoProj::~GeoProj() noexcept {
        if (m_proj) {
            proj_destroy((PJ*)m_proj);
        }

        if (m_proj_context) {
            proj_context_destroy((PJ_CONTEXT*)m_proj_context);
        }
    }


    bool GeoProj::isValid() noexcept {
        return _update() == ErrorCode::None && m_proj_context && m_proj;
    }


    ErrorCode GeoProj::setSrcCrsByFile(const String& file_path) noexcept {
        return m_src_crs.loadText(file_path);
    }


    ErrorCode GeoProj::setDstCrsByFile(const String& file_path) noexcept {
        return m_dst_crs.loadText(file_path);
    }


    bool GeoProj::transform(const Vec2d& pos, Vec2d& out_pos, Direction direction) noexcept {
        const PJ_DIRECTION pj_direction = direction == Direction::Forward ? PJ_FWD : PJ_INV;

        if (m_transform_action) {
            return m_transform_action(pos, out_pos); // Call specific method, TODO: Handle inverse!
        }
        else {
            if (m_must_update) {
                _update();
            }

            PJ_COORD in_coord = proj_coord(pos.m_x, pos.m_y, 0, 0);
            PJ_COORD out_coord = proj_trans((PJ*)m_proj, pj_direction, in_coord);

            if (proj_errno((PJ*)m_proj)) {
                return false;
            }

            out_pos.m_x = out_coord.xy.x;
            out_pos.m_y = out_coord.xy.y;

            return true;
        }
    }


    bool GeoProj::transform(const RangeRectd& range_rect, RangeRectd& out_range_rect, Direction direction) noexcept {

        Vec2d v1(range_rect.m_min_x, range_rect.m_min_y);
        Vec2d v2(range_rect.m_max_x, range_rect.m_max_y);

        if (!transform(v1, direction)) {
            return false;
        }

        if (!transform(v2, direction)) {
            return false;
        }

        out_range_rect.m_min_x = v1.m_x;
        out_range_rect.m_min_y = v1.m_y;
        out_range_rect.m_max_x = v2.m_x;
        out_range_rect.m_max_y = v2.m_y;

        return true;
    }


    bool GeoProj::transform(RangeRectd& range_rect, Direction direction) noexcept {

        Vec2d v1(range_rect.m_min_x, range_rect.m_min_y);
        Vec2d v2(range_rect.m_max_x, range_rect.m_max_y);

        if (!transform(v1, direction)) {
            return false;
        }

        if (!transform(v2, direction)) {
            return false;
        }

        range_rect.m_min_x = v1.m_x;
        range_rect.m_min_y = v1.m_y;
        range_rect.m_max_x = v2.m_x;
        range_rect.m_max_y = v2.m_y;

        return true;
    }


    bool GeoProj::transform(const RangeRectFix& range_rect, RangeRectFix& out_range_rect, Direction direction) noexcept {
        Vec2d v1(range_rect.m_min_x.asDouble(), range_rect.m_min_y.asDouble());
        Vec2d v2(range_rect.m_max_x.asDouble(), range_rect.m_max_y.asDouble());

        if (!transform(v1, direction)) {
            return false;
        }

        if (!transform(v2, direction)) {
            return false;
        }

        out_range_rect.m_min_x = v1.m_x;
        out_range_rect.m_min_y = v1.m_y;
        out_range_rect.m_max_x = v2.m_x;
        out_range_rect.m_max_y = v2.m_y;

        return true;
    }


    bool GeoProj::transform(RangeRectFix& range_rect, Direction direction) noexcept {
        Vec2d v1(range_rect.m_min_x.asDouble(), range_rect.m_min_y.asDouble());
        Vec2d v2(range_rect.m_max_x.asDouble(), range_rect.m_max_y.asDouble());

        if (!transform(v1, direction)) {
            return false;
        }

        if (!transform(v2, direction)) {
            return false;
        }

        range_rect.m_min_x = v1.m_x;
        range_rect.m_min_y = v1.m_y;
        range_rect.m_max_x = v2.m_x;
        range_rect.m_max_y = v2.m_y;

        return true;
    }


    bool GeoProj::transform(Quadrilateral& quadrilateral, Direction direction) noexcept {
        if (!transform(quadrilateral.m_points[0], direction)) { return false; }
        if (!transform(quadrilateral.m_points[1], direction)) { return false; }
        if (!transform(quadrilateral.m_points[2], direction)) { return false; }
        if (!transform(quadrilateral.m_points[3], direction)) { return false; }

        return true;
    }


    /**
     *  @brief Transforms a position from the source coordinate system to the destination viewport.
     *
     *  @param pos The input position in the source coordinate system.
     *  @param out_pos The output position mapped to the destination viewport.
     *  @return True if the transformation was successful, otherwise false.
     */
    bool GeoProj::transformToViewport(const Vec2d& pos, Vec2d& out_pos) noexcept {
        if (transform(pos, out_pos)) {
            m_remap_rect.mapVec2(out_pos);
            return true;
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Transforms a position from the viewport back to the source coordinate system.
     *
     *  @param pos The input position in the viewport coordinate system.
     *  @param out_pos The output position mapped back to the source coordinate system.
     *  @return True if the transformation was successful, otherwise false.
     */
    bool GeoProj::transformFromViewport(const Vec2d& pos, Vec2d& out_pos) noexcept {
        m_remap_rect.inverseMapVec2(pos, out_pos);
        return transform(out_pos, Direction::Forward);
    }


    /**
     *  @brief Calculate the aspect ratio of a bounding box in Mercator coordinates (SRID 3857).
     *
     *  This function computes the aspect ratio of a bounding box defined in the Web Mercator projection
     *  (SRID 3857). The aspect ratio is determined as the ratio of the width to the height of the bounding box.
     *
     *  @param bounds A `RangeRectd` object representing the bounding box in SRID 3857.
     *
     *  @return double The aspect ratio of the bounding box.
     *                 - If the height is zero (invalid bounding box), the function returns 0.
     *
     *  @note This function assumes the input coordinates are in meters, as required by SRID 3857.
     *        Ensure that the bounding box is valid (i.e., `bounds.minX < bounds.maxX` and `bounds.minY < bounds.maxY`).
     *        If the height is zero, an aspect ratio cannot be calculated.
     */
    double GeoProj::ratioByMercatorBounds(const RangeRectd& bounds) noexcept {
        double width = bounds.width();
        double height = bounds.height();
        return Safe::canSafelyDivideBy(height) ? width / height : -1.0;
    }


    /**
     *  @brief Converts geographic coordinates (EPSG:4326, WGS84) to Web Mercator (EPSG:3857).
     *
     *  This function performs a forward projection from latitude/longitude (in degrees)
     *  to Web Mercator meters using the spherical Mercator approximation.
     *
     *  @param pos       Input position in degrees: pos.m_x = latitude, pos.m_y = longitude.
     *  @param out_pos   Output projected position in meters: out_pos.m_x = X, out_pos.m_y = Y.
     *  @return          Always returns true (for consistency or future error checking).
     *
     *  @note This projection assumes a spherical Earth using the constant Geo::kEarthRadius_m.
     *        This implementation uses a spherical Earth model, which is fast but
     *        slightly less accurate than an ellipsoidal model like WGS84.
     */
    bool GeoProj::earthProject4326To3857(const Vec2d& pos, Vec2d& out_pos) noexcept {
        double lat_rad = pos.m_x * std::numbers::pi / 180;
        double lon_rad = pos.m_y * std::numbers::pi / 180;
        out_pos.m_x = Geo::kEarthRadius_m * lon_rad;
        out_pos.m_y = Geo::kEarthRadius_m * log(tan(std::numbers::pi / 4 + lat_rad / 2));
        return true;
    }


    /**
     *  @brief Converts Web Mercator coordinates (EPSG:3857) back to geographic (EPSG:4326).
     *
     *  This function performs an inverse projection from Web Mercator X/Y (in meters)
     *  back to latitude/longitude (in degrees).
     *
     *  @param pos       Input Web Mercator position in meters: pos.m_x = X, pos.m_y = Y.
     *  @param out_pos   Output geographic position in degrees: out_pos.m_x = latitude, out_pos.m_y = longitude.
     *  @return          Always returns true (for consistency or future error checking).
     *
     *  @note Assumes the same spherical Earth model used in the forward projection.
     *        This implementation uses a spherical Earth model, which is fast but
     *        slightly less accurate than an ellipsoidal model like WGS84.
     */
    bool GeoProj::earthProject3857To4326(const Vec2d& pos, Vec2d& out_pos) noexcept {
        double lon = (pos.m_x / Geo::kEarthRadius_m) * (180 / std::numbers::pi);
        double temp = exp(-pos.m_y / Geo::kEarthRadius_m);
        double lat = (std::numbers::pi / 2 - 2 * std::atan(temp)) * (180 / std::numbers::pi);
        out_pos.m_x = lat;
        out_pos.m_y = lon;
        return true;
    }


    ErrorCode GeoProj::_update() noexcept {
        static const char* crs_4326_lonlat = "+proj=longlat +datum=WGS84 +no_defs";
        auto result = ErrorCode::None;

        try {
            if (m_must_update) {

                if (!m_proj_context) {
                    m_proj_context = proj_context_create();
                    if (!m_proj_context) {
                        throw Error::specific(1);  // TODO: Enum error code.
                    }
                }

                if (m_proj) {
                    proj_destroy((PJ*)m_proj);
                    m_proj = nullptr;
                }

                const char* src_crs = m_src_crs.utf8();
                const char* dst_crs = m_dst_crs.utf8();

                if (strcmp(src_crs, "EPSG:4326") == 0) {
                    src_crs = crs_4326_lonlat;
                }
                if (strcmp(dst_crs, "EPSG:4326") == 0) {
                    dst_crs = crs_4326_lonlat;
                }

                m_proj = proj_create_crs_to_crs((PJ_CONTEXT*)m_proj_context, src_crs, dst_crs, NULL);
                if (!m_proj) { throw Error::specific(2); }

                if (m_src_crs == m_dst_crs) {
                    m_ignore = true;
                }

                m_must_update = false;
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


}  // End of namespace Grain
