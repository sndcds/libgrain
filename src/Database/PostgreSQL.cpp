//
//  PostgrSQL.cpp
//
//  Created by Roald Christesen on 13.09.2024.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 20.08.2025
//

#include "Database/PostgreSQL.hpp"
#include "Core/Log.hpp"

#include <libpq-fe.h>


namespace Grain {

    void _grain_psqlNoticeReceiver(void *arg, const PGresult *res) {
        auto notices = (StringList*)arg;

        // Extract the notice message from the result
        const char *message = PQresultErrorMessage(res);

        if (message != nullptr) {
            notices->pushString(message);
        }
    }


    PSQLParam::PSQLParam(PSQLType type, PSQLParamFormat format, const char* value, int32_t length, void* bin_ptr) {
        m_type = type;
        m_format = format;
        m_value = value;
        m_length = length;
        m_bin_ptr = bin_ptr;
    }


    ErrorCode PSQLParamList::addParam(PSQLType type, const char* value) noexcept {
        auto param = new (std::nothrow) PSQLParam(type, PSQLParamFormat::Text, value, 0, nullptr);
        if (!param) {
            return ErrorCode::MemCantAllocate;
        }
        else {
            push(param);
            return ErrorCode::None;
        }
    }


    ErrorCode PSQLConnection::open() noexcept {
        if (!_m_pg_conn_ptr) {
            PGconn* pg_conn = nullptr;
            char connection_info[2056];
            std::snprintf(connection_info, 2056, "host=%s port=%d dbname=%s user=%s password=%s", m_host.utf8(), m_port, m_dbname.utf8(), m_user.utf8(), m_password.utf8());

            pg_conn = PQconnectdb(connection_info);

            if (PQstatus(pg_conn) != CONNECTION_OK) {
                m_last_err_message = "Unable to connect to database.";
                return Error::specific(kErrConnectionFailed);
            }

            PQsetNoticeReceiver(pg_conn, _grain_psqlNoticeReceiver, &m_psql_notices);
            _m_pg_conn_ptr = pg_conn;
        }

        return ErrorCode::None;
    }


    void PSQLConnection::close() noexcept {
        if (_m_pg_conn_ptr) {
            PQfinish((PGconn*)_m_pg_conn_ptr);
            _m_pg_conn_ptr = nullptr;
        }
    }


    PSQLConnection::Status PSQLConnection::status() noexcept {
        switch (PQstatus((PGconn*)_m_pg_conn_ptr)) {
            case CONNECTION_OK: return Status::Ok;
            case CONNECTION_BAD: return Status::Bad;
            case CONNECTION_STARTED: return Status::Started;
            case CONNECTION_MADE: return Status::Made;
            case CONNECTION_AWAITING_RESPONSE: return Status::AwaitingResponse;
            case CONNECTION_AUTH_OK: return Status::AuthOk;
            case CONNECTION_SETENV: return Status::SetEnv;
            case CONNECTION_SSL_STARTUP: return Status::SSLStartup;
            case CONNECTION_NEEDED: return Status::Needed;
            case CONNECTION_CHECK_WRITABLE: return Status::CheckWriteable;
            case CONNECTION_CONSUME: return Status::Consume;
            case CONNECTION_GSS_STARTUP: return Status::GSSStartup;
            case CONNECTION_CHECK_TARGET: return Status::CheckTarget;
            case CONNECTION_CHECK_STANDBY: return Status::CheckStandby;
            default: return Status::Unknown;
        }
    }

