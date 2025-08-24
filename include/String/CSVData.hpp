//
//  CSVData.hpp
//
//  Created by Roald Christesen on 30.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#ifndef GrainCSVData_hpp
#define GrainCSVData_hpp

#include "Type/Type.hpp"
#include "String/String.hpp"
#include "String/CSVString.hpp"


namespace Grain {

    class Lua;

    typedef struct CSVDataColumnInfo {
    public:
        enum class DataType {
            Unknown = 0,
            Int64,
            Double,
            String,
            WKB             ///< Well known binary
        };

        enum class Usage {
            Unknown = 0,
            X,              ///< X-position
            Y               ///< Y-position
        };

    public:
        int32_t m_index;                        ///< Field index
        char m_key[32];                         ///< Key name, maximum 31 chars + '\0'
        DataType m_type = DataType::Unknown;    ///< Field data type
        Usage m_usage = Usage::Unknown;         ///< Field usage, defines, what the value is used for

        void set(int32_t index, const char* key, const char* type_name, const char* usage_name) {
            // Index
            m_index = index;
            // Key
            std::strncpy(m_key, key, 31);
            m_key[31] = '\0';
            // Type
            if (strcmp(type_name, "long") == 0) { m_type = DataType::Int64; }
            else if (strcmp(type_name, "double") == 0) { m_type = DataType::Double; }
            else if (strcmp(type_name, "string") == 0) { m_type = DataType::String; }
            else if (strcmp(type_name, "wkb") == 0) { m_type = DataType::WKB; }
            else { m_type = DataType::Unknown; }
            // Usage
            if (strcmp(usage_name, "x") == 0) { m_usage = Usage::X; }
            else if (strcmp(usage_name, "y") == 0) { m_usage = Usage::Y; }
            else { m_usage = Usage::Unknown; }
        }

        void set(const CSVDataColumnInfo* other) {
            if (other != nullptr) {
                m_index = other->m_index;
                memcpy(m_key, other->m_key, 32);
                m_type = other->m_type;
                m_usage = other->m_usage;
            }
        }

        void setEnd() {
            m_index = -1;
        }

    } CSVDataColumnInfo;


    class CSVData : public Object {
    protected:
        int64_t m_row_n = 0;
        int32_t m_used_column_n = 0;

        size_t m_item_n = 0;
        size_t m_data_mem_size = 0;
        size_t m_str_mem_size = 0;

        CSVDataColumnInfo* m_column_infos = nullptr;
        int64_t* m_data = nullptr;
        char* m_str_data = nullptr;

        CharSet m_char_set = CharSet::UTF8;
        char m_delimiter = ',';
        char m_quote = '"';
        ErrorCode m_last_err = ErrorCode::None;

    public:
        CSVData() = default;
        ~CSVData() {
            free(m_column_infos);
            free(m_data);
            free(m_str_data);
        };

        const char* className() const noexcept override { return "CSVData"; }

        friend std::ostream& operator << (std::ostream& os, const CSVData* o) {
            o == nullptr ? os << "CSVData nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const CSVData& o) {
            os << o.rowCount() << ", " << o.columCount() << std::endl;
            return os;
        }

        void log(Log& l);

        ErrorCode createFromFile(const String& file_path, const CSVDataColumnInfo* column_infos, bool has_header = false) noexcept;

        void clearLastErr() noexcept { m_last_err = ErrorCode::None; }
        [[nodiscard]] ErrorCode lastErr() noexcept {
            auto err = m_last_err;
            m_last_err = ErrorCode::None;
            return err;
        }
        void throwAtLastErr() {
            auto err = m_last_err;
            m_last_err = ErrorCode::None;
            Exception::throwStandard(err);
        }

        [[nodiscard]] CharSet charSet() const noexcept { return m_char_set; }
        [[nodiscard]] char delimiter() const noexcept { return m_delimiter; }
        [[nodiscard]] char quote() const noexcept { return m_quote; }

        void setCharSet(CharSet char_set) noexcept { m_char_set = char_set; }
        void setDelimiter(char delimiter) noexcept { m_delimiter = delimiter; }
        void setQuote(char quote) noexcept { m_quote = quote; }

