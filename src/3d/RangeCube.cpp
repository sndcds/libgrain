//
//  RangeCube.cpp
//
//  Created by Roald Christesen on from 17.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "3d/RangeCube.hpp"
#include "Math/Vec3Fix.hpp"


namespace Grain {

    RangeCubeFix& RangeCubeFix::operator = (const Vec3Fix& v) {
        m_min_x = m_max_x = v.m_x;
        m_min_y = m_max_y = v.m_y;
        m_min_z = m_max_z = v.m_z;
        return *this;
    }

    RangeCubeFix RangeCubeFix::operator + (const Vec3Fix& v) const {
        RangeCubeFix result = *this;
        if (v.m_x < m_min_x) { result.m_min_x = v.m_x; }
        if (v.m_x > m_max_x) { result.m_max_x = v.m_x; }
        if (v.m_y < m_min_y) { result.m_min_y = v.m_y; }
        if (v.m_y > m_max_y) { result.m_max_y = v.m_y; }
        if (v.m_z < m_min_z) { result.m_min_z = v.m_z; }
        if (v.m_z > m_max_z) { result.m_max_z = v.m_z; }
        return result;
    }

    RangeCubeFix& RangeCubeFix::operator += (const Vec3Fix& v) {
        if (v.m_x < m_min_x) { m_min_x = v.m_x; }
        if (v.m_x > m_max_x) { m_max_x = v.m_x; }
        if (v.m_y < m_min_y) { m_min_y = v.m_y; }
        if (v.m_y > m_max_y) { m_max_y = v.m_y; }
        if (v.m_z < m_min_z) { m_min_z = v.m_z; }
        if (v.m_z > m_max_z) { m_max_z = v.m_z; }
        return *this;
    }

} // End of namespace Grain
