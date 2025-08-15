//
//  GrDateTime.mm
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Time/DateTime.hpp"
#include "String/String.hpp"

#include <chrono>


namespace Grain {

    DateTime::DateTime(bool use_utc) {
        m_utc_flag = use_utc;
        now();
    }


    DateTime::DateTime(const Timestamp& timestamp) {
        // TODO: Implement!
    }


    DateTime::DateTime(uint16_t day, uint16_t month, int32_t year, uint16_t hour, uint16_t minute, uint16_t second) {
        // TODO: Implement!
    }


    void DateTime::now() noexcept {
        using namespace std::chrono;

        auto now_tp = system_clock::now();
        std::time_t now_tt = system_clock::to_time_t(now_tp);
        std::tm tm = *std::localtime(&now_tt);

        if (m_utc_flag) {
            m_utc_offset_sec = utcOffsetSeconds();
        }
        else {
            m_utc_offset_sec = 0;
        }

        set(tm);
    }


    void DateTime::set(std::tm tm) noexcept {
        using namespace std::chrono;

        m_year = static_cast<int32_t>(tm.tm_year + 1900); // tm_year is years since 1900
        m_month = static_cast<int8_t>(tm.tm_mon + 1); // tm_mon is 0-11
        m_day = static_cast<int8_t>(tm.tm_mday); // 1-31
        m_hour = static_cast<int8_t>(tm.tm_hour); // 0-23
        m_minute = static_cast<int8_t>(tm.tm_min); // 0-59
        m_second = static_cast<int8_t>(tm.tm_sec); // 0-59
        m_day_of_week = static_cast<int8_t>((tm.tm_wday + 6) % 7); // convert Sunday=0 to Monday=0
        m_day_of_year = tm.tm_yday + 1;

        // Week of year (ISO 8601)
        int wday = m_day_of_week; // Monday=0
        int doy = m_day_of_year;
        m_week_of_year = (doy - wday + 10) / 7;

        // Week of month
        m_week_of_month = (m_day + 6) / 7; // simple approximation
    }


    void DateTime::toStr(int32_t max_length, char* out_str) const noexcept {
        if (out_str != nullptr) {
            std::snprintf(
                    out_str, max_length, "%02d.%02d.%d %02d:%02d:%02d UTC%+03d",
                    m_day, m_month, m_year,
                    m_hour, m_minute, m_second,
                    m_utc_offset_sec / 60 / 60);
        }
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


    void DateTime::addSeconds(int32_t seconds) noexcept {
        std::tm temp = toTm();
        temp.tm_isdst = -1; // let mktime figure out DST
        std::time_t tt = std::mktime(&temp);
        tt += seconds;
        set(*std::localtime(&tt));
    }


    void DateTime::addMonths(int32_t months) noexcept {
        std::tm temp = toTm();
        temp.tm_mon += months; // Add months directly
        std::time_t tt;
        temp.tm_isdst = -1; // Let mktime determine DST
        tt = std::mktime(&temp);
        set(*std::localtime(&tt));
    }


    std::tm DateTime::toTm() noexcept {
        std::tm tm{};
        tm.tm_year = m_year - 1900; // tm_year is years since 1900
        tm.tm_mon = m_month - 1; // tm_mon is 0–11
        tm.tm_mday = m_day; // 1–31
        tm.tm_hour = m_hour; // 0–23
        tm.tm_min = m_minute; // 0–59
        tm.tm_sec = m_second; // 0–59
        tm.tm_isdst = 0;
        return tm;
    }


    int32_t DateTime::utcOffsetSeconds() noexcept {
        std::time_t now_tt = std::time(nullptr);
        std::tm tm = *std::gmtime(&now_tt); // UTC time
        std::tm local_tm = *std::localtime(&now_tt); // Local time
        // Convert both to time_t, interpreted as local time!
        std::time_t utc_tt = std::mktime(&tm);
        std::time_t local_tt = std::mktime(&local_tm);
        return static_cast<int32_t>(std::difftime(local_tt, utc_tt));
    }

} // End of namespace Grain
