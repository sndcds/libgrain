//
//  CVF2TileManager.hpp
//
//  Created by Roald Christesen on 20.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainCVF2TileManager_hpp
#define GrainCVF2TileManager_hpp

#include "Grain.hpp"
#include "CVF2.hpp"
#include "CVF2File.hpp"
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
    int32_t index_ = -1;
    int32_t init_counter_ = 0;          ///< Counts the number of times the tile is initialized. More than one initialization indicates a failure
    bool valid_ = false;                ///< Flag indicating whether the tile is valid for use
    ErrorCode last_err_code_ = ErrorCode::None;     ///< Last error occurred in tile preparation

    RangeRectFix bbox_;                 ///< Bounding box as Fix values in SRID of tile manager
    RangeRectd bbox_dbl_;               ///< Bounding box as doubles in SRID of tile manager

    int32_t x_index_ = 0;               ///< Tile x index in 2d tile array
    int32_t y_index_ = 0;               ///< Tile y index in 2d tile array

    int32_t x_offs_ = 0;                ///< Offset of the tile's X position within the tile space
    int32_t y_offs_ = 0;                ///< Offset of the tile's Y position within the tile space

    uint32_t width_ = 0;                ///< Tile width in tile space
    uint32_t height_ = 0;               ///< Tile height in tile space

    int32_t undefined_values_count_ = 0;

    String file_name_;                  ///< Name of cvf2 file
    String file_path_;                  ///< Path to cvf2 file
    String raw_file_path_;              ///< Path to raw file
    bool raw_file_exist_ = false;       ///< Indicates whether the raw file exists

    int32_t cache_cvf2_file_index_ = -1;   ///< Index of the file being used, or -1 if no file is being used

    int32_t reserved_ = 0;
    Flags error_flags_{};

    ValueGridf* value_grid_ = nullptr;

public:
    explicit CVF2Tile(int32_t index) {
        index_ = index;
    }

    ~CVF2Tile() override = default;

    [[nodiscard]] const char* className() const noexcept override {
        return "CVF2Tile";
    }

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
        log << "index: " << index_ << log.endl;
        log << "init_counter: " << init_counter_ << log.endl;
        log << "valid: " << valid_ << log.endl;
        log << "bbox_: " << bbox_ << log.endl;
        log << "x_index_: " << x_index_ << ", y_index_: " << y_index_ << log.endl;
        log << "x_offset: " << x_offs_ << ", y_offset: " << y_offs_ << log.endl;
        log << "width_: " << width_ << ", height_: " << height_ << log.endl;
        log << "undefined_values_count: " << undefined_values_count_ << log.endl;
        log << "file_path: " << file_path_ << log.endl;
        log << "raw_file_path: " << raw_file_path_ << log.endl;
        log << "raw_file_exist: " << raw_file_exist_ << log.endl;
        log << "cache_cvf2_file_index: " << cache_cvf2_file_index_ << log.endl;
        log << "error_flags: " << error_flags_ << log.endl;
    }

    [[nodiscard]] bool isValid() const noexcept { return valid_; }
    [[nodiscard]] Rectd rect() const noexcept { return bbox_dbl_.rect(); }
    [[nodiscard]] RangeRectd rangeRect() const noexcept { return bbox_dbl_; }

    [[nodiscard]] bool hasUndefinedValues() const noexcept { return undefined_values_count_ > 0; }
    [[nodiscard]] int32_t undefinedValuesCount() const noexcept { return undefined_values_count_; }

    [[nodiscard]] bool matchesSize(int32_t width, int32_t height) const noexcept {
        return width == static_cast<int32_t>(width_) && height == static_cast<int32_t>(height_);
    }


    [[nodiscard]] bool cvf2FileIsOpen() const noexcept { return cache_cvf2_file_index_ >= 0; }
    void crsPosToTileXY(const Vec2d& crs_pos, Vec2i& out_xy) const noexcept {
        out_xy.x_ = static_cast<int32_t>(Math::remap(bbox_dbl_.min_x_, bbox_dbl_.max_x_, 0, width_ - 1, crs_pos.x_));
        out_xy.y_ = static_cast<int32_t>(Math::remap(bbox_dbl_.min_y_, bbox_dbl_.max_y_, 0, height_ - 1, crs_pos.y_));
    }

    [[nodiscard]] bool hasValueGrid() noexcept;
    [[nodiscard]] ErrorCode checkValueGrid(CVF2TileManager* manager) noexcept;
    [[nodiscard]] ErrorCode generateValueGrid(CVF2TileManager* manager) noexcept;
    void freeValueGrid() noexcept;


    [[nodiscard]] ValueGridf* valueGridPtr() noexcept { return value_grid_; }
};


