//
//  Vec3Fix.cpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Math/Vec3Fix.hpp"


namespace Grain {

    /**
     *  @brief Parses a 3D fixed-point vector from a delimited string.
     *
     *  Efficiently extracts x, y, and z components from a string using a
     *  single-character delimiter. This fast version assumes each component string
     *  will not exceed 100 characters.
     *
     *  @param string The input string containing the vector components
     *                (e.g., "100,200,300").
     *  @param delimiter The character used to separate the components
     *                   (typically a comma).
     *  @return `true` if parsing was successful and all three components were
     *          extracted; `false` otherwise.
     *
     *  @note On success, the vector's internal fixed-point values are updated.
     */
    bool Vec3Fix::setByCSV(const String& string, char delimiter) noexcept {
        char buffer[3 * 100]; // Assuming maximum float representation size is 100 characters
        if (string.splitFast(delimiter, 3, 100, buffer) == 3) {
            set(&buffer[0], &buffer[100], &buffer[200]);
            return true;
        }
        else {
            return false;
        }
    }


} // End of namespace Grain
