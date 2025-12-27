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


namespace Grain {

    template <ScalarType T>
    void Vec2<T>::readFromFile(File& file) {
        x_ = file.readValue<T>();
        y_ = file.readValue<T>();
    }

    template void Vec2<int32_t>::readFromFile(File&);
    template void Vec2<int64_t>::readFromFile(File&);
    template void Vec2<float>::readFromFile(File&);
    template void Vec2<double>::readFromFile(File&);

    template <ScalarType T>
    void Vec2<T>::writeToFile(File& file) {
        file.writeValue<T>(x_);
        file.writeValue<T>(y_);
    }

    template void Vec2<int32_t>::writeToFile(File&);
    template void Vec2<int64_t>::writeToFile(File&);
    template void Vec2<float>::writeToFile(File&);
    template void Vec2<double>::writeToFile(File&);

} // End of namespace Grain
