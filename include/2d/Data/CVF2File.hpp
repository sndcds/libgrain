//
//  CVF2File.hpp
//
//  Created by Roald Christesen on 08.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainCVF2File_hpp
#define GrainCVF2File_hpp

#include "Grain.hpp"
#include "File/File.hpp"
#include "Image/Image.hpp"
#include "Core/Log.hpp"


namespace Grain {

class Image;
class CVF2TileManager;
class CVF2Tile;


template <typename T>
class ValueGrid;
using ValueGridl = ValueGrid<int64_t>;


struct CVF2Sequence {
    uint32_t offs_;
    uint32_t length_;
    int64_t min_;
};


class CVF2File : public File {

    friend class CVF2TileManager;
    friend class CVF2Tile;

public:
    enum {
        kErrNoValues = 0,
        kErrXOutOfRange,
        kErrYOutOfRange,
        kErrRowSeqCantAlloc,
        kErrXYOutOfRange,
        kErrValueNotAsOriginal
    };

    enum {
        kCRSStringLength = 16
    };

    enum class ScaleMode {
        None = 0,
        Auto,
        Factor
    };

    enum class ImageScaleMode {
        None = 0,
        Auto,
        MinMax
    };

protected:
    uint32_t width_ = 0;            ///< Field width
    uint32_t height_ = 0;           ///< Field height
    int32_t srid_ = 0;              ///< Spatial Reference System Identifier (SRID)
    RangeRectFix xy_range_;         ///< Range of XY values in this file
    int32_t undefined_values_count_ = 0;   ///< Number of undefined values in file
    int64_t min_value_ = 0;         ///< Minimum value in value field
    int64_t max_value_ = 0;         ///< Maximum value in value field
    Fix mean_value_ = 0;            ///< Mean of all valid values
    LengthUnit unit_ = LengthUnit::Undefined;
    int64_t row_offsets_pos_ = 0;   ///< Position of row index table in file

    bool cache_flag_ = false;       ///< true, if data is loaded to RAM cash, else false
    void* cache_data_    = nullptr;

    CVF2Sequence* row_seq_ = nullptr;
    int32_t row_seq_length_ = 0;
    int64_t* row_values_ = nullptr;

public:
    using File::filePath;
    using File::close;

public:
    explicit CVF2File(const String& file_path) noexcept;
    ~CVF2File() noexcept override;

    const char* className() const noexcept override {
        return "CVF2File";
    }

    friend std::ostream& operator << (std::ostream& os, const CVF2File* o) {
        o == nullptr ? os << "CVF2File nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const CVF2File& o) {
        return os << o.width_ << ", " << o.height_ << ", SRID: " << o.srid_;
    }

    void log(Log& l) const {
        l << "big_endian: " << l.boolValue(big_endian_) << Log::endl;
        l << "width: " << width_ << ", height " << height_ << Log::endl;
        l << "srid: " << srid_ << Log::endl;
        l << "xy_range: " << xy_range_ << Log::endl;
        l << "unit: " << Geometry::lengthUnitName(unit_) << Log::endl;
        l << "undefined_values_count: " << undefined_values_count_ << Log::endl;
        l << "min_value: " << min_value_ << ", max_value: " << max_value_ << ", mean_value: " << mean_value_ << Log::endl;
        l << "row_offsets_pos: " << row_offsets_pos_ << Log::endl;
    }


    int32_t srid() const noexcept { return srid_; }

    uint32_t width() const noexcept { return width_; }
    uint32_t height() const noexcept { return height_; }
    uint32_t valueCount() const noexcept { return width_ * height_; }
    RangeRectFix range() const noexcept { return xy_range_; }
    Fix minX() const noexcept { return xy_range_.min_x_; }
    Fix minY() const noexcept { return xy_range_.min_y_; }
    Fix maxX() const noexcept { return xy_range_.max_x_; }
    Fix maxY() const noexcept { return xy_range_.max_y_; }
    Vec2d centerAsVec2d() const noexcept { return xy_range_.centerAsVec2d(); }
    int64_t minValue() const noexcept { return min_value_; }
    int64_t maxValue() const noexcept { return max_value_; }
    LengthUnit unit() const noexcept { return unit_; }
    int32_t undefinedValuesCount() const noexcept { return undefined_values_count_; }

    const int64_t* ptrToRowValues() const noexcept { return row_values_; }
    int64_t* mutPtrToRowValues() noexcept { return row_values_; }

    void freeCache() noexcept;
    ErrorCode buildCacheData() noexcept;
    int64_t valueFromCache(uint32_t x, uint32_t y) noexcept;

    void startRead() override;
    int64_t valueAtPos(const Vec2i& pos, bool cache_mode) noexcept;

    int32_t readRow(int32_t y);

    bool hitBbox(const RangeRectd& bbox) const noexcept {
        if (xy_range_.min_x_.asDouble() <= bbox.max_x_ &&
            xy_range_.min_y_.asDouble() <= bbox.max_y_ &&
            xy_range_.max_x_.asDouble() >= bbox.min_x_ &&
            xy_range_.max_y_.asDouble() >= bbox.min_x_) {
            return true;
        }
        else {
            return false;
        }
    }


    ErrorCode xyzCompare(const String& xyz_file_path, int32_t z_decimals) noexcept;

    ErrorCode buildImage(ImageScaleMode scale_mode, float min_level, float max_level, Image* *out_image_ptr, bool flip_y) noexcept;
    ErrorCode buildValueGrid(ValueGridl** out_value_grid_ptr) noexcept;

    static void logCVF2File(const String& cvf2_file_path, std::ostream& os) noexcept;
    static void cvf2ToImage(const String& cvf2_file_path, const String& image_file_path) noexcept;
    static void cvf2ToImageBatch(const String& src_dir_path, const String& dst_dir_path, ImageScaleMode scale_mode, float min_level, float max_level, fourcc_t type) noexcept;
};


} // End of namespace Grain

#endif // GrainCVF2File_hpp