/**
 *  @brief Handles access to an CVF2 file slot.
 *
 *  This slot represents a cache or storage unit for managing CVF2 file instances.
 *  Each time the slot accesses an CVF2 file, it updates the `timestamp_` to the
 *  current timestamp. This updated timestamp is crucial for identifying the "oldest" slot
 *  in scenarios where a new slot is needed but no empty slots are available. The oldest slot
 *  may then be overwritten or cleared based on the cache management policy.
 *
 *  @note Assumes that the CVF2 file format is a specific format used in this context.
 */
struct CVF2ManagerFileSlot {
    timestamp_t timestamp_{};
    CVF2File* file_ = nullptr;
    int64_t tile_index_ = -1;
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
    String dir_path_;                       ///< Path to the main directory of the tile manager data structure

    // Bounding box
    RangeRectd provided_bbox_;             ///< Input bounding box defining the area of interest
    int32_t provided_bbox_srid_ = 0;       ///< Spatial Reference System Identifier (SRID) of the input bounding box
    RangeRectd bbox_;                       ///< Input bounding box projected to tile manager SRID
    bool bbox_used_ = false;               ///< Flag indicating whether the bounding box (bbox_) is in use
    bool bbox_valid_ = false;              ///< Flag indicating whether the bounding box (bbox_) is valid to use

    int32_t tile_width_{};                  ///< Full width of tile
    int32_t tile_height_{};                 ///< Full height of tile
    int32_t x_tile_count_{};                ///< Number of tiles in x direction
    int32_t y_tile_count_{};                ///< Number of tiles in y direction
    int32_t tile_count_{};                  ///< Number of tiles in 2d array
    int32_t tile_count_limit_ = 10000000;   ///< Limit of tiles in 2d array
    int32_t scanned_tile_count_{};          ///< Number of scanned tiles
    size_t min_cvf2_file_size_ = 1;         ///< CVF2 files smaller will be ignored
    size_t max_cvf2_file_size_ = Type::gigabytesToBytes(2);  ///< CVF2 files greater will be ignored

    ObjectList<CVF2Tile*> tiles;
    List<CVF2Tile*> started_tile_list_;

    bool scan_done_ = false;
    bool running_ = false;

    timestamp_t scan_ts1_{};
    timestamp_t scan_ts2_{};
    RangeRectFix scan_xy_range_;
    RangeRectd scan_xy_range_dbl_;
    int64_t scan_total_min_{};
    int64_t scan_total_max_{};
    int64_t scan_total_undefined_values_n_{};
    int32_t scan_files_n_{};
    int32_t scan_files_ignored_n_{};
    int32_t scan_file_err_count_{};
    int32_t scan_incomplete_files_n_{};
    int32_t scan_wrong_dimension_files_n_{};

    timestamp_t start_ts1_{};
    timestamp_t start_ts2_{};
    int32_t start_tile_multi_inititialized_n_{};    ///< Counts how many tiles where initialized more than one time
    int32_t start_error_n_{};                       ///< Counts how many tile errors happened when starting
    int32_t start_file_err_count_{};

    int64_t cvf2_file_open_n_{};                    ///< Counts how often a cfv2 file has been opened
    int64_t cvf2_file_close_n_{};                   ///< Counts how often a cfv2 file has been closed
    int64_t cvf2_file_open_failed_n_{};             ///< Counts how often a cfv2 file open failed

    int32_t file_slot_capacity_ = 10;               ///< Maximum number of open files
    CVF2ManagerFileSlot* file_slots_ = nullptr;     ///< Array with file slots

    int32_t tile_srid_ = 0;                         ///< Spatial Reference System Identifier (SRID) of tiles
    GeoProj wgs84_to_tile_proj_;
    ErrorCode last_read_err_ = ErrorCode::Unknown;

    bool cache_tile_flag_ = false;
    int32_t verbose_level_ = 0;


public:
    CVF2TileManager(const String& dir_path, int32_t tile_width, int32_t tile_height, int32_t open_files_capacity);
    ~CVF2TileManager() override;

