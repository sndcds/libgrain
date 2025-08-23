//
//  CVF2TileManager.hpp
//
//  Created by Roald Christesen on 20.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainCVF2TileManager_hpp
#define GrainCVF2TileManager_hpp

#include "Grain.hpp"
#include "CVF2.hpp"
#include "CVF2File.hpp"
#include "Math/Vec2Fix.hpp"
#include "Geo/Geo.hpp"
#include "Geo/GeoProj.hpp"
#include "2d/RangeRect.hpp"
#include "Time/Timestamp.hpp"
#include "Type/Flags.hpp"
#include "Image/Image.hpp"
#include "ValueGrid.hpp"


namespace Grain {

    /**
     *  @brief A single tile used in the tile manager.
     *
     *  This class represents a single tile in the tile manager. It stores
     *  information about the tile's position, dimensions, file paths, error status,
     *  and cache/file usage indices.
     */
    class CVF2Tile : public Object {

        friend class CVF2TileManager;
        friend class CVF2File;

    protected:
        int32_t m_index = -1;
        int32_t m_init_counter = 0;         ///< Counts the number of times the tile is initialized. More than one initialization indicates a failure
        bool m_valid = false;               ///< Flag indicating whether the tile is valid for use
        ErrorCode m_last_err_code = ErrorCode::None;     ///< Last error occurred in tile preparation

        RangeRectFix m_bbox;                ///< Bounding box as Fix values in SRID of tile manager
        RangeRectd m_bbox_dbl;              ///< Bounding box as doubles in SRID of tile manager

        int32_t m_x_index = 0;              ///< Tile x index in 2d tile array
        int32_t m_y_index = 0;              ///< Tile y index in 2d tile array

        int32_t m_x_offset = 0;             ///< Offset of the tile's X position within the tile space
        int32_t m_y_offset = 0;             ///< Offset of the tile's Y position within the tile space

        uint32_t m_width = 0;               ///< Tile width in tile space
        uint32_t m_height = 0;              ///< Tile height in tile space

        int32_t m_undefined_values_count = 0;

        String m_file_name;                 ///< Name of cvf2 file
        String m_file_path;                 ///< Path to cvf2 file
        String m_raw_file_path;             ///< Path to raw file
        bool m_raw_file_exist = false;      ///< Indicates whether the raw file exists

        int32_t m_cache_cvf2_file_index = -1;   ///< Index of the file being used, or -1 if no file is being used

        int32_t m_reserved = 0;
        Flags m_error_flags;

        ValueGridf* m_value_grid = nullptr;

    public:
        CVF2Tile(int32_t index) {
            m_index = index;
        }

        ~CVF2Tile() override = default;

        const char* className() const noexcept override { return "CVF2Tile"; }

        friend std::ostream& operator << (std::ostream& os, const CVF2Tile* o) {
            o == nullptr ? os << "CVF2Tile nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const CVF2Tile& o) {
            o.log(os, 0, o.className());
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log log(os, indent);
            log.header(label);
            log << "m_index: " << m_index << log.endl;
            log << "m_init_counter: " << m_init_counter << log.endl;
            log << "m_valid: " << m_valid << log.endl;
            log << "m_bbox: " << m_bbox << log.endl;
            log << "m_x_index: " << m_x_index << ", m_y_index: " << m_y_index << log.endl;
            log << "m_x_offset: " << m_x_offset << ", m_y_offset: " << m_y_offset << log.endl;
            log << "m_width: " << m_width << ", m_height: " << m_height << log.endl;
            log << "m_undefined_values_count: " << m_undefined_values_count << log.endl;
            log << "m_file_path: " << m_file_path << log.endl;
            log << "m_raw_file_path: " << m_raw_file_path << log.endl;
            log << "m_raw_file_exist: " << m_raw_file_exist << log.endl;
            log << "m_cache_cvf2_file_index: " << m_cache_cvf2_file_index << log.endl;
            log << "m_error_flags: " << m_error_flags << log.endl;
        }

