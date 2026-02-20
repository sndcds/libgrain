//
//  JSONWalker.cpp
//
//  Created by Roald Christesen on 15.02.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

// TODO: exceptions, throw

#include "Data/JSONWalker.hpp"

namespace Grain {

void JSONWalker::addProperty(const std::string& path, const json& value) {
    std::cout << "Primitive at " << path << " = ";

    if (value.is_string()) {
        std::cout << value.get<std::string>();
    }
    else if (value.is_number()) {
        std::cout << value;
    }
    else if (value.is_boolean()) {
        std::cout << (value.get<bool>() ? "true" : "false");
    }
    else if (value.is_null()) {
        std::cout << "null";
    }

    std::cout << "\n";
}

}