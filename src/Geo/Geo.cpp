//
//  Geo.cpp
//
//  Created by Roald Christesen on from 11.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  https://download.geofabrik.de/technical.html

#include "Geo/Geo.hpp"
#include "Math/Math.hpp"
#include "File/File.hpp"
#include "Geo/GeoProj.hpp"


namespace Grain {

    const GeoSRIDInfo Geo::g_srid_info[] = {
            { "3857", { -20037508.34, 20037508.34, -20048966.1, 20048966.1 }, Vec2d(0.0, 0.0), { -85.05112878, 85.05112878, -180.0, 180.0 } },
            { "4326", { -90.0, 90.0, -180.0, 180.0 }, Vec2d(0.0, 0.0), { -90.0, 90.0, -180.0, 180.0 } },
            { "25832", { -1877994.66, 3473041.38, 3638086.74, 9494203.2 }, Vec2d(675603.33, 6522325.42), { -16.1, 40.18, 32.88, 84.73 } },
            { "25833", { -2465144.8, 2885759.28, 3638055.41, 9493779.8 }, Vec2d(329015.88, 6522118.34), { -16.1, 40.18, 32.88, 84.73 } },
            { "Undefined", { 0.0, 0.0, 0.0, 0.0 }, Vec2d(0.0, 0.0), { 0.0, 0.0, 0.0, 0.0 } }
    };


    /**
     *  @brief Calculates the tile coordinates (x, y) for a given longitude and latitude at a specified zoom level.
     *
     *  @param zoom Zoom level, typically ranging from 0 to 20, where 0 represents the entire world and higher values increase detail.
     *  @param[in] lon Longitude in degrees, must be within the range [-180, 180].
     *  @param[in] lat Latitude in degrees, must be within the range [-90, 90].
     *  @param[out] out_tile_x The computed tile coordinate along the x-axis.
     *  @param[out] out_tile_y The computed tile coordinate along the y-axis.
     *
     *  @note If `lon` is out of range, `out_tile_x` is set to -1.
     *        If `lat` is out of range, `out_tile_y` is set to -1.
     *
     *  @see https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Lon..2Flat._to_tile_numbers_2
     */
    void Geo::wgs84ToTileIndex(int32_t zoom, double lon, double lat, int32_t& out_tile_x, int32_t& out_tile_y) noexcept {

        double n = pow(2, zoom);

        // Handle longitude by wrapping it to the [-180.0, 180.0] range.
        if (lon < -180.0 || lon > 180.0) {
            lon = fmod(lon + 180.0, 360.0);
            if (lon < 0.0) {
                lon += 360.0;
            }
            lon -= 180.0;
        }

        out_tile_x = static_cast<int32_t>(floor(n * ((lon + 180.0) / 360.0)));

        if (lat < -90.0 - std::numeric_limits<float>::epsilon() || lat > 90.0 + std::numeric_limits<float>::epsilon()) {
            out_tile_y = -1;
        }
        else {
            // Clamping value for Web Mercator projection.
            if (lat > kMaxLatDeg) {
                lat = kMaxLatDeg;
            }
            if (lat < kMinLatDeg) {
                lat = kMinLatDeg;
            }

            double lat_rad = Math::degtorad(lat);
            double y = log(tan(lat_rad) + 1.0 / std::cos(lat_rad));
            out_tile_y = static_cast<int32_t>(floor(n * (1.0 - y / std::numbers::pi) * 0.5));
        }
    }


    /**
     *  @brief latitude/longitude/zoom to OSM tile numbers.
     *
     *  @param zoom Zoom level. Minimum level is 0.
     *  @param[in] lonlat Longitude, latitude in degree.
     *  @param[out] out_tile_index Resulting tile number in x, y direction.
     */
    void Geo::wgs84ToTileIndex(int32_t zoom, Vec2d lonlat, Vec2i& out_tile_index) noexcept {

        wgs84ToTileIndex(zoom, lonlat.m_x, lonlat.m_y, out_tile_index.m_x, out_tile_index.m_y);
    }


