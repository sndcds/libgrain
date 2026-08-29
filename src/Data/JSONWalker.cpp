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
    void JSONWalker::addProperty(const String &path, const JSONValue& value) {
        std::cout << "Primitive at " << path << " = ";
        std::cout << value.asString() << std::endl;
    }
}
