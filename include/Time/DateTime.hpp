//
//  DateTime.hpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.06.2025
//

#ifndef GrainDateTime_hpp
#define GrainDateTime_hpp

#include "Type/Object.hpp"
#include <chrono>


namespace Grain {

    class Timestamp;


    /**
     *  @class DateTime
     *  @brief Represents a specific date and time.
     *
     *  The `DateTime` class encapsulates a point in time, represented by various
     *  attributes such as the day, month, year, hour, minute, second, and several
     *  other time-related properties. This class can be used for date and time
     *  manipulation, comparisons, and formatting. It is designed to hold values in
     *  a simple and efficient manner, making it ideal for applications where
     *  precise tracking of date and time is necessary.
     *
     * @note The `DateTime` class is designed to be used in conjunction with the
     *       `Calendar` class. The `Calendar` class has access to the private
     *       members of `DateTime` via the `friend` declaration.
     */
    class DateTime {
        friend class Timestamp;

    public:
        static constexpr int32_t kFormattedStrBufferSize = 60;

    public:
        DateTime(bool use_utc = false);
        explicit DateTime(const Timestamp& timestamp);
        explicit DateTime(uint16_t day, uint16_t month, int32_t year, uint16_t hour, uint16_t minute, uint16_t second);

        virtual ~DateTime() {}

        virtual const char* className() const noexcept { return "DateTime"; }

        friend std::ostream& operator << (std::ostream& os, const DateTime* o) {
            o == nullptr ? os << "DateTime nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const DateTime& o) {
            char buffer[kFormattedStrBufferSize];
            o.toStr(kFormattedStrBufferSize, buffer);
            return os << buffer;
        }

        void now() noexcept;
        void set(std::tm tm) noexcept;

        [[nodiscard]] int32_t day() const noexcept { return m_day; }
        [[nodiscard]] int32_t month() const noexcept { return m_month; }
        [[nodiscard]] int32_t year() const noexcept { return m_year; }
        [[nodiscard]] int32_t hour() const noexcept { return m_hour; }
        [[nodiscard]] int32_t minute() const noexcept { return m_minute; }
        [[nodiscard]] int32_t second() const noexcept { return m_second; }
        [[nodiscard]] int32_t dayOfWeek() const noexcept { return m_day_of_week; }
        [[nodiscard]] int32_t dayOfYear() const noexcept { return m_day_of_year; }
        [[nodiscard]] int32_t weekOfMonth() const noexcept { return m_week_of_month; }
        [[nodiscard]] int32_t weekOfYear() const noexcept { return m_week_of_year; }

        void toStr(int32_t max_length, char* out_str) const noexcept;
        void toDateStr(int32_t max_length, char* out_str) const noexcept;
        void toTimeStr(int32_t max_length, char* out_str) const noexcept;
        void toDateString(String& out_string) const noexcept;
        void toTimeString(String& out_string) const noexcept;

        void addSeconds(int32_t seconds) noexcept;
        void addMinutes(int32_t minutes) noexcept { addSeconds(minutes * 60); }
        void addHours(int32_t hours) noexcept { addSeconds(hours * 60 * 60); }
        void addDays(int32_t days) noexcept { addSeconds(days * 24 * 60 * 60); }
        void addMonths(int32_t months) noexcept;

        [[nodiscard]] std::tm toTm() noexcept;

        [[nodiscard]] static int32_t utcOffsetSeconds() noexcept;

    protected:
        int8_t m_day{};             ///< Day of the month (1-31)
        int8_t m_month{};           ///< Month of the year (1-12)
        int32_t m_year{};           ///< Year (e.g., 2023)
        int8_t m_hour{};            ///< Hour of the day (0-23)
        int8_t m_minute{};          ///< Minute of the hour (0-59)
        int8_t m_second{};          ///< Second of the minute (0-59)
        int8_t m_day_of_week{};     ///< Day of the week (0-6), where 0 is Monday
        int16_t m_day_of_year{};    ///< Day of the year (1-366)
        int8_t m_week_of_month{};   ///< Week of the month (1-5)
        int8_t m_week_of_year{};    ///< Week of the year (1-52/53)
        int32_t m_utc_offset_sec{}; ///< UTF offset in seconds
        bool m_utc_flag{};
    };


} // End of namespace Grain

#endif // GrainDateTime_hpp
