//
//  NumberSeries.cpp
//
//  Created by Roald Christesen on 14.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Math/NumberSeries.hpp"
#include <cmath>


namespace Grain {

    int32_t NumberSeries::countBits(int32_t n) {

        int32_t i = 0;
        while (n > 0) {
            n >>= 1;
            i++;
        }

        return i;
    }


    /**
     *  @brief Get a number of the Per Nørgård Infinite Series.
     *
     *  @param index The index, for which a number should be returned.
     *  @return The value between -6 and 6.
     */
    int32_t NumberSeries::perNoergaardInfinitNumber(int32_t index) {

        int32_t result = 0;
        int32_t length = countBits(index);
        int32_t mask = 1 << (length - 1);

        for (int32_t i = length - 1; i >= 0; i--) {
            if ((index & mask) != 0) {
                result += 1;
            }
            else {
                if (result >= 0) {
                    result = -result;
                }
                else {
                    result = std::abs(result);
                }
            }
            mask >>= 1;
        }

        return result;
    }


    int32_t NumberSeries::collatzSequenceNumber(int64_t v, int32_t max_depth) {

        for (int32_t i = 1; i < max_depth; i++) {
            if (v == 1) {
                return i;
            }

            if (v % 2 == 1) {
                v = v * 3 + 1;
            }
            else {
                v = v / 2;
            }
        }

        return -1;
    }


} // End of namespace Grain
