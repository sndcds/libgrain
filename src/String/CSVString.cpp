//
//  CSVString.cpp
//
//  Created by Roald Christesen on 30.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

// TODO: Preserve buffer overflow when a value string is longer than MAX_VALUE_STR_LENGTH chars.

#include "String/CSVString.hpp"
#include "String/StringList.hpp"
#include "Type/Fix.hpp"
#include "Type/Flags.hpp"

/* TODO: !!!!!
#include <RGB.hpp>
#include <RGBA.hpp>
#include <HSV.hpp>
#include <File.hpp>
*/

namespace Grain {


    bool CSVLineParser::setLine(const char *str) noexcept {

        m_length = str ? static_cast<int64_t>(strlen(str)) : 0;
        if (m_length < 1) {
            return false;
        }

        const char* p = str;
        while (*p != String::EOS) {
            if (!String::charIsWhiteSpace(*p)) {
                m_data = str;
                m_data_read_pos = 0;
                m_line_finished = false;
                m_unknown_ascii_count = 0;
                m_curr_field.clear();
                return true;
            }
        }

        // No none-space characters found
        return false;
    }


    void CSVLineParser::rewind() noexcept {

        m_data_read_pos = 0;
        m_line_finished = false;
        m_unknown_ascii_count = 0;
        m_curr_field.clear();
    }


    bool CSVLineParser::next() {

        char delimiter = m_delimiter;
        char quote = m_quote;
        bool in_quotes = false;
        const char *ptr = &m_data[m_data_read_pos];

        m_curr_field.clear();
        if (!m_curr_field.checkCapacity(1024)) {
            m_curr_field_err = ErrorCode::MemCantAllocate;
            return false;
        }

        if (*ptr == String::EOS) {
            return false;
        }

        while (*ptr != String::EOS) {

            // Handle UTF8 or ASCII characters
            char utf8_code[String::kMaxUtf8SeqLength];
            if (static_cast<uint8_t>(*ptr) > 127) {
                if (m_char_set == CharSet::UTF8) {
                    // Verify the validity of the UTF-8 encoding
                    int32_t utf8_seq_length = String::utf8SeqLength((uint8_t*)ptr);
                    if (utf8_seq_length < 0) {
                        m_err = kErrUTF8Mismatch;
                        m_err_data_offs = static_cast<int64_t>(ptr - m_data);
                        return false;
                    }
                }
                else {  // 7 Bit ASCII or extended ASCII
                    int32_t utf8_code_length = String::extendedAsciiToUTF8(*ptr, m_char_set, utf8_code);
                    if (utf8_code_length > 0) {
                        if (!m_curr_field.append(utf8_code)) {
                            m_curr_field_err = ErrorCode::MemCantAllocate;
                            return false;
                        }
                        m_data_read_pos++;
                        ptr = &m_data[m_data_read_pos];
                        continue;
                    }
                    else {
                        // Skip unknown ASCII character.
                        m_unknown_ascii_count++;
                        m_data_read_pos++;
                        ptr = &m_data[m_data_read_pos];
                        continue;
                    }
                }
            }

            if (static_cast<uint8_t>(*ptr) > 127 && m_char_set != CharSet::UTF8) {
                int32_t utf8_code_length = String::extendedAsciiToUTF8(*ptr, m_char_set, utf8_code);

                if (utf8_code_length > 0) {
                    if (!m_curr_field.append(utf8_code)) {
                        m_curr_field_err = ErrorCode::MemCantAllocate;
                        return false;
                    }
                    m_data_read_pos++;
                    ptr = &m_data[m_data_read_pos];
                    continue;
                }
                else {
                    // Skip unknown ASCII character.
                    m_unknown_ascii_count++;
                    m_data_read_pos++;
                    ptr = &m_data[m_data_read_pos];
                    continue;
                }
            }

            auto utf8_code_length = String::utf8SeqLengthByStartByte(*ptr);
            if (utf8_code_length > 1) {
                m_data_read_pos += utf8_code_length - 1;
                if (!m_curr_field.append(ptr, utf8_code_length)) {
                    m_curr_field_err = ErrorCode::MemCantAllocate;
                    return false;
                }
            }
            else if (*ptr == quote) {
                if (in_quotes && *(ptr + 1) == quote) {
                    // Handle double quotes inside quoted field.
                    if (!m_curr_field.appendChar(quote)) {
                        m_curr_field_err = ErrorCode::MemCantAllocate;
                        return false;
                    }
                    m_data_read_pos++;  // Skip the second quote.
                }
                else {
                    in_quotes = !in_quotes; // Toggle quote state
                }
            }
            else if (*ptr == delimiter && !in_quotes) {
                // Skip delimiter.
                m_data_read_pos++;
                m_curr_field_index++;
                return true;
            }
            else if (*ptr == '\\' && (*(ptr + 1) == quote || *(ptr + 1) == '\\')) {
                // Handle escaped quotes or backslashes.
                m_data_read_pos++;
                if (!m_curr_field.appendChar(*ptr)) {
                    m_curr_field_err = ErrorCode::MemCantAllocate;
                    return false;
                }
            }
            else {
                if (!m_curr_field.appendChar(*ptr)) {
                    m_curr_field_err = ErrorCode::MemCantAllocate;
                    return false;
                }
            }

            m_data_read_pos++;
            ptr = &m_data[m_data_read_pos];
        }

        m_curr_field_index++;
        return true;
    }


