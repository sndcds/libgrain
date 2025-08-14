//
//  GrDateTime.mm
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GrDateTime.hpp"
#include "GrCalendar.hpp"
#include "GrString.hpp"

#include <sys/time.h>  // For timeval structure


namespace Grain {


    DateTime::DateTime() {

    }


    DateTime::DateTime(const Calendar& calendar) {

        *this = calendar.currDateTime();
    }


    DateTime::DateTime(const Calendar& calendar, const Timestamp& timestamp) {

        *this = calendar.dateTimeByTimestamp(timestamp);
    }


    DateTime::DateTime(const Calendar& calendar, uint16_t day, uint16_t month, int32_t year, uint16_t hour, uint16_t minute, uint16_t second) {

        *this = calendar.dateTime(day, month, year, hour, minute, second);
    }


    void DateTime::log(Log& l) const {

        char buffer[kFormattedStrBufferSize];

        l.header(className());

        toDateStr(kFormattedStrBufferSize, buffer);
        l << "date: " << buffer << l.endl;

        toTimeStr(kFormattedStrBufferSize, buffer);
        l << "time: " << buffer << l.endl;

        l << "m_day_of_week: " << (int32_t)m_day_of_week << l.endl;
        l << "m_day_of_year: " << (int32_t)m_day_of_year << l.endl;
        l << "m_week_of_month: " << (int32_t)m_week_of_month << l.endl;
        l << "m_week_of_year: " << (int32_t)m_week_of_year << l.endl;

        l--;
    }


    void DateTime::toDateStr(int32_t max_length, char* out_str) const noexcept {

        if (out_str != nullptr) {
            std::snprintf(out_str, max_length, "%02d.%02d.%d", m_day, m_month, m_year);
        }
    }


    void DateTime::toTimeStr(int32_t max_length, char* out_str) const noexcept {

        if (out_str != nullptr) {
            std::snprintf(out_str, max_length, "%02d:%02d:%02d", m_hour, m_minute, m_second);
        }
    }


    void DateTime::toDateString(String& out_string) const noexcept {

        char buffer[20];
        toDateStr(20, buffer);
        out_string = buffer;
    }


    void DateTime::toTimeString(String& out_string) const noexcept {

        char buffer[20];
        toTimeStr(20, buffer);
        out_string = buffer;
    }


    void DateTime::addDays(const Calendar& calendar, int32_t days) noexcept {

        Timestamp ts = calendar.dateTimeToTimestampValue(*this);
        ts.addDays(days);
        *this = calendar.timestampToDateTime(ts);
    }


    void DateTime::addMonths(const Calendar& calendar, int32_t months) noexcept {

        int32_t years = months / 12;
        months = months % 12;
        if (months < 0) {
            months += 12;
            years -= 1;
        }
        m_year += years;
        m_month += months;

        Timestamp ts = calendar.dateTimeToTimestampValue(*this);
        *this = calendar.timestampToDateTime(ts);
    }


}  // End of namespace Grain
