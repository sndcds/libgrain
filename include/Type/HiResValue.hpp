//
//  HiResValue.hpp
//
//  Created by Roald Christesen on 03.08.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 18.07.2025
//

#ifndef GrainHiResValue_hpp
#define GrainHiResValue_hpp

#include "Grain.hpp"


namespace Grain {

    /**
     *  @brief High resolution value.
     *
     *  `HiResValue` combines an integer component and a floating-point component to represent a precise position.
     *  This class is designed to facilitate data traversal with high accuracy, offering methods for stepping through positions.
     *
     *  This class is particularly useful in scenarios where precise navigation through data orpositions is required.
     */
    class HiResValue {
    public:
        HiResValue() noexcept = default;
        explicit HiResValue(int64_t pos) noexcept : m_i(pos) {}
        explicit HiResValue(double pos) noexcept {
            m_i = static_cast<int64_t>(pos);
            m_f = pos - static_cast<double>(m_i);
        }
        explicit HiResValue(int64_t i, double f) noexcept : m_i(i), m_f(f) {}
        explicit HiResValue(int64_t i, double f, int64_t step_i, double step_f) noexcept
            : m_i(i), m_f(f), m_si(step_i), m_sf(step_f) {}


        friend std::ostream& operator << (std::ostream& os, const HiResValue& o) {
            return os << o.m_i << " + " << o.m_f;
        }

        HiResValue& operator = (int64_t value) {
            m_i = value;
            m_f = 0.0;
            return *this;
        }

        HiResValue& operator = (double value) {
            m_i = static_cast<int64_t>(value);
            m_f = value - static_cast<double>(m_i);
            return *this; }

        bool operator == (const HiResValue& other) const {
            return m_i == other.m_i && m_f == other.m_f;
        }

        bool operator != (const HiResValue& other) const {
            return !(*this == other);
        }

        bool operator > (const HiResValue& other) const {
            if (m_i > other.m_i) return true;
            if (m_i < other.m_i) return false;
            return m_f > other.m_f;
        }

        bool operator < (const HiResValue& other) const {
            if (m_i < other.m_i) return true;
            if (m_i > other.m_i) return false;
            return m_f < other.m_f;
        }

        HiResValue operator + (double value) const noexcept {
            HiResValue result = *this;
            result.addDouble(value);
            return result;
        }

        HiResValue operator - (double value) const noexcept {
            HiResValue result = *this;
            result.addDouble(-value);
            return result;
        }

        HiResValue& operator += (double value) noexcept {
            addDouble(value);
            return *this;
        }

        HiResValue& operator += (const HiResValue& other) {
            m_i += other.m_i; m_f += other.m_f; _validate(); return *this;
        }

        HiResValue& operator -= (double value) noexcept {
            addDouble(-value);
            return *this;
        }

        HiResValue& operator -= (const HiResValue& other) {
            m_i += other.m_i; m_f += other.m_f; _validate(); return *this;
        }

        HiResValue& operator ++ () noexcept { // Prefix increment
            stepForward();
            return *this;
        }

        HiResValue operator ++ (int) noexcept { // Postfix increment
            HiResValue temp = *this;
            stepForward();
            return temp;
        }

        HiResValue& operator -- () noexcept { // Prefix decrement
            stepBackward();
            return *this;
        }

        HiResValue operator -- (int) noexcept { // Postfix decrement
            HiResValue temp = *this;
            stepBackward();
            return temp;
        }

        [[nodiscard]] inline double pos() const noexcept {
            return static_cast<double>(m_i) + m_f;
        }

        [[nodiscard]] inline float posf() const noexcept {
            return static_cast<float>(m_i) + static_cast<float>(m_f);
        }

        [[nodiscard]] inline double step() const noexcept {
            return static_cast<double>(m_si) + m_sf;
        }

        [[nodiscard]] inline float stepf() const noexcept {
            return static_cast<float>(m_si) + static_cast<float>(m_f);
        }


        void setPos(int64_t i, double f = 0.0) noexcept {
            m_i = i;
            m_f = f;
            _validate();
        }

        void setStep(int64_t step_i, double step_f = 0.0) noexcept {
            m_si = step_i;
            m_sf = step_f;
            _validateStep();
        }

        void addDouble(double value) noexcept {
            auto i = static_cast<int64_t>(value);
            double r = value - static_cast<double>(i);
            m_i += i;
            m_f += r;
            _validate();
        }

        void stepForward() noexcept {
            m_i += m_si;
            m_f += m_sf;
            _validate();
        }

        void stepBackward() noexcept {
            m_i -= m_si;
            m_f -= m_sf;
            _validate();
        }

        void _validate() noexcept {
            const auto temp = static_cast<int64_t>(m_f);
            if (temp != 0) {
                m_i += temp;
                m_f -= static_cast<double>(temp);
            }
            if (m_f < 0) {
                m_i -= 1;
                m_f += 1;
            }
        }

        void _validateStep() noexcept {
            const auto temp = static_cast<int64_t>(m_sf);
            if (temp != 0) {
                m_si += temp;
                m_sf -= static_cast<double>(temp);
            }
            if (m_sf < 0) {
                m_si -= 1;
                m_sf += 1;
            }
        }

    public:
        int64_t m_i = 0;      ///< Integer part
        double m_f = 0.0;     ///< Fractional part
        int64_t m_si = 1;     ///< Integer step
        double m_sf = 0.0;    ///< Fractional step
    };


} // End of namespace Grain

#endif // GrainHiResValue_hpp
