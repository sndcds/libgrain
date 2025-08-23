//
//  CSVString.hpp
//
//  Created by Roald Christesen on 30.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

/*
 *  TODO: See: CSV Parser https://github.com/AriaFallah/csv-parser/
 *  TODO: Prescan all fields of a GrCSVString to effecient/random access by index.
 *  TODO: Call a lua script work with data and conditions.
 *  TODO: Support multiple delimiters?
 */

#ifndef GrainCSVString_hpp
#define GrainCSVString_hpp

#include "String/String.hpp"
#include "Type/List.hpp"
#include "Type/Fix.hpp"
#include "Type/Flags.hpp"

#include <iostream>
#include <cstdint>
#include <cmath>


namespace Grain {

    class CSVScanner;

    /**
     *  @brief Parser for a single line of data in CSV format.
     *
     *  The CSV (Comma-Separated Values) format is a simple and widely used data
     *  interchange format, but it doesn't have a single official specification.
     *  Instead, the format is informally defined by common usage and conventions.
     *  However, there are some recommendations and guidelines that are widely followed.
     *
     *  @see https://en.wikipedia.org/wiki/Comma-separated_values
     *  @see RFC 4180 - Common Format and MIME Type for Comma-Separated Values
     *       (CSV) Files
     */
    class CSVLineParser {
    public:
        enum class Status {
            Continue = 0,
            LineEnd,
            FieldEnd,
            ErrorInsideQuoteMismatch,
            FormatError
        };

        enum {
            kErrNone = 0,
            kErrFormatError,
            kErrInsideQuoteMismatch,
            kErrUTF8Mismatch,

            kErrCount,
            kErrLast = kErrCount - 1
        };

        struct FieldInfo {
            int64_t m_offs;
            int64_t m_length;
        };

        static constexpr int32_t kDefaultBufferLength = 10000;  // TODO: size ok?
        static constexpr int32_t kMaxFieldPerLine = 25600;  // TODO: size ok?

    private:
        const char* m_data = nullptr;
        int64_t m_data_read_pos = 0;
        int64_t m_length = 0;
        int32_t m_err = kErrNone;           ///< Last error occured
        int64_t m_err_data_offs = -1;       ///< Potition in `m_data`, where error occured

        CharSet m_char_set = CharSet::UTF8;
        char m_delimiter = ',';
        char m_quote = '"';

        bool m_line_finished = false;
        bool m_line_contains_info = false;  ///< Line contains information (none black characters)

        int64_t m_unknown_ascii_count = 0;

        int32_t m_curr_field_index = -1;
        String m_curr_field;
        ErrorCode m_curr_field_err = ErrorCode::None;

    public:
        CSVLineParser() noexcept {
            m_line_contains_info = setLine(nullptr);
        }

        explicit CSVLineParser(const char* str) noexcept {
            m_line_contains_info = setLine(str);
        }

        explicit CSVLineParser(const String& string) noexcept {
            m_line_contains_info = setLine(string.utf8());
        }

        ~CSVLineParser() = default;

        void _init() {
        }


        [[nodiscard]] const char* dataPtr() const noexcept { return m_data; }
        [[nodiscard]] int64_t length() const noexcept { return m_length; }
        [[nodiscard]] bool isLineFinished() const noexcept { return m_line_finished; }

        [[nodiscard]] int32_t parsedFieldsCount() const noexcept { return m_curr_field_index + 1; }
        [[nodiscard]] int32_t currFieldIndex() const noexcept { return m_curr_field_index; }
        [[nodiscard]] String currFieldString() noexcept { return m_curr_field; }
        [[nodiscard]] const char* currFieldStrPtr() noexcept { return m_curr_field.utf8(); }
        [[nodiscard]] char currFieldFirstChar() noexcept { return m_curr_field.firstAsciiChar(); }
        [[nodiscard]] int32_t currFieldCompare(const char* str) noexcept {
            return m_curr_field.compare(str); }
        [[nodiscard]] bool isCurrFieldSameAs(const char* str) noexcept {
            return m_curr_field.compare(str) == 0;
        }
        void trimCurrField() noexcept { m_curr_field.trim(); }

