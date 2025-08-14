//
//  Dimension.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Dimension.hpp"
#include "String/CSVString.hpp"


namespace Grain {

    template <> Dimension<int32_t>::Dimension(const String &csv) noexcept { setByCSV(csv.utf8());}
    template <> Dimension<int64_t>::Dimension(const String &csv) noexcept { setByCSV(csv.utf8()); }
    template <> Dimension<float>::Dimension(const String &csv) noexcept { setByCSV(csv.utf8()); }
    template <> Dimension<double>::Dimension(const String &csv) noexcept { setByCSV(csv.utf8()); }


// Specialized methods.


    template <>
    bool Dimensioni::setByCSV(const char* csv) noexcept {

        bool result = false;

        if (csv != nullptr) {
            CSVLineParser csv_line_parser(csv);
            result = csv_line_parser.next<int32_t>(m_width);
            result = csv_line_parser.next<int32_t>(m_height);
        }

        return result;
    }


    template <>
    bool Dimensionl::setByCSV(const char* csv) noexcept {

        bool result = false;

        if (csv != nullptr) {
            CSVLineParser csv_line_parser(csv);
            result = csv_line_parser.next<int64_t>(m_width);
            result = csv_line_parser.next<int64_t>(m_height);
        }

        return result;
    }


    template <>
    bool Dimensionf::setByCSV(const char* csv) noexcept {

        bool result = false;

        if (csv != nullptr) {
            CSVLineParser csv_line_parser(csv);
            result = csv_line_parser.next<float>(m_width);
            result = csv_line_parser.next<float>(m_height);
        }

        return result;
    }


    template <>
    bool Dimensiond::setByCSV(const char* csv) noexcept {

        bool result = false;

        if (csv != nullptr) {
            CSVLineParser csv_line_parser(csv);
            result = csv_line_parser.next<double>(m_width);
            result = csv_line_parser.next<double>(m_height);
        }

        return result;
    }


} // End of namespace Grain
