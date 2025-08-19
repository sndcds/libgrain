//
//  Random.cpp
//
//  Created by Roald Christesen on 06.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Math/Random.hpp"


namespace Grain {

    /**
     *  @brief Returns a random integer in [min, max].
     *  @param min Minimum value (inclusive).
     *  @param max Maximum value (inclusive).
     *  @return Random 32-bit signed integer.
     */
    int32_t Random::nextInt(int32_t min, int32_t max) noexcept {

        int32_t v = static_cast<int32_t>(std::roundf(next(static_cast<int32_t>(min) - 0.5f, static_cast<int32_t>(max) + 0.5f)));
        if (v < min) {
            return min;
        }
        else if (v > max) {
            return max;
        }
        else {
            return v;
        }
    }


    /**
     *  @brief Returns a random character from a given character table.
     *  @param table Pointer to a character array.
     *  @param table_length Number of characters in the array.
     *  @return Random character from the table.
     */
    char Random::nextChar(const char* table, int32_t table_length) noexcept {

        if (table) {
            return table[Random::nextInt(table_length - 1)];
        }
        else {
            return 0;
        }
    }


    /**
     *  @brief Configures the generator with a new integer range and optional reseed.
     *  @param min Minimum value.
     *  @param max Maximum value.
     *  @param seed_flag If true, reseeds the generator with system time.
     */
    void IntRand::setup(int32_t min, int32_t max, bool seed_flag) noexcept {

        if (seed_flag) {
            auto seed = std::chrono::system_clock::now().time_since_epoch().count();
            m_generator.seed((unsigned int)seed);
        }

        if (min > max) {
            std::swap(min, max);
        }

        m_min = min;
        m_max = max;

        m_distribution = std::uniform_int_distribution<int32_t>(min, max);
    }


} // End of namespace Grain
