//
//  PostgrSQL.hpp
//
//  Created by Roald Christesen on 13.09.2024.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 20.08.2025
//

#ifndef GrainPostgreSQL_hpp
#define GrainPostgreSQL_hpp

#include "Grain.hpp"
#include "Type/List.hpp"
#include "Type/ByteOrder.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"


namespace Grain {

    enum class PSQLType {
        Undefined = -1,
        Boolean = 16,       // 0 or 1
        ByteArray = 17,     // Array of bytes
        Char = 18,          // A single ASCII character
        Name = 19,          // A name, max 63 characters long
        BigInt = 20,        // 64 bit integer
        SmallInt = 21,      // 16 bit integer
        Integer = 23,       // 32 bit integer
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


    /**
     *  @brief A SQL database connection.
     */
    class PSQLConnection {

    public:
        enum {
            kErrConnectionFailed = 0,
        };

    public:
        String m_identifier;        ///< Unique identifier
        String m_host;              ///< Hostname, default is 'localhost'
        int32_t m_port;             ///< TCP port, default is 5432. The full range of valid TCP ports is 1 to 65535
        String m_dbname;            ///< Database name
        String m_user;              ///< User name
        String m_password;          ///< Password. Default is an empty password
        void* m_conn = nullptr;   ///< The real connection for using the database
        String m_last_err_message;  ///< Last error message
        StringList m_psql_notices;

    public:
        friend std::ostream& operator << (std::ostream& os, const PSQLConnection* o) {
            o == nullptr ? os << "PSQLConnection nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const PSQLConnection& o) {
            os << "PSQLConnection:\n";
            os << "  identifier: " << o.m_identifier << std::endl;
            os << "  host: " << o.m_host << ", port: " << o.m_port << std::endl;
            os << "  dbname: " << o.m_dbname << std::endl;
            os << "  user: " << o.m_user << ", password: " << o.m_password << std::endl;
            os << "  conn: " << o.m_conn;
            return os;
        }

        ErrorCode open();
        void close();
    };


    /**
     *  @brief A group of PostgreSQL connections.
     */
    class PSQLConnections {

    protected:
        ObjectList<PSQLConnection*>m_connections;

    public:
        ~PSQLConnections() {
            for (auto connection : m_connections) {
                connection->close();
            }
        }

        friend std::ostream& operator << (std::ostream& os, const PSQLConnections* o) {
            o == nullptr ? os << "PSQLConnections nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const PSQLConnections& o) {
            for (auto connection : o.m_connections) {
                os << connection << std::endl;
            }
            return os;
        }

        [[nodiscard]] PSQLConnection* addConnection() noexcept;
        [[nodiscard]] PSQLConnection* connectionByIdentifier(const String& identifier);
        [[nodiscard]] PSQLConnection* firstConnection();
    };


    class PSQL {

    public:
        [[nodiscard]] static const char* typeName(int32_t type) { return PSQL::typeName((PSQLType)type); }

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

        static bool numericDigitToCharBuffer(int16_t value, char* out_ptr) {
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

        static bool numericToString(uint8_t* data, String& out_string) {
            char buffer[5]{};
            const int16_t* ptr = (int16_t*)data;

            out_string.clear();

            // Read metadata
            int16_t n_digits = ntoh16(*ptr++);   // Number of digits
            int16_t weight = ntoh16(*ptr++);     // Position of decimal point
            int16_t sign = ntoh16(*ptr++);       // Sign: 0 = positive, 1 = negative, 2 = NaN
            int16_t dscale = ntoh16(*ptr++);     // Decimal scale

            // Read digit array
            int16_t digits[n_digits];
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


    enum class PSQLPropertyType {
        Undefined = 0,
        Boolean,
        Integer,
        Double,
        String,
        Numeric
    };


    struct PSQLProperty {
        PSQLType m_psql_type = PSQLType::Undefined;
        String m_name;
        PSQLPropertyType m_type = PSQLPropertyType::Undefined;
        int64_t m_integer;
        double m_double;
        String m_string;

    public:
        friend std::ostream& operator << (std::ostream& os, const PSQLProperty* o) {
            o == nullptr ? os << "PSQLProperty nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const PSQLProperty& o) {
            os << o.m_name << ", type: " << PSQL::typeName(o.m_psql_type) << ", value: ";
            switch (o.m_type) {
                case PSQLPropertyType::Boolean:
                    os << ((o.m_integer != 0) ? "true" : "false");
                    break;
                case PSQLPropertyType::Integer:
                    os << o.m_integer;
                    break;
                case PSQLPropertyType::Double:
                    os << o.m_double;
                    break;
                case PSQLPropertyType::String:
                    os << o.m_string;
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

        [[nodiscard]] const char* typeName() { return PSQLProperty::typeName(m_type); }
    };


    /**
     *
     */
    class PSQLPropertyList : public Object {

    protected:
        int32_t m_size = 0;
        PSQLProperty* m_properties = nullptr;

    public:
        PSQLPropertyList(int32_t size);
        ~PSQLPropertyList();

        [[nodiscard]] const char* className() const noexcept override { return "PSQLPropertyList"; }

        friend std::ostream& operator << (std::ostream& os, const PSQLPropertyList* o) {
            o == nullptr ? os << "PSQLPropertyList nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const PSQLPropertyList& o) {
            os << "size: " << o.m_size;
            return os;
        }

        [[nodiscard]] int32_t size() const noexcept { return m_size; }

        [[nodiscard]] PSQLProperty* mutPropertyPtrAtIndex(int32_t index) noexcept;
        void setPropertyAtIndexByPSQLBinaryData(int32_t index, PSQLType psql_type, const void* data, int32_t data_size);

        [[nodiscard]] const char* stringFromPropertyAtIndex(int32_t index) noexcept;
    };


} // End of namespace Grain

#endif // GrainPostgreSQL_hpp
