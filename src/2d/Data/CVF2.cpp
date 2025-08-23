//
//  CVF2.cpp
//
//  Created by Roald Christesen on 05.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Data//CVF2.hpp"
#include "Type/Type.hpp"


namespace Grain {

    CVF2::CVF2(int32_t width, int32_t height, LengthUnit unit, int32_t min_digits, int32_t max_digits) noexcept {
        m_width = width;
        m_height = height;
        m_unit = unit;
        m_min_digits = min_digits < 0 ? 2 : std::max(static_cast<uint32_t>(min_digits), 2U);
        m_max_digits = max_digits < 0 ? 4 : std::max(static_cast<uint32_t>(max_digits), m_min_digits);
        m_nibble_count = 0;
        m_min_value = std::numeric_limits<int64_t>::max();
        m_max_value = std::numeric_limits<int64_t>::min();
        m_data_undef_n = 0;
        m_data_def_n = 0;
        m_data_sum = 0.0;

        // Allocate memory
        m_row_values = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * width));
        m_row_offsets = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * height));
        m_seq_offsets = static_cast<uint32_t*>(std::malloc(sizeof(uint32_t) * width));
        m_seq_mins = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * width));
        m_byte_buffer_size = sizeof(uint8_t) * width * (max_digits / 2 + 1);
        m_byte_buffer = (uint8_t*)std::malloc(m_byte_buffer_size);
    }


    CVF2::~CVF2() noexcept {
        std::free(m_row_values);
        std::free(m_row_offsets);
        std::free(m_seq_offsets);
        std::free(m_seq_mins);
        std::free(m_byte_buffer);
        std::free(m_data);

        delete m_file;
    }


    void CVF2::openFileToWrite(const String& file_path) {
        if (m_file != nullptr) {
            throw Error::specific(kErrFileAllreadyOpened);
        }

        m_file = new (std::nothrow) File(file_path);
        if (m_file == nullptr) {
            throw ErrorCode::FileCantCreate;
        }

        m_file_data_saved = false;

        m_file->startWriteOverwrite();
        m_file->writeStr("CVF2");
        m_file->writeEndianSignature();

        m_file->writeValue<uint32_t>(m_width);
        m_file->writeValue<uint32_t>(m_height);
        m_file->writeValue<int32_t>(m_srid);

        m_file->writeFix(m_bbox.minX());
        m_file->writeFix(m_bbox.minY());
        m_file->writeFix(m_bbox.maxX());
        m_file->writeFix(m_bbox.maxY());

        // Undefined values counter, will be replaced later
        // Remember position in file for later usage
        m_file_pos_undef_values_counter = m_file->pos();
        m_file->writeValue<int32_t>(-1);

        // Min and max values
        // Remember position in file for later usage
        m_file_pos_min_max = m_file->pos();
        m_file->writeValue<int64_t>(m_min_value);
        m_file->writeValue<int64_t>(m_max_value);
        m_file->writeFix(m_mean_value);

        m_file->writeValue<int32_t>(static_cast<int32_t>(m_unit));

        m_file_pos_row_offsets = m_file->pos();
        m_file->writeValue<uint32_t>(0);  // Dummy value. Will be replaced later
    }


    void CVF2::pushValue(int64_t value) {

        if (m_curr_value_index >= static_cast<int32_t>(m_width)) {
            throw Error::specific(kErrFatal);
        }

        m_row_values[m_curr_value_index] = value;
        m_curr_value_index++;

        if (m_curr_value_index >= static_cast<int32_t>(m_width)) {
            _startRow();
            _encodeRow(m_row_values);
        }
    }


    void CVF2::pushValueToData(int32_t x, int32_t y, int64_t value) {

        if (m_data == nullptr) {
            auto n = (size_t)m_width * m_height;
            m_data = static_cast<int64_t*>(malloc(sizeof(int64_t) * n));
            if (m_data == nullptr) {
                throw ErrorCode::MemCantAllocate;
            }
            Type::fillStridedArray<int64_t>(m_data, 0, 1, n, n, kUndefinedValue);
        }

        if (x < 0 || x >= static_cast<int32_t>(m_width) ||
            y < 0 || y >= static_cast<int32_t>(m_height)) {
            throw ErrorCode::BadArgs;
        }

        m_data[(size_t)y * m_width + x] = value;
    }


    void CVF2::encodeData() {

        uint32_t w = m_width;
        uint32_t h = m_height;
        int64_t* src = m_data;

        for (uint32_t y = 0; y < h; y++) {
            _startRow();
            _encodeRow(&src[(size_t)y * w]);
        }
    }


    void CVF2::finish() {

        // Update information in header
        m_file->setPos(m_file_pos_undef_values_counter);
        m_file->writeValue<int32_t>(m_data_undef_n);

        m_file->setPos(m_file_pos_min_max);
        m_file->writeValue<int64_t>(m_min_value);
        m_file->writeValue<int64_t>(m_max_value);

        std::cout << ">>>>>>> m_data_def_n: " << m_data_def_n << std::endl;
        std::cout << ">>>>>>> m_data_undef_n: " << m_data_undef_n << std::endl;
        std::cout << ">>>>>>> m_data_sum: " << m_data_sum << std::endl;

        if (m_data_def_n > 0) {
            m_mean_value = m_data_sum / m_data_def_n;
        }
        std::cout << ">>>>>>> m_mean_value: " << m_mean_value << std::endl;
        m_file->writeFix(m_mean_value);

        m_file->close();
    }


    void CVF2::_startRow() {

        if (m_row_values == nullptr || m_row_offsets == nullptr) {
            throw Error::specific(kErrFatal);
        }

        m_curr_row_index++;

        if (m_curr_row_index == static_cast<int32_t>(m_height)) {
            throw Error::specific(kErrFatal);
        }

        m_curr_value_index = 0;
        m_high_nibble_flag = true;
    }


    uint8_t CVF2::_extractNibble(uint64_t value, int32_t position) {
        return static_cast<uint8_t>((value >> ((m_curr_row_digits - position - 1) * 4)) & 0xF);
    }


    void CVF2::_pushNibble(uint8_t nibble) {

        if (m_curr_byte_index >= static_cast<int32_t>(m_byte_buffer_size)) {
            throw Error::specific(kErrFatal);
        }

        if (m_high_nibble_flag) {
            m_byte_buffer[m_curr_byte_index] = ((nibble & 0xF) << 4);
            m_high_nibble_flag = false;
        }
        else {
            m_byte_buffer[m_curr_byte_index] |= nibble & 0xF;
            m_high_nibble_flag = true;
            m_curr_byte_index++;
        }

        m_nibble_count++;
    }


    void CVF2::_bufferValues(const int64_t* values, uint32_t seq_index, uint32_t seq_offset, uint32_t seq_length, int64_t min_value) {

        m_seq_offsets[seq_index] = seq_offset;
        m_seq_mins[seq_index] = min_value;

        for (uint32_t i = 0; i < seq_length; i++) {
            int64_t value = values[m_curr_row_value_index];

            if (value == kUndefinedValue) {
                value = m_curr_row_max_diff + 1;
            }
            else {
                value -= min_value;
            }

            if (value > m_curr_row_max_diff + 1) {
                throw ErrorCode::Fatal;
            }

            for (int32_t j = 0; j < m_curr_row_digits; j++) {
                _pushNibble(_extractNibble(value, j));
            }

            m_curr_row_value_index++;
        }
    }


    void CVF2::_encodeRow(const int64_t* values) {

        // Pass 1, find best compression for row data
        int64_t byte_count = std::numeric_limits<int64_t>::max();
        m_curr_row_digits = 0;

        for (auto d = m_min_digits; d <= m_max_digits; d++) {
            int32_t seq_count_result;
            int64_t byte_count_result;
            _encoderRowPrediction(values, d, seq_count_result, byte_count_result);

            if (byte_count_result < byte_count) {
                byte_count = byte_count_result;
                m_curr_row_digits = d;
            }

            if (seq_count_result <= 1) {
                break;
            }
        }

        if (m_curr_row_digits == 0) {
            throw Error::specific(kErrUnknownDigits);
        }

        m_curr_row_max_diff = _maxDiff(m_curr_row_digits);

        // Pass 2, prepare the byte buffer
        int64_t min = std::numeric_limits<int64_t>::max();
        int64_t max = std::numeric_limits<int64_t>::min();

        uint32_t seq_count = 0;
        uint32_t seq_length = 0;
        uint32_t seq_sum = 0;

        m_curr_row_value_index = 0;
        m_curr_byte_index = 0;
        m_nibble_count = 0;

        for (int32_t i = 0; i < static_cast<int32_t>(m_width); i++) {
            int64_t value = values[i];
            bool range_flag = false;
            int64_t prev_min = min;

            if (value != kUndefinedValue) {
                if (value < min) {
                    min = value;
                }
                if (value > max) {
                    max = value;
                }

                int64_t range = max - min;
                range_flag = range > m_curr_row_max_diff;

                // Statistics
                if (value < m_min_value) {
                    m_min_value = value;
                }
                if (value > m_max_value) {
                    m_max_value = value;
                }
                m_data_sum += value;
                m_data_def_n++;
            }
            else {
                // Statistics
                m_data_undef_n++;
            }

            bool stop = i == (static_cast<int32_t>(m_width) - 1);

            if (range_flag) {
                _bufferValues(values, seq_count, seq_sum, seq_length, prev_min);
                seq_sum += seq_length;
                seq_count++;
                min = std::numeric_limits<int64_t>::max();
                max = std::numeric_limits<int64_t>::min();
                seq_length = 0;
                m_data_def_n--;
                i--;
            }
            else if (stop) {
                seq_length += 1;
                _bufferValues(values, seq_count, seq_sum, seq_length, min);
                seq_sum += seq_length;
                seq_count++;
                break;
            }
            else {
                seq_length++;
            }
        }

        // Save file position for the row for later use
        m_row_offsets[m_curr_row_index] = m_file->pos();

        // Write row data to file
        m_file->writeValue<uint16_t>(m_curr_row_digits);
        m_file->writeValue<uint32_t>(seq_count);

        for (int32_t i = 0; i < static_cast<int32_t>(seq_count); i++) {
            if (i > 0) {
                // First offset is allways 0, therefor it is skipped
                m_file->writeValue<uint32_t>(m_seq_offsets[i]);
            }
            m_file->writeValue<int64_t>(m_seq_mins[i]);
        }

        m_file->writeData<uint8_t>(m_byte_buffer, m_curr_byte_index + 1);


        // When the last row was encoded, save row index table and update the file header
        if (m_curr_row_index == static_cast<int32_t>(m_height) - 1) {
            int64_t row_offsets_pos = m_file->pos();

            for (int32_t i = 0; i < static_cast<int32_t>(m_height); i++) {
                m_file->writeValue<uint32_t>(static_cast<uint32_t>(m_row_offsets[i]));
            }

            m_file->setPos(m_file_pos_row_offsets);
            m_file->writeValue<uint32_t>(static_cast<uint32_t>(row_offsets_pos));

            m_file_data_saved = true;
        }
    }


    bool CVF2::_encoderRowPrediction(const int64_t* values, int32_t digits, int32_t& seq_count, int64_t& byte_count) {

        int64_t min = std::numeric_limits<int64_t>::max();
        int64_t max = std::numeric_limits<int64_t>::min();
        int64_t max_diff = _maxDiff(digits);
        int32_t seq_length = 0;
        int64_t nibble_count = 0;

        seq_count = 1;

        for (int32_t i = 0; i < static_cast<int32_t>(m_width); i++) {

            int64_t value = values[i];
            bool range_flag = false;

            if (value != kUndefinedValue) {

                if (value < min) {
                    min = value;
                }
                if (value > max) {
                    max = value;
                }

                int64_t range = max - min;
                range_flag = range > max_diff;
            }

            bool stop = i == (static_cast<int32_t>(m_width) - 1);

            if (range_flag) {
                nibble_count += seq_length * digits;
                seq_count++;
                min = std::numeric_limits<int64_t>::max();
                max = std::numeric_limits<int64_t>::min();
                seq_length = 0;
                i -= 1;
            }
            else if (stop) {
                seq_length += 1;
                nibble_count += seq_length * digits;
                seq_count++;
                break;
            }
            else {
                seq_length++;
            }
        }

        byte_count = (nibble_count + 1) / 2;
        byte_count += 2;  // 2 bytes for number of digits used
        byte_count += seq_count * (4 + 8) - 4;  // Offset and min value for each sequence

        return true;
    }


} // End of namespace Grain
