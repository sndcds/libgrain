//
//  PostgrSQL.hpp
//
//  Created by Roald Christesen on 13.09.2024.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainPostgreSQL_hpp
#define GrainPostgreSQL_hpp

#include "Grain.hpp"
#include "Type/List.hpp"
#include "Type/ByteOrder.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"


namespace Grain {

class PSQLPropertyList;

enum class PSQLType {
    Undefined = -1,
    Boolean = 16,       // 0 or 1
    ByteArray = 17,     // Array of bytes
    Char = 18,          // A single ASCII character
    Name = 19,          // A name, max 63 characters long
    BigInt = 20,        // 64-bit integer
    SmallInt = 21,      // 16-bit integer
    Integer = 23,       // 32-bit integer
    Text = 25,          // Text string
    OID = 26,           // 32 bit unsigned int
    JSON = 114,         // json
    Real = 700,         // real
    Double = 701,       // double precision
    CharN = 1042,       // char(n)
    VarChar = 1043,     // varchar(n)
    Date = 1082,        // date
    Timestamp = 1114,   // timestamp
    TimestampZ = 1184,  // timestamptz
    Numeric = 1700,     // numeric
    Void = 2278,        // void
    WKB = 34219         // Wellknown Binary
};

enum class PSQLPropertyType {
    Undefined = 0,
    Boolean,
    Integer,
    Double,
    String,
    Numeric
};

enum class PSQLParamFormat {
    Text = 0,
    Binary = 1
};


class PSQLParam : Object {
    friend class PSQLConnection;
public:
    PSQLParam(PSQLType type, PSQLParamFormat format, const char* value, int32_t length, void* bin_ptr);
protected:
    PSQLType type_ = PSQLType::Undefined;
    PSQLParamFormat format_ = PSQLParamFormat::Text;
    String value_;
    int32_t length_;
    void* bin_ptr_ = nullptr;  ///< Pointer to data if format_ = Binary
};


class PSQLParamList : public ObjectList<PSQLParam*> {
public:
    PSQLParamList() = default;
    ~PSQLParamList() override = default;

    ErrorCode addParam(PSQLType type, const char* value) noexcept;
    ErrorCode addParam(PSQLType type, const String& value) noexcept;
};


class PSQLResult {
    friend class PSQLConnection;

public:
    enum class ExecStatus {
        Undefined = -1,
        EmptyQuery = 0, ///< Empty query string was executed
        CommandOK,      ///< A query command that doesn't return anything was executed properly by the backend
        TuplesOK,       ///< A query command that returns tuples was executed properly by the backend
        CopyOut,        ///< Copy Out data transfer in progress
        CopyIn,			///< Copy In data transfer in progress
        BadResponse,	///< An unexpected response was recv'd from the backend
        NonfatalError,	///< Notice or warning message
        FatalError,		///< Query failed
        CopyBoth,		///< Copy In/Out data transfer in progress
        SingleTuple,	///< Single tuple from larger resultset
        PipelineSync,   ///< Pipeline synchronization point
        PipelineAborted	///< Command didn't run because of an abort earlier in a pipeline
    };

    enum class Format {
        Text = 0,
        Binary = 1
    };

protected:
    ErrorCode last_err_ = ErrorCode::None;
    void* pg_result_ptr_ = nullptr; ///< The real PGresult* for using the result
    ExecStatus exec_status_ = ExecStatus::Undefined;
    int32_t tuple_n_ = -1;
    int32_t field_n_ = -1;
    int32_t rows_affected_ = -1;

public:
    PSQLResult() = default;
    ~PSQLResult() {
        clear();
    }

    void log(Log& l) const noexcept;

    [[nodiscard]] bool areTuplesOK() const noexcept;
    void clear() noexcept;
    [[nodiscard]] int32_t tupleCount() const noexcept { return tuple_n_; }
    [[nodiscard]] int32_t fieldCount() const noexcept { return field_n_; }
    [[nodiscard]] PSQLType fieldType(int32_t col) const noexcept;
    [[nodiscard]] bool fieldIsBinary(int32_t col) const noexcept;
    [[nodiscard]] const char* fieldName(int32_t col) const noexcept;
    [[nodiscard]] const char* fieldValue(int32_t row, int32_t col) const noexcept;
    [[nodiscard]] const void* fieldData(int32_t row, int32_t col) const noexcept;

    [[nodiscard]] bool fieldBinAsInt32(int32_t row, int32_t col, int32_t& out_value) const noexcept;
    [[nodiscard]] bool fieldBinAsInt64(int32_t row, int32_t col, int64_t& out_value) const noexcept;

    [[nodiscard]] bool fieldBinAsFloat(int32_t row, int32_t col, float& out_value) const noexcept;
    [[nodiscard]] bool fieldBinAsDouble(int32_t row, int32_t col, double& out_value) const noexcept;
    [[nodiscard]] bool fieldBinAsFix(int32_t row, int32_t col, Fix& out_value) const noexcept;


