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
    std::free(cache_data_);
    std::free(row_seq_);
    std::free(row_values_);
}


void CVF2File::freeCache() noexcept {
    if (cache_data_) {
        std::free(cache_data_);
        cache_data_ = nullptr;
        cache_flag_ = false;
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

        cache_data_ = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * value_count));
        if (!cache_data_) {
            return ErrorCode::MemCantAllocate;
        }

        auto dst = static_cast<int64_t*>(cache_data_);
        for (int32_t y = 0; y < static_cast<int32_t>(height_); y++) {
            readRow(y);
            for (uint32_t x = 0; x < width_; x++) {
                *dst++ = row_values_[x];
            }
        }
    }
    catch (const Exception& e) {
        result = e.code();
    }

    return result;
}


int64_t CVF2File::valueFromCache(uint32_t x, uint32_t y) noexcept {
    if (x >= width_ || y >= height_ || !cache_data_) {
        return CVF2::kUndefinedValue;
    }
    else {
        return (static_cast<int64_t*>(cache_data_))[y * width_ + x];
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
    width_ = readValue<uint32_t>();
    height_ = readValue<uint32_t>();
    srid_ = readValue<int32_t>();

    readFix(xy_range_.min_x_);
    readFix(xy_range_.min_y_);
    readFix(xy_range_.max_x_);
    readFix(xy_range_.max_y_);

    undefined_values_count_ = readValue<uint32_t>();

    min_value_ = readValue<int64_t>();
    max_value_ = readValue<int64_t>();
    readFix(mean_value_);
    unit_ = (LengthUnit)readValue<int32_t>();

    row_offsets_pos_ = readValue<uint32_t>();
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

    if (pos.x_ < 0 || pos.x_ >= static_cast<int32_t>(width_)) {
        return CVF2::kUndefinedValue;
    }

    if (pos.y_ < 0 || pos.y_ >= static_cast<int32_t>(height_)) {
        return CVF2::kUndefinedValue;
    }

    if (cache_mode) {
        if (!cache_data_) {
            auto err = buildCacheData();
            if (err != ErrorCode::None) {
                return 0; // TODO: Error handling needed!
            }
        }
        return (static_cast<int64_t*>(cache_data_))[pos.y_ * width_ + pos.x_];
    }

    setPos(row_offsets_pos_ + pos.y_ * 4);
    auto row_offset = readValue<uint32_t>();

    setPos(row_offset);
    auto digits = readValue<uint16_t>();
    auto seq_count = readValue<uint32_t>();
    // uint32_t seq_index = 0; // Unused variable
    // uint32_t seq_offset = 0; // Unused variable
    int64_t seq_min = 0;

    uint32_t offset = 0;
    for (uint32_t i = 0; i < seq_count; i++) {
        if (i > 0) {
            offset = readValue<uint32_t>();
        }
        auto min = readValue<int64_t>();
        if (pos.x_ >= static_cast<int32_t>(offset)) {
            // seq_index = i;
            // seq_offset = offset;
            seq_min = min;
        }
    }

    uint32_t data_pos = (row_offset + 2 + 4 + seq_count * (4 + 8) - 4) + (pos.x_ * digits / 2);
    bool second_nibble_flag = ((digits % 2) != 0) && ((pos.x_ % 2) != 0);

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

    if (static_cast<int64_t>(diff) > max_diffs[digits]) {
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

    if (y < 0 || y >= static_cast<int32_t>(height_)) {
        Exception::throwSpecific(kErrYOutOfRange);
    }

    if (!row_values_) {
        row_values_ = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * width_));
        if (!row_values_) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }
    }

    setPos(row_offsets_pos_ + y * 4);
    auto row_offset = readValue<uint32_t>();

    setPos(row_offset);
    auto digits = readValue<uint16_t>();
    auto seq_count = readValue<uint32_t>();

    if (static_cast<int32_t>(seq_count) > row_seq_length_) {
        if (row_seq_) {
            std::free(row_seq_);
            row_seq_ = nullptr;
        }
        row_seq_ = (CVF2Sequence*)std::malloc(sizeof(CVF2Sequence) * seq_count);
        if (row_seq_) {
            row_seq_length_ = static_cast<int32_t>(seq_count);
        }
        else {
            Exception::throwSpecific(kErrRowSeqCantAlloc);
        }
    }

    row_seq_[0].offs_ = 0;
    for (uint32_t i = 0; i < seq_count; i++) {
        if (i > 0) {
            row_seq_[i].offs_ = readValue<uint32_t>();
            row_seq_[i - 1].length_ = row_seq_[i].offs_ - row_seq_[i - 1].offs_;
        }
        row_seq_[i].min_ = readValue<int64_t>();
    }
    row_seq_[seq_count - 1].length_ = width_ - row_seq_[seq_count - 1].offs_;


    uint32_t data_pos = (row_offset + 2 + 4 + seq_count * (4 + 8) - 4);
    setPos(data_pos);

    uint32_t seq_index = 0;
    auto seq_cd = static_cast<int32_t>(row_seq_[0].length_);
    int64_t seq_min = row_seq_[0].min_;

    bool second_nibble_flag = false;

    uint8_t byte = 0x0;
    for (int32_t x = 0; x < static_cast<int32_t>(width_); x++) {
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

        if (static_cast<int64_t>(diff) > max_diffs[digits]) {
            row_values_[x] = CVF2::kUndefinedValue;
        }
        else {
            row_values_[x] = seq_min + static_cast<int64_t>(diff);
        }

        if (seq_cd <= 0) {
            seq_index++;
            if (seq_index >= seq_count) {
                break;
            }
            seq_cd = row_seq_[seq_index].length_;
            seq_min = row_seq_[seq_index].min_;
        }
    }

    return width_;
}