    /**
     *  @brief OSM tile numbers to WGS84 longitude/latitude.
     *
     *  @param zoom Zoom level. Minimum level is 0.
     *  @param[in] tile_x Tile number in x direction.
     *  @param[in] tile_y Tile number in y direction.
     *  @param[out] out_lon Resulting longitude in degree.
     *  @param[out] out_lat Resulting latitude in degree.
     *
     *  @see https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Lon..2Flat._to_tile_numbers_2
     */
    void Geo::wgs84FromTileIndex(int32_t zoom, int32_t tile_x, int32_t tile_y, double& out_lon, double& out_lat) noexcept {

        double n = pow(2, zoom);
        out_lon = (double)tile_x / n * 360.0 - 180.0;
        double lat_rad = std::atan(std::sinh(std::numbers::pi * (1.0 - 2.0 * tile_y / n)));
        out_lat = lat_rad * 180.0 / std::numbers::pi;
    }


    /**
     *  @brief OSM tile numbers to WGS84 longitude/latitude.
     *
     *  @param zoom Zoom level. Minimum level is 0.
     *  @param[in] tile_index Tile number in x, y direction.
     *  @param[out] out_lonlat Resulting longitude/latitude in degree.
     */
    void Geo::wgs84FromTileIndex(int32_t zoom, Vec2i tile_index, Vec2d& out_lonlat) noexcept {

        wgs84FromTileIndex(zoom, tile_index.m_x, tile_index.m_y, out_lonlat.m_x, out_lonlat.m_y);
    }


    /**
     *  @brief Generates directory and file paths for a tile based on XYZ coordinates,
     *         used in Slippy Maps.
     *
     *  This method constructs separate paths for the directory and file of a tile in the context
     *  of a mapping system. The structure of the path is determined by the `base_path`, zoom level,
     *  tile coordinates, and file extension provided.
     *
     *  Directory paths are created in a hierarchical format: `<base_path>/<zoom>/<tile_x>`.
     *  File paths are generated as `<tile_y>.<file_ext>`, appended to the directory path.
     *
     *  @param base_path The base directory path. If empty, the path starts directly from the zoom level.
     *  @param zoom The zoom level of the tile (z-coordinate).
     *  @param tile_index The x/y-coordinate of the tile.
     *  @param file_ext The file extension for the tile (e.g., `"png"`, `"jpg"`). If empty, only the `tile_y` value is used.
     *  @param[out] out_dir_path A reference to the string where the directory path is stored.
     *                          This is cleared if an error occurs.
     *  @param[out] out_file_name A reference to the string where the file path is stored.
     *                            This is cleared if an error occurs.
     *
     *  @return An `ErrorCode` indicating the outcome:
     *          - `ErrorCode::None`: Both paths generated successfully.
     *          - `ErrorCode::StrBufferTooSmall`: The generated path exceeds the buffer size.
     *
     *  @note The method uses an internal buffer with a maximum size of 2048 bytes.
     *        Paths exceeding this size will result in an error.
     */
    ErrorCode Geo::slippyTilePathForTile(const String& base_path, int32_t zoom, const Vec2i& tile_index, const String& file_ext, String& out_dir_path, String& out_file_name) noexcept {

        auto result = ErrorCode::None;

        char buffer[2048];
        int64_t n = 0;

        try {

            if (base_path.length() > 0) {
                n = std::snprintf(buffer, 2048, "%s/%d/%d", base_path.utf8(), zoom, tile_index.m_x);
            }
            else {
                n = std::snprintf(buffer, 2048, "%d/%d", zoom, tile_index.m_x);
            }

            if (n >= 2048) { throw ErrorCode::StrBufferTooSmall; }

            out_dir_path = buffer;

            if (file_ext.length() < 1) {
                n = std::snprintf(buffer, 2048, "%d", tile_index.m_y);
            }
            else {
                n = std::snprintf(buffer, 2048, "%d.%s", tile_index.m_y, file_ext.utf8());
            }

            if (n >= 2048) { throw ErrorCode::StrBufferTooSmall; }
            out_file_name = buffer;
        }
        catch (ErrorCode err) {
            out_dir_path.clear();
            out_file_name.clear();
            result = err;
        }

        return result;
    }


