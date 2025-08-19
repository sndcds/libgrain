//
//  CVF2File.cpp
//
//  Created by Roald Christesen on 08.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Data/CVF2File.hpp"
#include "2d/Data/CVF2TileManager.hpp"
#include "Image/Image.hpp"
#include "File/XYZFile.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"
#include "String/CSVString.hpp"
#include "2d/Data/ValueGrid.hpp"


namespace Grain {

    CVF2File::CVF2File(const String& file_path) noexcept : File(file_path) {
    }


    CVF2File::~CVF2File() noexcept {
        std::free(m_cache_data);
        std::free(m_row_seq);
        std::free(m_row_values);
    }


    void CVF2File::freeCache() noexcept {
        if (m_cache_data != nullptr) {
            std::free(m_cache_data);
            m_cache_data = nullptr;
            m_cache_flag = false;
        }
    }


    ErrorCode CVF2File::buildCacheData() noexcept {
        auto result = ErrorCode::None;

        try {
            uint32_t value_count = valueCount();
            if (value_count < 1) {
                return Error::specific(kErrNoValues);
            }

            freeCache();

            m_cache_data = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * value_count));
            if (!m_cache_data) {
                return ErrorCode::MemCantAllocate;
            }

            auto dst = static_cast<int64_t*>(m_cache_data);
            for (int32_t y = 0; y < m_height; y++) {
                readRow(y);
                for (uint32_t x = 0; x < m_width; x++) {
                    *dst++ = m_row_values[x];
                }
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }


    int64_t CVF2File::valueFromCache(uint32_t x, uint32_t y) noexcept {
        if (x >= m_width || y >= m_height || m_cache_data == nullptr) {
            return CVF2::kUndefinedValue;
        }
        else {
            return (static_cast<int64_t*>(m_cache_data))[y * m_width + x];
        }
    }


    void CVF2File::startRead() {
        char buffer[4];

        File::startRead();

        // Check the header
        setPos(0);
        readStr(4, buffer);
        checkSignature(buffer, 4, "CVF2");

        // Check endianess
        readStr(2, buffer);
        setEndianBySignature(buffer);

        // Read info
        m_width = readValue<uint32_t>();
        m_height = readValue<uint32_t>();
        m_srid = readValue<int32_t>();

        readFix(m_xy_range.m_min_x);
        readFix(m_xy_range.m_min_y);
        readFix(m_xy_range.m_max_x);
        readFix(m_xy_range.m_max_y);

        m_undefined_values_count = readValue<uint32_t>();

        m_min_value = readValue<int64_t>();
        m_max_value = readValue<int64_t>();
        readFix(m_mean_value);
        m_unit = (LengthUnit)readValue<int32_t>();

        m_row_offsets_pos = readValue<uint32_t>();
    }


    int64_t CVF2File::valueAtPos(const Vec2i& pos, bool cache_mode) noexcept {
        static const int64_t max_diffs[] = {
                0L,
                14L,
                254L,
                4094L,
                65534L,
                1048574L,
                16777214L,
                268435454L,
                4294967294L
        };

        if (pos.m_x < 0 || pos.m_x >= m_width) {
            return CVF2::kUndefinedValue;
        }

        if (pos.m_y < 0 || pos.m_y >= m_height) {
            return CVF2::kUndefinedValue;
        }

        if (cache_mode) {
            if (!m_cache_data) {
                auto err = buildCacheData();
                if (err != ErrorCode::None) {
                    return 0; // TODO: Error handling needed!
                }
            }
            return (static_cast<int64_t*>(m_cache_data))[pos.m_y * m_width + pos.m_x];
        }

        setPos(m_row_offsets_pos + pos.m_y * 4);
        auto row_offset = readValue<uint32_t>();

        setPos(row_offset);
        auto digits = readValue<uint16_t>();
        auto seq_count = readValue<uint32_t>();
        uint32_t seq_index = 0;
        uint32_t seq_offset = 0;
        int64_t seq_min = 0;

        uint32_t offset = 0;
        for (uint32_t i = 0; i < seq_count; i++) {
            if (i > 0) {
                offset = readValue<uint32_t>();
            }
            auto min = readValue<int64_t>();
            if (pos.m_x >= offset) {
                seq_index = i;
                seq_offset = offset;
                seq_min = min;
            }
        }

        uint32_t data_pos = (row_offset + 2 + 4 + seq_count * (4 + 8) - 4) + (pos.m_x * digits / 2);
        bool second_nibble_flag = ((digits % 2) != 0) && ((pos.m_x % 2) != 0);

        // Read nibbles
        setPos(data_pos);
        auto byte = readValue<uint8_t>();

        uint64_t diff = 0;
        for (uint16_t i = 0; i < digits; i++) {
            diff <<= 4;
            if (second_nibble_flag) {
                diff |= byte & 0xF;
                second_nibble_flag = false;
                byte = readValue<uint8_t>();
            }
            else {
                diff |= ((byte & 0xF0) >> 4);
                second_nibble_flag = true;
            }
        }

        if (diff > max_diffs[digits]) {
            return CVF2::kUndefinedValue;
        }
        else {
            return seq_min + diff;
        }
    }