        [[nodiscard]] bool hasError() const noexcept { return m_err != kErrNone; }
        [[nodiscard]] int32_t lastError() const noexcept { return m_err; }
        [[nodiscard]] const char* lastErrorName() const noexcept {
            static const char* _names[] = { "none", "format error", "inside quote mismatch", "UTF-8 mismatch", "unknown" };
            return m_err >= 0 && m_err < kErrCount ? _names[m_err] : _names[kErrCount];
        }
        [[nodiscard]] int64_t lastErrorOffs() const noexcept { return m_err_data_offs; }

        void setCharSet(CharSet char_set) noexcept { m_char_set = char_set; }
        void setDelimiter(char delimiter) noexcept { m_delimiter = delimiter;}
        void setQuote(char quote) noexcept { m_quote = quote; }

        bool setLine(const String& string) noexcept { return setLine(string.utf8()); }
        bool setLine(const char* str) noexcept;

        void rewind() noexcept;

        [[nodiscard]] const char* valueStr() noexcept { return m_curr_field.utf8(); }
        [[nodiscard]] int64_t valueStrByteLength() noexcept { return m_curr_field.byteLength(); }

        [[nodiscard]] bool next();

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>)
        bool next(T& out_value) {
            if (next()) {
                out_value = value<T>();
                return true;
            }
            return false;
        }

        template <typename T>
        requires (std::integral<T>)
        T value() {
            return static_cast<T>(atoll(valueStr()));
        }

        template <typename T>
        requires (std::floating_point<T>)
        T value() {
            return static_cast<T>(String::parseDoubleWithDotOrComma(valueStr()));
        }

        template <typename T>
        requires (std::integral<T>)
        int32_t values(int32_t n, T* out_values) {
            int32_t i = 0;
            char str[100];
            for (i = 0; i < n; i++) {
                if (nextStr(100, str)) {
                    String::strToVar(str, out_values[i]);
                }
                else {
                    break;
                }
            }

            return i;
        }

        template <typename T>
        requires (std::floating_point<T>)
        int32_t values(int32_t n, T* out_values) {
            int32_t i = 0;
            for (i = 0; i < n; i++) {
                char str[100];
                if (nextStr(100, str)) {
                    double value;
                    String::strToDouble(str, value);
                    out_values[i] = static_cast<T>(value);
                }
                else {
                    break;
                }
            }

            return i;
        }

        bool nextFix(Fix& out_value) noexcept;
        bool nextFlags(Flags& out_value) noexcept;
        bool nextStr(char** out_str) noexcept;
        bool nextStr(int64_t max_length, char* out_str) noexcept;
        bool nextString(String& out_string) noexcept;

        bool skipFields(int32_t n) noexcept;
    };


    typedef void (*CSVScannerFieldFunc)(const CSVScanner& scanner, const String& field);


    class CSVScanner {
    public:
        CSVScanner(const String& file_path, CSVScannerFieldFunc field_func);
        ~CSVScanner();
        void scan() noexcept;

        [[nodiscard]] int64_t rowCount() const noexcept { return m_row_count; }
        [[nodiscard]] int64_t rowIndex() const noexcept { return m_row_index; }
        [[nodiscard]] int64_t colIndex() const noexcept { return m_col_index; }

        [[nodiscard]] void* dataRef() const noexcept { return m_data_ref; }

        void setHasHeaderFlag(bool flag) noexcept { m_has_header_flag = flag; }
        void setTrimAll(bool flag) noexcept { m_trim_all_fields_flag = flag; }
        void setDataRef(void* data_ref) noexcept { m_data_ref = data_ref; }

    public:
        enum {
            kErrCSVStringError = 0
        };

    protected:
        String m_file_path;
        CharSet m_char_set = CharSet::UTF8;
        char m_delimiter = ',';
        char m_quote = '"';
        StringList* m_header_labels = nullptr;

        int64_t m_row_count = -1;
        int64_t m_row_index = -1;
        int64_t m_col_index = -1;

        bool m_has_header_flag = false;
        bool m_trim_all_fields_flag = false;
        bool m_break_on_err_flag = false;

        CSVScannerFieldFunc m_field_func = nullptr;
        void* m_data_ref = nullptr;
    };


} // End of namespace Grain

#endif // GrainCSVString_hpp