    /**
     *  @brief Converts lon/lat (WGS84) coordinates to Slippy Map tile indices.
     *
     *  @note Ensures correct rounding and stability in calculation.
     *
     *  @param pos The lon/lat (WGS84) coordinate.
     *  @param zoom The zoom level.
     *  @param[out] out_slippy_index The resulting slippy index.
     *  @return An error code.
     */
    ErrorCode Geo::slippyTileIndexFromLonlat(int32_t zoom, Vec2d pos, Vec2i& out_slippy_index) noexcept {

        constexpr double kCorrectionFactor = 1e-9;


        // Compute Slippy tile X.
        out_slippy_index.m_x = (int32_t)floor(((pos.m_x + 180.0) / 360.0 * (1 << zoom)) + kCorrectionFactor);

        // Compute Slippy tile Y (adjusted for Web Mercator).
        double lat_rad = pos.m_y * std::numbers::pi / 180.0;
        out_slippy_index.m_y = (int32_t)floor(((1.0 - log(tan(lat_rad) + 1.0 / cos(lat_rad)) / std::numbers::pi) / 2.0 * (1 << zoom)) + kCorrectionFactor);

        // Clamp tile values to valid range.
        int max_tile = (1 << zoom) - 1;
        out_slippy_index.m_x = fmin(fmax(out_slippy_index.m_x, 0), max_tile);
        out_slippy_index.m_y = fmin(fmax(out_slippy_index.m_y, 0), max_tile);

        return ErrorCode::None;
    }


    /**
     *  @brief Constructs the file path to an OSM (OpenStreetMap) meta-tile for a specified tile position (x, y) and zoom level.
     *
     *  This function calculates and builds the path to a meta-tile file based on the provided zoom level and tile coordinates (x, y).
     *  The path is stored in the buffer pointed to by `out_path`.
     *
     *  @param base_path The base directory to be used or nullptr, if a relative path should be created.
     *  @param zoom The zoom level for the tile.
     *  @param tile_index The x/y-coordinate of the tile.
     *  @param[out] out_dir_path The path where meta tile should be saved.
     *  @param[out] out_file_name The name of the meta tile file.
     *
     *  @return The offset of the tile within the meta-tile file.
     *
     *  @details The constructed file path format is:
     *    zoom/<16*A+a>/<16*B+b>/<16*C+c>/<16*D+d>/<8*(16*E+e)>.meta
     *
     *  @note The x and y tile positions are represented as follows:
     *    - x = AAAABBBBCCCCDDDDEMMM
     *    - y = aaaabbbbccccddddemmm
     *
     *  The tile index within the meta-tile is calculated as: 8 * M + m.
     */
    int32_t Geo::metaTilePathForTile(const String& base_path, int32_t zoom, const Vec2i& tile_index, const String& file_ext, String& out_dir_path, String& out_file_name) noexcept {

        char buffer[2560];

        static const int kMetaTileGridSize = 8;
        int32_t offset = -1;

        uint8_t hash[5];
        uint8_t mask = kMetaTileGridSize - 1;

        int32_t tile_x = tile_index.m_x;
        int32_t tile_y = tile_index.m_y;

        // Each meta tile winds up in its own file, with several in each leaf directory.
        // the .meta tile name is beasd on the sub-tile at (0, 0).
        offset = (tile_x & mask) * kMetaTileGridSize + (tile_y & mask);
        tile_x &= ~mask;
        tile_y &= ~mask;

        for (int32_t i = 0; i < 5; i++) {
            hash[i] = ((tile_x & 0x0f) << 4) | (tile_y & 0x0f);
            tile_x >>= 4;
            tile_y >>= 4;
        }

        if (base_path.isNotEmpty()) {
            std::snprintf(buffer, 2560, "%s/%d/%u/%u/%u/%u", base_path.utf8(), zoom, hash[4], hash[3], hash[2], hash[1]);
            out_dir_path = buffer;
        }
        else {
            std::snprintf(buffer, 2560, "%d/%u/%u/%u/%u", zoom, hash[4], hash[3], hash[2], hash[1]);
            out_dir_path = buffer;
        }

        std::snprintf(buffer, 2560, "%u.%s", hash[0], file_ext.utf8());
        out_file_name = buffer;

        return offset;
    }


    int32_t Geo::metaTilePathForTile(const String& base_path, int32_t zoom, const Vec2i& tile_index, const String& file_ext, String& out_file_path) noexcept {

        String file_name;
        auto offset = Geo::metaTilePathForTile(base_path, zoom, tile_index, file_ext, out_file_path, file_name);
        out_file_path += '/';
        out_file_path += file_name;

        return offset;
    }


