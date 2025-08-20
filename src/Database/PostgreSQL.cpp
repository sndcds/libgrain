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

#include <postgresql15/libpq-fe.h>


namespace Grain {

    void _grain_psqlNoticeReceiver(void *arg, const PGresult *res) {
        auto notices = (StringList*)arg;

        // Extract the notice message from the result
        const char *message = PQresultErrorMessage(res);

        if (message != nullptr) {
            notices->pushString(message);
        }
    }


    ErrorCode PSQLConnection::open() {
        if (!m_conn) {
            PGconn* pg_conn = nullptr;
            char connection_info[2056];
            std::snprintf(connection_info, 2056, "host=%s port=%d dbname=%s user=%s password=%s", m_host.utf8(), m_port, m_dbname.utf8(), m_user.utf8(), m_password.utf8());

            pg_conn = PQconnectdb(connection_info);

            if (PQstatus(pg_conn) != CONNECTION_OK) {
                m_last_err_message = "Unable to connect to database.";
                return Error::specific(kErrConnectionFailed);
            }

            PQsetNoticeReceiver(pg_conn, _grain_psqlNoticeReceiver, &m_psql_notices);
            m_conn = pg_conn;
        }

        return ErrorCode::None;
    }


    void PSQLConnection::close() {
        if (m_conn) {
            PQfinish((PGconn*)m_conn);
            m_conn = nullptr;
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
                    p->m_integer = ntohs(sql_value);
                    break;
                }

                case PSQLType::Integer: {
                    p->m_type = PSQLPropertyType::Integer;
                    int32_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(int32_t));
                    p->m_integer = ntohl(sql_value);
                    break;
                }

                case PSQLType::BigInt: {
                    p->m_type = PSQLPropertyType::Integer;
                    int64_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(int64_t));
                    p->m_integer = ntohll(sql_value);
                    break;
                }

                case PSQLType::OID: {
                    p->m_type = PSQLPropertyType::Integer;
                    uint32_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(uint32_t));
                    p->m_integer = ntohl(sql_value);
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
                    sql_value = ntohl(sql_value);  // Convert from network byte order to host byte order
                    p->m_double = static_cast<double>(*reinterpret_cast<float*>(&sql_value));
                    break;
                }

                case PSQLType::Double: {
                    p->m_type = PSQLPropertyType::Double;
                    uint64_t sql_value;
                    std::memcpy(&sql_value, data, sizeof(uint64_t));
                    sql_value = ntohll(sql_value);  // Convert from network byte order to host byte order
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