    void PSQLConnection::query(const String &sql, const PSQLParamList& param_list) noexcept {
        static constexpr int32_t kMaxParams = 32;

        Oid param_types[kMaxParams];
        const char* param_values[kMaxParams];
        int param_lengths[kMaxParams];
        int param_formats[kMaxParams];

        auto pg_conn = (PGconn*)_m_pg_conn_ptr;
        if (!pg_conn) {
            return;
        }

        Log l;

        auto param_count = (int32_t)param_list.size();
        int32_t index = 0;
        for (auto param : param_list) {
            param_types[index] = (Oid)param->m_type;
            param_values[index] = param->m_value.utf8();

            if (param->m_format == PSQLParamFormat::Text) {
                param_formats[index] = 0;   // 0 = text
                param_lengths[index] = 0;
            }
            else {
                param_formats[index] = 1;   // 1 = binary
                // TODO: Implement binary length for all types
            }

            index++;
        }

        PGresult* pg_res = PQexecParams(
                pg_conn,
                sql.utf8(),
                param_count,
                param_types,
                param_values,
                param_lengths,
                param_formats,
                0  // resultFormat: 0 = text, 1 = binary
        );

        _m_pg_res_ptr = pg_res;
        _m_pg_status = PQresultStatus(pg_res);
        if (_m_pg_status != PGRES_TUPLES_OK && _m_pg_status != PGRES_COMMAND_OK) {
            // TODO: Message
            std::cerr << "Execution failed: " << PQerrorMessage(pg_conn) << ", " << _m_pg_status << std::endl;
        }
        else {
            _m_field_n = PQnfields(pg_res);
            _m_tuple_n = PQntuples(pg_res);
        }
    }


    void PSQLConnection::clear() noexcept {
        if (_m_pg_res_ptr) {
            PQclear((PGresult *) _m_pg_res_ptr);
            _m_pg_res_ptr = nullptr;
        }
    }


    const char* PSQLConnection::fieldName(int32_t column_index) noexcept {
        if (_m_pg_res_ptr && column_index >= 0 && column_index < _m_field_n) {
            return PQfname((PGresult *) _m_pg_res_ptr, column_index);
        }
        else {
            return nullptr;
        }
    }


    const char* PSQLConnection::fieldValue(int32_t row_index, int32_t column_index) noexcept {
        if (_m_pg_res_ptr &&
            row_index >= 0 && row_index < _m_tuple_n &&
            column_index >= 0 && column_index < _m_field_n) {
            return PQgetvalue((PGresult *)_m_pg_res_ptr, row_index, column_index);
        }
        else {
            return nullptr;
        }
    }


    void PSQLConnection::logResult(Log& l) noexcept {
        if (_m_pg_res_ptr) {
            auto pg_res = (PGresult *)_m_pg_res_ptr;
            if (_m_pg_status == PGRES_TUPLES_OK || _m_pg_status == PGRES_COMMAND_OK) {

                // Print column headers
                for (int i = 0; i < _m_field_n; i++) {
                    std::cout << PQfname(pg_res, i);
                    if (i < _m_field_n - 1) std::cout << " | ";
                }
                std::cout << "\n";

                // Print rows
                for (int row = 0; row < _m_tuple_n; row++) {
                    for (int col = 0; col < _m_field_n; col++) {
                        char* value = PQgetvalue(pg_res, row, col);
                        std::cout << (value ? value : "NULL");
                        if (col < _m_field_n - 1) std::cout << " | ";
                    }
                    std::cout << "\n";
                }

            }
        }
    }


    PSQLConnection *PSQLConnections::addConnection() noexcept {
        auto connection = new(std::nothrow) PSQLConnection();
        if (connection) {
            m_connections.push(connection);
        }
        return connection;
    }


    PSQLConnection *PSQLConnections::connectionByIdentifier(const String &identifier) {
        for (auto connection : m_connections) {
            if (connection->m_identifier == identifier) {
                connection->open();
                return connection;
            }
        }
        return nullptr;
    }


    PSQLConnection *PSQLConnections::firstConnection() {
        if (m_connections.size() > 0) {
            auto connection = m_connections.first();
            if (connection != nullptr) {
                connection->open();
                return connection;
            }
        }
        return nullptr;
    }


    PSQLPropertyList::PSQLPropertyList(int32_t size) {
        m_properties = new PSQLProperty[size];
        if (m_properties != nullptr) {
            m_size = size;
        }
        else {
            m_size = 0;
        }
    }