    bool CSVLineParser::nextFix(Fix &out_value) noexcept {
        if (next()) {
            out_value.setStr(m_curr_field.utf8());
            return true;
        }
        return false;
    }


    bool CSVLineParser::nextFlags(Flags &outFlags) noexcept {
        if (next()) {
            outFlags.set(m_curr_field.utf8());
            return true;
        }
        return false;
    }


    bool CSVLineParser::nextStr(char **out_str) noexcept {

        if (next() && out_str) {
            *out_str = (char*)m_curr_field.utf8();
            return true;
        }

        return false;
    }


    bool CSVLineParser::nextStr(int64_t max_length, char *out_str) noexcept {

        char *str;

        if (nextStr(&str)) {
            if (strlen(str) >= max_length) {
                return false;
            }
            strcpy(out_str, str);
            return true;
        }

        return false;
    }


    bool CSVLineParser::nextString(String &out_string) noexcept {

        if (next()) {
            out_string = m_curr_field;
            return true;
        }

        return false;
    }


/* TODO: !!!!!
    bool CSVLineParser::nextMat3(Mat3f &out_matrix) noexcept {

        float values[9];
        int32_t result = nextFloats(9, values);
        if (result == 9) {
            out_matrix.set(values);
        }

        return result == 9;
    }


    bool CSVLineParser::nextMat3(Mat3d &out_matrix) noexcept {

        double values[9];
        int32_t result = nextDoubles(9, values);
        if (result == 9) {
            out_matrix.set(values);
        }

        return result == 9;
    }


    bool CSVLineParser::nextRGB(RGB& out_color) noexcept {

        float values[3];
        int32_t result = nextFloats(3, values);
        if (result == 3) {
            out_color.setValues(values);
        }
        return result == 3;
    }


    bool CSVLineParser::nextRGBA(RGBA &out_color) noexcept {

        float values[4];
        int32_t result = nextFloats(4, values);
        if (result == 4) {
            out_color.setValues(values);
        }
        return result == 4;
    }


    bool CSVLineParser::nextHSV(HSV &out_color) noexcept {

        float values[3];
        int32_t result = nextFloats(3, values);
        if (result == 3) {
            out_color.set(values);
        }
        return result == 3;
    }
*/

    bool CSVLineParser::skipFields(int32_t n) noexcept {

        while (n > 0) {
            if (!next()) {
                return false;
            }
            n--;
        }

        return true;
    }


    CSVScanner::CSVScanner(const String &file_path, CSVScannerFieldFunc field_func) {

        m_file_path = file_path;
        m_field_func = field_func;
    }


    CSVScanner::~CSVScanner() {

        delete m_header_labels;
    }


    void CSVScanner::scan() noexcept {
/* TODO: !!!!!
        File *file = nullptr;

        try {
            file = new (std::nothrow) File(m_file_path);
            if (!file) { throw ErrorCode::ClassInstantiationFailed; }

            file->startRead();

            String line;
            CSVLineParser csv_line_parser;
            csv_line_parser.setDelimiter(m_delimiter);
            csv_line_parser.setQuote(m_quote);
            csv_line_parser.setCharSet(m_char_set);

            m_row_index = 0;
            while (file->readLine(line)) {
                m_row_index++;
            }
            m_row_count = m_row_index;

            file->rewind();
            m_row_index = 0;

            bool check_header_flag = m_has_header_flag;

            while (file->readLine(line)) {
                if (csv_line_parser.setLine(line)) {
                    m_col_index = 0;

                    while (csv_line_parser.next()) {
                        if (csv_line_parser.hasError()) {
                            if (m_break_on_err_flag) {
                                Error::throwSpecific(kErrCSVStringError);
                            }
                        }
                        else {
                            if (m_trim_all_fields_flag) {
                                csv_line_parser.trimCurrField();
                            }

                            if (check_header_flag) {
                                if (!m_header_labels) {
                                    m_header_labels = new (std::nothrow) StringList();
                                    Error::throwCheckInstantiation(m_header_labels);
                                }
                                if (!m_header_labels->pushString(csv_line_parser.currFieldString())) {
                                    throw ErrorCode::MemCantGrow;
                                }
                            }

                            if (m_field_func) {
                                m_field_func(*this, csv_line_parser.currFieldString());
                            }
                        }

                        m_col_index++;
                    }

                    if (check_header_flag) {
                        std::cout << m_header_labels << std::endl;
                        for (auto &label : *m_header_labels) {
                            std::cout << label << std::endl;
                        }
                        check_header_flag = false;
                    }

                    m_row_index++;
                }
            }
        }
        catch (ErrorCode err) {
            std::cout << err << std::endl;
        }
        */
    }


} // End of namespace Grain.