ErrorCode CVF2File::xyzCompare(const String& xyz_file_path, int32_t z_decimals) noexcept {
    auto result = ErrorCode::None;
    XYZFile* xyz_file = nullptr;

    try {
        xyz_file = new (std::nothrow) XYZFile(xyz_file_path);
        if (!xyz_file) {
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

        // int32_t xyz_line_count = 0; // Unused
        String xyz_line;
        Vec3Fix xyz_coord;
        Fix z_value;

        while (xyz_file->readTrimmedLine(xyz_line)) {
            if (xyz_line.length() > 0) {
                if (!xyz_coord.setByCSV(xyz_line, ' ')) {
                    Exception::throwStandard(ErrorCode::UnexpectedData);
                }

                auto x = static_cast<int64_t>(round(xyz_coord.x_.asDouble() - xyz_min_x));
                auto y = static_cast<int64_t>(round(xyz_coord.y_.asDouble() - xyz_min_y));
                if (x < 0 || y < 0 || x > xyz_last_x || y >= xyz_last_y) {
                    Exception::throwSpecific(kErrXYOutOfRange);
                }

                int64_t value = valueAtPos(Vec2i(static_cast<int32_t>(x), static_cast<int32_t>(y)), true);
                z_value.setInt64(value, z_decimals);

                if (z_value != xyz_coord.z_) {
                    Exception::throwSpecific(kErrValueNotAsOriginal);
                }

                // xyz_line_count++; // Unused
            }
        }
    }
    catch (const Exception& e) {
        result = e.code();
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
        if (width_ < 1 || height_ < 1) {
            Exception::throwStandard(ErrorCode::UnsupportedDimension);
        }

        auto image = *out_image_ptr;
        if (image) {
            if (image->width() != static_cast<int32_t>(width_) || image->height() != static_cast<int32_t>(height_)) {
                delete image;
                image = *out_image_ptr = nullptr;
            }
        }

        if (!image) {
            image = Image::createLuminaAlphaFloat(static_cast<int32_t>(width_), static_cast<int32_t>(height_));
            if (!image) {
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
            min_level = static_cast<float>(min_value_);
            max_level = static_cast<float>(max_value_);
        }

        float range = (max_level - min_level);
        float scale = range != 0.0f ? 1.0f / range : 1.0f;

        int32_t y = flip_y ? ia.height() - 1 : 0;
        int32_t y_step = flip_y ? -1 : 1;

        while (ia.stepY()) {
            readRow(y);
            int32_t x = 0;

            while (ia.stepX()) {
                auto value = row_values_[x];
                if (value == CVF2::kUndefinedValue) {
                    pixel[0] = 1.0f;
                    pixel[1] = 0.0f;
                }
                else {
                    switch (scale_mode) {
                        case ImageScaleMode::None:
                            pixel[0] = row_values_[x];
                            pixel[1] = 1.0f;
                            break;
                        case ImageScaleMode::Auto:
                        case ImageScaleMode::MinMax:
                            pixel[0] = (row_values_[x] - min_level) * scale;
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
    catch (const Exception& e) {
        result = e.code();
    }
    catch (...) {
        result = ErrorCode::Unknown;
    }

    return result;
}


ErrorCode CVF2File::buildValueGrid(ValueGridl** out_value_grid_ptr) noexcept {
    auto result = ErrorCode::None;

    try {
        if (!out_value_grid_ptr) {
            Exception::throwStandard(ErrorCode::NullData);
        }

        if (width_ < 1 || height_ < 1) {
            Exception::throwStandard(ErrorCode::UnsupportedDimension);
        }

        if (!out_value_grid_ptr) {
            Exception::throwStandard(ErrorCode::NullPointer);
        }

        auto value_grid = *out_value_grid_ptr;
        if (!value_grid) {
            value_grid = new (std::nothrow) ValueGridl(width_, height_);
            if (!value_grid) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }
            *out_value_grid_ptr = value_grid;
        }

        if (value_grid->width() < static_cast<int32_t>(width_) ||
            value_grid->height() < static_cast<int32_t>(height_)) {
            Exception::throwStandard(ErrorCode::UnsupportedDimension);
        }

        for (int32_t y = 0; y < static_cast<int32_t>(height_); y++) {
            readRow(y);
            for (int32_t x = 0; x < static_cast<int32_t>(width_); x++) {
                auto value = row_values_[x];
                if (value == CVF2::kUndefinedValue) {
                    value_grid->setValueAtXY(x, y, std::numeric_limits<int64_t>::min());
                }
                else {
                    value_grid->setValueAtXY(x, y, value);
                }
            }
        }
    }
    catch (const Exception& e) {
        result = e.code();
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
    catch (const Exception& e) {
        std::cerr << "CVF2File::logCVF2File() err: " << (int32_t)e.code() << ", file_path: " << cvf2_file_path << std::endl;
    }
}


void CVF2File::cvf2ToImage(const String& cvf2_file_path, const String& image_file_path) noexcept {
    Image* image = nullptr;

    try {
        CVF2File cvf2_file(cvf2_file_path);
        cvf2_file.startRead();

        auto err = cvf2_file.buildImage(ImageScaleMode::Auto, 0, 1, &image, false);
        Exception::throwStandard(err);

        image->writePng(image_file_path, 1.0f, true);
    }
    catch (const Exception& e) {
        std::cerr << "CVF2File::cvf2ToImage() err: " << (int32_t)e.code() << ", cvf2_file_path: " << cvf2_file_path << ", image_file_path: " << image_file_path << std::endl;
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

        for (auto file_name : file_name_list) {
            String cvf2_file_path = src_dir_path + "/" + *file_name;
            CVF2File cvf2_file(cvf2_file_path);
            cvf2_file.startRead();

            cvf2_file.buildImage(scale_mode, min_level, max_level, &image, false);
            if (image) {
                auto file_base_name = file_name->fileBaseNameWithoutExtension();
                String image_file_path = dst_dir_path + "/" + file_base_name + '.';
                image->writeImage(image_file_path, type, 1.0f, true);
            }
        }
    }
    catch (const Exception& e) {
        std::cerr << "CVF2MapRenderer::cvf2ToImageBatch() err ..." << (int32_t)e.code() << std::endl;
    }

    delete image;
}


} // End of namespace Grain
