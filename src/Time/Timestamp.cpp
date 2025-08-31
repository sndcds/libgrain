//
//  Timestamp.cpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Time/Timestamp.hpp"

#include <cinttypes>


namespace Grain {
    void Timestamp::dateTimeStr(int32_t out_str_length, char* out_str) const noexcept {
        if (out_str) {
            // Convert milliseconds to seconds & remaining milliseconds
            std::chrono::milliseconds ms(m_value);
            std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
            int milliseconds = static_cast<int>(ms.count() % 1000);

            // Convert to time_t and format as local time
            std::time_t time = sec.count();
            std::tm local_time{};
            localtime_r(&time, &local_time);  // Thread-safe version of localtime

            std::snprintf(out_str, out_str_length, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                          local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday,
                          local_time.tm_hour, local_time.tm_min, local_time.tm_sec, milliseconds);
        }
    }


    void Timestamp::durationStr(int32_t out_str_length, char* out_str) const noexcept {
        if (out_str) {
            int64_t ms = elapsedMillis();
            int64_t total_seconds = ms / 1000;
            int64_t hours = total_seconds / 3600;
            int64_t minutes = (total_seconds % 3600) / 60;
            int64_t seconds = total_seconds % 60;
            ms = ms % 1000;

            std::snprintf(
                    out_str, out_str_length,
                    "%" PRId64 ":%02" PRId64 ":%02" PRId64 ".%03" PRId64 "\n",
                    hours, minutes, seconds, ms);
        }
    }


    void Timestamp::durationStr(const Timestamp begin_timestamp, int32_t out_str_length, char* out_str) const noexcept {
        if (out_str) {
            int64_t ms = (*this - begin_timestamp).milliseconds();
            int64_t total_seconds = ms / 1000;
            int64_t hours = total_seconds / 3600;
            int64_t minutes = (total_seconds % 3600) / 60;
            int64_t seconds = total_seconds % 60;
            ms = ms % 1000;
            std::snprintf(
                    out_str, out_str_length,
                    "%" PRId64 ":%02" PRId64 ":%02" PRId64 ".%03" PRId64 "\n",
                    hours, minutes, seconds, ms);        }
    }


    void Timestamp::dateTimeUTCStr(int32_t out_str_length, char* out_str) const noexcept {
        if (out_str) {

            // Convert the timestamp to a duration
            std::chrono::milliseconds ms(m_value);  // Convert int64_t to milliseconds duration
            auto duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(ms);

            // Get the epoch time point (time since epoch)
            auto epoch_time_point = std::chrono::time_point<std::chrono::system_clock>(duration);

            // Convert epoch time point to a time_t object
            std::time_t time_t_epoch = std::chrono::system_clock::to_time_t(epoch_time_point);

            // Convert time_t to tm struct
            std::tm* tm_utc = std::gmtime(&time_t_epoch);  // UTC time
            // std::tm* tm_local = std::localtime(&time_t_epoch);  // Local time

            // Format the time string including the UTC offset
            std::strftime(out_str, out_str_length, "%Y-%m-%d %H:%M:%S %z", tm_utc);
        }
    }


} // End of namespace Grain
