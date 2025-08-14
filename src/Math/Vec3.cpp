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

    template <class T>
    void Vec3<T>::readFromFile(File& file) {
        m_x = file.readValue<T>();
        m_y = file.readValue<T>();
        m_z = file.readValue<T>();
    }

    template void Vec3<int32_t>::readFromFile(File&);
    template void Vec3<int64_t>::readFromFile(File&);
    template void Vec3<float>::readFromFile(File&);
    template void Vec3<double>::readFromFile(File&);

    template <class T>
    void Vec3<T>::writeToFile(File& file) {
        file.writeValue<T>(m_x);
        file.writeValue<T>(m_y);
        file.writeValue<T>(m_z);
    }

    template void Vec3<int32_t>::writeToFile(File&);
    template void Vec3<int64_t>::writeToFile(File&);
    template void Vec3<float>::writeToFile(File&);
    template void Vec3<double>::writeToFile(File&);

} // End of namespace Grain
