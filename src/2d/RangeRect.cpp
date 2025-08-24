//
//  RangeRect.cpp
//  GrainLib
//
//  Created by Roald Christesen on 24.01.24.
//

#include "2d/RangeRect.hpp"


namespace Grain {

    // General template implementation
    template <ScalarType T>
        void RangeRect<T>::initForMinMaxSearch() noexcept {
        m_min_x = std::numeric_limits<T>::max();
        m_min_y = std::numeric_limits<T>::max();
        m_max_x = std::numeric_limits<T>::lowest();
        m_max_y = std::numeric_limits<T>::lowest();
    }

    template <>
    void RangeRect<int32_t>::initForMinMaxSearch() noexcept {
        m_min_x = std::numeric_limits<int32_t>::max();
        m_min_y = std::numeric_limits<int32_t>::max();
        m_max_x = std::numeric_limits<int32_t>::lowest();
        m_max_y = std::numeric_limits<int32_t>::lowest();
    }

    template <>
    void RangeRect<int64_t>::initForMinMaxSearch() noexcept {
        m_min_x = std::numeric_limits<int64_t>::max();
        m_min_y = std::numeric_limits<int64_t>::max();
        m_max_x = std::numeric_limits<int64_t>::lowest();
        m_max_y = std::numeric_limits<int64_t>::lowest();
    }

    template <>
    void RangeRect<float>::initForMinMaxSearch() noexcept {
        m_min_x = std::numeric_limits<float>::max();
        m_min_y = std::numeric_limits<float>::max();
        m_max_x = std::numeric_limits<float>::lowest();
        m_max_y = std::numeric_limits<float>::lowest();
    }

    template <>
    void RangeRect<double>::initForMinMaxSearch() noexcept {
        m_min_x = std::numeric_limits<double>::max();
        m_min_y = std::numeric_limits<double>::max();
        m_max_x = std::numeric_limits<double>::lowest();
        m_max_y = std::numeric_limits<double>::lowest();
    }


    template <ScalarType T>
    void RangeRect<T>::writeToFile(File& file) {
        file.writeValue<T>(m_min_x);
        file.writeValue<T>(m_min_y);
        file.writeValue<T>(m_max_x);
        file.writeValue<T>(m_max_y);
    }

    template void RangeRect<int32_t>::writeToFile(File&);
    template void RangeRect<int64_t>::writeToFile(File&);
    template void RangeRect<float>::writeToFile(File&);
    template void RangeRect<double>::writeToFile(File&);


    template <ScalarType T>
    void RangeRect<T>::readFromFile(File& file) {
        m_min_x = file.readValue<T>();
        m_min_y = file.readValue<T>();
        m_max_x = file.readValue<T>();
        m_max_y = file.readValue<T>();
    }

    template void RangeRect<int32_t>::readFromFile(File&);
    template void RangeRect<int64_t>::readFromFile(File&);
    template void RangeRect<float>::readFromFile(File&);
    template void RangeRect<double>::readFromFile(File&);

}  // End of namespace Grain
