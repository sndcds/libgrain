//
//  NumberSeries.hpp
//
//  Created by Roald Christesen on 14.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainNumberSeries_hpp
#define GrainNumberSeries_hpp

#include "Grain.hpp"
#include <random>
#include <cmath>
#include <chrono>


namespace Grain {

    class NumberSeries {
    public:
        NumberSeries() noexcept {}

        static int32_t countBits(int32_t n);
        static int32_t perNoergaardInfinitNumber(int32_t index);
        static int32_t collatzSequenceNumber(int64_t v, int32_t max_depth);
    };

} // End of namespace Grain

#endif // GrainNumberSeries_hpp
