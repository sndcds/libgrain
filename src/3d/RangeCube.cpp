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
        min_x_ = max_x_ = v.x_;
        min_y_ = max_y_ = v.y_;
        min_z_ = max_z_ = v.z_;
        return *this;
    }

    RangeCubeFix RangeCubeFix::operator + (const Vec3Fix& v) const {
        RangeCubeFix result = *this;
        if (v.x_ < min_x_) { result.min_x_ = v.x_; }
        if (v.x_ > max_x_) { result.max_x_ = v.x_; }
        if (v.y_ < min_y_) { result.min_y_ = v.y_; }
        if (v.y_ > max_y_) { result.max_y_ = v.y_; }
        if (v.z_ < min_z_) { result.min_z_ = v.z_; }
        if (v.z_ > max_z_) { result.max_z_ = v.z_; }
        return result;
    }

    RangeCubeFix& RangeCubeFix::operator += (const Vec3Fix& v) {
        if (v.x_ < min_x_) { min_x_ = v.x_; }
        if (v.x_ > max_x_) { max_x_ = v.x_; }
        if (v.y_ < min_y_) { min_y_ = v.y_; }
        if (v.y_ > max_y_) { max_y_ = v.y_; }
        if (v.z_ < min_z_) { min_z_ = v.z_; }
        if (v.z_ > max_z_) { max_z_ = v.z_; }
        return *this;
    }

} // End of namespace Grain
