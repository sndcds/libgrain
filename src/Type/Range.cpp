//
//  Range.hpp
//
//  Created by Roald Christesen on from 20.07.24
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Type/Range.hpp"
#include "Math/Random.hpp"


namespace Grain {

    // General template implementation
    template <typename T>
        void Range<T>::initForMinMaxSearch() noexcept {
        m_min = std::numeric_limits<T>::max();
        m_max = std::numeric_limits<T>::lowest();
    }

    template <>
    void Range<int32_t>::initForMinMaxSearch() noexcept {
        m_min = std::numeric_limits<int32_t>::max();
        m_max = std::numeric_limits<int32_t>::lowest();
    }

    template <>
    void Range<int64_t>::initForMinMaxSearch() noexcept {
        m_min = std::numeric_limits<int64_t>::max();
        m_max = std::numeric_limits<int64_t>::lowest();
    }

    template <>
    void Range<float>::initForMinMaxSearch() noexcept {
        m_min = std::numeric_limits<float>::max();
        m_max = std::numeric_limits<float>::lowest();
    }

    template <>
    void Range<double>::initForMinMaxSearch() noexcept {
        m_min = std::numeric_limits<double>::max();
        m_max = std::numeric_limits<double>::lowest();
    }

    template <typename T>
    T Range<T>::random() const noexcept {
        return static_cast<T>(m_min + Random::next(m_max - m_min));
    }

    double RangeFix::random() const noexcept {
        return m_min.asDouble() + Random::next(m_max.asDouble() - m_min.asDouble());
    }

} // End of namespace Grain
