//
//  RangeRect.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/RangeRect.hpp"


namespace Grain {

// Explicit instantiations for the types you need
template class RangeRect<int32_t>;
template class RangeRect<int64_t>;
template class RangeRect<float>;
template class RangeRect<double>;


} // End of namespace Grain
