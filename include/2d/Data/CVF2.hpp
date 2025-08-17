//
//  CVF2.hpp
//
//  Created by Roald Christesen on 05.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
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
        uint32_t m_width = 0;           ///< Width, number of values in X direction
        uint32_t m_height = 0;          ///< Height, number of values in Y direction
        LengthUnit m_unit = LengthUnit::Undefined;      ///< Unit of measurement for the stored values
        int32_t m_srid;                 ///< Spatial Reference System Identifier (SRID)
        RangeRectFix m_bbox;            ///< Bounding box for the region. If used in a geographic context, the coordinates must be in the coordinate system specified by `m_srid`
        int64_t m_min_value;            ///< Minimum value in value field
        int64_t m_max_value;            ///< Maximum value in value field
        Fix m_mean_value;               ///< Mean of all values in value field

        uint32_t m_min_digits;          ///< Minimum digits for row compression
        uint32_t m_max_digits;          ///< Maximum digits for row compression

        int32_t m_curr_row_index = -1;  ///< Current index for encoding rows
        int32_t m_curr_value_index;     ///< Current index for pushing values
        bool m_file_data_saved = false;

        bool m_high_nibble_flag;        ///< Flag for handling nibble to byte conversion
        int32_t m_nibble_count;
        uint8_t* m_byte_buffer = nullptr;
        size_t m_byte_buffer_size = 0;
        int32_t m_curr_byte_index;

        int64_t* m_row_values = nullptr;
        int64_t* m_row_offsets = nullptr;
        uint32_t* m_seq_offsets = nullptr;
        int64_t* m_seq_mins = nullptr;

        int32_t m_curr_row_value_index;
        uint16_t m_curr_row_digits;
        int64_t m_curr_row_max_diff;

        int64_t m_file_pos_undef_values_counter;
        int64_t m_file_pos_min_max;
        int64_t m_file_pos_row_offsets;

        int64_t* m_data = nullptr;      ///< Memory for all values in field
        int32_t m_data_undef_n = 0;     ///< Number of undefined values in `m_data`
        int32_t m_data_def_n = 0;       ///< Number of valid values in `m_data`
        double m_data_sum;              ///< Sum of all valid values


        File* m_file = nullptr;         ///< File, where data will be written to


    public:
        CVF2(int32_t width, int32_t height, LengthUnit unit, int32_t min_digits, int32_t max_digits) noexcept ;
        ~CVF2() noexcept ;

        void setSRID(int32_t srid) noexcept {
            m_srid = srid;
        }

        void setBbox(double min_x, double min_y, double max_x, double max_y) noexcept {
            m_bbox.m_min_x = min_x;
            m_bbox.m_min_y = min_y;
            m_bbox.m_max_x = max_x;
            m_bbox.m_max_y = max_y;
        }

        void setBbox(const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept {
            m_bbox.m_min_x = min_x;
            m_bbox.m_min_y = min_y;
            m_bbox.m_max_x = max_x;
            m_bbox.m_max_y = max_y;
        }

        void setBbox(const RangeRectFix& bbox) noexcept {
            m_bbox = bbox;
        }

        void setBbox(const RangeCubeFix& range_cube) noexcept {
            m_bbox.m_min_x = range_cube.minX();
            m_bbox.m_min_y = range_cube.minY();
            m_bbox.m_max_x = range_cube.maxX();
            m_bbox.m_max_y = range_cube.maxY();
        }

        void setUnit(LengthUnit unit) noexcept { m_unit = unit; }

        uint32_t width() const noexcept { return m_width; }
        uint32_t height() const noexcept { return m_height; }
        LengthUnit unit() const noexcept { return m_unit; }

        int32_t undefinedValuesCount() const noexcept { return m_data_undef_n; }

        void openFileToWrite(const String& file_path);
        void pushValue(int64_t value);
        void pushValueToData(int32_t x, int32_t y, int64_t value);
        void encodeData();
        void finish();

        static int64_t _maxDiff(uint32_t digits) { return static_cast<int64_t>(round(pow(16, digits))) - 1 - 1; }

        void _startRow();
        uint8_t _extractNibble(uint64_t value, int32_t position);
        void _pushNibble(uint8_t nibble);
        void _bufferValues(const int64_t* values, uint32_t seq_index, uint32_t seq_offset, uint32_t seq_length, int64_t min_value);
        void _encodeRow(const int64_t* values);
        bool _encoderRowPrediction(const int64_t* values, int32_t digits, int32_t& seq_count, int64_t& byte_count);
    };


} // End of namespace Grain

#endif // GrainCVF2_hpp
