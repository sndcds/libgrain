//
//  Timestamp.hpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.06.2025
//

#ifndef GrainTimestamp_hpp
#define GrainTimestamp_hpp

#include <cmath>
#include <chrono>
#include <sys/time.h>  // For timeval structure
#include <iostream>


namespace Grain {

    using timestamp_t = int64_t;

    class String;
    class DateTime;

    /**
     *  @class Timestamp
     *  @brief A class that represents a point in time using a 64-bit Unix timestamp.
     *
     *  This class stores and manipulates the time as a 64-bit signed integer, which
     *  represents the number of seconds since the Unix epoch (January 1, 1970). The
     *  timestamp is based on macOS's 64-bit timestamp format. This format can
     *  represent dates within a wide range, from the beginning of the Unix epoch to
     *  a far-future date.
     *
     *  The maximum date that can be represented is
     *  **Tuesday, 4 December 292,278,994,248,175,999 UTC**.
     *  This corresponds to the largest possible value of a 64-bit signed integer
     *  timestamp, which is 9223372036854775807 seconds since the Unix epoch.
     *
     *  @note The timestamp is stored in UTC (Coordinated Universal Time).
     */
    class Timestamp {
    public:
        Timestamp() noexcept {
            m_value = currentMillis();
        }

        explicit Timestamp(timestamp_t value) noexcept {
            m_value = value;
        }

        ~Timestamp() noexcept = default;


        friend std::ostream& operator << (std::ostream& os, const Timestamp& o) {
            char buffer[30];
            o.dateTimeStr(30, buffer);
            return os << buffer << ", " << o.m_value;
        }


        // Operator overloading
        Timestamp& operator = (const timestamp_t v) { this->setValue(v); return *this; }
        Timestamp& operator = (const Timestamp& other) { this->set(other); return *this; }

        bool operator == (const Timestamp& other) const { return m_value == other.m_value; }
        bool operator == (const timestamp_t v) const { return m_value == v; }
        bool operator != (const Timestamp& other) const { return m_value != other.m_value; }
        bool operator < (const Timestamp& other) const { return m_value < other.m_value; }
        bool operator <= (const Timestamp& other) const { return m_value <= other.m_value; }
        bool operator > (const Timestamp& other) const { return m_value > other.m_value; }
        bool operator >= (const Timestamp& other) const { return m_value >= other.m_value; }

        Timestamp operator + (const Timestamp& other) const { return Timestamp(m_value + other.m_value); }
        Timestamp operator + (timestamp_t v) const { return Timestamp(m_value + v); }
        Timestamp operator - (const Timestamp& other) const { return Timestamp(m_value - other.m_value); }
        Timestamp operator - (timestamp_t v) const { return Timestamp(m_value - v); }

        Timestamp& operator += (const Timestamp& other) { m_value += other.m_value; return *this; }
        Timestamp& operator += (timestamp_t v) { m_value += v; return *this; }
        Timestamp& operator -= (const Timestamp& other) { m_value -= other.m_value; return *this; }
        Timestamp& operator -= (timestamp_t v) { m_value -= v; return *this; }

        // Set
        void now() noexcept { m_value = currentMillis(); }
        void setValue(timestamp_t value) noexcept { m_value = value; }
        void set(const Timestamp& ts) noexcept { m_value = ts.m_value; }
        void setDistance(Timestamp& ts) noexcept {
            m_value = ts.m_value > m_value ? ts.m_value - m_value : m_value - ts.m_value;
        }
        void setMilliseconds(timestamp_t milliseconds) noexcept { m_value = milliseconds; }
        void setSeconds(double seconds) noexcept { m_value = (timestamp_t)(seconds * k_sec_to_ms); }
        void setMinutes(double minutes) noexcept { m_value = (timestamp_t)(minutes * k_min_to_msec); }
        void setHours(double hours) noexcept { m_value = (timestamp_t)(hours * k_hour_to_msec); }
        void setDays(double days) noexcept { m_value = (timestamp_t)(days * k_day_to_msec); }
        void setWeeks(double weeks) noexcept { m_value = (timestamp_t)(weeks * k_week_to_msec); }
        void setQuickTimeMovieTime() noexcept { m_value = ((timestamp_t)time - k_sec_1904_to_1970) * 1000; }

        // Utils
        [[nodiscard]] timestamp_t milliseconds() const noexcept { return m_value; }
        [[nodiscard]] double seconds() const noexcept { return static_cast<double>(m_value) * k_ms_to_sec; }
        [[nodiscard]] double minutes() const noexcept { return static_cast<double>(m_value) * k_ms_to_min; }
        [[nodiscard]] double hours() const noexcept { return static_cast<double>(m_value) * k_ms_to_hour; }
        [[nodiscard]] double days() const noexcept { return static_cast<double>(m_value) * k_ms_to_day; }
        [[nodiscard]] double weeks() const noexcept { return static_cast<double>(m_value) * k_ms_to_week; }

