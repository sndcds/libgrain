//
//  CVF2.hpp
//
//  Created by Roald Christesen on 05.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainCVF2_hpp
#define GrainCVF2_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "File/File.hpp"
#include "2d/RangeRect.hpp"
#include "3d/RangeCube.hpp"


namespace Grain {

/**
 *  @brief A compression algorithm for data stored in a 2-dimensional array.
 *
 *  This class compresses data from a 2-dimensional array into a file.
 *
 *  The compression process utilizes Delta Encoding, which calculates the
 *  difference between consecutive data samples and encodes these differences
 *  using a multiple of 4 bits per sample in a row. For each row of data,
 *  the algorithm dynamically selects the most suitable bit depth to represent
 *  the delta values, optimizing compression efficiency.
 *  Delta Encoding excels when applied to data exhibiting a smooth or slowly
 *  changing trend, such as sequential measurements over time, where the
 *  differences between consecutive values tend to be small.
 *  Furthermore, Delta Encoding can be synergistically combined with other
 *  compression techniques to achieve enhanced compression ratios and improved
 *  performance.
 *
 *
 *  The file format adheres to the following specifications:
 *
 *  1. Validation:
 *     Ensures that the entire dataset is intact, without any missing data points.
 *  2. Preservation of values:
 *     Guarantees that values remain unaltered during compression and
 *     decompression. This ensures that when reading a value from the
 *     compressed file, it matches exactly with the original data.
 *  3. Compression Rate:
 *     Aims for an optimal compression rate compared to the original file and
 *     alternative methods, such as storing data as an uncompressed Float-Array.
 *  4. Accuracy:
 *     Maintains high accuracy across a wide range of values.
 *  5. Detection of Invalid values:
 *     Identifies and flags invalid values in the compressed file.
 *  6. Efficient Single Value Reading:
 *     Provides relatively fast access to individual values from the compressed
 *     file.
 *  7. Faster Array Reading:
 *     Facilitates faster reading of arrays of values from the compressed file.
 */
class CVF2 : public Object {

public:
    enum {
        kErrFatal = 0,
        kErrRowIndexOutOfRange,
        kErrFileAllreadyOpened,
        kErrUnknownDigits
    };

    static constexpr int64_t kUndefinedValue = std::numeric_limits<int64_t>::min();

protected:
    uint32_t width_{};              ///< Width, number of values in X direction
    uint32_t height_{};             ///< Height, number of values in Y direction
    LengthUnit unit_ = LengthUnit::Undefined;      ///< Unit of measurement for the stored values
    int32_t srid_{};                ///< Spatial Reference System Identifier (SRID)
    RangeRectFix bbox_{};           ///< Bounding box for the region. If used in a geographic context, the coordinates must be in the coordinate system specified by `srid_`
    int64_t min_value_{};           ///< Minimum value in value field
    int64_t max_value_{};           ///< Maximum value in value field
    Fix mean_value_{};              ///< Mean of all values in value field

    uint32_t min_digits_{};         ///< Minimum digits for row compression
    uint32_t max_digits_{};         ///< Maximum digits for row compression

    int32_t curr_row_index_ = -1;   ///< Current index for encoding rows
    int32_t curr_value_index_{};    ///< Current index for pushing values
    bool file_data_saved_ = false;

    bool high_nibble_flag_{};       ///< Flag for handling nibble to byte conversion
    int32_t nibble_count_{};
    uint8_t* byte_buffer_ = nullptr;
    size_t byte_buffer_size_ = 0;
    int32_t curr_byte_index_{};

    int64_t* row_values_ = nullptr;
    int64_t* row_offsets_ = nullptr;
    uint32_t* seq_offsets_ = nullptr;
    int64_t* seq_mins_ = nullptr;

    int32_t curr_row_value_index_{};
    uint16_t curr_row_digits_{};
    int64_t curr_row_max_diff_{};

    int64_t file_pos_undef_values_counter_{};
    int64_t file_pos_min_max_{};
    int64_t file_pos_row_offsets_{};

    int64_t* data_ = nullptr;      ///< Memory for all values in field
    int32_t data_undef_n_ = 0;     ///< Number of undefined values in `data_`
    int32_t data_def_n_ = 0;       ///< Number of valid values in `data_`
    double data_sum_;              ///< Sum of all valid values

    File* file_ = nullptr;         ///< File, where data will be written to

public:
    CVF2(int32_t width, int32_t height, LengthUnit unit, int32_t min_digits, int32_t max_digits) noexcept ;
    ~CVF2() noexcept override;

    [[nodiscard]] const char* className() const noexcept override {
        return "CVF2";
    }

    void setSRID(int32_t srid) noexcept {
        srid_ = srid;
    }

    void setBbox(double min_x, double min_y, double max_x, double max_y) noexcept {
        bbox_.min_x_ = min_x;
        bbox_.min_y_ = min_y;
        bbox_.max_x_ = max_x;
        bbox_.max_y_ = max_y;
    }

    void setBbox(const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept {
        bbox_.min_x_ = min_x;
        bbox_.min_y_ = min_y;
        bbox_.max_x_ = max_x;
        bbox_.max_y_ = max_y;
    }

    void setBbox(const RangeRectFix& bbox) noexcept {
        bbox_ = bbox;
    }

    void setBbox(const RangeCubeFix& range_cube) noexcept {
        bbox_.min_x_ = range_cube.minX();
        bbox_.min_y_ = range_cube.minY();
        bbox_.max_x_ = range_cube.maxX();
        bbox_.max_y_ = range_cube.maxY();
    }

    void setUnit(LengthUnit unit) noexcept { unit_ = unit; }

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] LengthUnit unit() const noexcept { return unit_; }

    [[nodiscard]] int32_t undefinedValuesCount() const noexcept { return data_undef_n_; }

    void openFileToWrite(const String& file_path);
    void pushValue(int64_t value);
    void pushValueToData(int32_t x, int32_t y, int64_t value);
    void encodeData();
    void finish();

    [[nodiscard]] static int64_t _maxDiff(uint32_t digits) {
        return static_cast<int64_t>(round(pow(16, digits))) - 1 - 1;
    }

    void _startRow();
    [[nodiscard]] uint8_t _extractNibble(uint64_t value, int32_t position) const;
    void _pushNibble(uint8_t nibble);
    void _bufferValues(const int64_t* values, uint32_t seq_index, uint32_t seq_offset, uint32_t seq_length, int64_t min_value);
    void _encodeRow(const int64_t* values);
    bool _encoderRowPrediction(const int64_t* values, int32_t digits, int32_t& seq_count, int64_t& byte_count) const;
};


} // End of namespace Grain

#endif // GrainCVF2_hpp
