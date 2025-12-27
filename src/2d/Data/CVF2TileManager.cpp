//
//  CVF2TileManager.cpp
//
//  Created by Roald Christesen on 20.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Data/CVF2TileManager.hpp"
#include "Type/List.hpp"
#include "String/StringList.hpp"
#include "Image/Image.hpp"
#include "2d/Data/ValueGrid.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Geo/Geo.hpp"
#include "Geo/GeoMetaTile.hpp"
#include "App/App.hpp"


namespace Grain {

    /**
     *  @brief Checks if a ValueGrid already exists for the specified tile.
     *
     *  @return true if a ValueGrid exists for the tile; otherwise, false.
     */
    bool CVF2Tile::hasValueGrid() noexcept {
        return value_grid_ != nullptr;
    }


    /**
     *  @brief Ensures that a ValueGrid exists for the tile.
     *         If it does not exist or has an incorrect size, a new ValueGrid will be created.
     *
     *  @param manager A pointer to the CVF2TileManager instance, which provides the tile dimensions.
     *  @return ErrorCode::None if the ValueGrid is correctly created or reused; an appropriate error code otherwise.
     */
    ErrorCode CVF2Tile::checkValueGrid(CVF2TileManager* manager) noexcept {

        if (!isValid()) {
            return Error::specific(CVF2TileManager::kErrTileIsInvalid);
        }

        if (!manager) {
            // Requires valid Tile Manager information
            return ErrorCode::NullData;
        }

        int32_t w = manager->tile_width_;
        int32_t h = manager->tile_height_;

        if (value_grid_) {
            if (value_grid_->width() == w && value_grid_->height() == h) {
                // ValueGrid has the correct size and can be reused
                return ErrorCode::None;
            }
            else {
                // Free existing ValueGrid to allow new allocation
                freeValueGrid();
            }
        }

        // Attempt to allocate a new ValueGrid with the required dimensions
        value_grid_ = new (std::nothrow) ValueGridf(w, h);
        if (value_grid_) {
            value_grid_->setInvalidValueDefault();
            return ErrorCode::None;
        }
        else {
            return ErrorCode::MemCantAllocate;
        }
    }


