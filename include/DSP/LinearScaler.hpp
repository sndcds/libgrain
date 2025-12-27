//
//  LinearScaler.hpp
//
//  Created by Roald Christesen on 24.03.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainLinearScaler_hpp
#define GrainLinearScaler_hpp

#include "Grain.hpp"

#include <type_traits>
#include <limits>
#include <cmath>


namespace Grain {

    template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
    class LinearScaler {
    public:
        LinearScaler() = default;

        LinearScaler(T in_min, T in_max, T out_min, T out_max) {
            set(in_min, in_max, out_min, out_max);
        }

        ~LinearScaler() = default;

        void set(T in_min, T in_max, T out_min, T out_max) {
            m_in_min = in_min;
            m_in_max = in_max;
            m_out_min = out_min;
            m_out_max = out_max;
            _update();
        }

        void setIn(T min, T max) noexcept {
            m_in_min = min;
            m_in_max = max;
            _update();
        }

        void setOut(T min, T max) noexcept {
            m_out_min = min;
            m_out_max = max;
            _update();
        }

        /// Remap value into output range (no clamping)
        [[nodiscard]] T remap(T v) const noexcept {
            if (_m_in_valid) {
                return m_out_min + ((v - m_in_min) / _m_in_range) * _m_out_range;
            }
            return m_out_min;
        }

        /// Remap value into output range with clamping
        [[nodiscard]] T remapClamped(T v) const noexcept {
            if (_m_in_valid) {
                v = m_out_min + ((v - m_in_min) / _m_in_range) * _m_out_range;
                return v < m_out_min ? m_out_min : (v > m_out_max ? m_out_max : v);
            }
            return m_out_min;
        }

        /// Normalize input value into [0, 1] range
        [[nodiscard]] T remapNormalized(T v) const noexcept {
            if (_m_in_valid) {
                return (v - m_in_min) / _m_in_range;
            }
            return static_cast<T>(0.0);
        }

        /// Normalize and clamp input value into [0, 1]
        [[nodiscard]] T remapNormalizedClamped(T v) const noexcept {
            if (_m_in_valid) {
                v = (v - m_in_min) / _m_in_range;
                return v < static_cast<T>(0.0) ? static_cast<T>(0.0)
                                               : (v > static_cast<T>(1.0) ? static_cast<T>(1.0) : v);
            }
            return static_cast<T>(0.0);
        }

    private:
        void _update() noexcept {
            _m_in_range = (m_in_max - m_in_min);
            _m_in_valid = std::fabs(_m_in_range) > std::numeric_limits<T>::epsilon();
            _m_out_range = (m_out_max - m_out_min);
            _m_out_valid = std::fabs(_m_out_range) > std::numeric_limits<T>::epsilon();
        }

    public:
        T m_in_min = static_cast<T>(0.0);
        T m_in_max = static_cast<T>(1.0);
        T m_out_min = static_cast<T>(0.0);
        T m_out_max = static_cast<T>(1.0);

        T _m_in_range{};
        T _m_out_range{};
        bool _m_in_valid{};
        bool _m_out_valid{};
    };

} // End of namespace Grain

#endif // GrainLinearScaler_hpp
