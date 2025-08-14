//
//  Vec2.cpp
//
//  Created by Roald Christesen on 30.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Math/Vec2.hpp"
#include "String/String.hpp"
#include "String/CSVString.hpp"


namespace Grain {

    template <arithmetic T>
    bool Vec2<T>::parseFromCSVLine(CSVLineParser& parser) noexcept {

        T values[2];
        int32_t result = parser.values<T>(2, values);
        if (result == 2) {
            m_x = values[0];
            m_y = values[1];
        }
        return result == 2;
    }


    /**
     *  @brief Append a 2d vector with specified precision.
     *
     *  @param string The String.
     *  @param precision Number of decimal places to include (0 to 9). Values
     *                   outside this range will be clamped.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    template <arithmetic T>
    bool Vec2<T>::appendToString(String& string, char delimiter, int32_t precision) const noexcept {
        bool result = true;

        if constexpr (std::is_floating_point_v<T>) {
            result = string.appendFix(Fix(m_x), precision);
            result &= string.appendChar(delimiter);
            result &= string.appendFix(Fix(m_y), precision);
        }
        else if constexpr (std::is_integral_v<T>) {
            result = string.appendInt64(m_x);
            result &= string.appendChar(delimiter);
            result &= string.appendInt64(m_y);
        }
        else {
            static_assert(std::is_arithmetic_v<T>, "Vec2<T>::appendToString only supports arithmetic types");
        }

        return result;
    }


}  // End of namespace Grain
