//
//  ValueGrid.hpp
//
//  Created by Roald Christesen on 07.05.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Data/ValueGrid.hpp"
#include "Image/Image.hpp"
#include "2d/Data/CVF2.hpp"


namespace Grain {


    template <typename T> int16_t ValueGrid<T>::valueDataType() const noexcept { return Type::kType_Undefined; }
    template <> int16_t ValueGrid<uint8_t>::valueDataType() const noexcept { return Type::kType_UInt8; }
    template <> int16_t ValueGrid<int32_t>::valueDataType() const noexcept { return Type::kType_Int32; }
    template <> int16_t ValueGrid<int64_t>::valueDataType() const noexcept { return Type::kType_Int64; }
    template <> int16_t ValueGrid<float>::valueDataType() const noexcept { return Type::kType_Float; }
    template <> int16_t ValueGrid<double>::valueDataType() const noexcept { return Type::kType_Double; }


    template <typename T>
    void ValueGrid<T>::setGeoInfo(int32_t srid, const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept {

        m_srid = srid;
        m_bbox.set(min_x, min_y, max_x, max_y);
        setFeature(kFeature_GeoInfo);
    }


    template <typename T>
    void ValueGrid<T>::setGeoInfo(int32_t srid, const RangeRectFix& bbox) noexcept {

        m_srid = srid;
        m_bbox = bbox;
        setFeature(kFeature_GeoInfo);
    }


    template <typename T>
    void ValueGrid<T>::setGeoInfo(int32_t srid, const RangeRectd& bbox) noexcept {

        m_srid = srid;
        m_bbox.set(bbox.m_min_x, bbox.m_min_y, bbox.m_max_x, bbox.m_max_y);
        setFeature(kFeature_GeoInfo);
    }


    /**
     *  @brief Downsampling of four value grids into one.
     */
    template <typename T>
    ErrorCode ValueGrid<T>::fourToOne(ValueGrid<T>* src_grids[4], uint8_t mask) noexcept {

        auto result = ErrorCode::None;

        try {
            if (m_values == nullptr) {
                _initMemThrow();
            }

            if (m_width % 2 != 0 || m_height % 2 != 0) {
                // Width and height must be a multiple of 2
                throw ErrorCode::UnsupportedDimension;
            }

            for (int32_t i = 0; i < 4; i++) {
                uint8_t bit = 0x1 << i;

                // Check all used grids
                if (bit & mask) {
                    if (src_grids[i] == nullptr) {
                        throw ErrorCode::NullData;
                    }

                    if (src_grids[i]->m_width != m_width || src_grids[i]->m_height != m_height) {
                        throw Error::specific(1);
                        throw ErrorCode::FormatMismatch;
                    }

                    if (src_grids[i]->hasFeature(kFeature_Invalid_Value) != hasFeature(kFeature_Invalid_Value)) {
                        throw Error::specific(2);
                        throw ErrorCode::FormatMismatch;
                    }

                    if (hasFeature(kFeature_Invalid_Value) && src_grids[i]->m_invalid_value != m_invalid_value) {
                        throw Error::specific(3);
                        throw ErrorCode::FormatMismatch;
                    }
                }
            }


            bool check_invalid = hasFeature(kFeature_Invalid_Value);

            for (int32_t src_y_index = 0; src_y_index < 2; src_y_index++) {

                for (int32_t src_x_index = 0; src_x_index < 2; src_x_index++) {
                    int32_t grid_index = src_y_index * 2 + src_x_index;
                    uint8_t bit = 0x1 << grid_index;


                    int32_t dst_x_offset = src_x_index * m_width / 2;
                    int32_t dst_y_offset = src_y_index * m_height / 2;

                    if (mask & bit) {
                        auto values = src_grids[grid_index]->m_values;

                        for (int32_t y = 0; y < m_height; y += 2) {

                            for (int32_t x = 0; x < m_width; x += 2) {
                                T sum{};
                                T v[4];

                                v[0] = values[_indexForXY(x, y)];
                                v[1] = values[_indexForXY(x + 1, y)];
                                v[2] = values[_indexForXY(x, y + 1)];
                                v[3] = values[_indexForXY(x + 1, y + 1)];

                                int32_t n = 0;
                                if (check_invalid) {

                                    for (int32_t i = 0; i < 4; i++) {
                                        if (v[i] != m_invalid_value) {
                                            sum += v[i];
                                            n++;
                                        }
                                    }
                                }
                                else {
                                    sum = v[0] + v[1] + v[2] + v[3];
                                    n = 4;
                                }

                                if (n == 0) {
                                    invalidateValueAtXY(dst_x_offset + x / 2, dst_y_offset + y / 2);
                                }
                                else {
                                    if (n > 1) {
                                        sum /= n;
                                    }
                                    setValueAtXY(dst_x_offset + x / 2, dst_y_offset + y / 2, sum);
                                }
                            }  // End of x loop
                        }   // End of y loop
                    }
                    else {
                        for (int32_t y = 0; y < m_height / 2; y++) {
                            for (int32_t x = 0; x < m_width / 2; x++) {
                                invalidateValueAtXY(dst_x_offset + x, dst_y_offset + y);
                            }
                        }
                    }
                }
            }

            updateMinMax();
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    /**
     *  @brief
     */
    template <typename T>
    ErrorCode ValueGrid<T>::fillMipmapQuadrant(const ValueGrid<T>* value_grid, int32_t quadrant_index) noexcept {

        auto result = ErrorCode::None;

        try {
            if (!value_grid) { throw ErrorCode::BadArgs; }
            if (quadrant_index < 0 || quadrant_index > 3) { throw ErrorCode::BadArgs; }

            // Width and height must be a multiple of 2
            if (m_width % 2 != 0 || m_height % 2 != 0) { throw ErrorCode::UnsupportedDimension; }

            if (value_grid->width() != m_width || value_grid->height() != m_height) {
                throw ErrorCode::UnsupportedDimension;
            }

            if (m_values == nullptr) {
                _initMemThrow();
            }

            int32_t dst_x_offset = quadrant_index & 0x1 ? m_width / 2 : 0;
            int32_t dst_y_offset = quadrant_index & 0x2 ? m_height / 2 : 0;

            auto values = value_grid->m_values;

            for (int32_t y = 0; y < m_height; y += 2) {

                for (int32_t x = 0; x < m_width; x += 2) {
                    T sum{};
                    T v[4];

                    v[0] = values[_indexForXY(x, y)];
                    v[1] = values[_indexForXY(x + 1, y)];
                    v[2] = values[_indexForXY(x, y + 1)];
                    v[3] = values[_indexForXY(x + 1, y + 1)];

                    int32_t n = 0;
                    for (int32_t i = 0; i < 4; i++) {
                        if (v[i] != m_invalid_value) {
                            sum += v[i];
                            n++;
                        }
                    }

                    if (n == 0) {
                        invalidateValueAtXY(dst_x_offset + x / 2, dst_y_offset + y / 2);
                    }
                    else {
                        if (n > 1) {
                            sum = (T)std::round(static_cast<double>(sum) / n);
                        }
                        setValueAtXY(dst_x_offset + x / 2, dst_y_offset + y / 2, sum);
                    }
                }  // End of x loop
            }   // End of y loop
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    /**
     *  @brief Write ValueGrid to a file.
     *
     *  @param file_path Path to the file where the ValueGrid should be written.
     *  @return `ErrorCode::None` on success, otherwise an error code.
     */
    template <typename T>
    ErrorCode ValueGrid<T>::writeFile(const String& file_path) noexcept {

        auto result = ErrorCode::None;

        File* file = nullptr;

        try {

            file = new(std::nothrow) File(file_path);
            if (file == nullptr) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            file->startWriteOverwrite();

            // Header
            file->writeStr(fileSignature());
            file->writeEndianSignature();

            // Version
            file->writeValue<uint16_t>(m_main_version);
            file->writeValue<uint16_t>(m_sub_version);

            // Data type
            file->writeValue<uint16_t>(valueDataType());

            // Dimension and position inside 2d tile array
            file->writeValue<int32_t>(m_width);
            file->writeValue<int32_t>(m_height);
            file->writeValue<int32_t>(m_x_index);
            file->writeValue<int32_t>(m_y_index);

            // Features
            file->writeValue<uint32_t>(m_feature_flags.bits());

            // Feature min/max
            if (hasFeature(kFeature_MinMax)) {
                _writeTypeValue(file, m_min_value);
                _writeTypeValue(file, m_max_value);
            }

            // Feature invalid value
            if (hasFeature(kFeature_Invalid_Value)) {
                _writeTypeValue(file, m_invalid_value);
            }

            // Feature Geo information
            if (hasFeature(kFeature_GeoInfo)) {
                file->writeValue<int32_t>(m_srid);
                file->writeFix(m_bbox.minX());
                file->writeFix(m_bbox.minY());
                file->writeFix(m_bbox.maxX());
                file->writeFix(m_bbox.maxY());
            }

            // Custom infos
            writeCustomInfo();

            // Save values
            _writeDataToFile(file);


            file->close();
        }
        catch (ErrorCode err) {
            result = err;
        }

        // Cleanup
        delete file;

        return result;
    }


    /**
     *  @brief Read ValueGrid from a file.
     *
     *  @param file_path Path to the file where the ValueGrid should be read from.
     *  @return `ErrorCode::None` on success, otherwise an error code.
     */
    template <typename T>
    ErrorCode ValueGrid<T>::readFile(const String& file_path) noexcept {

        auto result = ErrorCode::None;

        File* file = nullptr;

        try {

            file = new(std::nothrow) File(file_path);
            if (file == nullptr) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            file->startRead();

            // Header
            char buffer[kSignatureLength];
            file->readStr(kSignatureLength, buffer);
            file->checkSignature(buffer, kSignatureLength, fileSignature());

            file->readStr(2, buffer);
            file->setEndianBySignature(buffer);

            // Version
            m_main_version = file->readValue<uint16_t>();
            m_sub_version = file->readValue<uint16_t>();

            // Data type
            m_data_type = file->readValue<int16_t>();
            if (valueDataType() != m_data_type) {
                throw ErrorCode::UnsupportedDataType;
            }

            // Dimension and position inside 2d tile array
            m_width = file->readValue<int32_t>();
            m_height = file->readValue<int32_t>();
            m_value_count = m_width * m_height;
            m_x_index = file->readValue<int32_t>();
            m_y_index = file->readValue<int32_t>();

            // Features
            m_feature_flags.set(file->readValue<uint32_t>());

            // Feature min/max
            if (hasFeature(kFeature_MinMax)) {
                m_min_value = _readTypeValue(file);
                m_max_value = _readTypeValue(file);
            }

            // Feature invalid value
            if (hasFeature(kFeature_Invalid_Value)) {
                m_invalid_value = _readTypeValue(file);
            }

            // Feature Geo information
            if (hasFeature(kFeature_GeoInfo)) {
                m_srid = file->readValue<int32_t>();
                file->readFix(m_bbox.m_min_x);
                file->readFix(m_bbox.m_min_y);
                file->readFix(m_bbox.m_max_x);
                file->readFix(m_bbox.m_max_y);
            }

            // Custom infos
            readCustomInfo();

            // Read values
            _readDataFromFile(file);

            file->close();
        }
        catch (ErrorCode err) {
            result = err;
        }

        // Cleanup
        delete file;

        return result;
    }


    template <> void ValueGrid<uint8_t>::_writeDataToFile(File* file) {
        for (int32_t i = 0; i < m_value_count; i++) {
            file->writeValue<uint8_t>(m_values[i]);
        }
    }

    template <> void ValueGrid<int32_t>::_writeDataToFile(File* file) {
        for (int32_t i = 0; i < m_value_count; i++) {
            file->writeValue<int32_t>(m_values[i]);
        }
    }

    template <> void ValueGrid<int64_t>::_writeDataToFile(File* file) {
        for (int32_t i = 0; i < m_value_count; i++) {
            file->writeValue<int64_t>(m_values[i]);
        }
    }

    template <> void ValueGrid<float>::_writeDataToFile(File* file) {
        for (int32_t i = 0; i < m_value_count; i++) {
            file->writeValue<float>(m_values[i]);
        }
    }

    template <> void ValueGrid<double>::_writeDataToFile(File* file) {
        for (int32_t i = 0; i < m_value_count; i++) {
            file->writeValue<double>(m_values[i]);
        }
    }


    template <> void ValueGrid<uint8_t>::_writeTypeValue(File* file, uint8_t value) { file->writeValue<uint8_t>(value); }
    template <> void ValueGrid<int32_t>::_writeTypeValue(File* file, int32_t value) { file->writeValue<uint32_t>(value);}
    template <> void ValueGrid<int64_t>::_writeTypeValue(File* file, int64_t value) { file->writeValue<uint64_t>(value); }
    template <> void ValueGrid<float>::_writeTypeValue(File* file, float value) { file->writeValue<float>(value); }
    template <> void ValueGrid<double>::_writeTypeValue(File* file, double value) { file->writeValue<double>(value); }


    template <> void ValueGrid<uint8_t>::_readDataFromFile(File* file) {
        _initMemThrow();
        for (int32_t i = 0; i < m_value_count; i++) {
            m_values[i] = file->readValue<uint8_t>();
        }
    }

    template <> void ValueGrid<int32_t>::_readDataFromFile(File* file) {
        _initMemThrow();
        for (int32_t i = 0; i < m_value_count; i++) {
            m_values[i] = file->readValue<int32_t>();
        }
    }

    template <> void ValueGrid<int64_t>::_readDataFromFile(File* file) {
        _initMemThrow();
        for (int32_t i = 0; i < m_value_count; i++) {
            m_values[i] = file->readValue<int64_t>();
        }
    }

    template <> void ValueGrid<float>::_readDataFromFile(File* file) {
        _initMemThrow();
        for (int32_t i = 0; i < m_value_count; i++) {
            m_values[i] = file->readValue<float>();
        }
    }

    template <> void ValueGrid<double>::_readDataFromFile(File* file) {
        _initMemThrow();
        for (int32_t i = 0; i < m_value_count; i++) {
            m_values[i] = file->readValue<double>();
        }
    }

    template <> uint8_t ValueGrid<uint8_t>::_readTypeValue(File* file) { return file->readValue<uint8_t>(); }
    template <> int32_t ValueGrid<int32_t>::_readTypeValue(File* file) { return file->readValue<uint32_t>();}
    template <> int64_t ValueGrid<int64_t>::_readTypeValue(File* file) { return file->readValue<uint64_t>(); }
    template <> float ValueGrid<float>::_readTypeValue(File* file) { return file->readValue<float>(); }
    template <> double ValueGrid<double>::_readTypeValue(File* file) { return file->readValue<double>(); }


    /**
     *  @brief Writes the grid values to a CVF2 file at the specified path.
     *
     *  Exports the values stored in the `ValueGrid<int64_t>` to a CVF2 file format, which is used for
     *  storing grid data with specified precision and length units.
     *
     *  @param file_path The file path where the CVF2 file will be saved.
     *  @param length_unit The unit of length for the grid values, such as meters or kilometers.
     *  @param min_digits The minimum number of digits used in the compression algorithm (default 2).
     *  @param max_digits The maximum number of digits used in the compression algorithm (default 4).
     *  @return `ErrorCode` indicating the success or failure of the file write operation.
     */
    template <>
    ErrorCode ValueGrid<int64_t>::writeCVF2File(const String& file_path, LengthUnit length_unit, int32_t min_digits, int32_t max_digits) noexcept {

        auto result = ErrorCode::None;

        CVF2* cvf2 = nullptr;

        try {

            cvf2 = new(std::nothrow) CVF2(m_width, m_height, length_unit, min_digits, max_digits);
            if (cvf2 == nullptr) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            cvf2->setSRID(m_srid);
            cvf2->setUnit(length_unit);
            cvf2->setBbox(m_bbox);
            cvf2->openFileToWrite(file_path);

            for (int32_t y = 0; y < m_height; y++) {
                for (int32_t x = 0; x < m_width; x++) {
                    cvf2->pushValueToData(x, y, m_values[y * m_width + x]);
                }
            }

            cvf2->encodeData();
            cvf2->finish();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        delete cvf2;

        return result;
    }


    /**
     *  @brief Create an image from provided data.
     *
     *  @param flip_y If true, image will be created vertically flipped.
     *  @return A pointer to the created image if successful, or nullptr if creation failed.
     */
    template <typename T>
    Image* ValueGrid<T>::buildImage(bool flip_y) const noexcept {

        Image* image = nullptr;

        if (m_values != nullptr && m_width > 0 && m_height > 0) {

            image = Image::createLuminaFloat(m_width, m_height);

            if (image != nullptr) {
                image->setSampleValueRange(m_min_value, m_max_value);

                float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                ImageAccess ia(image, pixel);

                const T* src = nullptr;
                while (ia.stepY()) {

                    src = flip_y ? ptrForRow(ia.flippedY()) : ptrForRow(ia.y());

                    while (ia.stepX()) {
                        float v = static_cast<float>(*src++);
                        if (v < 0.0f) {
                            v = 0.0f;
                        }
                        pixel[0] = v;
                        ia.write();
                    }
                }
            }
        }

        return image;
    }


    /**
     *  @brief Creates an image from the grid data, using alpha transparency to mark undefined areas.
     *
     *  This function generates an image based on the values in the grid, with an alpha channel to indicate
     *  areas where values are undefined, specified by `undefined_value`.
     *
     *  @tparam T The type of values in the grid.
     *  @param undefined_value The value in the grid representing "undefined" data. These regions will be
     *         rendered with transparency in the output image.
     *  @param flip_y If true, the image will be vertically flipped during creation, which can be useful for
     *         coordinate system adjustments.
     *  @return A pointer to the created `Image` object if successful, or `nullptr` if image creation failed.
     *
     */
    template <typename T>
    Image* ValueGrid<T>::buildImageAlphaWhereUndefined(T undefined_value, bool flip_y) const noexcept {

        Image* image = nullptr;

        if (m_values != nullptr && m_width > 0 && m_height > 0) {

            image = Image::createLuminaAlphaFloat(m_width, m_height);

            int32_t undefined_value_count = 0;

            if (image != nullptr) {
                image->setSampleValueRange(m_min_value, m_max_value);

                float pixel[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                ImageAccess ia(image, pixel);

                const T* src = nullptr;
                while (ia.stepY()) {

                    src = flip_y ? ptrForRow(ia.flippedY()) : ptrForRow(ia.y());

                    while (ia.stepX()) {
                        T v =* src++;
                        if (v == undefined_value) {
                            pixel[0] = static_cast<float>(m_min_value);
                            pixel[1] = 0.0f;
                            undefined_value_count++;
                        }
                        else {
                            pixel[0] = static_cast<float>(v);
                            pixel[1] = 1.0f;
                        }
                        ia.write();
                    }
                }

                image->flipVertical();
            }
        }


        return image;
    }



    // Instantiate for specific types
    template class ValueGrid<uint8_t>;
    template class ValueGrid<int32_t>;
    template class ValueGrid<int64_t>;
    template class ValueGrid<float>;
    template class ValueGrid<double>;


} // End of namespace Grain
