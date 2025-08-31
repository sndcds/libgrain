//
//  CSVData.cpp
//
//  Created by Roald Christesen on 30.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#include "Core/Log.hpp"
#include "String/CSVData.hpp"
#include "String/StringList.hpp"
#include "File/File.hpp"
#include "Scripting/Lua.hpp"


namespace Grain {

    void CSVData::log(Log& l) {
        l.header(className());
        l << "m_row_n: " << m_row_n << l.endl;
        l << "m_used_column_n: " << m_used_column_n << l.endl;
        l << "m_item_n: " << m_item_n << l.endl;
        l << "m_str_mem_size: " << m_str_mem_size << l.endl;
        l << "m_data_mem_size: " << m_data_mem_size << l.endl;
        if (m_used_column_n > 0 && m_column_infos != nullptr) {
            l.header("Column infos");
            for (int32_t i = 0; i < m_used_column_n; i++) {
                l << i << ": index: " << m_column_infos[i].m_index << ", key: " << m_column_infos[i].m_key << ", type " << (int32_t)m_column_infos[i].m_type << l.endl;
            }
        }
    }


    /**
     *  @brief Create CSV data from a CSV file.
     *
     *  @param file_path File path to CSV file.
     *  @param column_infos Name/value pairs for each column of CSV data which should be recognized.
     *
     *  @return `ErrorCode::None` or an error code.
     */
    ErrorCode CSVData::createFromFile(const String& file_path, const CSVDataColumnInfo* column_infos, bool has_header) noexcept {
        auto result = ErrorCode::None;

        // TODO: Check if CSV has columns defined in field_infos!

        File* file = nullptr;

        try {
            if (!column_infos) {
                throw ErrorCode::NullPointer;
            }

            file = File::createFile(file_path);
            file->startReadAscii();

            String text_line;
            CSVLineParser csv_line_parser;
            csv_line_parser.setDelimiter(m_delimiter);
            csv_line_parser.setQuote(m_quote);

            // file->readLine(text_line);

            m_used_column_n = 0;
            for (int32_t i = 0; column_infos[i].m_index >= 0; i++) {
                m_used_column_n++;
            }

            if (m_used_column_n < 1) {
                throw ErrorCode::BadArgs;
            }

            m_column_infos = (CSVDataColumnInfo*)malloc(m_used_column_n * sizeof(CSVDataColumnInfo));
            if (!(m_column_infos)) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }

            for (int32_t i = 0; i < m_used_column_n; i++) {
                m_column_infos[i].set(&column_infos[i]);
            }

            for (int32_t pass = 0; pass < 2; pass++) {
                if (pass > 0) {
                    file->rewind();
                }

                if (has_header == true) {
                    file->skipLine();
                }

                if (pass == 0) {
                    m_row_n = 0;

                    while (file->readLine(text_line) == true) {
                        if (csv_line_parser.setLine(text_line) == true) {
                            String string_value;
                            int32_t column_index = 0;

                            while (csv_line_parser.next() == true) {
                                for (int32_t i = 0; i < m_used_column_n; i++) {
                                    if (column_infos[i].m_index == column_index) {
                                        switch (column_infos[i].m_type) {
                                            case CSVDataColumnInfo::DataType::String:
                                            case CSVDataColumnInfo::DataType::WKB:
                                                m_str_mem_size += csv_line_parser.valueStrByteLength() + 1;
                                                break;
                                            default:
                                                break;
                                        }
                                    }
                                }
                                column_index++;
                            }
                            m_row_n++;
                        }
                    }

                    m_item_n = m_row_n * m_used_column_n;

                    // End of pass 0
                }
                else if (pass == 1) {
                    // Allocate memory
                    m_data_mem_size = sizeof(int64_t) * m_item_n;
                    m_data = (int64_t*)malloc(m_data_mem_size);
                    if (!m_data) {
                        Exception::throwStandard(ErrorCode::MemCantAllocate);
                    }

                    m_str_data = (char*)malloc(m_str_mem_size);
                    if (!m_str_data) {
                        Exception::throwStandard(ErrorCode::MemCantAllocate);
                    }

                    int64_t row_index = 0;
                    int64_t string_offset = 0;

                    while (file->readLine(text_line) == true) {
                        if (csv_line_parser.setLine(text_line) == true) {
                            int32_t column_index = 0;

                            if (row_index >= m_row_n) {
                                Exception::throwSpecific(1); // TODO: !!! Message
                            }

                            while (csv_line_parser.next() == true) {
                                for (int32_t i = 0; i < m_used_column_n; i++) {
                                    if (column_infos[i].m_index == column_index) {
                                        switch (column_infos[i].m_type) {
                                            case CSVDataColumnInfo::DataType::Int64:
                                                setInt64Value(row_index, i, csv_line_parser.value<int64_t>());
                                                break;
                                            case CSVDataColumnInfo::DataType::Double:
                                                setDoubleValue(row_index, i, csv_line_parser.value<double>());
                                                break;
                                            case CSVDataColumnInfo::DataType::String:
                                            case CSVDataColumnInfo::DataType::WKB:
                                            {
                                                strcpy(&m_str_data[string_offset], csv_line_parser.valueStr());
                                                setInt64Value(row_index, i, string_offset);
                                                string_offset += csv_line_parser.valueStrByteLength();
                                                m_str_data[string_offset] = '\0';
                                                string_offset++;
                                            }
                                                break;
                                            default:
                                                break;
                                        }
                                    }
                                }

                                column_index++;
                            }

                            row_index++;
                        }
                    }
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        delete file;

        return result;
    }


    void CSVData::setLuaGlobal(Lua* lua, const char* table_name, const char* name_prefix, int64_t row_index, int32_t column_index) {
        #pragma message("CSVData::setLuaGlobal must be implemented!")

        /*
        if (lua != nullptr) {

            if (isIndex(row_index, column_index)) {

                CSVDataField* field_info = &m_field_infos[column_index];
                int64_t data_index = row_index * m_used_field_n + column_index;

                char* var_name = nullptr;
                char var_name_buffer[256];
                if (name_prefix != nullptr) {
                    const char* p = name_prefix;
                    char* g = var_name_buffer;
                    while (*p != '\0') {
                        *g++ = *p++;
                    }
                    char* k = field_info->m_key;
                    while (*k != '\0') {
                        *g++ = *k++;
                    }
                    *g = '\0';
                    var_name = var_name_buffer;
                }
                else {
                    var_name = field_info->m_key;
                }

                if (table_name != nullptr) {
                    switch (field_info->m_type) {
                        case CSVDataField::Type::Int64:
                            lua->setGlobalTableInteger(table_name, var_name, m_data[data_index]);
                            break;

                        case CSVDataField::Type::Double:
                            lua->setGlobalTableNumber(table_name, var_name, (*((double*)&m_data[data_index])));
                            break;

                        case CSVDataField::Type::String:
                        case CSVDataField::Type::WKB:
                            lua->setGlobalTableString(table_name, var_name, &m_str_data[m_data[data_index]]);
                            break;

                        default:
                            break;
                    }
                }
                else {
                    switch (field_info->m_type) {
                        case CSVDataField::Type::Int64:
                            lua->setGlobalInteger(var_name, m_data[data_index]);
                            break;

                        case CSVDataField::Type::Double:
                            lua->setGlobalNumber(var_name, (*((double*)&m_data[data_index])));
                            break;

                        case CSVDataField::Type::String:
                        case CSVDataField::Type::WKB:
                            lua->setGlobalString(var_name, &m_str_data[m_data[data_index]]);
                            break;

                        default:
                            break;
                    }
                }
            }
        }
         */
    }


}  // End of namespace Grain