        [[nodiscard]] int64_t rowCount() const noexcept { return m_row_n; }
        [[nodiscard]] int32_t columCount() const noexcept { return m_used_column_n; }

        [[nodiscard]] inline bool isRow(int32_t row_index) const noexcept {
            return (row_index >= 0 && row_index < m_row_n);
        }

        [[nodiscard]] inline bool isColumn(int32_t column_index) const noexcept {
            return (column_index >= 0 && column_index < m_used_column_n);
        }

        [[nodiscard]] inline bool isIndex(int64_t row_index, int32_t column_index) const noexcept {
            return (row_index >= 0 && row_index < m_row_n && column_index >= 0 && column_index < m_used_column_n);
        }

        [[nodiscard]] inline CSVDataColumnInfo* columnInfoPtr(int32_t column_index) const noexcept {
            return column_index >= 0 && column_index < m_used_column_n ? &m_column_infos[column_index] : nullptr;
        }

        [[nodiscard]] inline CSVDataColumnInfo::DataType columnType(int32_t column_index) const noexcept {
            return column_index >= 0 && column_index < m_used_column_n ? m_column_infos[column_index].m_type : CSVDataColumnInfo::DataType::Unknown;
        }

        [[nodiscard]] inline int64_t index(int64_t row_index, int32_t column_index) const noexcept {
            return row_index * m_used_column_n + column_index;
        }

        [[nodiscard]] int32_t int32Value(int64_t row_index, int32_t column_index) noexcept {
            auto value = int64Value(row_index, column_index);
            if (value < INT32_MIN || value > INT32_MAX) {
                m_last_err = ErrorCode::CSVValueOutOfRange;
                return 0;
            }
            return (int32_t)value;
        }

        [[nodiscard]] int64_t int64Value(int64_t row_index, int32_t column_index) noexcept {
            if (isIndex(row_index, column_index) == false) {
                m_last_err = ErrorCode::CSVIndexOutOfRange;
            }
            else {
                if (columnType(column_index) == CSVDataColumnInfo::DataType::Int64) {
                    return m_data[index(row_index, column_index)];
                }
                else {
                    m_last_err = ErrorCode::CSVTypeError;
                }
            }
            return 0;
        }

        [[nodiscard]] double doubleValue(int64_t row_index, int32_t column_index) noexcept {
            if (isIndex(row_index, column_index) == false) {
                m_last_err = ErrorCode::CSVIndexOutOfRange;
            }
            else {
                auto field_type = columnType(column_index);
                if (field_type == CSVDataColumnInfo::DataType::Double) {
                    return *((double*)&m_data[index(row_index, column_index)]);
                }
                else if (field_type == CSVDataColumnInfo::DataType::Int64) {
                    return (double)(*((int64_t*)&m_data[index(row_index, column_index)]));
                }
                else {
                    m_last_err = ErrorCode::CSVTypeError;
                }
            }
            return 0.0;
        }

        [[nodiscard]] const char* strValue(int64_t row_index, int32_t column_index) noexcept {
            static const char* empty_str = "";
            if (isIndex(row_index, column_index) == false) {
                m_last_err = ErrorCode::CSVIndexOutOfRange;
            }
            else {
                if (columnType(column_index) == CSVDataColumnInfo::DataType::String) {
                    return &m_str_data[m_data[index(row_index, column_index)]];
                }
                else {
                    m_last_err = ErrorCode::CSVTypeError;
                }
            }
            return empty_str;
        }

        void setInt64Value(int64_t row_index, int32_t column_index, int64_t value) noexcept {
            if (isIndex(row_index, column_index)) {
                m_data[index(row_index, column_index)] = value;
            }
        }

        void setDoubleValue(int64_t row_index, int32_t column_index, double value) noexcept {
            if (isIndex(row_index, column_index)) {
                *((double*)&m_data[index(row_index, column_index)]) = value;
            }
        }

        void setLuaGlobal(Lua* lua, const char* table_name, const char* name_prefix, int64_t row_index, int32_t column_index);
    };


} // End of namespace Grain

#endif // GrainCSVData_hpp