    [[nodiscard]] int32_t fieldLength(int32_t row, int32_t col) const noexcept;
    [[nodiscard]] bool fieldIsNull(int32_t row, int32_t col) const noexcept;
};


/**
 *  @brief A SQL database connection.
 */
class PSQLConnection {

public:
    enum class Status {
        Unknown = -1,
        Ok = 0,
        Bad = 1,
        Started = 2,
        Made = 3,
        AwaitingResponse = 4,
        AuthOk = 5,
        SetEnv = 6,
        SSLStartup = 7,
        Needed = 8,
        CheckWriteable = 9,
        Consume = 10,
        GSSStartup = 11,
        CheckTarget = 12,
        CheckStandby = 13
    };

    enum {
        kErrConnectionFailed = 0,
    };

public:
    String identifier_;             ///< Unique identifier
    String host_;                    ///< Hostname, default is 'localhost'
    int32_t port_{};                ///< TCP port, default is 5432. The full range of valid TCP ports is 1 to 65535
    String db_name_;                ///< Database name
    String user_;                   ///< User name
    String password_;               ///< Password. Default is an empty password
    double timeout_sec_ = 30.0;     ///< Maximum seconds to wait for a database statement
    String last_err_message_;       ///< Last error message
    StringList psql_notices_;
    void* pg_conn_ptr_ = nullptr;   ///< The real PGconn* for using the database

public:
    PSQLConnection() = default;
    ~PSQLConnection();

    friend std::ostream& operator<<(std::ostream& os, const PSQLConnection* o) {
        o == nullptr ? os << "PSQLConnection nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const PSQLConnection& o) {
        os << "PSQLConnection:\n";
        os << "  identifier: " << o.identifier_ << std::endl;
        os << "  host: " << o.host_ << ", port: " << o.port_ << std::endl;
        os << "  db_name: " << o.db_name_ << std::endl;
        os << "  user: " << o.user_ << ", password: " << o.password_ << std::endl;
        os << "  conn: " << o.pg_conn_ptr_;
        return os;
    }

    ErrorCode open() noexcept;
    void close() noexcept;
    Status status() noexcept;
    PSQLResult query(const String& sql, PSQLResult::Format result_format) noexcept;
    PSQLResult query(const String& sql, const PSQLParamList& param_list, PSQLResult::Format result_format) noexcept;

    [[nodiscard]] const char* errorMessage() const noexcept;

    void _collectResult(void* pg_result_ptr, PSQLResult& out_result);

    void setTimeoutSec(double sec) noexcept { timeout_sec_ = sec; };
    ErrorCode useTimeout() noexcept { return _psqlStatementTimeout(timeout_sec_); }
    ErrorCode disableTimeout() noexcept { return _psqlStatementTimeout(0.0); }
    ErrorCode _psqlStatementTimeout(double sec) noexcept;
};


/**
 *  @brief A group of PostgreSQL connections.
 */
class PSQLConnections {

protected:
    ObjectList<PSQLConnection*>connections_;

public:
    ~PSQLConnections() {
        for (auto connection : connections_) {
            connection->close();
        }
    }

    friend std::ostream& operator << (std::ostream& os, const PSQLConnections* o) {
        o == nullptr ? os << "PSQLConnections nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const PSQLConnections& o) {
        for (auto connection : o.connections_) {
            os << connection << std::endl;
        }
        return os;
    }

    [[nodiscard]] PSQLConnection* addConnection() noexcept;
    [[nodiscard]] PSQLConnection* addConnection(const char* identifier, const char* host, int32_t port, const char* db_name, const char* user, const char* password) noexcept;


    [[nodiscard]] PSQLConnection* connectionByIdentifier(const String& identifier);
    [[nodiscard]] PSQLConnection* firstConnection();
};


class PSQLYY {

public:
    [[nodiscard]] static const char* typeName(int32_t type) { return PSQLYY::typeName((PSQLType)type); }

    [[nodiscard]] static const char* typeName(PSQLType type) {
        switch (type) {
            case PSQLType::Boolean: return "Boolean";
            case PSQLType::ByteArray: return "ByteArray";
            case PSQLType::Char: return "Char";
            case PSQLType::Name: return "Name";
            case PSQLType::BigInt: return "BigInt";
            case PSQLType::SmallInt: return "SmallInt";
            case PSQLType::Integer: return "Integer";
            case PSQLType::Text: return "Text";
            case PSQLType::OID: return "OID";
            case PSQLType::JSON: return "JSON";
            case PSQLType::Real: return "Real";
            case PSQLType::Double: return "Double";
            case PSQLType::CharN: return "CharN";
            case PSQLType::VarChar: return "VarChar";
            case PSQLType::Date: return "Date";
            case PSQLType::Timestamp: return "Timestamp";
            case PSQLType::TimestampZ: return "TimestampZ";
            case PSQLType::Numeric: return "Numeric";
            case PSQLType::Void: return "Void";
            case PSQLType::WKB: return "WKB";
            default: break;
        }
        return "Unknown";
    }

