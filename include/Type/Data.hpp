//
//  Data.hpp
//
//  Created by Roald Christesen on 27.10.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainData_hpp
#define GrainData_hpp

#include "Type/Object.hpp"
#include "String/String.hpp"


namespace Grain {


    /**
     *  @class Data
     *  @brief A dynamically allocated byte buffer with automatic capacity growth.
     *
     *  The `Data` class provides a simple mechanism for managing a dynamically
     *  allocated memory buffer of bytes (`uint8_t*`). It supports automatic
     *  capacity growth when more space is required, making it suitable for
     *  scenarios like building binary data blocks or streaming buffers.
     *
     *  This class inherits from `Object` and offers accessors for its internal
     *  buffer, size, and growth step. Memory is automatically freed on destruction.
     */
    class Data : public Object {
    protected:
        int64_t m_capacity = 0;
        int64_t m_grow_step = 0;
        uint8_t* m_data = nullptr;

    public:
        Data(int64_t capacity = 1024) noexcept {
            m_capacity = std::max<int64_t>(capacity, int64_t(1));
            m_grow_step = 1024;
            if (m_capacity > 0) {
                m_data = (uint8_t*)calloc(m_capacity, 1);
                if (!m_data) {
                    m_capacity = 0;
                }
            }
        }

        ~Data() noexcept {
            std::free(m_data);
        }

        const char* className() const noexcept { return "Data"; }

        friend std::ostream& operator << (std::ostream& os, const Data* o) {
            o == nullptr ? os << "Data nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Data& o) {
            os << "Data m_capacity: " << o.m_capacity << ", m_grow_step: " << o.m_grow_step;
            return os;
        }

        /* TODO: !!!!!
        void log(Log& l) const {
            l << "Data m_capacity: " << m_capacity << ", m_grow_step: " << m_grow_step << Log::endl;
        }
         */

        [[nodiscard]] int64_t size() const noexcept { return m_capacity; }
        [[nodiscard]] int64_t growStep() const noexcept { return m_grow_step; }
        [[nodiscard]] uint8_t* data() const noexcept { return m_data; }

        void setGrowStep(int64_t grow_step) noexcept { m_grow_step = std::max<int64_t>(grow_step, 1024); }

        uint8_t* checkCapacity(int64_t size) noexcept {

            if (size > m_capacity) {
                int64_t new_capacity = m_capacity + m_grow_step;
                auto new_data = (uint8_t*)std::realloc(m_data, new_capacity);
                if (!new_data) {
                    return nullptr;
                }

                m_data = new_data;
                m_capacity = new_capacity;
            }

            return m_data;
        }
    };


/**
 *  @class Base64Data
 *  @brief Low-level handler for Base64 encoding and decoding operations.
 *
 *  The `Base64Data` class provides tools for encoding raw binary data into
 *  Base64 format and decoding Base64 back into binary. It manages internal
 *  state, handles padding, and tracks errors that may occur during processing.
 *
 *  This class is designed for internal use within Base64 encoding/decoding
 *  workflows.
 */
    class Base64Data {
    public:
        enum {
            kErr_None = 0,
            kErr_PaddingMismatch,
            kErr_PaddingMoreThanTwo,
            kErr_NoBase64Code,
            kErr_PostPaddingData
        };

    protected:
        int64_t m_base64_size = 0;  ///< Size of Base64 encoded data in bytes (Base64 characters)
        int32_t m_padding = 0;      ///< Number of padding chars
        int32_t m_last_err = 0;     ///< Last error code
        uint8_t m_bytes[3];
        uint8_t m_bits[4];
        unsigned char m_codes[4];
        int32_t m_byte_index = 0;

    public:
        [[nodiscard]] int32_t lastErr() { return m_last_err; }
        [[nodiscard]] int64_t base64Size() { return m_base64_size; }
        [[nodiscard]] int64_t rawDataSize() { return m_last_err != kErr_None ? -m_last_err : m_base64_size / 4 * 3 - m_padding; }
        [[nodiscard]] static int64_t rawDataMaxSize(int64_t base64_size) { return base64_size / 4 * 3; }

        [[nodiscard]] static uint8_t code2bits(const unsigned char code);
        [[nodiscard]] static unsigned char bits2code(uint8_t bits);
        bool _bits2code();
        static int32_t decodeBlock(const char* codes, uint8_t* out_bytes);
        bool encodeByte(uint8_t byte);
        bool encodeFinalize();
        [[nodiscard]] const char* codePtr() { return (const char*)m_codes; }

        int32_t _scanBase64Code(uint8_t code);
    };


} // End of namespace Grain

#endif // GrainData_hpp
