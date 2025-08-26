//
//  TimeMeasure.hpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.06.2025
//

#ifndef GrainTimeMeasure_hpp
#define GrainTimeMeasure_hpp

#include <chrono>


namespace Grain {

    class TimeMeasure {
    protected:
        std::chrono::steady_clock::time_point m_start;

    public:
        TimeMeasure() {
            m_start = std::chrono::steady_clock::now();
        }

        void reset() {
            m_start = std::chrono::steady_clock::now();
        }

        /**
         *  @brief Get elapsed time in nanoseconds
         */
        int64_t elapsedNanos() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_start).count();
        }

        /**
         *  @brief Get elapsed time in milliseconds
         */
        int64_t elapsedMillis() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();
        }
    };

} // End of namespace Grain

#endif // GrainTimeMeasure_hpp