    [[nodiscard]] const char* className() const noexcept override {
        return "CVF2TileManager";
    }

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
        log << "scan_file_err_count: " << scan_file_err_count_ << log.endl;
        log << "scan_xy_range: " << scan_xy_range_ << log.endl;
        log << "scan_xy_range_dbl.width(): " << scan_xy_range_dbl_.width() << log.endl;
        log << "tile_width: " << tile_width_ << log.endl;
        log << "scan_xy_range_dbl.height(): " << scan_xy_range_dbl_.height() << log.endl;
        log << "tile_height: " << tile_height_ << log.endl;
        log << "x_tile_count: " << x_tile_count_ << log.endl;
        log << "y_tile_count: " << y_tile_count_ << log.endl;
        log << "tile_count: " << tile_count_ << log.endl;
        log << "started_tile_list size: " << started_tile_list_.size() << log.endl;
        log << "scan_total_min: " << scan_total_min_ << log.endl;
        log << "scan_total_max: " << scan_total_max_ << log.endl;
        log << "cache_tile_flag: " << cache_tile_flag_ << log.endl;
        log << "total width: " << totalWidth() << log.endl;
        log << "total height: " << totalHeight() << log.endl;
        log << "total covered values: " << totalCoveredValues() << log.endl;
    }

    [[nodiscard]] int32_t tileSRID() const noexcept { return tile_srid_; }
    [[nodiscard]] int32_t tileWidth() const noexcept { return tile_width_; }
    [[nodiscard]] int32_t tileHeight() const noexcept { return tile_height_; }
    [[nodiscard]] int32_t totalWidth() const noexcept { return x_tile_count_ * tile_width_; }
    [[nodiscard]] int32_t totalHeight() const noexcept { return y_tile_count_ * tile_height_; }
    [[nodiscard]] int32_t totalCoveredValues() const noexcept {
        return x_tile_count_ * tile_width_ * y_tile_count_ * tile_height_;
    }


    void setTileSRID(int32_t srid) noexcept { tile_srid_ = srid; }
    void setTileCountLimit(int32_t limit) noexcept { tile_count_limit_ = limit; }

    void enableTileCache() noexcept { cache_tile_flag_ = true; }
    void disableTileCache() noexcept { cache_tile_flag_ = false; }
    [[nodiscard]] bool useTileCache() const noexcept { return cache_tile_flag_; }


    ErrorCode scan(const RangeRectd& bbox, int32_t bbox_srid) noexcept;
    ErrorCode scan() noexcept;
    void _scanFile(const String& dir_path, const String& file_name);

    ErrorCode start() noexcept;
    void _startFile(const String& dir_path, const String& file_name);

    [[nodiscard]] int32_t tileCount() const noexcept { return tile_count_; }

    int64_t tileIndexAtTileManagerPos(const Vec2d& pos, Vec2i& out_tile_xy_index) noexcept;
    int64_t tileIndexAtLonlat(const Vec2d& lonlat, Vec2i& out_tile_xy_index) noexcept;
    CVF2Tile* tileAtIndex(int64_t index) noexcept;

    List<CVF2Tile*>*startedTileList() noexcept { return &started_tile_list_; }

    int64_t valueAtWGS84Pos(const Vec2d& lonlat) noexcept;
    int64_t valueAtPos(const Vec2d& pos) noexcept;
    double doubleAtPos(const Vec2d& pos) noexcept {
        int64_t value = valueAtPos(pos);
        return value == CVF2::kUndefinedValue ? NAN : value;
    }


    [[nodiscard]] bool hasReadError() const noexcept { return last_read_err_ != ErrorCode::None; }
    [[nodiscard]] ErrorCode lastReadError() const noexcept { return last_read_err_; }
    void clearReadError() noexcept { last_read_err_ = ErrorCode::None; }


    [[nodiscard]] CVF2File* cvf2FileForTile(CVF2Tile* tile) noexcept;

    ErrorCode generateRawTiles() noexcept;

    ErrorCode collectImage(const RangeRectd& bbox, int32_t bbox_srid, float min_level, float max_level, Image** out_image_ptr, RangeRectd& out_bounds) noexcept;
    ErrorCode renderToValueGrid(int32_t srid, const RangeRectd& bbox, int32_t antialias_level, ValueGrid<int64_t>* out_value_grid) noexcept;
    ErrorCode exportCSV(const String& file_path) noexcept;

    ErrorCode renderMetaTiles(const String& dir_path, int32_t zoom, const RangeRectd& bbox, int32_t antialias_level, int64_t start_index = 0, int64_t end_index = std::numeric_limits<int64_t>::max()) noexcept;

    static ErrorCode renderDownsampledMetaTiles(const String& base_path, int32_t srid, int32_t src_zoom, int32_t meta_tile_size, const RangeRectd& bbox) noexcept;

    static ErrorCode imageFromRawFile(const String& raw_file_path, Image* image) noexcept;

    [[nodiscard]] int32_t verboseLevel() const noexcept { return verbose_level_; }
    void setVerboseLevel(int32_t verbose_level) noexcept { verbose_level_ = verbose_level; }

    void saveLog(const String& log_file_path) noexcept;

private:
    ErrorCode _projectBbox() noexcept;
};


}  // End of namespace Grain

#endif  // GrCVF2TileManager_hpp
