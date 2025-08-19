//
//  CVF2File.hpp
//
//  Created by Roald Christesen on 08.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainCVF2File_hpp
#define GrainCVF2File_hpp

#include "Grain.hpp"
#include "File/File.hpp"
#include "Type/Type.hpp"
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
        uint32_t m_offset;
        uint32_t m_length;
        int64_t m_min;
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
        uint32_t m_width = 0;           ///< Field width
        uint32_t m_height = 0;          ///< Field height
        int32_t m_srid = 0;             ///< Spatial Reference System Identifier (SRID)
        RangeRectFix m_xy_range;        ///< Range of XY values in this file
        int32_t m_undefined_values_count = 0;   ///< Number of undefined values in file
        int64_t m_min_value = 0;        ///< Minimum value in value field
        int64_t m_max_value = 0;        ///< Maximum value in value field
        Fix m_mean_value = 0;           ///< Mean of all valid values
        LengthUnit m_unit = LengthUnit::Undefined;
        int64_t m_row_offsets_pos = 0;  ///< Position of row index table in file

        bool m_cache_flag = false;      ///< true, if data is loaded to RAM cash, else false
        void* m_cache_data = nullptr;

        CVF2Sequence* m_row_seq = nullptr;
        int32_t m_row_seq_length = 0;
        int64_t* m_row_values = nullptr;

    public:
        using File::filePath;
        using File::close;

    public:
        explicit CVF2File(const String& file_path) noexcept;
        ~CVF2File() noexcept override;

        const char* className() const noexcept override { return "CVF2File"; }

        friend std::ostream& operator << (std::ostream& os, const CVF2File* o) {
            o == nullptr ? os << "CVF2File nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const CVF2File& o) {
            return os << o.m_width << ", " << o.m_height << ", SRID: " << o.m_srid;
        }

        void log(Log& l) const {
            l << "big_endian: " << l.boolValue(m_big_endian) << Log::endl;
            l << "width: " << m_width << ", height " << m_height << Log::endl;
            l << "srid: " << m_srid << Log::endl;
            l << "xy_range: " << m_xy_range << Log::endl;
            l << "unit: " << Geometry::lengthUnitName(m_unit) << Log::endl;
            l << "undefined_values_count: " << m_undefined_values_count << Log::endl;
            l << "min_value: " << m_min_value << ", max_value: " << m_max_value << ", mean_value: " << m_mean_value << Log::endl;
            l << "row_offsets_pos: " << m_row_offsets_pos << Log::endl;
        }


        int32_t srid() const noexcept { return m_srid; }

        uint32_t width() const noexcept { return m_width; }
        uint32_t height() const noexcept { return m_height; }
        uint32_t valueCount() const noexcept { return m_width * m_height; }
        RangeRectFix range() const noexcept { return m_xy_range; }
        Fix minX() const noexcept { return m_xy_range.m_min_x; }
        Fix minY() const noexcept { return m_xy_range.m_min_y; }
        Fix maxX() const noexcept { return m_xy_range.m_max_x; }
        Fix maxY() const noexcept { return m_xy_range.m_max_y; }
        Vec2d centerAsVec2d() const noexcept { return m_xy_range.centerAsVec2d(); }
        int64_t minValue() const noexcept { return m_min_value; }
        int64_t maxValue() const noexcept { return m_max_value; }
        LengthUnit unit() const noexcept { return m_unit; }
        int32_t undefinedValuesCount() const noexcept { return m_undefined_values_count; }

        const int64_t* ptrToRowValues() const noexcept { return m_row_values; }
        int64_t* mutPtrToRowValues() noexcept { return m_row_values; }

        void freeCache() noexcept;
        ErrorCode buildCacheData() noexcept;
        int64_t valueFromCache(uint32_t x, uint32_t y) noexcept;

        void startRead() override;
        int64_t valueAtPos(const Vec2i& pos, bool cache_mode) noexcept;

        int32_t readRow(int32_t y);

        bool hitBbox(const RangeRectd& bbox) const noexcept {
            if (m_xy_range.m_min_x.asDouble() <= bbox.m_max_x &&
                m_xy_range.m_min_y.asDouble() <= bbox.m_max_y &&
                m_xy_range.m_max_x.asDouble() >= bbox.m_min_x &&
                m_xy_range.m_max_y.asDouble() >= bbox.m_min_x) {
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