        // Modify
        void add(Timestamp& ts) noexcept { m_value += ts.m_value; }
        void sub(Timestamp& ts) noexcept { m_value -= ts.m_value; }
        void addMilliseconds(timestamp_t milliseconds) noexcept { m_value += milliseconds; }
        void addSeconds(double seconds) noexcept { m_value += (timestamp_t)(seconds * k_sec_to_ms); }
        void addMinutes(double minutes) noexcept { m_value += (timestamp_t)(minutes * k_min_to_msec); }
        void addHours(double hours) noexcept { m_value += (timestamp_t)(hours * k_hour_to_msec); }
        void addDays(double days) noexcept { m_value += (timestamp_t)(days * k_day_to_msec); }
        void addWeeks(double weeks) noexcept { m_value += (timestamp_t)(weeks * k_week_to_msec); }

        [[nodiscard]] timestamp_t measure() const noexcept { Timestamp t; return t.m_value - m_value; }

        /**
         *  @brief Get the elapsed time as timestamp value.
         */
        [[nodiscard]] timestamp_t elapsed() const noexcept { return currentMillis() - m_value; }

        /**
         *  @brief Get the elapsed time in minutes.
         */
        [[nodiscard]] double elapsedDays() const noexcept { return static_cast<double>(elapsed()) * k_ms_to_day; }

        /**
         *  @brief Get the elapsed time in minutes.
         */
        [[nodiscard]] double elapsedHours() const noexcept { return static_cast<double>(elapsed()) * k_ms_to_hour; }

        /**
         *  @brief Get the elapsed time in minutes.
         */
        [[nodiscard]] double elapsedMinutes() const noexcept { return static_cast<double>(elapsed()) * k_ms_to_min; }

        /**
         *  @brief Get the elapsed time in seconds.
         */
        [[nodiscard]] double elapsedSeconds() const noexcept { return static_cast<double>(elapsed()) * k_ms_to_sec; }

        [[nodiscard]] int64_t elapsedMillis() const noexcept { return static_cast<int64_t>(elapsed()); }

        void dateTimeStr(int32_t out_str_length, char* out_str) const noexcept;
        void durationStr(int32_t out_str_length, char* out_str) const noexcept;
        void durationStr(const Timestamp begin_timestamp, int32_t out_str_length, char* out_str) const noexcept;

        void dateTimeUTCStr(int32_t out_str_length, char* out_str) const noexcept;
        void dateTimeUTCString(String& out_string) const noexcept;


        /**
         *  @brief Get the currentMillis time point and convert it to milliseconds since the epoch.
         */
        [[nodiscard]] static timestamp_t currentMillis() noexcept {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        }

        [[nodiscard]] static timestamp_t elapsed(timestamp_t ts1, timestamp_t ts2) noexcept { return (ts2 - ts1); }
        [[nodiscard]] static double elapsedSeconds(timestamp_t ts1, timestamp_t ts2) noexcept {
            return 0.001 * static_cast<double>((ts2 - ts1));
        }

        static void waitSeconds(double seconds) noexcept {
            Timestamp end_timestamp = Timestamp();
            end_timestamp.addSeconds(seconds);
            while (currentMillis() < end_timestamp.m_value) {};
        }

        [[nodiscard]] inline static double secondsToMillis(double seconds) { return seconds * 1000; }
        [[nodiscard]] inline static double minutesToMillis(double minutes) { return minutes * 60000; }
        [[nodiscard]] inline static double hoursToMillis(double hours) { return hours * 3600000; }
        [[nodiscard]] inline static double daysToMillis(double days) { return days * 86400000; }
        [[nodiscard]] inline static double weeksToMillis(double weeks) { return weeks * 604800000; }

        [[nodiscard]] inline static double millisToSeconds(int64_t ms) { return static_cast<double>(ms) / 1000; }
        [[nodiscard]] inline static double millisToMinutes(int64_t ms) { return static_cast<double>(ms) / 60000; }
        [[nodiscard]] inline static double millisToHours(int64_t ms) { return static_cast<double>(ms) / 3600000; }
        [[nodiscard]] inline static double millisToDays(int64_t ms) { return static_cast<double>(ms) / 86400000; }
        [[nodiscard]] inline static double millisToWeeks(int64_t ms) { return static_cast<double>(ms) / 604800000; }

    public:
        static constexpr double k_sec_to_ms = 1000.0;
        static constexpr double k_min_to_msec = 60000.0;
        static constexpr double k_hour_to_msec = 3600000.0;
        static constexpr double k_day_to_msec = 86400000.0;
        static constexpr double k_week_to_msec = 604800000.0;

        static constexpr double k_ms_to_sec = 1.0 / 1000.0;
        static constexpr double k_ms_to_min = 1.0 / 60000.0;
        static constexpr double k_ms_to_hour = 1.0 / 3600000.0;
        static constexpr double k_ms_to_day = 1.0 / 86400000.0;
        static constexpr double k_ms_to_week = 1.0 / 604800000.0;

        static constexpr int64_t k_sec_1904_to_1970 = 2082844800;
        static constexpr int64_t k_sec_1904_to_2001 = 3061152000;

    protected:
        timestamp_t m_value;
    };

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;

    inline TimePoint now() {
        return Clock::now();
    }

} // End of namespace Grain

#endif // GrainTimestamp_hpp
