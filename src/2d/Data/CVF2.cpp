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
    width_ = width;
    height_ = height;
    unit_ = unit;
    min_digits_ = min_digits < 0 ? 2 : std::max(static_cast<uint32_t>(min_digits), 2U);
    max_digits_ = max_digits < 0 ? 4 : std::max(static_cast<uint32_t>(max_digits), min_digits_);
    nibble_count_ = 0;
    min_value_ = std::numeric_limits<int64_t>::max();
    max_value_ = std::numeric_limits<int64_t>::min();
    data_undef_n_ = 0;
    data_def_n_ = 0;
    data_sum_ = 0.0;

    // Allocate memory
    row_values_ = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * width));
    row_offsets_ = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * height));
    seq_offsets_ = static_cast<uint32_t*>(std::malloc(sizeof(uint32_t) * width));
    seq_mins_ = static_cast<int64_t*>(std::malloc(sizeof(int64_t) * width));
    byte_buffer_size_ = sizeof(uint8_t) * width * (max_digits / 2 + 1);
    byte_buffer_ = (uint8_t*)std::malloc(byte_buffer_size_);
}


CVF2::~CVF2() noexcept {
    std::free(row_values_);
    std::free(row_offsets_);
    std::free(seq_offsets_);
    std::free(seq_mins_);
    std::free(byte_buffer_);
    std::free(data_);

    delete file_;
}


void CVF2::openFileToWrite(const String& file_path) {
    if (file_) {
        Exception::throwSpecific(kErrFileAllreadyOpened);
    }

    file_ = new (std::nothrow) File(file_path);
    if (!file_) {
        Exception::throwStandard(ErrorCode::FileCantCreate);
    }

    file_data_saved_ = false;

    file_->startWriteOverwrite();
    file_->writeStr("CVF2");
    file_->writeEndianSignature();

    file_->writeValue<uint32_t>(width_);
    file_->writeValue<uint32_t>(height_);
    file_->writeValue<int32_t>(srid_);

    file_->writeFix(bbox_.minX());
    file_->writeFix(bbox_.minY());
    file_->writeFix(bbox_.maxX());
    file_->writeFix(bbox_.maxY());

    // Undefined values counter, will be replaced later
    // Remember position in file for later usage
    file_pos_undef_values_counter_ = file_->pos();
    file_->writeValue<int32_t>(-1);

    // Min and max values
    // Remember position in file for later usage
    file_pos_min_max_ = file_->pos();
    file_->writeValue<int64_t>(min_value_);
    file_->writeValue<int64_t>(max_value_);
    file_->writeFix(mean_value_);

    file_->writeValue<int32_t>(static_cast<int32_t>(unit_));

    file_pos_row_offsets_ = file_->pos();
    file_->writeValue<uint32_t>(0);  // Dummy value. Will be replaced later
}


void CVF2::pushValue(int64_t value) {
    if (curr_value_index_ >= static_cast<int32_t>(width_)) {
        Exception::throwSpecific(kErrFatal);
    }

    row_values_[curr_value_index_] = value;
    curr_value_index_++;

    if (curr_value_index_ >= static_cast<int32_t>(width_)) {
        _startRow();
        _encodeRow(row_values_);
    }
}


void CVF2::pushValueToData(int32_t x, int32_t y, int64_t value) {
    if (!data_) {
        auto n = (size_t)width_ * height_;
        data_ = static_cast<int64_t*>(malloc(sizeof(int64_t) * n));
        if (!data_) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }
        Type::fillStridedArray<int64_t>(data_, 0, 1, n, n, kUndefinedValue);
    }

    if (x < 0 || x >= static_cast<int32_t>(width_) ||
        y < 0 || y >= static_cast<int32_t>(height_)) {
        Exception::throwStandard(ErrorCode::BadArgs);
    }

    data_[(size_t)y * width_ + x] = value;
}


void CVF2::encodeData() {
    uint32_t w = width_;
    uint32_t h = height_;
    int64_t* src = data_;

    for (uint32_t y = 0; y < h; y++) {
        _startRow();
        _encodeRow(&src[(size_t)y * w]);
    }
}


void CVF2::finish() {
    // Update information in header
    file_->setPos(file_pos_undef_values_counter_);
    file_->writeValue<int32_t>(data_undef_n_);

    file_->setPos(file_pos_min_max_);
    file_->writeValue<int64_t>(min_value_);
    file_->writeValue<int64_t>(max_value_);

    if (data_def_n_ > 0) {
        mean_value_ = data_sum_ / data_def_n_;
    }

    file_->writeFix(mean_value_);

    file_->close();
}


void CVF2::_startRow() {
    if (!row_values_ || !row_offsets_) {
        Exception::throwSpecific(kErrFatal);
    }

    curr_row_index_++;

    if (curr_row_index_ == static_cast<int32_t>(height_)) {
        Exception::throwSpecific(kErrFatal);
    }

    curr_value_index_ = 0;
    high_nibble_flag_ = true;
}


uint8_t CVF2::_extractNibble(uint64_t value, int32_t position) const {
    return static_cast<uint8_t>((value >> ((curr_row_digits_ - position - 1) * 4)) & 0xF);
}


