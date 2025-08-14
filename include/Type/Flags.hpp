//
//  Flags.hpp
//
//  Created by Roald Christesen on 11.10.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 27.07.2025
//

#ifndef GrainFlags_hpp
#define GrainFlags_hpp

#include <cstdint>
#include <iostream>


namespace Grain {

    class String;


    /**
     *  @brief Flags with 32 individual bits.
     *
     *  Flags facilitates the management of 32 individual boolean flags. These flags
     *  can be toggled on or off, modified, compared, and otherwise manipulated.
     *  This class is particularly useful for scenarios where discrete settings or
     *  options need to be tracked and controlled in an organized manner.
     */
    class Flags {
    public:
        enum {
            kFlagsStrLength = 32 + 3 + 1  // 32 bits, 3 spaces, 1 EOL
        };

    public:
        Flags() noexcept : m_bits(0x0) {}
        Flags(uint32_t bits) noexcept : m_bits(bits) {}

        friend std::ostream& operator << (std::ostream& os, const Flags& o) {
            char buffer[Flags::kFlagsStrLength];
            o.toStr(buffer);
            return os << buffer;
        }

        Flags& operator = (uint32_t bits) noexcept {
            m_bits = bits;
            return *this;
        }

        // Conversion to uint32_t
        operator uint32_t() const noexcept { return m_bits; }

        bool operator == (const Flags& other) const { return m_bits == other.m_bits; }
        bool operator != (const Flags& other) const { return m_bits != other.m_bits; }

        // Bitwise OR, AND, XOR
        Flags operator | (Flags other) const noexcept { return Flags(m_bits | other.m_bits); }
        Flags& operator |= (Flags other) noexcept { m_bits |= other.m_bits; return *this; }
        Flags operator & (Flags other) const noexcept { return Flags(m_bits & other.m_bits); }
        Flags& operator &= (Flags other) noexcept { m_bits &= other.m_bits; return *this; }
        Flags operator ^ (Flags other) const noexcept { return Flags(m_bits ^ other.m_bits); }
        Flags& operator ^= (Flags other) noexcept { m_bits ^= other.m_bits; return *this; }


        [[nodiscard]] inline uint32_t bits() const noexcept { return m_bits; }

        [[nodiscard]] inline bool isSet(int32_t index) const noexcept {
            return m_bits & (0x1 << index);
        }

        [[nodiscard]] inline bool allSet(uint32_t flags) const noexcept {
            return (m_bits & flags) == flags;
        }

        [[nodiscard]] inline bool atLeastOneSet() const noexcept {
            return m_bits != 0x0;
        }

        [[nodiscard]] inline bool atLeastOneOfSet(uint32_t flags) const noexcept {
            return m_bits & flags;
        }

        bool clear() noexcept {
            bool changed = m_bits != 0x0;
            m_bits = 0x0;
            return changed;
        }

        bool clearFlag(int32_t index) noexcept {
            uint32_t prev = m_bits;
            m_bits &= ~(0x1 << index);
            return m_bits != prev;
        }

        bool clearFlags(uint32_t bits) noexcept {
            bits = m_bits & (~bits);
            bool changed = bits != m_bits;
            m_bits = bits;
            return changed;
        }

        bool set(const Flags& flags) noexcept {
            bool changed = flags.m_bits != m_bits;
            m_bits = flags.m_bits;
            return changed;
        }

        bool set(const char* str) noexcept;

        bool setFlag(int32_t index) noexcept {
            uint32_t prev = m_bits;
            m_bits |= (0x1 << index);
            return m_bits != prev;
        }

        bool setFlags(uint32_t bits) noexcept {
            bool changed = bits != m_bits;
            m_bits = bits;
            return changed;
        }

        bool toggleFlag(int32_t index) noexcept {
            uint32_t prev = m_bits;
            m_bits ^= (0x1 << index);
            return m_bits != prev;
        }

        bool toggleFlags(uint32_t flags) noexcept {
            uint32_t bits = m_bits ^ flags;
            bool changed = bits != m_bits;
            m_bits = bits;
            return changed;
        }

        void toStr(char* out_str) const noexcept;
        void toString(String& out_string) const noexcept;

    protected:
        uint32_t m_bits = 0x0;
    };


} // End of namespace Grain

#endif // Flags_hpp
