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
#include <bit>


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
    type_ = type;
    format_ = format;
    value_ = value;
    length_ = length;
    bin_ptr_ = bin_ptr;
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
    if (!pg_conn_ptr_) {
        PGconn* pg_conn = nullptr;

        String connection_info;
        connection_info += "host=";
        connection_info += host_;
        connection_info += " port=";
        connection_info += port_;
        connection_info += " dbname=";
        connection_info += db_name_;
        connection_info += " user=";
        connection_info += user_;
        connection_info += " password=";
        connection_info += password_;
        connection_info += " connect_timeout=5";

        pg_conn = PQconnectdb(connection_info.utf8());

        if (PQstatus(pg_conn) != CONNECTION_OK) {
            last_err_message_ = "Unable to connect to database.";
            PQfinish(pg_conn);
            return Error::specific(kErrConnectionFailed);
        }

        PQsetNoticeReceiver(pg_conn, _grain_psqlNoticeReceiver, &psql_notices_);
        pg_conn_ptr_ = pg_conn;
    }

    return ErrorCode::None;
}


void PSQLConnection::close() noexcept {
    if (pg_conn_ptr_) {
        PQfinish((PGconn*)pg_conn_ptr_);
        pg_conn_ptr_ = nullptr;
    }
}


PSQLConnection::Status PSQLConnection::status() noexcept {
    switch (PQstatus((PGconn*)pg_conn_ptr_)) {
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

    auto pg_conn = (PGconn*)pg_conn_ptr_;
    if (!pg_conn) {
        psql_result.last_err_ = ErrorCode::DatabaseNotConnected;
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

    auto pg_conn = static_cast<PGconn*>(pg_conn_ptr_);
    if (!pg_conn) {
        psql_result.last_err_ = ErrorCode::DatabaseNotConnected;
        return psql_result;
    }

    auto param_count = static_cast<int32_t>(param_list.size());
    int32_t index = 0;
    for (auto param : param_list) {
        param_types[index] = (Oid)param->type_;
        param_values[index] = param->value_.utf8();

        if (param->format_ == PSQLParamFormat::Text) {
            param_formats[index] = 0;   // 0 = text
            param_lengths[index] = 0;
        }
        else {
            param_formats[index] = 1;   // 1 = binary
            switch (param->type_) {
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
    return PQerrorMessage(static_cast<PGconn*>(pg_conn_ptr_));
}


void PSQLConnection::_collectResult(void* pg_result_ptr, Grain::PSQLResult &out_result) {
    out_result.clear();
    if (!pg_result_ptr) {
        out_result.exec_status_ = PSQLResult::ExecStatus::FatalError;
        return;
    }

    out_result.pg_result_ptr_ = pg_result_ptr;

    auto pg_status = PQresultStatus(static_cast<PGresult*>(pg_result_ptr));
    switch (pg_status) {
        case PGRES_EMPTY_QUERY:
            out_result.exec_status_ = PSQLResult::ExecStatus::EmptyQuery;
            break;
        case PGRES_COMMAND_OK:
            out_result.exec_status_ = PSQLResult::ExecStatus::CommandOK;
            if (!String::strToVar(PQcmdTuples(static_cast<PGresult*>(pg_result_ptr)), out_result.rows_affected_)) {
                out_result.rows_affected_ = -1;
            }
            break;
        case PGRES_TUPLES_OK:
            out_result.exec_status_ = PSQLResult::ExecStatus::TuplesOK;
            out_result.field_n_ = PQnfields(static_cast<PGresult*>(pg_result_ptr));
            out_result.tuple_n_ = PQntuples(static_cast<PGresult*>(pg_result_ptr));
            break;
        case PGRES_COPY_OUT:
            out_result.exec_status_ = PSQLResult::ExecStatus::CopyOut;
            break;
        case PGRES_COPY_IN:
            out_result.exec_status_ = PSQLResult::ExecStatus::CopyIn;
            break;
        case PGRES_BAD_RESPONSE:
            out_result.exec_status_ = PSQLResult::ExecStatus::BadResponse;
            break;
        case PGRES_NONFATAL_ERROR:
            out_result.exec_status_ = PSQLResult::ExecStatus::NonfatalError;
            break;
        case PGRES_FATAL_ERROR:
            out_result.exec_status_ = PSQLResult::ExecStatus::FatalError;
            break;
        case PGRES_COPY_BOTH:
            out_result.exec_status_ = PSQLResult::ExecStatus::CopyBoth;
            break;
        case PGRES_SINGLE_TUPLE:
            out_result.exec_status_ = PSQLResult::ExecStatus::SingleTuple;
            break;
        case PGRES_PIPELINE_SYNC:
            out_result.exec_status_ = PSQLResult::ExecStatus::PipelineSync;
            break;
        case PGRES_PIPELINE_ABORTED:
            out_result.exec_status_ = PSQLResult::ExecStatus::PipelineAborted;
            break;
    }
}


ErrorCode PSQLConnection::_psqlStatementTimeout(double sec) noexcept {
    auto pg_conn = (PGconn*)pg_conn_ptr_;
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
        last_err_message_ = "Unable to set statement timeout: ";
        last_err_message_ += PQerrorMessage(pg_conn);
        return ErrorCode::DatabaseSetTimeoutFailed;
    }

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK) {
        last_err_message_ = "Error while setting statement timeout ";
        last_err_message_ += PQresultErrorMessage(res);
        PQclear(res);
        return ErrorCode::DatabaseSetTimeoutFailed;
    }

    PQclear(res);

    return ErrorCode::None;
}


PSQLConnection* PSQLConnections::addConnection() noexcept {
    auto connection = new (std::nothrow) PSQLConnection();
    if (connection) {
        connections_.push(connection);
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
        conn->identifier_ = identifier;
        conn->host_ = host;
        conn->port_ = port;
        conn->db_name_ = db_name;
        conn->user_ = user;
        conn->password_ = password;
    }
    return conn;
}


PSQLConnection* PSQLConnections::connectionByIdentifier(const String& identifier) {
    for (auto connection : connections_) {
        if (connection->identifier_ == identifier) {
            connection->open();
            return connection;
        }
    }
    return nullptr;
}


PSQLConnection* PSQLConnections::firstConnection() {
    if (connections_.size() > 0) {
        auto connection = connections_.first();
        if (connection != nullptr) {
            connection->open();
            return connection;
        }
    }
    return nullptr;
}


void PSQLResult::log(Log& l) const noexcept {
    l << "PSQLResult" << l.endl;
    if (pg_result_ptr_) {
        l++;
        auto pg_res = static_cast<PGresult*>(pg_result_ptr_);
        if (exec_status_ == ExecStatus::TuplesOK) {
            l << "m_exec_status: TuplesOK" << l.endl;
            l << "m_tuple_n: " << tuple_n_ << l.endl;
            l << "m_field_n: " << field_n_ << l.endl;
        }
        else if (exec_status_ == ExecStatus::CommandOK) {
            l << "m_exec_status: CommandOK" << l.endl;
            l << "affected rows: " << rows_affected_ << l.endl;
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
    if (!pg_result_ptr_) {
        return false;
    }
    return PQresultStatus(static_cast<PGresult*>(pg_result_ptr_)) == PGRES_TUPLES_OK;
}


void PSQLResult::clear() noexcept {
    if (pg_result_ptr_) {
        PQclear(static_cast<PGresult*>(pg_result_ptr_));
        pg_result_ptr_ = nullptr;
    }
}


PSQLType PSQLResult::fieldType(int32_t col) const noexcept {
    return static_cast<PSQLType>(PQftype(static_cast<PGresult*>(pg_result_ptr_), col));
}


bool PSQLResult::fieldIsBinary(int32_t col) const noexcept {
    if (pg_result_ptr_ && col >= 0 && col < field_n_){
        return PQfformat(static_cast<PGresult*>(pg_result_ptr_), col) == 1;
    }
    return false;
}


/**
 *  @note // Note: Does allways return the real name and not the alias
 */
const char* PSQLResult::fieldName(int32_t col) const noexcept {
    if (pg_result_ptr_ && col >= 0 && col < field_n_) {
        return PQfname(static_cast<PGresult*>(pg_result_ptr_), col);
    }
    return nullptr;
}


const char* PSQLResult::fieldValue(int32_t row, int32_t col) const noexcept {
    if (pg_result_ptr_ &&
        row >= 0 && row < tuple_n_ &&
        col >= 0 && col < field_n_) {
        return PQgetvalue(static_cast<PGresult*>(pg_result_ptr_), row, col);
    }
    return nullptr;
}


const void* PSQLResult::fieldData(int32_t row, int32_t col) const noexcept {
    if (pg_result_ptr_ &&
        row >= 0 && row < tuple_n_ &&
        col >= 0 && col < field_n_) {
        return PQgetvalue(
            static_cast<PGresult*>(pg_result_ptr_),
            row,
            col);
    }
    return nullptr;
}


bool PSQLResult::fieldBinAsInt32(int32_t row, int32_t col, int32_t& out_value) const noexcept {
    auto data = fieldData(row, col);
    if (!data) {
        return false;
    }

    uint32_t bits;
    std::memcpy(&bits, data, 4);
    out_value = static_cast<int32_t>(ntohl(bits));
    return true;
}

bool PSQLResult::fieldBinAsInt64(int32_t row, int32_t col, int64_t& out_value) const noexcept {
    auto data = fieldData(row, col);
    if (!data) {
        return false;
    }

    uint64_t bits;
    std::memcpy(&bits, data, 8);
    bits = ntohll(bits);
    out_value = static_cast<int64_t>(bits);
    return true;
}


bool PSQLResult::fieldBinAsFloat(int32_t row, int32_t col, float& out_value) const noexcept {
    auto data = fieldData(row, col);
    if (!data)
        return false;

    uint32_t bits;
    std::memcpy(&bits, data, 4);
    bits = ntohl(bits);
    std::memcpy(&out_value, &bits, 4);
    return true;
}


bool PSQLResult::fieldBinAsDouble(int32_t row, int32_t col, double& out_value) const noexcept {
    auto data = fieldData(row, col);
    if (!data)
        return false;

    uint64_t bits;
    std::memcpy(&bits, data, 8);
    bits = ntohll(bits);
    out_value = std::bit_cast<double>(bits);
    return true;
}


bool PSQLResult::fieldBinAsFix(int32_t row, int32_t col, Fix& out_value) const noexcept {
    auto data = static_cast<const uint8_t*>(fieldData(row, col));

    if (!data) return false;

    std::cout << "PTR: " << (void*)data << std::endl;

    for (int i = 0; i < 32; ++i)

    {

        std::cout << std::hex << (int)data[i] << " ";

    }

    std::cout << std::dec << std::endl;

    return false;
}


int32_t PSQLResult::fieldLength(int32_t row, int32_t col) const noexcept {
    return PQgetlength(static_cast<PGresult*>(pg_result_ptr_), row, col);
}


bool PSQLResult::fieldIsNull(int32_t row, int32_t col) const noexcept {
    return PQgetisnull(static_cast<PGresult*>(pg_result_ptr_), row, col);
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
    if (p != nullptr && p->type_ == PSQLPropertyType::String) {
        return p->string_.utf8();
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
        p->psql_type_ = psql_type;

        switch (psql_type) {
            case PSQLType::Undefined:
                p->type_ = PSQLPropertyType::Undefined;
                break;

            case PSQLType::Boolean:
                p->type_ = PSQLPropertyType::Boolean;
                p->integer_ = (*(const char*)data);
                break;

            case PSQLType::SmallInt: {
                p->type_ = PSQLPropertyType::Integer;
                int16_t sql_value;
                std::memcpy(&sql_value, data, sizeof(int16_t));
                p->integer_ = ntoh16(sql_value);
                break;
            }

            case PSQLType::Integer: {
                p->type_ = PSQLPropertyType::Integer;
                int32_t sql_value;
                std::memcpy(&sql_value, data, sizeof(int32_t));
                p->integer_ = ntoh32(sql_value);
                break;
            }

            case PSQLType::BigInt: {
                p->type_ = PSQLPropertyType::Integer;
                int64_t sql_value;
                std::memcpy(&sql_value, data, sizeof(int64_t));
                p->integer_ = ntoh64(sql_value);
                break;
            }

            case PSQLType::OID: {
                p->type_ = PSQLPropertyType::Integer;
                uint32_t sql_value;
                std::memcpy(&sql_value, data, sizeof(uint32_t));
                p->integer_ = ntoh32(sql_value);
                break;
            }

            case PSQLType::Char:
            case PSQLType::VarChar:
            case PSQLType::Text:
            case PSQLType::CharN:
            case PSQLType::Name:
            case PSQLType::JSON:
                p->type_ = PSQLPropertyType::String;
                p->string_.setByStr((const char*)data, data_size);
                break;

            case PSQLType::Real: {
                p->type_ = PSQLPropertyType::Double;
                uint32_t sql_value;
                std::memcpy(&sql_value, data, sizeof(uint32_t));
                sql_value = ntoh32(sql_value);  // Convert from network byte order to host byte order
                p->double_ = static_cast<double>(*reinterpret_cast<float*>(&sql_value));
                break;
            }

            case PSQLType::Double: {
                p->type_ = PSQLPropertyType::Double;
                uint64_t sql_value;
                std::memcpy(&sql_value, data, sizeof(uint64_t));
                sql_value = ntoh64(sql_value);  // Convert from network byte order to host byte order
                p->double_ = *reinterpret_cast<double*>(&sql_value);
                break;
            }

            case PSQLType::Numeric: {
                p->type_ = PSQLPropertyType::Numeric;
                PSQLYY::numericToString((uint8_t*)data, p->string_);
                p->double_ = p->string_.asDouble();
                p->integer_ = p->string_.asInt64();
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