    static bool numericDigitToCharBuffer(uint16_t value, char* out_ptr) {
        // Ensure the value is non-negative and within the four-digit range
        if (value < 0 || value > 9999) {
            return false;
        }
        else {
            // Extract digits from the integer
            out_ptr[0] = '0' + (value / 1000) % 10;   // Thousands place
            out_ptr[1] = '0' + (value / 100) % 10;    // Hundreds place
            out_ptr[2] = '0' + (value / 10) % 10;     // Tens place
            out_ptr[3] = '0' + value % 10;            // Units place
            return true;
        }
    }

    static bool numericToString(const uint8_t* data, String& out_string) {
        constexpr int32_t kMaxDigits = 256;
        char buffer[5]{};
        const int16_t* ptr = (int16_t*)data;

        out_string.clear();

        // Read metadata
        uint16_t n_digits = ntoh16(*ptr++);   // Number of digits
        if (n_digits > kMaxDigits) {
            return false;
        }

        uint16_t weight = ntoh16(*ptr++);     // Position of decimal point
        uint16_t sign = ntoh16(*ptr++);       // Sign: 0 = positive, 1 = negative, 2 = NaN
        uint16_t dscale = ntoh16(*ptr++);     // Decimal scale

        // Read digit array
        uint16_t digits[kMaxDigits];
        for (int32_t i = 0; i < n_digits; ++i) {
            digits[i] = ntoh16(*ptr++);
        }


        // Convert digit array to a numeric value (e.g., a string or Decimal library representation)

        if (sign == 2) {
            out_string = "NaN";
        }
        else {
            if (sign == 1) {
                out_string = "-";
            }

            int digit_index = 0;
            for (int32_t i = weight + 1; i > 0; --i) {
                if (digit_index < n_digits) {
                    numericDigitToCharBuffer(digits[digit_index++], buffer);
                    out_string += buffer;
                }
                else {
                    out_string += "0000";
                }
            }

            if (dscale > 0) {
                out_string += '.';
                for (int32_t i = 0; i < dscale; ++i) {
                    if (digit_index < n_digits) {
                        numericDigitToCharBuffer(digits[digit_index++], buffer);
                        out_string += buffer;
                    }
                    else {
                        out_string += "0000";
                    }
                }
            }
        }

        return true;
    }
};


struct PSQLProperty {
    PSQLType psql_type_ = PSQLType::Undefined;
    String name_;
    PSQLPropertyType type_ = PSQLPropertyType::Undefined;
    int64_t integer_;
    double double_;
    String string_;

public:
    friend std::ostream& operator << (std::ostream& os, const PSQLProperty* o) {
        o == nullptr ? os << "PSQLProperty nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const PSQLProperty& o) {
        os << o.name_ << ", type: " << PSQLYY::typeName(o.psql_type_) << ", value: ";
        switch (o.type_) {
            case PSQLPropertyType::Boolean:
                os << ((o.integer_ != 0) ? "true" : "false");
                break;
            case PSQLPropertyType::Integer:
                os << o.integer_;
                break;
            case PSQLPropertyType::Double:
                os << o.double_;
                break;
            case PSQLPropertyType::String:
                os << o.string_;
                break;
            default:
                break;
        }
        return os;
    }

    [[nodiscard]] static const char* typeName(PSQLPropertyType type) {
        switch (type) {
            case PSQLPropertyType::Undefined: return "Undefined";
            case PSQLPropertyType::Boolean: return "Boolean";
            case PSQLPropertyType::Integer: return "Integer";
            case PSQLPropertyType::Double: return "Double";
            case PSQLPropertyType::String: return "String";
            default: return "Unknown";
        }
    }

    [[nodiscard]] const char* typeName() const { return PSQLProperty::typeName(type_); }
};


/**
 */
class PSQLPropertyList : public Object {

protected:
    int32_t size_ = 0;
    PSQLProperty* properties_ = nullptr;

public:
    PSQLPropertyList(int32_t size);
    ~PSQLPropertyList() override;

    [[nodiscard]] const char* className() const noexcept override { return "PSQLPropertyList"; }

    friend std::ostream& operator << (std::ostream& os, const PSQLPropertyList* o) {
        o == nullptr ? os << "PSQLPropertyList nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const PSQLPropertyList& o) {
        os << "size: " << o.size_;
        return os;
    }

    [[nodiscard]] int32_t size() const noexcept { return size_; }

    [[nodiscard]] PSQLProperty* mutPropertyPtrAtIndex(int32_t index) noexcept;
    void setPropertyAtIndexByPSQLBinaryData(int32_t index, PSQLType psql_type, const void* data, int32_t data_size);

    [[nodiscard]] const char* stringFromPropertyAtIndex(int32_t index) noexcept;
};


} // End of namespace Grain

#endif // GrainPostgreSQL_hpp
