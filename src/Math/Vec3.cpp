//
//  Vec3.cpp
//
//  Created by Roald Christesen on 30.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Math/Vec3.hpp"
#include "File/File.hpp"


namespace Grain {

    template <ScalarType T>
    void Vec3<T>::readFromFile(File& file) {
        x_ = file.readValue<T>();
        y_ = file.readValue<T>();
        z_ = file.readValue<T>();
    }

    template void Vec3<int32_t>::readFromFile(File&);
    template void Vec3<int64_t>::readFromFile(File&);
    template void Vec3<float>::readFromFile(File&);
    template void Vec3<double>::readFromFile(File&);

    template <ScalarType T>
    void Vec3<T>::writeToFile(File& file) {
        file.writeValue<T>(x_);
        file.writeValue<T>(y_);
        file.writeValue<T>(z_);
    }

    template void Vec3<int32_t>::writeToFile(File&);
    template void Vec3<int64_t>::writeToFile(File&);
    template void Vec3<float>::writeToFile(File&);
    template void Vec3<double>::writeToFile(File&);

} // End of namespace Grain
