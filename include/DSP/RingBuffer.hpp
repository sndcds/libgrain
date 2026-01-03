//
//  RingBuffer.hpp
//
//  Created by Roald Christesen on 03.03.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.07.2025
//

#ifndef GrainRingBuffer_hpp
#define GrainRingBuffer_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/Type.hpp"


namespace Grain {

    template <typename T>
    class RingBuffer : public Object {
    public:
        RingBuffer() noexcept = default;

        explicit RingBuffer(int64_t capacity) noexcept {
            capacity_ = capacity > 0 ? capacity : 1;
            data_ = (T*)std::malloc(capacity * sizeof(T));
            use_external_mem_ = false;
        }

        RingBuffer(int64_t capacity, T* mem) noexcept {
            data_ = nullptr;
            use_external_mem_ = false;
            setupExternalMem(capacity, mem);
        }

        ~RingBuffer() noexcept override {
            if (!use_external_mem_ && data_) {
                std::free(data_);
            }
        }

        [[nodiscard]] const char* className() const noexcept override { return "RingBuffer"; }

        friend std::ostream& operator << (std::ostream& os, const RingBuffer* o) {
            o == nullptr ? os << "Data nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RingBuffer& o) {
            os << "Ringbuffer m_capacity: " << o.capacity_;
            os << ", m_read_pos: " << o.read_pos_;
            os << ", m_write_pos: " << o.write_pos_;
            return os;
        }

        [[nodiscard]] int64_t capacity() const noexcept { return capacity_; }
        [[nodiscard]] int64_t readPos() const noexcept { return read_pos_; }
        [[nodiscard]] int64_t writePos() const noexcept { return write_pos_; }
        [[nodiscard]] T* mutDataPtr() const noexcept { return data_; }
        [[nodiscard]] bool usesExternalMemory() const noexcept { return use_external_mem_; }
        [[nodiscard]] bool isUsable() const noexcept { return data_ && capacity_ > 0; }

        int64_t setReadPos(int64_t pos) noexcept {
            read_pos_ = ((pos % capacity_) + capacity_) % capacity_;
            return read_pos_;
        }

        int64_t setWritePos(int64_t pos) noexcept {
            write_pos_ = ((pos % capacity_) + capacity_) % capacity_;
            return write_pos_;
        }

        void shiftReadPos(int64_t n) noexcept {
            setReadPos(read_pos_ + n);
        }

        void shiftWritePos(int64_t n) noexcept {
            setWritePos(write_pos_ + n);
        }

        T read() noexcept {
            int64_t pos = read_pos_;
            read_pos_++;
            if (read_pos_ >= capacity_) {
                read_pos_ = 0;
            }
            return data_[pos];
        }

        void read(int64_t length, int64_t stride, T* out_values) noexcept {
            if (out_values) {
                float *d = out_values;
                for (int64_t i = 0; i < length; i++) {
                    *d = data_[read_pos_];
                    read_pos_++;
                    if (read_pos_ >= capacity_) {
                        read_pos_ = 0;
                    }
                    d += stride;
                }
            }
        }

        void skipRead() noexcept {
            read_pos_++;
            if (read_pos_ >= capacity_) {
                read_pos_ = 0;
            }
        }


        void write(T value) noexcept {
            data_[write_pos_] = value;
            write_pos_++;
            if (write_pos_ >= capacity_) {
                write_pos_ = 0;
            }
        }

        void writeZeros(int64_t length) noexcept {
            for (int64_t i = 0; i < length; i++) {
                data_[write_pos_] = 0;
                write_pos_++;
                if (write_pos_ >= capacity_) {
                    write_pos_ = 0;
                }
            }
        }

        void write(const T* values, int64_t length) noexcept {
            if (values) {
                for (int64_t i = 0; i < length; i++) {
                    data_[write_pos_] = values[i];
                    write_pos_++;
                    if (write_pos_ >= capacity_) {
                        write_pos_ = 0;
                    }
                }
            }
        }

        void add(T value) noexcept {
            data_[write_pos_] += value;
            write_pos_++;
            if (write_pos_ >= capacity_) {
                write_pos_ = 0;
            }
        }

        void add(const T* values, int64_t length) noexcept {
            if (values) {
                for (int64_t i = 0; i < length; i++) {
                    data_[write_pos_] += values[i];
                    write_pos_++;
                    if (write_pos_ >= capacity_) {
                        write_pos_ = 0;
                    }
                }
            }
        }

        void mul(T value) noexcept {
            data_[write_pos_] *= value;
            write_pos_++;
            if (write_pos_ >= capacity_) {
                write_pos_ = 0;
            }
        }

        void mul(const T* values, int64_t length) noexcept {
            if (values) {
                for (int64_t i = 0; i < length; i++) {
                    data_[write_pos_] *= values[i];
                    write_pos_++;
                    if (write_pos_ >= capacity_) {
                        write_pos_ = 0;
                    }
                }
            }
        }

        void setupExternalMem(int64_t capacity, T* mem) noexcept {
            if (data_ && !use_external_mem_) {
                std::free(data_);
            }
            capacity_ = capacity;
            read_pos_ = 0;
            write_pos_ = 0;
            data_ = mem;
            use_external_mem_ = true;
        }

    protected:
        int64_t capacity_ = 0;
        int64_t read_pos_ = 0;
        int64_t write_pos_ = 0;
        T* data_ = nullptr;
        bool use_external_mem_ = false;
    };


} // End of namespace Grain

#endif // GrainRingBuffer_hpp
