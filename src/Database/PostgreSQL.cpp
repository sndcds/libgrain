//
//  PostgrSQL.cpp
//
//  Created by Roald Christesen on 13.09.2024.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 21.08.2025
//

#include "Database/PostgreSQL.hpp"
#include "Core/Log.hpp"

#include <libpq-fe.h>


namespace Grain {

    void _grain_psqlNoticeReceiver(void* arg, const PGresult* res) {
        auto notices = (StringList*)arg;

        // Extract the notice message from the result
        const char* message = PQresultErrorMessage(res);

        if (message != nullptr) {
            notices->pushString(message);
        }
    }


    PSQLParam::PSQLParam(PSQLType type, PSQLParamFormat format, const char* value, int32_t length, void* bin_ptr) {
        m_type = type;
        m_format = format;
        m_value = value;
        length_ = length;
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


    ErrorCode PSQLParamList::addParam(PSQLType type, const String& value) noexcept {
        return addParam(type, value.utf8());
    }


    PSQLConnection::~PSQLConnection() {
        close();
    }


    ErrorCode PSQLConnection::open() noexcept {
        if (!_m_pg_conn_ptr) {
            PGconn* pg_conn = nullptr;

            String connection_info;
            connection_info += "host=";
            connection_info += m_host;
            connection_info += " port=";
            connection_info += m_port;
            connection_info += " dbname=";
            connection_info += m_db_name;
            connection_info += " user=";
            connection_info += m_user;
            connection_info += " password=";
            connection_info += m_password;
            connection_info += " connect_timeout=5";

            pg_conn = PQconnectdb(connection_info.utf8());

            if (PQstatus(pg_conn) != CONNECTION_OK) {
                m_last_err_message = "Unable to connect to database.";
                PQfinish(pg_conn);
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


    PSQLResult PSQLConnection::query(
            const String& sql,
            PSQLResult::Format result_format) noexcept {

        static constexpr int32_t kMaxParams = 32;

        PSQLResult psql_result;

        auto pg_conn = (PGconn*)_m_pg_conn_ptr;
        if (!pg_conn) {
            psql_result.m_last_err = ErrorCode::DatabaseNotConnected;
            return psql_result;
        }

        PGresult* pg_res = PQexecParams(
                pg_conn,
                sql.utf8(),
                0,
                NULL,
                NULL,
                NULL,
                NULL,
                result_format == PSQLResult::Format::Text ? 0 : 1
        );

        _collectResult(pg_res, psql_result);
        return psql_result;
    }


    PSQLResult PSQLConnection::query(
            const String& sql,
            const PSQLParamList& param_list,
            PSQLResult::Format result_format) noexcept {

        static constexpr int32_t kMaxParams = 32;

        PSQLResult psql_result;

        Oid param_types[kMaxParams];
        const char* param_values[kMaxParams];
        int param_lengths[kMaxParams];
        int param_formats[kMaxParams];

        auto pg_conn = static_cast<PGconn*>(_m_pg_conn_ptr);
        if (!pg_conn) {
            psql_result.m_last_err = ErrorCode::DatabaseNotConnected;
            return psql_result;
        }

        auto param_count = static_cast<int32_t>(param_list.size());
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
                switch (param->m_type) {
                    /* TODO: Implement !!!
                    case PSQLType::Boolean: {
                        // Binary = 1 byte (0 or 1)
                        static unsigned char b;
                        b = param->m_value.asBool() ? 1 : 0;
                        param_values[index] = reinterpret_cast<char*>(&b);
                        param_lengths[index] = 1;
                        break;
                    }

                    case PSQLType::ByteArray: {
                        // Binary raw data
                        param_values[index] = param->m_value.bytes();
                        param_lengths[index] = param->m_value.size();
                        break;
                    }

                    case PSQLType::Char:
                    case PSQLType::Name:
                    case PSQLType::Text:
                    case PSQLType::VarChar:
                    case PSQLType::CharN: {
                        // All string-like types: just use text format usually.
                        // If you insist on binary: it's a length-prefixed internal format (messy).
                        // → Recommend keeping these as TEXT (param_formats=0).
                        break;
                    }

                    case PSQLType::SmallInt: {
                        // int16, must be sent in network byte order
                        static int16_t v;
                        v = htons(param->m_value.asInt16());
                        param_values[index] = reinterpret_cast<char*>(&v);
                        param_lengths[index] = sizeof(v);
                        break;
                    }

                    case PSQLType::Integer: {
                        // int32
                        static int32_t v;
                        v = htonl(param->m_value.asInt32());
                        param_values[index] = reinterpret_cast<char*>(&v);
                        param_lengths[index] = sizeof(v);
                        break;
                    }

                    case PSQLType::BigInt: {
                        // int64 (need 64-bit network order helper)
                        static int64_t v;
                        v = htobe64(param->m_value.asInt64()); // use portable macro
                        param_values[index] = reinterpret_cast<char*>(&v);
                        param_lengths[index] = sizeof(v);
                        break;
                    }

                    case PSQLType::OID: {
                        // OID is uint32
                        static uint32_t v;
                        v = htonl(param->m_value.asUInt32());
                        param_values[index] = reinterpret_cast<char*>(&v);
                        param_lengths[index] = sizeof(v);
                        break;
                    }

                    case PSQLType::Real: {
                        // float4: must be bitwise, in network order
                        static uint32_t v;
                        float f = param->m_value.asFloat();
                        memcpy(&v, &f, 4);
                        v = htonl(v);
                        param_values[index] = reinterpret_cast<char*>(&v);
                        param_lengths[index] = 4;
                        break;
                    }

                    case PSQLType::Double: {
                        // float8
                        static uint64_t v;
                        double d = param->m_value.asDouble();
                        memcpy(&v, &d, 8);
                        v = htobe64(v);
                        param_values[index] = reinterpret_cast<char*>(&v);
                        param_lengths[index] = 8;
                        break;
                    }

                    case PSQLType::Date:
                    case PSQLType::Timestamp:
                    case PSQLType::TimestampZ:
                    case PSQLType::Numeric:
                    case PSQLType::JSON:
                    case PSQLType::Void:
                    case PSQLType::WKB: {
                        // These are *complicated binary formats* (internal PostgreSQL encodings).
                        // Unless you *really need binary*, keep them in TEXT mode.
                        break;
                    }
                     */
                }
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
                result_format == PSQLResult::Format::Text ? 0 : 1
        );

        _collectResult(pg_res, psql_result);
        return psql_result;
    }


    const char* PSQLConnection::errorMessage() const noexcept {
        return PQerrorMessage(static_cast<PGconn*>(_m_pg_conn_ptr));
    }


    void PSQLConnection::_collectResult(void* pg_result_ptr, Grain::PSQLResult &out_result) {
        out_result.clear();
        if (!pg_result_ptr) {
            out_result.m_exec_status = PSQLResult::ExecStatus::FatalError;
            return;
        }

        out_result.m_pg_result_ptr = pg_result_ptr;

        auto pg_status = PQresultStatus(static_cast<PGresult*>(pg_result_ptr));
        switch (pg_status) {
            case PGRES_EMPTY_QUERY:
                out_result.m_exec_status = PSQLResult::ExecStatus::EmptyQuery;
                break;
            case PGRES_COMMAND_OK:
                out_result.m_exec_status = PSQLResult::ExecStatus::CommandOK;
                if (!String::strToVar(PQcmdTuples(static_cast<PGresult*>(pg_result_ptr)), out_result.m_rows_affected)) {
                    out_result.m_rows_affected = -1;
                }
                break;
            case PGRES_TUPLES_OK:
                out_result.m_exec_status = PSQLResult::ExecStatus::TuplesOK;
                out_result.m_field_n = PQnfields(static_cast<PGresult*>(pg_result_ptr));
                out_result.m_tuple_n = PQntuples(static_cast<PGresult*>(pg_result_ptr));
                break;
            case PGRES_COPY_OUT:
                out_result.m_exec_status = PSQLResult::ExecStatus::CopyOut;
                break;
            case PGRES_COPY_IN:
                out_result.m_exec_status = PSQLResult::ExecStatus::CopyIn;
                break;
            case PGRES_BAD_RESPONSE:
                out_result.m_exec_status = PSQLResult::ExecStatus::BadResponse;
                break;
            case PGRES_NONFATAL_ERROR:
                out_result.m_exec_status = PSQLResult::ExecStatus::NonfatalError;
                break;
            case PGRES_FATAL_ERROR:
                out_result.m_exec_status = PSQLResult::ExecStatus::FatalError;
                break;
            case PGRES_COPY_BOTH:
                out_result.m_exec_status = PSQLResult::ExecStatus::CopyBoth;
                break;
            case PGRES_SINGLE_TUPLE:
                out_result.m_exec_status = PSQLResult::ExecStatus::SingleTuple;
                break;
            case PGRES_PIPELINE_SYNC:
                out_result.m_exec_status = PSQLResult::ExecStatus::PipelineSync;
                break;
            case PGRES_PIPELINE_ABORTED:
                out_result.m_exec_status = PSQLResult::ExecStatus::PipelineAborted;
                break;
        }
    }


    ErrorCode PSQLConnection::_psqlStatementTimeout(double sec) noexcept {
        auto pg_conn = (PGconn*)_m_pg_conn_ptr;
        if (!pg_conn || PQstatus(pg_conn) != CONNECTION_OK) {
            return ErrorCode::DatabaseNotConnected;
        }

        String sql = "SET statement_timeout = ";
        if (sec <= 0.0) {
            sql += "0";
        }
        else {
            sql += sec;
            sql += "s";
        }
        PGresult* res = PQexec(pg_conn, sql.utf8());
        if (!res) {
            m_last_err_message = "Unable to set statement timeout: ";
            m_last_err_message += PQerrorMessage(pg_conn);
            return ErrorCode::DatabaseSetTimeoutFailed;
        }

        ExecStatusType status = PQresultStatus(res);
        if (status != PGRES_COMMAND_OK) {
            m_last_err_message = "Error while setting statement timeout ";
            m_last_err_message += PQresultErrorMessage(res);
            PQclear(res);
            return ErrorCode::DatabaseSetTimeoutFailed;
        }

        PQclear(res);

        return ErrorCode::None;
    }


    PSQLConnection* PSQLConnections::addConnection() noexcept {
        auto connection = new (std::nothrow) PSQLConnection();
        if (connection) {
            m_connections.push(connection);
        }
        return connection;
    }


    PSQLConnection* PSQLConnections::addConnection(
            const char* identifier,
            const char* host,
            int32_t port,
            const char* db_name,
            const char* user,
            const char* password) noexcept {

        auto conn = addConnection();
        if (conn) {
            conn->m_identifier = identifier;
            conn->m_host = host;
            conn->m_port = port;
            conn->m_db_name = db_name;
            conn->m_user = user;
            conn->m_password = password;
        }
        return conn;
    }


    PSQLConnection* PSQLConnections::connectionByIdentifier(const String& identifier) {
        for (auto connection : m_connections) {
            if (connection->m_identifier == identifier) {
                connection->open();
                return connection;
            }
        }
        return nullptr;
    }


    PSQLConnection* PSQLConnections::firstConnection() {
        if (m_connections.size() > 0) {
            auto connection = m_connections.first();
            if (connection != nullptr) {
                connection->open();
                return connection;
            }
        }
        return nullptr;
    }


    void PSQLResult::log(Log& l) const noexcept {
        l << "PSQLResult" << l.endl;
        if (m_pg_result_ptr) {
            l++;
            auto pg_res = static_cast<PGresult*>(m_pg_result_ptr);
            if (m_exec_status == ExecStatus::TuplesOK) {
                l << "m_exec_status: TuplesOK" << l.endl;
                l << "m_tuple_n: " << m_tuple_n << l.endl;
                l << "m_field_n: " << m_field_n << l.endl;
            }
            else if (m_exec_status == ExecStatus::CommandOK) {
                l << "m_exec_status: CommandOK" << l.endl;
                l << "affected rows: " << m_rows_affected << l.endl;
            }
            l--;
        }
    }

    /**
     *  @brief Check if the query result contains usable tuples (rows).
     *
     *  Returns true only if the underlying PostgreSQL result status
     *  is PGRES_TUPLES_OK, meaning the query successfully produced
     *  a row set that can be iterated over.
     */
    bool PSQLResult::areTuplesOK() const noexcept {
        if (!m_pg_result_ptr) {
            return false;
        }
        return PQresultStatus(static_cast<PGresult*>(m_pg_result_ptr)) == PGRES_TUPLES_OK;
    }


    void PSQLResult::clear() noexcept {
        if (m_pg_result_ptr) {
            PQclear(static_cast<PGresult*>(m_pg_result_ptr));
            m_pg_result_ptr = nullptr;
        }
    }


    PSQLType PSQLResult::fieldType(int32_t column_index) const noexcept {
        return static_cast<PSQLType>(PQftype(static_cast<PGresult*>(m_pg_result_ptr), column_index));
    }


    /**
     *  @note // Note: Does allways return the real name and not the alias
     */
    const char* PSQLResult::fieldName(int32_t column_index) const noexcept {
        if (m_pg_result_ptr && column_index >= 0 && column_index < m_field_n) {
            return PQfname(static_cast<PGresult*>(m_pg_result_ptr), column_index);
        }
        return nullptr;
    }


    const char* PSQLResult::fieldValue(int32_t row_index, int32_t column_index) const noexcept {
        if (m_pg_result_ptr &&
            row_index >= 0 && row_index < m_tuple_n &&
            column_index >= 0 && column_index < m_field_n) {
            return PQgetvalue(static_cast<PGresult*>(m_pg_result_ptr), row_index, column_index);
        }
        return nullptr;
    }


    int32_t PSQLResult::fieldLength(int32_t row_index, int32_t column_index) const noexcept {
        return PQgetlength(static_cast<PGresult*>(m_pg_result_ptr), row_index, column_index);
    }


    bool PSQLResult::fieldIsNull(int32_t row_index, int32_t column_index) const noexcept {
        return PQgetisnull(static_cast<PGresult*>(m_pg_result_ptr), row_index, column_index);
    }


    PSQLPropertyList::PSQLPropertyList(int32_t size) {
        properties_ = new (std::nothrow) PSQLProperty[size];
        if (!properties_) {
            size_ = 0;
        }
        else {
            size_ = size;
        }
    }


    PSQLPropertyList::~PSQLPropertyList() {
        delete [] properties_;
    }


    PSQLProperty* PSQLPropertyList::mutPropertyPtrAtIndex(int32_t index) noexcept {
        if (properties_ != nullptr && index >= 0 && index < size_) {
            return &properties_[index];
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
            const void* data,
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
