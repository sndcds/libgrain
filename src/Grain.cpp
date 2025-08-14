//
//  Grain.cpp
//
//  Created by Roald Christesen on 14.01.24.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Grain.hpp"


namespace Grain {

    template<>
    bool Safe::canSafelyDivideBy<float>(float v) {
        return std::abs(v) > 1e-6f;
    }

    template<>
    bool Safe::canSafelyDivideBy<double>(double v) {
        return std::abs(v) > 1e-12;
    }

}