    PSQLPropertyList::~PSQLPropertyList() {
        delete [] m_properties;
    }


    PSQLProperty *PSQLPropertyList::mutPropertyPtrAtIndex(int32_t index) noexcept {
        if (m_properties != nullptr && index >= 0 && index < m_size) {
            return &m_properties[index];
        }
        else {
            return nullptr;
        }
    }


    const char* PSQLPropertyList::stringFromPropertyAtIndex(int32_t index) noexcept {
        auto p = mutPropertyPtrAtIndex(index);
        if (p != nullptr && p->m_type == PSQLPropertyType::String) {
            return p->m_string.utf8();
        }

        return nullptr;
    }


    void PSQLPropertyList::setPropertyAtIndexByPSQLBinaryData(
            int32_t index,
            PSQLType psql_type,
            const void *data,
            int32_t data_size) {
        auto p = mutPropertyPtrAtIndex(index);
        if (p != nullptr) {
            p->m_psql_type = psql_type;

            switch (psql_type) {
                case PSQLType::Undefined:
                    p->m_type = PSQLPropertyType::Undefined;
                    break;

                case PSQLType::Boolean:
                    p->m_type = PSQLPropertyType::Boolean;
                    p->m_integer = (*(const char*)data);
                    break;

                case PSQLType::SmallInt: {
                    p->m_type = PSQLPropertyType::Integer;
                    int16_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(int16_t));
                    p->m_integer = ntoh16(sql_value);
                    break;
                }

                case PSQLType::Integer: {
                    p->m_type = PSQLPropertyType::Integer;
                    int32_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(int32_t));
                    p->m_integer = ntoh32(sql_value);
                    break;
                }

                case PSQLType::BigInt: {
                    p->m_type = PSQLPropertyType::Integer;
                    int64_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(int64_t));
                    p->m_integer = ntoh64(sql_value);
                    break;
                }

                case PSQLType::OID: {
                    p->m_type = PSQLPropertyType::Integer;
                    uint32_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(uint32_t));
                    p->m_integer = ntoh32(sql_value);
                    break;
                }

                case PSQLType::Char:
                case PSQLType::VarChar:
                case PSQLType::Text:
                case PSQLType::CharN:
                case PSQLType::Name:
                case PSQLType::JSON:
                    p->m_type = PSQLPropertyType::String;
                    p->m_string.setByStr((const char*)data, data_size);
                    break;

                case PSQLType::Real: {
                    p->m_type = PSQLPropertyType::Double;
                    uint32_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(uint32_t));
                    sql_value = ntoh32(sql_value);  // Convert from network byte order to host byte order
                    p->m_double = static_cast<double>(*reinterpret_cast<float*>(&sql_value));
                    break;
                }

                case PSQLType::Double: {
                    p->m_type = PSQLPropertyType::Double;
                    uint64_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(uint64_t));
                    sql_value = ntoh64(sql_value);  // Convert from network byte order to host byte order
                    p->m_double = *reinterpret_cast<double*>(&sql_value);
                    break;
                }

                case PSQLType::Numeric: {
                    p->m_type = PSQLPropertyType::Numeric;
                    PSQL::numericToString((uint8_t*)data, p->m_string);
                    p->m_double = p->m_string.asDouble();
                    p->m_integer = p->m_string.asInt64();
                    break;
                }

                case PSQLType::ByteArray:
                case PSQLType::Date:
                case PSQLType::Timestamp:
                case PSQLType::TimestampZ:
                case PSQLType::Void:
                case PSQLType::WKB:
                default:
                    /* TODO: !!!!
                     // Do something with the binary data (for example, printing byte values)
                     std::cout << "Binary data (length: " << sql_data_size << "): ";
                     for (int i = 0; i < sql_data_size; ++i) {
                     printf("%02X ", data[i]);
                     }
                     std::cout << std::endl;
                     */
                    break;
            }
        }
    }

}  // End of namespace Grain