        bool isValid() const noexcept { return m_valid; }
        Rectd rect() const noexcept { return m_bbox_dbl.rect(); }
        RangeRectd rangeRect() const noexcept { return m_bbox_dbl; }

        bool hasUndefinedValues() const noexcept { return m_undefined_values_count > 0; }
        int32_t undefinedValuesCount() const noexcept { return m_undefined_values_count; }

        bool matchesSize(int32_t width, int32_t height) const noexcept {
            return width == static_cast<int32_t>(m_width) && height == static_cast<int32_t>(m_height);
        }


        bool cvf2FileIsOpen() const noexcept { return m_cache_cvf2_file_index >= 0; }
        void crsPosToTileXY(const Vec2d& crs_pos, Vec2i& out_xy) const noexcept {
            out_xy.m_x = static_cast<int32_t>(Math::remap(m_bbox_dbl.m_min_x, m_bbox_dbl.m_max_x, 0, m_width - 1, crs_pos.m_x));
            out_xy.m_y = static_cast<int32_t>(Math::remap(m_bbox_dbl.m_min_y, m_bbox_dbl.m_max_y, 0, m_height - 1, crs_pos.m_y));
        }

        bool hasValueGrid() noexcept;
        ErrorCode checkValueGrid(CVF2TileManager* manager) noexcept;
        ErrorCode generateValueGrid(CVF2TileManager* manager) noexcept;
        void freeValueGrid() noexcept;


        ValueGridf* valueGridPtr() noexcept { return m_value_grid; }
    };


    /**
     *  @brief Handles access to an CVF2 file slot.
     *
     *  This slot represents a cache or storage unit for managing CVF2 file instances.
     *  Each time the slot accesses an CVF2 file, it updates the `m_timestamp` to the
     *  current timestamp. This updated timestamp is crucial for identifying the "oldest" slot
     *  in scenarios where a new slot is needed but no empty slots are available. The oldest slot
     *  may then be overwritten or cleared based on the cache management policy.
     *
     *  @note Assumes that the CVF2 file format is a specific format used in this context.
     */
    struct CVF2ManagerFileSlot {

        timestamp_t m_timestamp;
        CVF2File* m_file = nullptr;
        int64_t m_tile_index = -1;
    };


    /**
     *  @brief Tilemanager.
     */
    class CVF2TileManager : public Object {

        friend class CVF2File;
        friend class CVF2Tile;

    public:
        enum {
            kErrNotScanned = 0,
            kErrRangeNotValid,
            kErrToManyTilesFound,
            kErrNoCVF2FilesInDir,
            kErrNoTiles,
            kErrTileListInitFailed,
            kErrTileTileInstantiationFailed,
            kErrTileIndexOutOfRange,
            kErrTileOffsetOutOfRange,
            kErrTileSizeOutOfRange,
            kErrTileIsNull,
            kErrTileIsInvalid,
            kErrTileFileNotPresent,
            kErrReadFromCVF2Failed,
            kErrTileManagerNotRunning,
            kErrGenerateTileValuesFailed,
            kErrNoRawFilesInDir,
            kErrRawFileMissingXYInName,
            kErrTileCrsMismatch,
            kErrNoTilesInvolved,
            kErrMetaTileRangeFailed,
            kErrZoomOutOfRange,
            kErrBboxTransformFailed,
            kErrTileSRIDMissing,
            kErrTileProvidedBboxSRIDMissing,
            kErrToManyTilesStarted
        };

    protected:
        String m_dir_path;                      ///< Path to the main directory of the tile manager data structure

        // Bounding box
        RangeRectd m_provided_bbox;             ///< Input bounding box defining the area of interest
        int32_t m_provided_bbox_srid = 0;       ///< Spatial Reference System Identifier (SRID) of the input bounding box
        RangeRectd m_bbox;                      ///< Input bounding box projected to tile manager SRID
        bool m_bbox_used = false;               ///< Flag indicating whether the bounding box (m_bbox) is in use
        bool m_bbox_valid = false;              ///< Flag indicating whether the bounding box (m_bbox) is valid to use