    /**
     *  @brief Calculate the Great-Circle distance between two points on a sphere.
     *
     *  This function uses the Haversine formula to compute the shortest distance
     *  over the Earth's surface, accounting for its curvature.
     *
     *  @param lon1 Longitude of point 1 in degrees.
     *  @param lat1 Latitude of point 1 in degrees.
     *  @param lon2 Longitude of point 2 in degrees.
     *  @param lat2 Latitude of point 2 in degrees.
     *  @param radius Radius of the sphere (e.g., Earth's radius in meters).
     *  @return Distance between point 1 and point 2 in meters.
     */
    double Geo::haversineDistance(double lon1, double lat1, double lon2, double lat2, double radius) noexcept {

        lon1 = Math::degtorad(lon1);
        lat1 = Math::degtorad(lat1);
        lon2 = Math::degtorad(lon2);
        lat2 = Math::degtorad(lat2);

        double dlat = lat2 - lat1;
        double dlon = lon2 - lon1;

        double a = std::sin(0.5 * dlat) * std::sin(0.5 * dlat) + std::cos(lat1) * std::cos(lat2) * std::sin(0.5 * dlon) * std::sin(0.5 * dlon);
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        return radius * c;
    }


    /**
     *  @brief Calculate the distance between two points on a sphere using the
     *         Spherical Law of Cosines.
     *
     *  @param bounds The bounding box in WGS84 coordinates.
     *  @param srid The SRID used in `bounds`.
     *  @param radius Radius of the sphere (e.g., Earth's radius in meters).
     *  @param bound_type Specifies the type of bound calculation for `bounds`, using one of Min, Max, or Mean.
     *  @param[out] out_width The resulting width in meters.
     *  @param[out] out_height The resulting height in meters.
     */
    void Geo::haversineWidthAndHeight(const RangeRectd& bounds, int32_t srid, double radius, BoundType bound_type, double& out_width, double& out_height) noexcept {

        RangeRectd used_bounds;
        Vec2d p1, p2;

        if (srid != 4326) {
            GeoProj proj;
            proj.setSrcSRID(srid);
            proj.setDstSRID(4326);
            proj.transform(bounds, used_bounds);
        }
        else {
            used_bounds = bounds;
        }

        p1.m_x = used_bounds.m_min_x;
        p2.m_x = used_bounds.m_max_x;
        p1.m_y = p2.m_y = used_bounds.m_min_y;
        auto w1 = haversineDistance(p1, p2, radius);
        p1.m_y = p2.m_y = used_bounds.m_max_y;
        auto w2 = haversineDistance(p1, p2, radius);
        p1.m_y = used_bounds.m_min_y;
        p2.m_y = used_bounds.m_max_y;
        p1.m_x = p2.m_x = used_bounds.m_min_x;
        auto h1 = haversineDistance(p1, p2, radius);
        p1.m_x = p2.m_x = used_bounds.m_max_x;
        auto h2 = haversineDistance(p1, p2, radius);

        switch (bound_type) {
            case BoundType::Min:
                out_width = std::min(w1, w2);
                out_height = std::min(h1, h2);
                break;
            case BoundType::Max:
                out_width = std::max(w1, w2);
                out_height = std::max(h1, h2);
                break;
            case BoundType::Mean:
            default:
                out_width = 0.5 * (w1 + w2);
                out_height = 0.5 * (h1 + h2);
                break;
        }
    }


    /**
     *  @brief Calculate the distance between two points on a sphere using the
     *         Spherical Law of Cosines.
     *
     *  @param lon1 Longitude of point 1 in degree.
     *  @param lat1 Latitude of point 1 in degree.
     *  @param lon2 Longitude of point 2 in degree.
     *  @param lat2 Latitude of point 2 in degree.

     *  @return Distance of point 1 and point 2 in meters.
     */
    double Geo::sphericalLawOfCosinesDistance(double lon1, double lat1, double lon2, double lat2, double radius) noexcept {

        lon1 = Math::degtorad(lon1);
        lat1 = Math::degtorad(lat1);
        lon2 = Math::degtorad(lon2);
        lat2 = Math::degtorad(lat2);

        double dlat = lon2 - lon1;

        return std::acos(std::sin(lat1) * std::sin(lat2) + std::cos(lat1) * std::cos(lat2) * std::cos(dlat)) * radius;
    }


} // End of namespace Grain