void CVF2::_pushNibble(uint8_t nibble) {
    if (curr_byte_index_ >= static_cast<int32_t>(byte_buffer_size_)) {
        Exception::throwSpecific(kErrFatal);
    }

    if (high_nibble_flag_) {
        byte_buffer_[curr_byte_index_] = ((nibble & 0xF) << 4);
        high_nibble_flag_ = false;
    }
    else {
        byte_buffer_[curr_byte_index_] |= nibble & 0xF;
        high_nibble_flag_ = true;
        curr_byte_index_++;
    }

    nibble_count_++;
}


void CVF2::_bufferValues(const int64_t* values, uint32_t seq_index, uint32_t seq_offset, uint32_t seq_length, int64_t min_value) {
    seq_offsets_[seq_index] = seq_offset;
    seq_mins_[seq_index] = min_value;

    for (uint32_t i = 0; i < seq_length; i++) {
        int64_t value = values[curr_row_value_index_];

        if (value == kUndefinedValue) {
            value = curr_row_max_diff_ + 1;
        }
        else {
            value -= min_value;
        }

        if (value > curr_row_max_diff_ + 1) {
            Exception::throwStandard(ErrorCode::Fatal);
        }

        for (int32_t j = 0; j < curr_row_digits_; j++) {
            _pushNibble(_extractNibble(value, j));
        }

        curr_row_value_index_++;
    }
}


void CVF2::_encodeRow(const int64_t* values) {
    // Pass 1, find best compression for row data
    int64_t byte_count = std::numeric_limits<int64_t>::max();
    curr_row_digits_ = 0;

    for (auto d = min_digits_; d <= max_digits_; d++) {
        int32_t seq_count_result;
        int64_t byte_count_result;
        _encoderRowPrediction(values, d, seq_count_result, byte_count_result);

        if (byte_count_result < byte_count) {
            byte_count = byte_count_result;
            curr_row_digits_ = d;
        }

        if (seq_count_result <= 1) {
            break;
        }
    }

    if (curr_row_digits_ == 0) {
        Exception::throwSpecific(kErrUnknownDigits);
    }

    curr_row_max_diff_ = _maxDiff(curr_row_digits_);

    // Pass 2, prepare the byte buffer
    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();

    uint32_t seq_count = 0;
    uint32_t seq_length = 0;
    uint32_t seq_sum = 0;

    curr_row_value_index_ = 0;
    curr_byte_index_ = 0;
    nibble_count_ = 0;

    for (int32_t i = 0; i < static_cast<int32_t>(width_); i++) {
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
            range_flag = range > curr_row_max_diff_;

            // Statistics
            if (value < min_value_) {
                min_value_ = value;
            }
            if (value > max_value_) {
                max_value_ = value;
            }
            data_sum_ += static_cast<double>(value);
            data_def_n_++;
        }
        else {
            // Statistics
            data_undef_n_++;
        }

        bool stop = i == (static_cast<int32_t>(width_) - 1);

        if (range_flag) {
            _bufferValues(values, seq_count, seq_sum, seq_length, prev_min);
            seq_sum += seq_length;
            seq_count++;
            min = std::numeric_limits<int64_t>::max();
            max = std::numeric_limits<int64_t>::min();
            seq_length = 0;
            data_def_n_--;
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
    row_offsets_[curr_row_index_] = file_->pos();

    // Write row data to file
    file_->writeValue<uint16_t>(curr_row_digits_);
    file_->writeValue<uint32_t>(seq_count);

    for (int32_t i = 0; i < static_cast<int32_t>(seq_count); i++) {
        if (i > 0) {
            // First offset is allways 0, therefor it is skipped
            file_->writeValue<uint32_t>(seq_offsets_[i]);
        }
        file_->writeValue<int64_t>(seq_mins_[i]);
    }

    file_->writeData<uint8_t>(byte_buffer_, curr_byte_index_ + 1);


    // When the last row was encoded, save row index table and update the file header
    if (curr_row_index_ == static_cast<int32_t>(height_) - 1) {
        int64_t row_offsets_pos = file_->pos();

        for (int32_t i = 0; i < static_cast<int32_t>(height_); i++) {
            file_->writeValue<uint32_t>(static_cast<uint32_t>(row_offsets_[i]));
        }

        file_->setPos(file_pos_row_offsets_);
        file_->writeValue<uint32_t>(static_cast<uint32_t>(row_offsets_pos));

        file_data_saved_ = true;
    }
}


bool CVF2::_encoderRowPrediction(
        const int64_t* values,
        int32_t digits,
        int32_t& seq_count,
        int64_t& byte_count)
const {
    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();
    int64_t max_diff = _maxDiff(digits);
    int32_t seq_length = 0;
    int64_t nibble_count = 0;

    seq_count = 1;

    for (int32_t i = 0; i < static_cast<int32_t>(width_); i++) {

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

        bool stop = i == (static_cast<int32_t>(width_) - 1);

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
    byte_count += 2; // 2 bytes for number of digits used
    byte_count += seq_count * (4 + 8) - 4; // Offset and min value for each sequence

    return true;
}


} // End of namespace Grain