        int32_t m_tile_width{};                 ///< Full width of tile
        int32_t m_tile_height{};                ///< Full height of tile
        int32_t m_x_tile_count{};               ///< Number of tiles in x direction
        int32_t m_y_tile_count{};               ///< Number of tiles in y direction
        int32_t m_tile_count{};                 ///< Number of tiles in 2d array
        int32_t m_scanned_tile_count{};         ///< Number of scanned tiles
        int32_t m_tile_count_limit = 10000000;  ///< Limit of tiles in 2d array
        size_t m_min_cvf2_file_size = 1;        ///< CVF2 files smaller will be ignored
        size_t m_max_cvf2_file_size = Type::gigabytesToBytes(2);  ///< CVF2 files greater will be ignored

        ObjectList<CVF2Tile*> m_tiles;
        List<CVF2Tile*> m_started_tile_list;

        bool m_scan_done = false;
        bool m_running = false;

        timestamp_t m_scan_ts1{};
        timestamp_t m_scan_ts2{};
        RangeRectFix m_scan_xy_range;
        RangeRectd m_scan_xy_range_dbl;
        int64_t m_scan_total_min{};
        int64_t m_scan_total_max{};
        int64_t m_scan_total_undefined_values_n{};
        int32_t m_scan_files_n{};
        int32_t m_scan_files_ignored_n{};
        int32_t m_scan_file_err_count{};
        int32_t m_scan_incomplete_files_n{};
        int32_t m_scan_wrong_dimension_files_n{};

        timestamp_t m_start_ts1{};
        timestamp_t m_start_ts2{};
        int32_t m_start_tile_multi_inititialized_n{};   ///< Counts how many tiles where initialized more than one time
        int32_t m_start_error_n{};                      ///< Counts how many tile errors happened when starting
        int32_t m_start_file_err_count{};

        int64_t m_cvf2_file_open_n{};                   ///< Counts how often a cfv2 file has been opened
        int64_t m_cvf2_file_close_n{};                  ///< Counts how often a cfv2 file has been closed
        int64_t m_cvf2_file_open_failed_n{};            ///< Counts how often a cfv2 file open failed

        int32_t m_file_slot_capacity = 10;              ///< Maximum number of open files
        CVF2ManagerFileSlot* m_file_slots = nullptr;    ///< Array with file slots

        int32_t m_tile_srid = 0;                        ///< Spatial Reference System Identifier (SRID) of tiles
        GeoProj m_wgs84_to_tile_proj;
        ErrorCode m_last_read_err = ErrorCode::Unknown;

        bool m_cache_tile_flag = false;
        int32_t m_verbose_level = 0;


    public:
        CVF2TileManager(const String& dir_path, int32_t tile_width, int32_t tile_height, int32_t open_files_capacity);
        ~CVF2TileManager();

        const char* className() const noexcept override { return "CVF2TileManager"; }

        friend std::ostream& operator << (std::ostream& os, const CVF2TileManager* o) {
            o == nullptr ? os << "CVF2TileManager nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const CVF2TileManager& o) {
            o.log(os, 0, o.className());
            return os;
        }

        void log(std::ostream& os, int32_t indent, const char* label) const {
            Log log(os, indent);
            log.header(label);
            log << "m_scan_file_err_count: " << m_scan_file_err_count << log.endl;
            log << "m_scan_xy_range: " << m_scan_xy_range << log.endl;
            log << "m_scan_xy_range_dbl.width(): " << m_scan_xy_range_dbl.width() << log.endl;
            log << "m_tile_width: " << m_tile_width << log.endl;
            log << "m_scan_xy_range_dbl.height(): " << m_scan_xy_range_dbl.height() << log.endl;
            log << "m_tile_height: " << m_tile_height << log.endl;
            log << "m_x_tile_count: " << m_x_tile_count << log.endl;
            log << "m_y_tile_count: " << m_y_tile_count << log.endl;
            log << "m_tile_count: " << m_tile_count << log.endl;
            log << "m_started_tile_list size: " << m_started_tile_list.size() << log.endl;
            log << "m_scan_total_min: " << m_scan_total_min << log.endl;
            log << "m_scan_total_max: " << m_scan_total_max << log.endl;
            log << "m_cache_tile_flag: " << m_cache_tile_flag << log.endl;
            log << "total width: " << totalWidth() << log.endl;
            log << "total height: " << totalHeight() << log.endl;
            log << "total covered values: " << totalCoveredValues() << log.endl;
        }