    int32_t CVF2File::readRow(int32_t y) {
        static const int64_t max_diffs[] = {
                0L,
                14L,
                254L,
                4094L,
                65534L,
                1048574L,
                16777214L,
                268435454L,
                4294967294L
        };

        if (y < 0 || y >= m_height) {
            Exception::throwSpecific(kErrYOutOfRange);
        }

        if (!m_row_values) {
            m_row_values = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * m_width));
            if (!m_row_values) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }
        }

        std::cout << "CVF2File::readRow m_row_offsets_pos: " << m_row_offsets_pos << std::endl;
        std::cout << "CVF2File::readRow y: " << y << std::endl;
        std::cout << "CVF2File::readRow pos: " << (m_row_offsets_pos + y * 4) << std::endl;
        std::cout << "CVF2File::readRow file size(): " << size() << std::endl;

        setPos(m_row_offsets_pos + y * 4);
        auto row_offset = readValue<uint32_t>();
        std::cout << "CVF2File::readRow row_offset: " << row_offset << std::endl;

        setPos(row_offset);
        auto digits = readValue<uint16_t>();
        auto seq_count = readValue<uint32_t>();
        std::cout << "CVF2File::readRow digits: " << digits << std::endl;
        std::cout << "CVF2File::readRow seq_count: " << seq_count << std::endl;

        if (seq_count > m_row_seq_length) {
            if (m_row_seq != nullptr) {
                std::free(m_row_seq);
                m_row_seq = nullptr;
            }
            m_row_seq = (CVF2Sequence*)std::malloc(sizeof(CVF2Sequence) * seq_count);
            if (m_row_seq != nullptr) {
                m_row_seq_length = static_cast<int32_t>(seq_count);
            }
            else {
                Exception::throwSpecific(kErrRowSeqCantAlloc);
            }
        }

        m_row_seq[0].m_offset = 0;
        for (uint32_t i = 0; i < seq_count; i++) {
            if (i > 0) {
                m_row_seq[i].m_offset = readValue<uint32_t>();
                m_row_seq[i - 1].m_length = m_row_seq[i].m_offset - m_row_seq[i - 1].m_offset;
            }
            m_row_seq[i].m_min = readValue<int64_t>();
        }
        m_row_seq[seq_count - 1].m_length = m_width - m_row_seq[seq_count - 1].m_offset;


        uint32_t data_pos = (row_offset + 2 + 4 + seq_count * (4 + 8) - 4);
        setPos(data_pos);

        uint32_t seq_index = 0;
        auto seq_cd = static_cast<int32_t>(m_row_seq[0].m_length);
        int64_t seq_min = m_row_seq[0].m_min;

        bool second_nibble_flag = false;

        uint8_t byte = 0x0;
        for (int32_t x = 0; x < m_width; x++) {
            seq_cd--;

            uint64_t diff = 0;
            for (int32_t digit_index = 0; digit_index < digits; digit_index++) {
                diff <<= 4;
                if (second_nibble_flag) {
                    diff |= byte & 0xF;
                    second_nibble_flag = false;
                }
                else {
                    byte = readValue<uint8_t>();
                    diff |= ((byte & 0xF0) >> 4);
                    second_nibble_flag = true;
                }
            }

            if (diff > max_diffs[digits]) {
                m_row_values[x] = CVF2::kUndefinedValue;
            }
            else {
                m_row_values[x] = seq_min + static_cast<int64_t>(diff);
            }

            if (seq_cd <= 0) {
                seq_index++;
                if (seq_index >= seq_count) {
                    break;
                }
                seq_cd = m_row_seq[seq_index].m_length;
                seq_min = m_row_seq[seq_index].m_min;
            }
        }

        return m_width;
    }


    ErrorCode CVF2File::xyzCompare(const String& xyz_file_path, int32_t z_decimals) noexcept {
        auto result = ErrorCode::None;
        XYZFile* xyz_file = nullptr;

        try {
            xyz_file = new(std::nothrow) XYZFile(xyz_file_path);
            if (xyz_file == nullptr) {
                Exception::throwStandard(ErrorCode::FileNotFound);
            }

            xyz_file->startReadAscii();
            xyz_file->scan();
            xyz_file->rewind();

            auto range = xyz_file->range();

            double xyz_min_x = range.minX().asDouble();
            double xyz_min_y = range.minY().asDouble();
            int64_t xyz_last_x = static_cast<int64_t>(round(range.maxX().asDouble()));
            int64_t xyz_last_y = static_cast<int64_t>(round(range.maxY().asDouble()));

            int32_t xyz_line_count = 0;
            String xyz_line;
            Vec3Fix xyz_coord;
            Fix z_value;

            while (xyz_file->readTrimmedLine(xyz_line)) {
                if (xyz_line.length() > 0) {
                    if (!xyz_coord.setByCSV(xyz_line, ' ')) {
                        Exception::throwStandard(ErrorCode::UnexpectedData);
                    }

                    int64_t x = static_cast<int64_t>(round(xyz_coord.m_x.asDouble() - xyz_min_x));
                    int64_t y = static_cast<int64_t>(round(xyz_coord.m_y.asDouble() - xyz_min_y));
                    if (x < 0 || y < 0 || x > xyz_last_x || y >= xyz_last_y) {
                        Exception::throwSpecific(kErrXYOutOfRange);
                    }

                    int64_t value = valueAtPos(Vec2i(static_cast<int32_t>(x), static_cast<int32_t>(y)), true);
                    z_value.setInt64(value, z_decimals);

                    if (z_value != xyz_coord.m_z) {
                        Exception::throwSpecific(kErrValueNotAsOriginal);
                    }

                    xyz_line_count++;
                }
            }
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
     *  @brief Builds a greyscale image from ValueGrid data.
     *
     *  This function creates a greyscale image based on data from a ValueGrid file. The resulting image
     *  will contain pixel values as floats, where the intensity of each pixel falls between `min_level`
     *  and `max_level`. Any pixel that doesn't match a defined value will be set to `undefined_value`.
     *
     *  @param[in] scale_mode How to scale values from ValueGrid to pixel values.
     *  @param[in] min_level Minimum value for pixel intensity in the greyscale image.
     *  @param[in] max_level Maximum value for pixel intensity in the greyscale image.
     *  @param[in] flip_y If true, image will be created vertically flipped.
     *  @param[out] out_image_ptr Pointer to the created greyscale Image. Memory is allocated within the
     *              function, and the caller is responsible for deallocating it.
     *
     *  @return ErrorCode indicating the success or failure of the image creation process.
     */
    ErrorCode CVF2File::buildImage(ImageScaleMode scale_mode, float min_level, float max_level, Image** out_image_ptr, bool flip_y) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!out_image_ptr) {
                Exception::throwStandard(ErrorCode::NullData);
            }
            if (m_width < 1 || m_height < 1) {
                Exception::throwStandard(ErrorCode::UnsupportedDimension);
            }

            auto image = *out_image_ptr;
            if (image != nullptr) {
                if (image->width() != m_width || image->height() != m_height) {
                    delete image;
                    image = *out_image_ptr = nullptr;
                }
            }

            if (image == nullptr) {
                image = Image::createLuminaAlphaFloat(static_cast<int32_t>(m_width), static_cast<int32_t>(m_height));
                if (image == nullptr) {
                    Exception::throwStandard(ErrorCode::MemCantAllocate);
                }
                *out_image_ptr = image;
            }

            if (image->pixelType() != Image::PixelType::Float) {
                Exception::throwStandard(ErrorCode::UnsupportedDataType);
            }

            float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia(image, pixel);

            if (scale_mode == ImageScaleMode::Auto) {
                min_level = static_cast<float>(m_min_value);
                max_level = static_cast<float>(m_max_value);
            }

            float range = (max_level - min_level);
            float scale = range != 0.0f ? 1.0f / range : 1.0f;

            int32_t y = flip_y ? ia.height() - 1 : 0;
            int32_t y_step = flip_y ? -1 : 1;

            while (ia.stepY()) {
                readRow(y);
                int32_t x = 0;

                while (ia.stepX()) {
                    auto value = m_row_values[x];
                    if (value == CVF2::kUndefinedValue) {
                        pixel[0] = 1.0f;
                        pixel[1] = 0.0f;
                    }
                    else {
                        switch (scale_mode) {
                            case ImageScaleMode::None:
                                pixel[0] = m_row_values[x];
                                pixel[1] = 1.0f;
                                break;
                            case ImageScaleMode::Auto:
                            case ImageScaleMode::MinMax:
                                pixel[0] = (m_row_values[x] - min_level) * scale;
                                pixel[1] = 1.0f;
                                break;
                        }
                    }
                    ia.write();
                    x++;
                }
                y += y_step;
            }
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        return result;
    }


    ErrorCode CVF2File::buildValueGrid(ValueGridl** out_value_grid_ptr) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!out_value_grid_ptr) { throw ErrorCode::NullData; }
            if (m_width < 1 || m_height < 1) { throw ErrorCode::UnsupportedDimension; }

            auto value_grid = *out_value_grid_ptr;

            if (value_grid == nullptr) {
                value_grid = new(std::nothrow) ValueGridl(m_width, m_height);
                if (!value_grid) {
                    throw ErrorCode::MemCantAllocate;
                }
                *out_value_grid_ptr = value_grid;
            }

            if (value_grid->width() < m_width || value_grid->height() < m_height) {
                throw ErrorCode::UnsupportedDimension;
            }

            for (int32_t y = 0; y < m_height; y++) {
                readRow(y);
                for (int32_t x = 0; x < m_width; x++) {
                    auto value = m_row_values[x];
                    if (value == CVF2::kUndefinedValue) {
                        value_grid->setValueAtXY(x, y, std::numeric_limits<int64_t>::min());
                    }
                    else {
                        value_grid->setValueAtXY(x, y, value);
                    }
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        return result;
    }


    void CVF2File::logCVF2File(const String& cvf2_file_path, std::ostream& os) noexcept {
        try {
            CVF2File cvf2_file(cvf2_file_path);
            cvf2_file.startRead();
            Log l(os);
            cvf2_file.log(l);
        }
        catch (ErrorCode err) {
            std::cout << "CVF2File::logCVF2File err: " << (int)err << ", file_path: " << cvf2_file_path << std::endl;
        }
    }


    void CVF2File::cvf2ToImage(const String& cvf2_file_path, const String& image_file_path) noexcept {
        Image* image = nullptr;

        try {
            CVF2File cvf2_file(cvf2_file_path);
            cvf2_file.startRead();
            auto err = cvf2_file.buildImage(ImageScaleMode::Auto, 0, 1, &image, false);
            if (err != ErrorCode::None) {
                throw err;
            }
            image->writePng(image_file_path, 1.0f, true);
        }
        catch (ErrorCode err) {
            std::cout << "CVF2File::cvf2ToImage err: " << (int)err << ", cvf2_file_path: " << cvf2_file_path << ", image_file_path: " << image_file_path << std::endl;
        }
        delete image;
    }


    /**
     *  @brief Converts a batch of ValueGrid files to image files (JPEG format).
     *
     *  This function processes all `.cvf` files in the specified source directory (`src_dir_path`),
     *  reads the data from each file, builds an image using the `CVF2File` class, and writes
     *  the image file in the destination directory (`dst_dir_path`). It handles multiple
     *  files in a batch and outputs each converted image to the specified destination directory.
     *
     *  @param src_dir_path The source directory containing `.cvf` files to be converted.
     *  @param dst_dir_path The destination directory where the resulting images will be saved.
     *  @param scale_mode The scaling mode to apply to elevation data when generating the image.
     *                    - None: No scaling.
     *                    - Normalize: Scales values to fit the full intensity range.
     *                    - Custom: Applies user-defined min/max scaling.
     *  @param min_level Minimum elevation level used for scaling (only used if scale_mode is Custom).
     *  @param max_level Maximum elevation level used for scaling (only used if scale_mode is Custom).
     *  @param type The fourcc image type (e.g., 'jpg ', 'png ', 'webp').
     *
     *  @note This function ignores files larger than 10 GB, logs the number of files processed,
     *        and continues with the conversion process for each file. It uses the `CVF2File` class
     *        to load, process, and save the image.
     */
    void CVF2File::cvf2ToImageBatch(const String& src_dir_path, const String& dst_dir_path, ImageScaleMode scale_mode, float min_level, float max_level, fourcc_t type) noexcept {
        Image* image = nullptr;

        try {
            // Build list of all XYZFile file names
            StringList file_name_list;
            int32_t ignored_files_count;
            int32_t file_count = File::fileNameList(src_dir_path, "cvf", 1, Type::gigabytesToBytes(10), &ignored_files_count, file_name_list);

            int32_t index = 0;
            for (auto file_name : file_name_list) {
                std::cout << index << ": " << file_name << std::endl;
                String cvf2_file_path = src_dir_path + "/" + *file_name;
                CVF2File cvf2_file(cvf2_file_path);
                cvf2_file.startRead();
                cvf2_file.buildImage(scale_mode, min_level, max_level, &image, false);
                auto file_base_name = file_name->fileBaseNameWithoutExtension();

                String image_file_path = dst_dir_path + "/" + file_base_name + '.';
                if (image != nullptr) {
                    image->writeImage(image_file_path, type, 1.0f, true);
                }

                index++;
            }
        }
        catch (ErrorCode err) {
            std::cerr << "CVF2MapRenderer::cvf2ToImageBatch() err ..." << (int)err << std::endl;
        }

        delete image;
    }


} // End of namespace Grain
