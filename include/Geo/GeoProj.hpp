//
//  GeoProj.hpp
//
//  Created by Roald Christesen on from 11.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainGeoProj_hpp
#define GrainGeoProj_hpp


#include "Geo/Geo.hpp"
#include "2d/Quadrilateral.hpp"


namespace Grain {

    typedef bool (*GepProjTransformAction)(const Vec2d& pos, Vec2d& out_pos);


    /**
     *  @brief Helper class for GIS and Geographic based apps.
     *
     *  @see https://proj.org/en/
     *  @see https://epsg.io/4647
     */
    class GeoProj : public Object {

    protected:
        PJ_CONTEXT* m_proj_context{};
        PJ* m_proj{};
        String m_src_crs;
        String m_dst_crs;
        GepProjTransformAction m_transform_action{};
        bool m_must_update = true;
        bool m_ignore = false;      // Ignore transformation, if `m_src_crs`and `m_dst_crs` are the same.

        RemapRectd m_remap_rect;

    public:
        GeoProj() noexcept;

        GeoProj(int32_t src_srid, int32_t dst_srid) noexcept {
            setSrcSRID(src_srid);
            setDstSRID(dst_srid);
            _update();
        }

        GeoProj(const String& src_crs, const String& dst_crs) noexcept {
            setSrcCrs(src_crs.utf8());
            setDstCrs(dst_crs.utf8());
            _update();
        }

        GeoProj(const char* src_crs, const char* dst_crs) noexcept {
            setSrcCrs(src_crs);
            setDstCrs(dst_crs);
            _update();
        }

        ~GeoProj() noexcept;

        friend std::ostream& operator << (std::ostream& os, const GeoProj* o) {
            o == nullptr ? os << "nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GeoProj& o) {
            if (o.m_transform_action != nullptr) {
                os << "Uses specific transformation method." << o.m_dst_crs;
            }
            os << "src_crs: " << o.m_src_crs << ", m_dst_crs: " << o.m_dst_crs;
            return os;
        }


        bool isValid() noexcept;

        void setSrcCrs(const char* src_str) noexcept {
            m_src_crs = src_str;
            m_must_update = true;
        }

        void setSrcCrs(const String& src_string) noexcept {
            m_src_crs = src_string;
            m_must_update = true;
        }

        void setSrcSRID(int32_t srid) noexcept {
            m_src_crs = "EPSG:";
            m_src_crs += srid;
            m_must_update = true;
        }

        ErrorCode setSrcCrsByFile(const String& file_path) noexcept;

        void setDstCrs(const char* dst_str) noexcept {
            m_dst_crs = dst_str;
            m_must_update = true;
        }

        void setDstCrs(const String& dst_string) noexcept {
            m_dst_crs = dst_string;
            m_must_update = true;
        }

        void setDstSRID(int32_t srid) noexcept {
            m_dst_crs = "EPSG:";
            m_dst_crs += srid;
            m_must_update = true;
        }


        ErrorCode setDstCrsByFile(const String& file_path) noexcept;

        void setupRemapRect(const Rectd& src_rect, const Rectd& dst_rect, bool flip_y = false) noexcept {
            m_remap_rect.set(src_rect, dst_rect, flip_y);
        }

        bool transform(const Vec2d& pos, Vec2d& out_pos, bool inverse = false) noexcept;
        bool transform(Vec2d& pos, bool inverse = false) noexcept { return transform(pos, pos, inverse); }
        bool transform(Vec2d* pos, bool inverse = false) noexcept {
            if (pos != nullptr) {
                return transform(*pos, *pos, inverse);
            }
            else {
                return false;
            }
        }
        bool transform(Vec2d* pos, int32_t n, bool inverse = false) noexcept {
            if (pos != nullptr) {
                for (int32_t i = 0; i < n; i++) {
                    if (transform(*pos, *pos, inverse) == false) {
                        return false;
                    }
                }
                return true;
            }
            else {
                return false;
            }
        }

        bool transform(const RangeRectd& range_rect, RangeRectd& out_range_rect, bool inverse = false) noexcept;
        bool transform(RangeRectd& range_rect, bool inverse = false) noexcept;

        bool transform(const RangeRectFix& range_rect, RangeRectFix& out_range_rect, bool inverse = false) noexcept;
        bool transform(RangeRectFix& range_rect, bool inverse = false) noexcept;

        bool transform(Quadrilateral& quadrilateral, bool inverse = false) noexcept;


        bool transformToViewport(const Vec2d& pos, Vec2d& out_pos) noexcept;
        bool transformFromViewport(const Vec2d& pos, Vec2d& out_pos) noexcept;


        static bool isWGS84Lon(double lon) { return lon >= -180.0 && lon <= 180.0; }
        static bool isWGS84Lat(double lat) { return lat >= -90.0 && lat <= 85.0511; }

        static bool isWGS84Pos(const Vec2d& pos) {
            return isWGS84Lon(pos.m_x) && isWGS84Lat(pos.m_y);
        }

        static bool isWGS84Pos(double lon, double lat) {
            return isWGS84Lon(lon) && isWGS84Lat(lat);
        }

        static bool isWGS84Bbox(const RangeRectd& bounding_box) {
            return isWGS84Pos(bounding_box.m_min_x, bounding_box.m_min_y) &&
                   isWGS84Pos(bounding_box.m_max_x, bounding_box.m_max_y);
        }

        static double ratioByMercatorBounds(const RangeRectd& bounds) noexcept;

        static bool earthProject4326To3857(const Vec2d& pos, Vec2d& out_pos) noexcept;
        static bool earthProject3857To4326(const Vec2d& pos, Vec2d& out_pos) noexcept;

        ErrorCode _update() noexcept;
    };


} // End of namespace Grain

#endif // GrainGeoProj_hpp