        int32_t tileSRID() const noexcept { return m_tile_srid; }
        int32_t tileWidth() const noexcept { return m_tile_width; }
        int32_t tileHeight() const noexcept { return m_tile_height; }
        int32_t totalWidth() const noexcept { return m_x_tile_count * m_tile_width; }
        int32_t totalHeight() const noexcept { return m_y_tile_count * m_tile_height; }
        int32_t totalCoveredValues() const noexcept {
            return m_x_tile_count * m_tile_width * m_y_tile_count * m_tile_height;
        }


        void setTileSRID(int32_t srid) noexcept { m_tile_srid = srid; }
        void setTileCountLimit(int32_t limit) noexcept { m_tile_count_limit = limit; }

        void enableTileCache() noexcept { m_cache_tile_flag = true; }
        void disableTileCache() noexcept { m_cache_tile_flag = false; }
        bool useTileCache() const noexcept { return m_cache_tile_flag; }


        ErrorCode scan(const RangeRectd& bbox, int32_t bbox_srid) noexcept;
        ErrorCode scan() noexcept;
        void _scanFile(const String& dir_path, const String& file_name);

        ErrorCode start() noexcept;
        void _startFile(const String& dir_path, const String& file_name);

        int32_t tileCount() const noexcept { return m_tile_count; }

        int64_t tileIndexAtTileManagerPos(const Vec2d& pos, Vec2i& out_tile_xy_index) noexcept;
        int64_t tileIndexAtLonlat(const Vec2d& lonlat, Vec2i& out_tile_xy_index) noexcept;
        CVF2Tile* tileAtIndex(int64_t index) noexcept;

        List<CVF2Tile*>*startedTileList() noexcept { return &m_started_tile_list; }

        int64_t valueAtWGS84Pos(const Vec2d& lonlat) noexcept;
        int64_t valueAtPos(const Vec2d& pos) noexcept;
        double doubleAtPos(const Vec2d& pos) noexcept {
            int64_t value = valueAtPos(pos);
            return value == CVF2::kUndefinedValue ? NAN : value;
        }


        bool hasReadError() const noexcept { return m_last_read_err != ErrorCode::None; }
        ErrorCode lastReadError() const noexcept { return m_last_read_err; }
        void clearReadError() noexcept { m_last_read_err = ErrorCode::None; }


        CVF2File* cvf2FileForTile(CVF2Tile* tile) noexcept;

        ErrorCode generateRawTiles() noexcept;

        ErrorCode collectImage(const RangeRectd& bbox, int32_t bbox_srid, float min_level, float max_level, Image** out_image_ptr, RangeRectd& out_bounds) noexcept;
        ErrorCode renderToValueGrid(int32_t srid, const RangeRectd& bbox, int32_t antialias_level, ValueGrid<int64_t>* out_value_grid) noexcept;
        ErrorCode exportCSV(const String& file_path) noexcept;

        ErrorCode renderMetaTiles(const String& dir_path, int32_t zoom, const RangeRectd& bbox, int32_t antialias_level, int64_t start_index = 0, int64_t end_index = std::numeric_limits<int64_t>::max()) noexcept;

        static ErrorCode renderDownsampledMetaTiles(const String& base_path, int32_t srid, int32_t src_zoom, int32_t meta_tile_size, const RangeRectd& bbox) noexcept;

        static ErrorCode imageFromRawFile(const String& raw_file_path, Image* image) noexcept;

        int32_t verboseLevel() const noexcept { return m_verbose_level; }
        void setVerboseLevel(int32_t verbose_level) noexcept { m_verbose_level = verbose_level; }

        void saveLog(const String& log_file_path) noexcept;

    private:
        ErrorCode _projectBbox() noexcept;
    };


}  // End of namespace Grain

#endif  // GrCVF2TileManager_hpp
