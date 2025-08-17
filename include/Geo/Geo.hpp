//
//  Geo.hpp
//
//  Created by Roald Christesen on from 11.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainGeo_hpp
#define GrainGeo_hpp


#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Math/Vec2.hpp"
#include "2d/RangeRect.hpp"
#include "String/String.hpp"
#include "Math/Math.hpp"

#include <proj.h>


namespace Grain {


    /**
     *  @brief Coordinate Reference System information.
     */
    typedef struct GeoSRIDInfo {
        const char* m_id_str;       ///< SRID number as C-stirng.
        RangeRectd m_bounds;        ///< Bounds of thhe coordinate system.
        Vec2d m_center;             ///< Center.
        RangeRectd m_wgs84_bounds;  ///< Corresponding bounds in WGS84 lon/lat coordinates.
    } GeoSRIDInfo;


    /**
     * @brief Utility functions and constants for geographic (Geo) applications.
     */
    class Geo {

    public:

        enum class SRIDIndex {
            Undefined = -1,
            SRID_3857 = 0,
            SRID_4326,
            SRID_25832,
            SRID_25833,

            Count,
            First = SRID_3857,
            Last = Count - 1
        };

        enum class BoundType {
            Min, Max, Mean
        };

        enum {
            kHaversineDistance = 0,
            kSphericalLawOfCosinesDistance
        };

        enum {
            kErrFlag_LonlatOutOfBounds = 0x1,
            kErrFlag_UnsupportedZoom = 0x2,
            kErrFlag_UnsupportedMetaTileSize = 0x4
        };

        static constexpr int32_t kMetaTileGridSize = 8;     ///< Typical size of a meta tile in map applications.
        static constexpr int32_t kMaxMapZoomLevel = 30;     ///< Max zoom level for map applications.

        static constexpr double kEarthRadius_m = 6378137.0;  ///< Radius of Earth in meters.
        static constexpr double kEarthRadius_km = 6378.137;  ///< Radius of Earth in kilometers.
        static constexpr double kWGS84EllipsoidFlattening = 1.0 / 298.257223563;  ///< Flattening of the WGS 84 ellipsoid.
        static constexpr double kEarthCircumferenceAtEquator = kEarthRadius_m * 2.0 * std::numbers::pi;

        // OSM Limits.
        static constexpr double kMinLatDeg = -85.0511;
        static constexpr double kMaxLatDeg = 85.0511;
        static constexpr double kMinLonDeg = -180.0;
        static constexpr double kMaxLonDeg = 180.0;


    protected:
        static const GeoSRIDInfo g_srid_info[];

    public:
        static void wgs84ToTileIndex(int32_t zoom, double lon, double lat, int32_t& out_tile_x, int32_t& out_tile_y) noexcept;
        static void wgs84ToTileIndex(int32_t zoom, Vec2d lonlat, Vec2i& out_tile_index) noexcept;
        static void wgs84FromTileIndex(int32_t zoom, int32_t tile_x, int32_t tile_y, double& out_lon, double& out_lat) noexcept;
        static void wgs84FromTileIndex(int32_t zoom, Vec2i tile_index, Vec2d& out_lonlat) noexcept;

        static ErrorCode slippyTilePathForTile(const String& base_path, int32_t zoom, const Vec2i& tile_index, const String& file_ext, String& out_dir_path, String& out_file_name) noexcept;
        static ErrorCode slippyTileIndexFromLonlat(int32_t zoom, Vec2d lonlat, Vec2i& out_slippy_index) noexcept;


        static int32_t metaTilePathForTile(const String& base_path, int32_t zoom, const Vec2i& tile_index, const String& file_ext, String& out_dir_path, String& out_file_name) noexcept;
        static int32_t metaTilePathForTile(const String& base_path, int32_t zoom, const Vec2i& tile_index, const String& file_ext, String& out_file_path) noexcept;

        static int64_t slippyMapTileCount(int32_t zoom) noexcept;
        static int64_t slippyMapPixelWidth(int32_t zoom, int32_t tile_size) noexcept;
        static double meterPerPixelAtLat(int32_t zoom, int32_t tile_size, double lat, double radius) noexcept;
        static int32_t findBestZoomLevel(double target_mpp, int32_t tile_size, double lat, double earth_radius) noexcept;


        /**
         *  @brief Convert horizontal distance (in meters) to degrees of longitude at the equator.
         *
         *  @param distance The horizontal distance in meters.
         *  @return The equivalent degrees of longitude at the equator.
         */
        static double lonFromMeterAtEquator(double distance) {
            return (distance / (2.0 * std::numbers::pi * kEarthRadius_m)) * 360.0;
        }


        /**
         *  @brief Calculate the horizontal distance in meters represented by one pixel at a given zoom level and latitude.
         *
         *  @param lat The latitude in degrees where the calculation is made.
         *  @param zoom The zoom level in a Leaflet map.
         *  @return The number of meters per pixel at the specified latitude and zoom level.
         */
        static double leafletMeterPerPixel(double lat, double zoom) {
            return kEarthCircumferenceAtEquator * std::cos(lat * std::numbers::pi / 180.0) / pow(2, (zoom + 8));
        }


        static double shortestDistanceOnSphere(const Vec2d& lonlat1, const Vec2d& lonlat2, double radius, int32_t mode = kHaversineDistance) noexcept {
            switch (mode) {
                case kHaversineDistance:
                    return haversineDistance(lonlat1, lonlat2, radius);
                default:
                    return sphericalLawOfCosinesDistance(lonlat1, lonlat2, radius);
            }
        }

        static double haversineDistance(const Vec2d& lonlat1, const Vec2d& lonlat2, double radius) noexcept {
            return haversineDistance(lonlat1.m_x, lonlat1.m_y, lonlat2.m_x, lonlat2.m_y, radius);
        }
        static double haversineDistance(double lon1, double lat1, double lon2, double lat2, double radius) noexcept;
        static double haversineDistanceAtLon(double lon, double lat1, double lat2, double radius) noexcept {
            return haversineDistance(lon, lat1, lon, lat2, radius);
        }
        static double haversineDistanceAtLat(double lat, double lon1, double lon2, double radius) noexcept {
            return haversineDistance(lon1, lat, lon2, lat, radius);
        }


        static void haversineWidthAndHeight(const RangeRectd& bounds, int32_t srid, double radius, BoundType bound_type, double& out_width, double& out_height) noexcept;

        static double sphericalLawOfCosinesDistance(const Vec2d& lonlat1, const Vec2d& lonlat2, double radius) noexcept {
            return sphericalLawOfCosinesDistance(lonlat1.m_x, lonlat1.m_y, lonlat2.m_x, lonlat2.m_y, radius);
        }
        static double sphericalLawOfCosinesDistance(double lon1, double lat1, double lon2, double lat2, double radius) noexcept;


        static const bool sridInfo(SRIDIndex index, GeoSRIDInfo& out_info) noexcept {
            if (index >= SRIDIndex::First && index <= SRIDIndex::Last) {
                out_info = g_srid_info[static_cast<int32_t>(index)];
                return true;
            }
            else {
                out_info = g_srid_info[static_cast<int32_t>(SRIDIndex::Count)];
                return false;
            }
        }
    };


} // End of namespace Grain

#endif // GrainGeo_hpp