    /**
     *  @brief Generates a value grid for the CVF2 tile.
     *
     *  This function creates and populates a value grid specific to the CVF2 tile,
     *  leveraging the provided CVF2Manager instance for context or resources.
     *
     *  @param manager A pointer to the CVF2TileManager responsible for managing CVF2 tiles.
     *                 Must not be nullptr.
     *  @return ErrorCode::None if the grid was generated successfully, or an appropriate
     *          error code indicating the type of failure.
     */
    ErrorCode CVF2Tile::generateValueGrid(CVF2TileManager* manager) noexcept {

        auto result = ErrorCode::None;

        try {
            if (!isValid()) { throw Error::specific(CVF2TileManager::kErrTileIsInvalid); }

            auto err = checkValueGrid(manager);
            if (err != ErrorCode::None) { throw err; }

            value_grid_->invalidate();

            // Read data from CVF2File

            CVF2File cvf2_file(file_path_);
            cvf2_file.startRead();

            for (int32_t y = 0; y < static_cast<int32_t>(cvf2_file.height()); y++) {
                cvf2_file.readRow(y);
                int64_t* src = cvf2_file.row_values_;
                float* dst = value_grid_->mutPtrAtXY(x_offs_, y + y_offs_);
                if (!src || !dst) {
                    throw ErrorCode::Fatal;
                }
                for (int32_t x = 0; x < static_cast<int32_t>(cvf2_file.width()); x++) {
                    *dst++ = src[x];
                }
            }


            // Update value grid information

            value_grid_->setXIndex(x_index_);
            value_grid_->setYIndex(y_index_);
            value_grid_->updateMinMax();
            value_grid_->setGeoInfo(manager->tile_srid_, bbox_.minX(), bbox_.minY(), bbox_.maxX(), bbox_.maxY());
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        return result;
    }


    /**
     *  @brief Releases the memory allocated for the ValueGrid, if any.
     *
     *  This function deletes the existing ValueGrid associated with the tile, if it exists,
     *  and sets the pointer to nullptr to avoid dangling references.
     */
    void CVF2Tile::freeValueGrid() noexcept {

        if (value_grid_) {
            delete value_grid_;
            value_grid_ = nullptr;
        }
    }


    /**
     *  @brief Constructs a CVF2TileManager with specified directory path, tile dimensions, and file slot capacity.
     *
     *  Initializes a CVF2TileManager object with the provided directory path, tile width, tile height,
     *  and a capacity for open file slots. The `open_files_capacity` is set to a minimum of 16 if a
     *  smaller value is provided.
     *
     *  @param dir_path The path to the directory where CVF2 files with data will be located.
     *  @param tile_width The width of each tile, in pixels or units.
     *  @param tile_height The height of each tile, in pixels or units.
     *  @param open_files_capacity The maximum number of files that can be open at once;
     *                             a minimum value of 16 is enforced.
     */
    CVF2TileManager::CVF2TileManager(const String& dir_path, int32_t tile_width, int32_t tile_height, int32_t open_files_capacity) {

        dir_path_ = dir_path;
        tile_width_ = tile_width;
        tile_height_ = tile_height;
        file_slot_capacity_ = std::max(open_files_capacity, 16);
        file_slots_ = nullptr;
    }


    CVF2TileManager::~CVF2TileManager() {

        if (file_slots_) {
            for (int32_t i = 0; i < file_slot_capacity_; i++) {
                if (file_slots_[i].file_) {
                    delete file_slots_[i].file_;
                }
            }

            std::free(file_slots_);
        }
    }


    /**
     * @brief Scans the directory for CVF2 files and determines tile placement ranges.
     *
     * This method scans the specified directory for all CVF2 files and sets up the manager
     * for efficient access to these files. Additionally, it computes the total range for
     * the x and y axes, indicating where all tiles can be placed within the tile manager.
     *
     * @return ErrorCode::None if the scan completes successfully; an appropriate error code
     *         otherwise, indicating the nature of any failure.
     */
    ErrorCode CVF2TileManager::scan(const RangeRectd& bbox, int32_t bbox_srid) noexcept {

        provided_bbox_ = bbox;
        provided_bbox_srid_ = bbox_srid;
        bbox_used_ = true;

        // If `tile_srid_` has been set in advance the bounding box can be projected to `tile_srid_`
        if (tile_srid_ != 0) {
            wgs84_to_tile_proj_.setSrcSRID(4326);
            wgs84_to_tile_proj_.setDstSRID(tile_srid_);
            auto err = _projectBbox();
            if (err != ErrorCode::None) {
                return err;
            }
        }

        return scan();
    }


    ErrorCode CVF2TileManager::scan() noexcept {

        auto result = ErrorCode::None;

        if (scan_done_) {
            return ErrorCode::None;
        }

        // Time measuring
        scan_ts1_ = Timestamp::currentMillis();

        // Init results
        scan_xy_range_.initForMinMaxSearch();
        scan_total_min_ = std::numeric_limits<int64_t>::max();
        scan_total_max_ = std::numeric_limits<int64_t>::min();
        scan_files_n_ = 0;
        scan_files_ignored_n_ = 0;
        scan_total_undefined_values_n_ = 0;
        scan_file_err_count_ = 0;
        scan_incomplete_files_n_ = 0;
        scan_wrong_dimension_files_n_ = 0;
        start_tile_multi_inititialized_n_ = 0;
        start_error_n_ = 0;

        Log log;

        StringList* file_list = nullptr;

        try {

            // Build list of all CVF2 files, which exists in the directory "cvf2"

            file_list = new (std::nothrow) StringList();
            if (!file_list) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            scan_files_n_ = File::fileNameList(dir_path_, "cvf", min_cvf2_file_size_, max_cvf2_file_size_, &scan_files_ignored_n_, *file_list);
            if (scan_files_n_ < 0) { throw Error::specific(kErrNoCVF2FilesInDir); }

            log << "scan_files_n: " << scan_files_n_ << std::endl;

            // Go through all files in list
            int32_t index = 0;
            scan_file_err_count_ = 0;
            for (const auto file_name : *file_list) {
                if (verbose_level_ > 0 && (index % 1000) == 0) {
                    log << "CVF2TileManager::scan(): " << index << " of " << scan_files_n_ << std::endl;
                }
                try {
                    _scanFile(dir_path_, *file_name);
                }
                catch (ErrorCode err) {
                    scan_file_err_count_++;
                }
                index++;
            }

            scan_xy_range_dbl_ = scan_xy_range_;
            if (scan_xy_range_dbl_.width() < std::numeric_limits<float>::epsilon() || scan_xy_range_dbl_.height() < std::numeric_limits<float>::epsilon()) {
                throw Error::specific(kErrRangeNotValid);
            }

            x_tile_count_ = static_cast<int32_t>(floor(scan_xy_range_dbl_.width() / tile_width_)) + 1;
            y_tile_count_ = static_cast<int32_t>(floor(scan_xy_range_dbl_.height() / tile_height_)) + 1;
            tile_count_ = x_tile_count_ * y_tile_count_;

            if (tile_count_ > tile_count_limit_) { throw Error::specific(kErrToManyTilesFound); }

            scan_ts2_ = Timestamp::currentMillis();
            scan_done_ = true;
        }
        catch (ErrorCode err) {
            result = err;
        }

        delete file_list;

        return result;
    }


    void CVF2TileManager::_scanFile(const String& dir_path, const String& file_name) {

        String file_path = dir_path + "/" + file_name;

        auto file = new (std::nothrow) CVF2File(file_path);
        if (!file) { throw ErrorCode::ClassInstantiationFailed; }

        file->startRead();

        if (tile_srid_ == 0) {
            tile_srid_ = file->srid();
            wgs84_to_tile_proj_.setSrcSRID(4326);
            wgs84_to_tile_proj_.setDstSRID(tile_srid_);

            auto err = _projectBbox();
            if (err != ErrorCode::None) { throw err; }
        }
        else {
            if (tile_srid_ != file->srid()) {
                throw Error::specific(kErrTileCrsMismatch);
            }
        }

        if (!bbox_used_ || file->hitBbox(bbox_)) {

            // Extend the total range covered by the tiles
            scan_xy_range_.add(file->range());

            // Update min and max values
            if (file->minValue() < scan_total_min_) {
                scan_total_min_ = file->minValue();
            }

            if (file->maxValue() > scan_total_max_) {
                scan_total_max_ = file->maxValue();
            }

            // Update statistical values
            if (file->undefinedValuesCount() > 0) {
                scan_incomplete_files_n_++;
                scan_total_undefined_values_n_ += file->undefinedValuesCount();
            }

            if (tile_width_ != static_cast<int32_t>(file->width()) ||
                tile_height_ != static_cast<int32_t>(file->height())) {
                scan_wrong_dimension_files_n_++;
            }

            scanned_tile_count_++;
        }

        file->close();
        delete file;
    }


    ErrorCode CVF2TileManager::start() noexcept {
        auto result = ErrorCode::None;

        if (running_) {
            return ErrorCode::None;
        }


        StringList* file_list = nullptr;

        try {
            if (!scan_done_) { throw Error::specific(kErrNotScanned); }
            if (scan_files_n_ < 1) { throw Error::specific(kErrNoCVF2FilesInDir); }
            if (tile_count_ < 1) { throw Error::specific(kErrNoTiles); }

            start_ts1_ = Timestamp::currentMillis();

            // Allocate file slots
            file_slots_ = (CVF2ManagerFileSlot*)calloc(file_slot_capacity_, sizeof(CVF2ManagerFileSlot));
            if (!file_slots_) {
                throw ErrorCode::MemCantAllocate;
            }

            // Initialize file slots
            for (int32_t i = 0; i < file_slot_capacity_; i++) {
                file_slots_[i].tile_index_ = -1;
                file_slots_[i].timestamp_ = std::numeric_limits<int64_t>::min();
                file_slots_[i].file_ = nullptr;
            }

            cvf2_file_open_n_ = 0;
            cvf2_file_close_n_ = 0;
            cvf2_file_open_failed_n_ = 0;

            // Prepare tiles memory
            tiles.reserve(tile_count_);
            int32_t tile_index = 0;
            for (int32_t y = 0; y < y_tile_count_; y++) {
                for (int32_t x = 0; x < x_tile_count_; x++) {
                    auto tile = new (std::nothrow) CVF2Tile(tile_index);
                    if (!tile) { throw Error::specific(kErrTileTileInstantiationFailed); }
                    tiles.push(tile);
                    tile->x_index_ = x;
                    tile->y_index_ = y;
                    tile_index++;
                }
            }

            if (tiles.size() != tile_count_) {
                throw Error::specific(kErrTileListInitFailed);
            }

            started_tile_list_.reserve(scanned_tile_count_);
            if (started_tile_list_.capacity() != scanned_tile_count_) {
                throw ErrorCode::MemCantAllocate;
            }

            // Build tiles
            file_list = new (std::nothrow) StringList();
            if (!file_list) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            scan_files_n_ = File::fileNameList(dir_path_, "cvf", min_cvf2_file_size_, max_cvf2_file_size_, &scan_files_ignored_n_, *file_list);


            // Iterate over all files in list
            start_file_err_count_ = 0;
            for (const auto file_name : *file_list) {
                try {
                    _startFile(dir_path_, *file_name);
                }
                catch (ErrorCode err) {
                    start_file_err_count_++;
                    throw err;
                }
            }

            start_ts2_ = Timestamp::currentMillis();
            running_ = true;
        }
        catch (ErrorCode err) {
            result = err;
        }

        delete file_list;

        return result;
    }


    void CVF2TileManager::_startFile(const String& dir_path, const String& file_name) {
        String file_path = dir_path + "/" + file_name;

        auto file = new (std::nothrow) CVF2File(file_path);
        if (!file) { throw ErrorCode::ClassInstantiationFailed; }

        file->startRead();

        if (!bbox_used_ || file->hitBbox(bbox_)) {

            Vec2i tile_xy_index;
            Vec2d center = file->centerAsVec2d();
            auto tile_index = tileIndexAtTileManagerPos(center, tile_xy_index);

            if (tile_index < 0) {
                throw Error::specific(kErrTileIndexOutOfRange);
            }

            auto tile = tileAtIndex(tile_index);
            if (!tile) {
                throw Error::specific(kErrTileIsNull);
            }


            // Remember pointer to tile in `started_tile_list_`
            bool success = started_tile_list_.push(tile);
            if (!success) {
                throw ErrorCode::Fatal;
            }


            if (tile->init_counter_ > 0) {
                tile->init_counter_++;
                start_tile_multi_inititialized_n_++;
                start_error_n_++;
            }
            else {
                tile->init_counter_++;
                tile->valid_ = true;

                tile->bbox_.min_x_ = file->minX();
                tile->bbox_.max_x_ = file->maxX();
                tile->bbox_.min_y_ = file->minY();
                tile->bbox_.max_y_ = file->maxY();

                tile->bbox_dbl_.min_x_ = file->minX().asDouble();
                tile->bbox_dbl_.max_x_ = file->maxX().asDouble();
                tile->bbox_dbl_.min_y_ = file->minY().asDouble();
                tile->bbox_dbl_.max_y_ = file->maxY().asDouble();

                /* TODO: Should these be handled as errors?
                if (tile_xy_index.x_ != tile->x_index_) {
                }
                if (tile_xy_index.m_y != tile->y_index_) {
                }
                 */

                Fix tile_x_offset = file->minX() - scan_xy_range_.minX() - (tile_xy_index.x_ * tile_width_);
                Fix tile_y_offset = file->minY() - scan_xy_range_.minY() - (tile_xy_index.y_ * tile_height_);

                tile->x_offs_ = tile_x_offset.asInt32();
                tile->y_offs_ = tile_y_offset.asInt32();

                tile->width_ = file->width();
                tile->height_ = file->height();
                tile->undefined_values_count_ = file->undefinedValuesCount();

                tile->file_name_ = file_name;
                tile->file_path_ = file_path;


                // Set file handle to be undefined
                tile->cache_cvf2_file_index_ = -1;

                tile->error_flags_.clear();

                if (tile->x_offs_ < 0 || tile->y_offs_ < 0) {
                    tile->valid_ = false;
                    tile->last_err_code_ = Error::specific(kErrTileOffsetOutOfRange);
                    tile->error_flags_.setFlag(0);
                }

                if (static_cast<int32_t>(tile->width_) > tile_width_ ||
                    static_cast<int32_t>(tile->height_) > tile_height_) {
                    tile->valid_ = false;
                    tile->last_err_code_ = Error::specific(kErrTileSizeOutOfRange);
                    tile->error_flags_.setFlag(1);
                }

                if (tile->x_offs_ + static_cast<int32_t>(tile->width_) > tile_width_ ||
                    tile->y_offs_ + static_cast<int32_t>(tile->height_) > tile_height_) {
                    tile->valid_ = false;
                    tile->last_err_code_ = Error::specific(kErrTileOffsetOutOfRange);
                    tile->error_flags_.setFlag(2);
                }

                if (tile_x_offset.isFloat() || tile_y_offset.isFloat()) {
                    tile->error_flags_.setFlag(3);
                }
            }
        }

        file->close();
        delete file;
    }


    int64_t CVF2TileManager::valueAtWGS84Pos(const Vec2d& lonlat) noexcept {
        Vec2d pos;
        wgs84_to_tile_proj_.transform(lonlat, pos);
        return valueAtPos(pos);
    }


    /**
     *  @brief Get a value for a position.
     *
     *  @return The value at `pos` or `CVF2::kUndefinedValue`, if no value exists.
     */
    int64_t CVF2TileManager::valueAtPos(const Vec2d& pos) noexcept {
        Vec2i tile_xy_index;
        int64_t tile_index = tileIndexAtTileManagerPos(pos, tile_xy_index);
        if (tile_index < 0) {
            return CVF2::kUndefinedValue;
        }
        std::cout << "CVF2TileManager::valueAtPos tile_index: " << tile_index << std::endl;
        std::cout << "CVF2TileManager::valueAtPos tile_xy_index: " << tile_xy_index << std::endl;

        auto tile = tileAtIndex(tile_index);
        if (!tile) {
            return CVF2::kUndefinedValue;
        }
        std::cout << tile << std::endl;

        if (!tile->valid_) {
            return CVF2::kUndefinedValue;
        }

        auto cvf2_file = cvf2FileForTile(tile);
        if (!cvf2_file) {
            return CVF2::kUndefinedValue;
        }
        std::cout << cvf2_file << std::endl;

        Vec2i tile_xy;  // Position in Tile space
        tile->crsPosToTileXY(pos, tile_xy);
        std::cout << "tile_xy: " << tile_xy << std::endl;
        int64_t value = cvf2_file->valueAtPos(tile_xy, cache_tile_flag_);

        return value;
    }


    /**
     *  @brief Retrieves the tile index for a given position.
     *
     *  @param pos The position (in the coordinate system of the tile manager) for which to find the corresponding tile.
     *  @param[out] out_tile_xy_index The x and y indices of the tile in the 2D tile map.
     *  @return The absolute tile index, or -1 if the position is println of bounds.
     */
    int64_t CVF2TileManager::tileIndexAtTileManagerPos(const Vec2d& pos, Vec2i& out_tile_xy_index) noexcept {

        int32_t xi = static_cast<int32_t>((pos.x_ - scan_xy_range_dbl_.min_x_) / tile_width_);
        int32_t yi = static_cast<int32_t>((pos.y_ - scan_xy_range_dbl_.min_y_) / tile_height_);

        if (xi >= 0 && xi < x_tile_count_ && yi >= 0 && yi < y_tile_count_) {
            out_tile_xy_index.x_ = xi;
            out_tile_xy_index.y_ = yi;
            return static_cast<int64_t>(yi) * x_tile_count_ + xi;
        }
        else {
            out_tile_xy_index.x_ = -1;
            out_tile_xy_index.y_ = -1;
            return -1;
        }
    }


    /**
     *  @brief Retrieves the tile index for a given lon/lat position.
     *
     *  @param lonlat The lon/lat position for which to find the corresponding tile in SRID 4326.
     *  @param[out] out_tile_xy_index The x and y indices of the tile in the 2D tile map.
     *  @return The absolute tile index, or -1 if the position is println of bounds.
     */
    int64_t CVF2TileManager::tileIndexAtLonlat(const Vec2d& lonlat, Vec2i& out_tile_xy_index) noexcept {

        if (tile_srid_ == 0) {
            out_tile_xy_index.x_ = -1;
            out_tile_xy_index.y_ = -1;
            return -1;
        }
        else {
            Vec2d xy;
            wgs84_to_tile_proj_.transform(lonlat, xy);
            return tileIndexAtTileManagerPos(xy, out_tile_xy_index);
        }
    }


    CVF2Tile* CVF2TileManager::tileAtIndex(int64_t index) noexcept {

        return tiles.elementAtIndex(index);
    }


    CVF2File* CVF2TileManager::cvf2FileForTile(CVF2Tile* tile) noexcept {

        try {
            if (!tile) {
                throw ErrorCode::NullData;
            }

            if (tile->cache_cvf2_file_index_ >= 0) {

                // CVF2 file is open and in cache
                // Set new timestamp and return pointer to file

                auto slot = &file_slots_[tile->cache_cvf2_file_index_];
                slot->timestamp_ = Timestamp::currentMillis();
                return slot->file_;
            }
            else {

                // CVF2 file is not open and not in cache
                // Find open slot or slot with the oldest open file

                auto ts = Timestamp::currentMillis();
                timestamp_t max_delta = std::numeric_limits<int64_t>::min();

                // bool free_slot_flag = false; // Will be true, if a free slot was found, unused

                int32_t slot_index = -1; // Will become the slot index to use
                for (int32_t i = 0; i < file_slot_capacity_; i++) {
                    auto slot = &file_slots_[i];
                    if (slot->tile_index_ < 0) { // This is a free slot
                        slot_index = i;
                        // free_slot_flag = true; // Unused
                        break;
                    }
                    else {
                        timestamp_t ts_delta = ts - slot->timestamp_;
                        if (ts_delta > max_delta) {
                            max_delta = ts_delta;
                            slot_index = i;
                        }
                    }
                }

                if (slot_index >= 0) {

                    // The slot to be used is found
                    auto slot = &file_slots_[slot_index];
                    if (slot->file_) {
                        // Current CVF2 file must be closed and deleted first
                        slot->file_->close();

                        auto t = tileAtIndex(slot->tile_index_);
                        if (t) {
                            t->cache_cvf2_file_index_ = -1;
                        }

                        delete slot->file_;

                        slot->file_ = nullptr;
                        slot->tile_index_ = -1;
                        cvf2_file_close_n_++;
                    }

                    slot->file_ = new (std::nothrow) CVF2File(tile->file_path_);

                    if (slot->file_) {
                        slot->file_->startRead();
                        slot->tile_index_ = tile->index_;
                        slot->timestamp_ = Timestamp::currentMillis();
                        tile->cache_cvf2_file_index_ = slot_index;
                        cvf2_file_open_n_++;

                        return slot->file_;
                    }
                    else {
                        cvf2_file_open_failed_n_++;
                    }
                }
            }
        }
        catch (ErrorCode err) {
        }

        return nullptr;
    }


    /**
     *  TODO: Documentation
     */
    ErrorCode CVF2TileManager::generateRawTiles() noexcept {
        auto result = ErrorCode::None;

        try {
            if (!running_) {
                throw Error::specific(kErrTileManagerNotRunning);
            }

            String raw_dir_path = dir_path_ + "/raw/1";
            File::makeDirs(raw_dir_path);
            if (File::isDir(raw_dir_path) != true) {
                throw ErrorCode::FileDirNotFound;
            }

            for (const auto tile : tiles) {

                if (tile->isValid()) {

                    auto err = tile->generateValueGrid(this);
                    if (err != ErrorCode::None) { throw err; }

                    auto value_grid = tile->value_grid_;

                    String file_path;
                    file_path.setFormatted(1000, "%s/%d_%d.vgr", raw_dir_path.utf8(), tile->y_index_, tile->x_index_);
                    value_grid->writeFile(file_path);

                    tile->freeValueGrid();
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    ErrorCode CVF2TileManager::imageFromRawFile(const String& raw_file_path, Image* image) noexcept {

        auto result = ErrorCode::None;

        File* raw_file = nullptr;

        try {
            if (!image) {
                throw ErrorCode::NullData;
            }

            raw_file = new (std::nothrow) File(raw_file_path);
            if (!raw_file) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            raw_file->startRead();


            char buffer[8];
            raw_file->readStr(8, buffer);
            raw_file->checkSignature(buffer, 8, "GeoVTile");

            raw_file->readStr(2, buffer);
            raw_file->setEndianBySignature(buffer);

            // Version
            // uint16_t main_version = raw_file->readValue<uint16_t>(); // Unused
            // uint16_t sub_version = raw_file->readValue<uint16_t>(); // Unused

            // Data type
            uint16_t data_type = raw_file->readValue<uint16_t>();

            if (data_type != Type::kType_Float) {
                // TODO: Support other data types!
                throw ErrorCode::UnsupportedDataType;
            }

            // Dimension and position inside 2d tile array
            uint32_t image_width = raw_file->readValue<uint32_t>();
            uint32_t image_height = raw_file->readValue<uint32_t>();

            if (image_width < 1 || image_height < 0) {
                throw ErrorCode::UnsupportedDimension;
            }

            if (static_cast<int32_t>(image_width) != image->width() ||
                static_cast<int32_t>(image_height) != image->height()) {
                throw ErrorCode::UnsupportedDimension;
            }

            // int32_t tile_x_index = raw_file->readValue<int32_t>(); // Unused
            // int32_t tile_y_index = raw_file->readValue<int32_t>(); // Unused

            // Geo information
            String tile_crs;
            raw_file->readToString(16, tile_crs);

            RangeRectFix tile_crs_range;
            raw_file->readFix(tile_crs_range.min_x_);
            raw_file->readFix(tile_crs_range.min_y_);
            raw_file->readFix(tile_crs_range.max_x_);
            raw_file->readFix(tile_crs_range.max_y_);

            // Copy pixel data
            float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia(image, pixel);

            // TODO: Support other data types.

            while (ia.stepY()) {
                while (ia.stepX()) {
                    float v = raw_file->readValue<float>();
                    if (v < 0.0f) {
                        v = 0.0f;
                    }
                    pixel[0] = pixel[1] = pixel[2] = v / 15000;
                    ia.write();
                }
            }

            raw_file->close();
            delete raw_file;
            raw_file = nullptr;
        }
        catch (ErrorCode err) {
            result = err;
        }


        // Cleanup
        delete raw_file;

        return result;
    }


    /**
     *  Collects all CVF2 tiles that intersect the specified `bbox` and combines them into a grayscale image.
     *
     *  @param bbox A rectangular bounding box (`RangeRectd`) defining the area of interest.
     *  @param bbox_srid An integer SRID representing the spatial reference ID in which `bbox` are provided (e.g., EPSG code).
     *  @param min_level Minimum value which represents black in the resulting greyscale image.
     *  @param max_level Minimum value which represents white in the resulting greyscale image.
     *  @param[out] out_image_ptr A pointer to a pointer to an `Image` object. Upon
     *              successful completion, this will point to the generated image
     *              representing the tiles within `bbox`. The caller is responsible
     *              for ensuring `out_image_ptr` is valid and for deallocating the
     *              `Image` object when no longer needed.
     *  @param[out] out_bounds The bounds of the generated image in the SRID of the
     *              tiles.
     *
     *  @return An error code of type `ErrorCode`. Returns `ErrorCode::None` upon
     *          successful completion, or an appropriate error code if an error occurs
     *          during processing (e.g., `ErrorCode::InvalidBounds`,
     *          `ErrorCode::SRIDUnsupported`).
     */
    ErrorCode CVF2TileManager::collectImage(const RangeRectd& bbox, int32_t bbox_srid, float min_level, float max_level, Image** out_image_ptr, RangeRectd& out_bounds) noexcept {


        // TODO: Check! Needed? Drop?

        auto result = ErrorCode::None;

        try {
            if (!out_image_ptr) {
                throw ErrorCode::NullData;
            }

            GeoProj proj;
            proj.setSrcSRID(tile_srid_);
            proj.setDstSRID(bbox_srid);

            RangeRectd total_bounds;
            total_bounds.initForMinMaxSearch();

            ObjectList<CVF2Tile*> tiles_involved;

            for (auto tile : tiles) {
                if (tile->valid_) {
                    Quadrilateral quadrilateral(tile->bbox_dbl_);
                    proj.transform(quadrilateral);
                    auto tile_aabb = quadrilateral.axisAlignedBbox();
                    if (tile_aabb.overlaps(bbox)) {
                        total_bounds.add(tile->bbox_dbl_);
                        tiles_involved.push(tile);
                    }
                }
            }

            if (tiles_involved.size() <= 0) {
                throw Error::specific(kErrNoTilesInvolved);
            }

            int32_t image_width = static_cast<int32_t>(round(total_bounds.width())) + 1;
            int32_t image_height = static_cast<int32_t>(round(total_bounds.height())) + 1;


            auto image = Image::createRGBAFloat(image_width, image_height);
            if (!image) {
                throw Error::specific(1);
            }

            for (auto tile : tiles_involved) {

                auto cvf2_file = cvf2FileForTile(tile);

                if (!cvf2_file) {
                    throw Error::specific(kErrTileFileNotPresent);
                }

                Image* tile_image = nullptr;
                ErrorCode err = cvf2_file->buildImage(CVF2File::ImageScaleMode::MinMax, min_level, max_level, &tile_image, false);
                if (err != ErrorCode::None) { throw err; }

                if (!tile_image) { throw ErrorCode::Fatal; }

                Rectd tile_image_rect(tile->bbox_dbl_.min_x_ - total_bounds.min_x_,
                                      (total_bounds.height() + 1) - (tile->bbox_dbl_.min_y_ - total_bounds.min_y_) - (tile->bbox_dbl_.height() + 1),
                                      tile->bbox_dbl_.width() + 1,
                                      tile->bbox_dbl_.height() + 1);

                err = image->drawImage(tile_image, tile_image_rect);

                delete tile_image;
            }

            *out_image_ptr = image;
            out_bounds = total_bounds;
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    /**
     *  @brief Render a projected region defined by `srid` and `bbox` to a Value Grid.
     *
     *  Projects a geographic region, specified by `bbox`, into a coordinate system
     *  defined by `srid` and renders it to the provided `ValueGrid`.
     *
     *  @param srid The SRID for the target projection. For example, use 3857 for Web Mercator.
     *  @param bbox The geographic bounding box of the region in SRID 4326 (WGS 84)
     *         coordinates.
     *  @param antialias_level The antialiasing level, ranging from 1 to 16. Values
     *         outside this range will be clamped to the nearest valid value.
     *  @param out_value_grid Pointer to the preallocated `ValueGrid<int64_t>`
     *         where computed values will be stored.
     *  @return `ErrorCode` indicating the success or failure of the rendering process.
     */
    ErrorCode CVF2TileManager::renderToValueGrid(int32_t srid, const RangeRectd& bbox, int32_t antialias_level, ValueGrid<int64_t>* out_value_grid) noexcept {

        auto result = ErrorCode::None;


        try {
            if (!out_value_grid) {
                throw ErrorCode::NullData;
            }

            if (!out_value_grid->hasValues()) {
                throw Error::specific(1);  // TODO: Error Code!
            }

            antialias_level = std::clamp<int32_t>(antialias_level, 1, 16);

            GeoProj proj_wgs84_to_dst;
            proj_wgs84_to_dst.setSrcSRID(4326);
            proj_wgs84_to_dst.setDstSRID(srid);
            if (!proj_wgs84_to_dst.isValid()) {
                // TODO: Save error message!
                throw ErrorCode::InvalidProjection;
            }

            GeoProj proj_dst_to_tm;
            proj_dst_to_tm.setSrcSRID(srid);
            proj_dst_to_tm.setDstSRID(tile_srid_);
            if (!proj_dst_to_tm.isValid()) {
                // TODO: Save error message!
                throw ErrorCode::InvalidProjection;
            }


            RangeRectd bbox_dst;  // Bounding box in the SRID of destination `srid`
            proj_wgs84_to_dst.transform(bbox, bbox_dst);

            RemapRectd remap_vg_to_tm(out_value_grid->rect(), bbox_dst.rect());

            int32_t w = out_value_grid->width();
            int32_t h = out_value_grid->height();

            Vec2d pos_vg;
            Vec2d pos_dst;
            Vec2d pos_tm;

            out_value_grid->invalidate();

            if (antialias_level > 1) {
                // Antialiasing applied
                double aa_scale = 1.0 / (antialias_level - 1);
                double value_scale = 1.0 / (antialias_level * antialias_level);
                for (int32_t y = 0; y < h; y++) {
                    for (int32_t x = 0; x < w; x++) {
                        // Antialiasing steps
                        double value = 0.0;
                        for (int32_t aa_y = 0; aa_y < antialias_level; aa_y++) {
                            for (int32_t aa_x = 0; aa_x < antialias_level; aa_x++) {
                                pos_vg.x_ = x + aa_x * aa_scale;
                                pos_vg.y_ = h - 1 - y +  aa_y * aa_scale;
                                remap_vg_to_tm.mapVec2(pos_vg, pos_dst);
                                proj_dst_to_tm.transform(pos_dst, pos_tm);
                                value += valueAtPos(pos_tm);
                                out_value_grid->setValueAtXY(x, y, value);
                            }
                        }
                        value *= value_scale;
                    }
                }
            }
            else {
                // No antialiasing applied
                for (int32_t y = 0; y < h; y++) {
                    for (int32_t x = 0; x < w; x++) {
                        pos_vg.x_ = x;
                        pos_vg.y_ = h - 1 - y;
                        remap_vg_to_tm.mapVec2(pos_vg, pos_dst);
                        proj_dst_to_tm.transform(pos_dst, pos_tm);
                        auto value = valueAtPos(pos_tm);
                        out_value_grid->setValueAtXY(x, y, value);
                    }
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        out_value_grid->updateMinMax();

        return result;
    }


    /**
     *  @brief Export a CSV file with information about the managed tiles.
     */
    ErrorCode CVF2TileManager::exportCSV(const String& file_path) noexcept {

        auto result = ErrorCode::None;


        try {
            File file(file_path);
            file.startWriteAsciiOverwrite();

            file.writeStr("crs,range_min_x,range_min_y,range_max_x,range_max_y,width,height,undefined_values,file_name,errors");
            for (auto tile : tiles) {
                if (tile->valid_) {
                    file.writeNewLine();
                    file.writeTextInt32(tile_srid_);
                    file.writeComma();
                    file.writeTextFix(tile->bbox_.minX());
                    file.writeComma();
                    file.writeTextFix(tile->bbox_.minY());
                    file.writeComma();
                    file.writeTextFix(tile->bbox_.maxX());
                    file.writeComma();
                    file.writeTextFix(tile->bbox_.maxY());
                    file.writeComma();
                    file.writeTextUInt32(tile->width_);
                    file.writeComma();
                    file.writeTextUInt32(tile->height_);
                    file.writeComma();
                    file.writeTextInt32(tile->undefined_values_count_);
                    file.writeComma();
                    file.writeString(tile->file_name_);
                    file.writeComma();
                    file.writeTextFlags(tile->error_flags_);
                }
            }

            file.close();
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    /**
     *  @brief Renders meta tiles and outputs them to the specified path.
     *
     *  This function processes meta tiles based on the provided parameters.
     *  The meta tiles are rendered in SRID 3857.
     *
     *  @param dst_path The directory path where the rendered meta tiles will be saved.
     *                  This should be a valid writable path.
     *  @param zoom     The zoom level for rendering. Must be a non-negative integer.
     *  @param bbox     The geographical bbox within which to render meta tiles.
     *                  Specified as a rectangle in longitude/latitude coordinates
     *                  in EPSG:4326 projection.
     *  @param antialias_level The antialiasing level, ranging from 1 to 16. Values outside this range
     *                         will be clamped to the nearest valid value.
     *
     *  @return ErrorCode::None if rendering and saving were successful, or an
     *          appropriate error code indicating the type of failure.
     */
    ErrorCode CVF2TileManager::renderMetaTiles(const String& dst_path, int32_t zoom, const RangeRectd& bbox, int32_t antialias_level, int64_t start_index, int64_t end_index) noexcept {

        auto result = ErrorCode::None;

        ValueGrid<int64_t>* meta_value_grid = nullptr;

        if (verboseLevel() > 0) {
            Log log;
            log << "CVF2TileManager::renderMetaTiles() dir_path: " << dst_path << ", zoom: " << zoom << ", bbox: " << bbox << ", antialias_level: " << antialias_level << std::endl;
        }

        try {
            if (zoom < 0 || zoom > 20) { throw Error::specific(kErrZoomOutOfRange); }

            GeoMetaTileRange mtr(zoom, bbox);
            if (!mtr.valid()) { throw Error::specific(kErrMetaTileRangeFailed); }

            if (!mtr.setStartIndex(start_index)) {
                throw ErrorCode::None;  // Nothing to render
            }

            // Allocate value grid
            meta_value_grid = new ValueGrid<int64_t>(mtr.metaTileSize(), mtr.metaTileSize());
            if (!meta_value_grid) { throw ErrorCode::MemCantAllocate; }
            meta_value_grid->setInvalidValue(CVF2::kUndefinedValue);


            int64_t index = start_index;
            Vec2i tile_index;
            while (mtr.nextTilePos(end_index, tile_index)) {

                // TODO: Implement multithreading!

                // Preparation for rendering a single meta-tile
                Vec2d top_left;
                Vec2d bottom_right;
                Geo::wgs84FromTileIndex(zoom, tile_index, top_left);
                Geo::wgs84FromTileIndex(zoom, tile_index + Vec2i(mtr.gridSize(), mtr.gridSize()), bottom_right);
                RangeRectd tile_bbox = { top_left.x_, bottom_right.y_, bottom_right.x_, top_left.y_ };

                RangeRectd tile_bbox_crs;
                wgs84_to_tile_proj_.transform(tile_bbox, tile_bbox_crs);


                // Render to a value grid
                ErrorCode err = renderToValueGrid(3857, tile_bbox, antialias_level, meta_value_grid);
                if (err != ErrorCode::None) { throw err; }

                meta_value_grid->setGeoInfo(4326, tile_bbox);

                // Save file
                String meta_dir_path;
                String meta_file_name;
                Geo::metaTilePathForTile(dst_path, zoom, tile_index, "cvf", meta_dir_path, meta_file_name);

                if (!File::makeDirs(meta_dir_path)) {
                    throw ErrorCode::FileDirNotFound;
                }

                String file_path = meta_dir_path + "/" + meta_file_name;
                std::cout << index << " of " << mtr.metaTilesNeeded() << ": " << file_path << std::endl;

                meta_value_grid->writeCVF2File(file_path, LengthUnit::GeoDegrees, 2, 4);

                index++;
                if (index > end_index) {
                    break;
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }


        // Cleanup
        delete meta_value_grid;

        return result;
    }


    /**
     *  @brief Renders zoom levels below a given source zoom level.
     *
     *  The resolution is halved for the resulting meta tiles.
     *  The downsampled meta tiles are written to the specified directory.
     *
     *  @param base_path The base directory path for the tiles.
     *  @param srid Spatial Reference System Identifier (SRID) of the tiles, e.g. 3857.
     *  @param src_zoom The source zoom level from which the downsampling process begins.
     *  @param meta_tile_size The pixel size of meta tiles.
     *  @param bbox The geographic bounding box within which tiles should be rendered.
     */
    ErrorCode CVF2TileManager::renderDownsampledMetaTiles(const String& base_path, int32_t srid, int32_t src_zoom, int32_t meta_tile_size, const RangeRectd& bbox) noexcept {

        // TODO: Logging!
        // TODO: Multitreading!
        // TODO: Overwrite mode or check existing tile mode!

        const Vec2i tr[4] = { Vec2i(2, 2), Vec2i(6, 2), Vec2i(2, 6), Vec2i(6, 6) };

        auto result = ErrorCode::None;

        ValueGridl* value_grid = nullptr;
        ValueGridl* sub_value_grid = nullptr;

        try {

            if (src_zoom < 1) { throw ErrorCode::BadArgs; }

            int32_t dst_zoom = src_zoom - 1;

            GeoMetaTileRange mtr(dst_zoom, bbox);
            if (!mtr.valid()) { throw Error::specific(kErrMetaTileRangeFailed); }

            value_grid = new (std::nothrow) ValueGridl(meta_tile_size, meta_tile_size);
            if (!value_grid) { throw ErrorCode::ClassInstantiationFailed; }

            value_grid->setInvalidValue(value_grid->minValueForType());

            Vec2i tile_index;
            int32_t file_index = 0;
            while (mtr.nextTilePos(tile_index)) {

                std::cout << file_index << " of " << mtr.metaTilesNeeded() << std::endl;

                value_grid->invalidate();

                Vec2d tile_center_dst[4];
                String file_path;
                RangeRectFix tile_bbox;
                tile_bbox.initForMinMaxSearch();

                for (int32_t i = 0; i < 4; i++) {

                    Geo::wgs84FromTileIndex(dst_zoom, tile_index + tr[i], tile_center_dst[i]);
                    Vec2i tile_index_src;
                    Geo::wgs84ToTileIndex(src_zoom, tile_center_dst[i], tile_index_src);

                    Geo::metaTilePathForTile(base_path, src_zoom, tile_index_src, "cvf", file_path);

                    if (File::fileExists(file_path)) {
                        CVF2File cvf2_file(file_path);
                        cvf2_file.startRead();

                        if (cvf2_file.srid() == srid) {
                            tile_bbox += cvf2_file.range();

                            auto err = cvf2_file.buildValueGrid(&sub_value_grid);
                            if (err != ErrorCode::None) { throw err; }

                            value_grid->fillMipmapQuadrant(sub_value_grid, i);
                        }
                    }
                    else {
                        // TODO: !!!
                    }
                }

                value_grid->setGeoInfo(4326, tile_bbox);
                value_grid->updateMinMax();

                GeoProj proj(srid, 4326);
                auto center = tile_bbox.centerAsVec2d();
                proj.transform(center);

                {
                    String dir_path;
                    String file_name;
                    Geo::metaTilePathForTile(base_path, dst_zoom, tile_index, "cvf", dir_path, file_name);

                    File::makeDirs(dir_path);
                    value_grid->writeCVF2File(dir_path + "/" + file_name, LengthUnit::GeoDegrees, 1, 4);

                    /*
                    auto img = value_grid->buildImage();
                    if (img) {
                        Geo::metaTilePathForTile(base_path, dst_zoom, tile_index, "jpg", dir_path, file_name);
                        img->normalize();
                        img->writeJpg(dir_path + "/" + file_name);
                        delete img;
                    }
                     */
                }

                file_index++;
            }
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Cleanup
        delete value_grid;
        delete sub_value_grid;

        return result;
    }


    void CVF2TileManager::saveLog(const String& log_file_path) noexcept {

        try {
            File log_file(log_file_path);
            log_file.startWriteAsciiOverwrite();

            Log log(*log_file.stream());

            log << "scan_done: " << scan_done_ << log.endl;
            log << "running: " << running_ << log.endl;
            log << "file_slot_capacity: " << file_slot_capacity_ << log.endl;

            log << "dir_path: " << dir_path_ << log.endl;
            log << "tile_dimensions: " << tile_width_ << " x " << tile_height_ << log.endl;
            log << "tile_count x: " << x_tile_count_ << ", y: " << y_tile_count_ << log.endl;
            log << "tile_count: " << tile_count_ << ", tile_count_limit: " << tile_count_limit_ << log.endl;

            log << "Scan results:\n";
            log << "  duration: " << Timestamp::elapsedSeconds(scan_ts1_, scan_ts2_) << " sec.\n";
            log << "  files: " << scan_files_n_ << log.endl;
            log << "  files ignored: " << scan_files_ignored_n_ << log.endl;
            log << "  files incomplete: " << scan_incomplete_files_n_ << log.endl;
            log << "  files with wrong dimensions: " << scan_wrong_dimension_files_n_ << log.endl;
            log << "  xy range: " << scan_xy_range_ << log.endl;
            log << "  value min: " << scan_total_min_ << ", max: " << scan_total_max_ << log.endl;
            log << "  undefined values: " << scan_total_undefined_values_n_ << log.endl;
            log << "  overlapping tiles: " << start_tile_multi_inititialized_n_ << log.endl;

            log << "\nStart results:\n";
            if (!running_) {
                log << "  Not started.\n";
            }
            else {
                log << "  duration: " << Timestamp::elapsedSeconds(start_ts1_, start_ts2_) << " sec.\n";
                log << "  cvf2 files open calls: " << cvf2_file_open_n_ << log.endl;
                log << "  cvf2 files close calls: " << cvf2_file_close_n_ << log.endl;
                log << "  cvf2 files open failed: " << cvf2_file_open_failed_n_ << log.endl;
                log << "  number of errors: " << start_error_n_ << log.endl;

                log << "\nErrors:\n";
                if (tiles.size() != tile_count_) {
                    log << "  Tiles buffer does not match tile count: " <<  tiles.size() << log.endl;
                }
            }
        }
        catch (ErrorCode err) {

        }
    }


    ErrorCode CVF2TileManager::_projectBbox() noexcept {

        if (tile_srid_ == 0) {
            return Error::specific(kErrTileSRIDMissing);
        }

        if (provided_bbox_srid_ == 0) {
            return Error::specific(kErrTileProvidedBboxSRIDMissing);
        }

        GeoProj proj;
        proj.setSrcSRID(provided_bbox_srid_);
        proj.setDstSRID(tile_srid_);

        if (!proj.isValid()) {
            return Error::specific(kErrBboxTransformFailed);
        }

        proj.transform(provided_bbox_, bbox_);

        return ErrorCode::None;
    }


} // End of namespace Grain

