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
            m_capacity = capacity > 0 ? capacity : 1;
            m_data = (T*)std::malloc(capacity * sizeof(T));
            m_use_external_mem = false;
        }

        RingBuffer(int64_t capacity, T* mem) noexcept {
            m_data = nullptr;
            m_use_external_mem = false;
            setupExternalMem(capacity, mem);
        }

        ~RingBuffer() noexcept override {
            if (!m_use_external_mem) {
                if (m_data) {
                    std::free(m_data);
                }
            }
        }

        [[nodiscard]] const char* className() const noexcept override { return "RingBuffer"; }

        friend std::ostream& operator << (std::ostream& os, const RingBuffer* o) {
            o == nullptr ? os << "Data nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RingBuffer& o) {
            os << "Ringbuffer m_capacity: " << o.m_capacity;
            os << ", m_read_pos: " << o.m_read_pos;
            os << ", m_write_pos: " << o.m_write_pos;
            return os;
        }

        [[nodiscard]] int64_t capacity() const noexcept { return m_capacity; }
        [[nodiscard]] int64_t readPos() const noexcept { return m_read_pos; }
        [[nodiscard]] int64_t writePos() const noexcept { return m_write_pos; }
        [[nodiscard]] T* mutDataPtr() const noexcept { return m_data; }
        [[nodiscard]] bool usesExternalMemory() const noexcept { return m_use_external_mem; }
        [[nodiscard]] bool isUsable() const noexcept { return m_data && m_capacity > 0; }

        void clear() noexcept {
            Type::clearArray<T>(m_data, m_capacity);
            m_read_pos = 0;
            m_write_pos = 0;
        }

        void clear(int64_t length) noexcept {
            if (m_data) {
                int64_t pos = m_write_pos;
                for (int64_t i = 0; i < length; i++) {
                    m_data[pos] = 0;
                    pos++;
                    if (pos >= m_capacity) {
                        pos = 0;
                    }
                }
            }
        }

        T read() noexcept {
            if (!m_data) {
                return 0;
            }
            else {
                int64_t pos = m_read_pos;
                m_read_pos++;
                if (m_read_pos >= m_capacity) {
                    m_read_pos = 0;
                }
                return m_data[pos];
            }
        }

        void read(int64_t length, int64_t step, T* out_values) noexcept {
            if (m_data && out_values) {
                float *d = out_values;
                for (int64_t i = 0; i < length; i++) {
                    *d = m_data[m_read_pos];
                    m_read_pos++;
                    if (m_read_pos >= m_capacity) {
                        m_read_pos = 0;
                    }
                    d += step;
                }
            }
        }

        int64_t setReadPos(int64_t pos) noexcept {
            if (pos >= m_capacity) {
                pos -= (pos / m_capacity) * m_capacity;
            }
            else if (pos < 0) {
                pos += (-pos / m_capacity + 1) * m_capacity;
            }
            m_read_pos = pos;
            return m_read_pos;
        }

        [[nodiscard]] T readAtRelativeIndex(int32_t index) const noexcept {
            return m_data ? m_data[relativeIndex(m_read_pos, index)] : 0;
        }

        void skipRead() noexcept {
            m_read_pos++;
            if (m_read_pos >= m_capacity) {
                m_read_pos = 0;
            }
        }

        void shiftReadPos(int64_t n) noexcept {
            setReadPos(m_read_pos + n);
        }



        void write(T value) noexcept {
            if (m_data) {
                m_data[m_write_pos] = value;
                m_write_pos++;
                if (m_write_pos >= m_capacity) {
                    m_write_pos = 0;
                }
            }
        }

        void write(const T* values, int64_t length) noexcept {
            if (m_data && values) {
                for (int64_t i = 0; i < length; i++) {
                    m_data[m_write_pos] = values[i];
                    m_write_pos++;
                    if (m_write_pos >= m_capacity) {
                        m_write_pos = 0;
                    }
                }
            }
        }

        void writeAtRelativeIndex(int64_t index, T value) noexcept {
            if (m_data) {
                m_data[relativeIndex(index)] = value;
            }
        }

        int64_t setWritePos(int64_t pos) noexcept {
            if (pos >= m_capacity) {
                pos -= (pos / m_capacity) * m_capacity;
            }
            else if (pos < 0) {
                pos += (-pos / m_capacity + 1) * m_capacity;
            }
            m_write_pos = pos;
            return m_write_pos;
        }

        void shiftWritePos(int64_t n) noexcept {
            setWritePos(m_write_pos + n);
        }

        void add(T value) noexcept {
            if (m_data) {
                m_data[m_write_pos] += value;
                m_write_pos++;
                if (m_write_pos >= m_capacity) {
                    m_write_pos = 0;
                }
            }
        }

        void add(const T* values, int64_t length) noexcept {
            if (m_data && values) {
                for (int64_t i = 0; i < length; i++) {
                    m_data[m_write_pos] += values[i];
                    m_write_pos++;
                    if (m_write_pos >= m_capacity) {
                        m_write_pos = 0;
                    }
                }
            }
        }

        void mul(T value) noexcept {
            if (m_data) {
                m_data[m_write_pos] *= value;
                m_write_pos++;
                if (m_write_pos >= m_capacity) {
                    m_write_pos = 0;
                }
            }
        }

        void mul(const T* values, int64_t length) noexcept {
            if (m_data && values) {
                for (int64_t i = 0; i < length; i++) {
                    m_data[m_write_pos] *= values[i];
                    m_write_pos++;
                    if (m_write_pos >= m_capacity) {
                        m_write_pos = 0;
                    }
                }
            }
        }

        void setupExternalMem(int64_t capacity, T* mem) noexcept {
            if (m_data && !m_use_external_mem) {
                std::free(m_data);
            }
            m_capacity = capacity;
            m_read_pos = 0;
            m_write_pos = 0;
            m_data = mem;
            m_use_external_mem = true;
        }

        [[nodiscard]] int64_t relativeIndex(int64_t pos, int64_t index) const noexcept {
            index += pos;
            if (index < 0) {
                index += ((-index / m_capacity + 1) * m_capacity);
            }
            else if (index >= m_capacity) {
                index -= (index / m_capacity) * m_capacity;
            }
            return index;
        }

    protected:
        int64_t m_capacity = 0;
        int64_t m_read_pos = 0;
        int64_t m_write_pos = 0;
        T* m_data = nullptr;
        bool m_use_external_mem = false;
    };


} // End of namespace Grain

#endif // GrainRingBuffer_hpp
