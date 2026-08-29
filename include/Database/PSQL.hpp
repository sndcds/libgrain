#ifndef GrainPSQL_hpp
#define GrainPSQL_hpp

#include "Grain.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"

#include <pqxx/pqxx>
#include <iterator>
#include <memory>
#include <iostream>

namespace Grain {

    enum class PSQLFieldType {
        Undefined = -1,
        Boolean = 16,
        ByteArray = 17,
        Char = 18,
        Name = 19,
        BigInt = 20,
        SmallInt = 21,
        Integer = 23,
        Text = 25,
        OID = 26,
        JSON = 114,
        Real = 700,
        Double = 701,
        CharN = 1042,
        VarChar = 1043,
        Date = 1082,
        Timestamp = 1114,
        TimestampZ = 1184,
        Numeric = 1700,
        Void = 2278,
        WKB = 34219
    };

    class PSQL;

    class PSQLField {
        pqxx::field_ref field_;

    public:
        explicit PSQLField(pqxx::field_ref field)
            : field_(field)
        {}

        bool isNull() const {
            return field_.is_null();
        }

        const char* c_str() const {
            return field_.c_str();
        }

        template<typename T>
        T as() const {
            return field_.as<T>();
        }
    };

    class PSQLRow {
        using row_type = decltype(std::declval<pqxx::result>()[0]);

        row_type row_;

    public:
        explicit PSQLRow(row_type row)
            : row_(row)
        {}

        std::size_t size() const {
            return row_.size();
        }

        PSQLField operator[](std::size_t i) const {
            return PSQLField(row_[i]);
        }
    };

    class PSQLTxResult {
        pqxx::result result_;

    public:
        explicit PSQLTxResult(pqxx::result r)
            : result_(std::move(r))
        {}

        [[nodiscard]] std::size_t size() const {
            return result_.size();
        }

        [[nodiscard]] bool empty() const {
            return result_.empty();
        }

        [[nodiscard]] std::size_t columns() const {
            return result_.columns();
        }

        [[nodiscard]] const char* columnName(std::size_t i) const {
            return result_.column_name(i);
        }

        [[nodiscard]] PSQLFieldType columnType(std::size_t i) const {
            return static_cast<PSQLFieldType>(result_.column_type(i));
        }

        [[nodiscard]] PSQLRow operator[](std::size_t i) const {
            return PSQLRow(result_[i]);
        }

        class const_iterator {
            pqxx::result::const_iterator it_;

        public:
            using value_type = PSQLRow;
            using reference = PSQLRow;
            using pointer = void;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            explicit const_iterator(pqxx::result::const_iterator it)
                : it_(it)
            {}

            PSQLRow operator*() const {
                return PSQLRow(*it_);
            }

            const_iterator& operator++() {
                ++it_;
                return *this;
            }

            friend bool operator==(const const_iterator&, const const_iterator&) = default;
        };

        const_iterator begin() const {
            return const_iterator(result_.begin());
        }

        const_iterator end() const {
            return const_iterator(result_.end());
        }
    };

    class PSQLReadTx {
        pqxx::read_transaction tx_;

    public:
        explicit PSQLReadTx(pqxx::connection& conn)
            : tx_(conn)
        {}

        PSQLTxResult exec(std::string const& sql) {
            return PSQLTxResult(tx_.exec(sql));
        }
    };

    class PSQLWriteTx {
        pqxx::work tx_;
        bool committed_ = false;

    public:
        explicit PSQLWriteTx(pqxx::connection& conn)
            : tx_(conn)
        {}

        ~PSQLWriteTx() = default;

        PSQLTxResult exec(std::string const& sql) {
            return PSQLTxResult(tx_.exec(sql));
        }

        void commit() {
            tx_.commit();
            committed_ = true;
        }
    };

    class PSQLReadWriteTx {
        pqxx::work tx_;
        bool committed_ = false;

    public:
        explicit PSQLReadWriteTx(pqxx::connection& conn)
            : tx_(conn)
        {}

        ~PSQLReadWriteTx() = default;

        PSQLTxResult exec(std::string const& sql) {
            return PSQLTxResult(tx_.exec(sql));
        }

        PSQLTxResult exec_write(std::string const& sql) {
            return PSQLTxResult(tx_.exec(sql));
        }

        void commit() {
            tx_.commit();
            committed_ = true;
        }
    };

    class PSQL {
    public:
        enum {
            kErrConnectionFailed = 0,
        };

        String identifier_;
        String host_;
        int32_t port_{};
        String db_name_;
        String user_;
        String password_;
        double timeout_ = 30.0;
        String last_err_;

        std::unique_ptr<pqxx::connection> connection_;

    public:
        PSQL() = default;
        ~PSQL() = default;

        friend std::ostream& operator<<(std::ostream& os, const PSQL& o) {
            os << "PSQL:\n";
            os << "  identifier: " << o.identifier_ << "\n";
            os << "  host: " << o.host_ << ", port: " << o.port_ << "\n";
            os << "  db_name: " << o.db_name_ << "\n";
            os << "  user: " << o.user_ << ", password: " << o.password_ << "\n";
            os << "  conn: " << o.connection_;
            return os;
        }

        [[nodiscard]] ErrorCode connect(const String& connection_string) noexcept {
            try {
                connection_ = std::make_unique<pqxx::connection>(
                    connection_string.utf8()
                );
            } catch (const std::exception& e) {
                std::cerr << "PSQL connection failed: " << e.what() << std::endl;
                return Error::specific(kErrConnectionFailed);
            }

            return ErrorCode::None;
        }

        pqxx::connection& connection() {
            return *connection_;
        }

        pqxx::connection const& connection() const {
            return *connection_;
        }

        [[nodiscard]] PSQLReadTx read() const {
            return PSQLReadTx(const_cast<pqxx::connection&>(connection()));
        }

        [[nodiscard]] PSQLWriteTx write() {
            return PSQLWriteTx(connection());
        }

        [[nodiscard]] PSQLReadWriteTx transaction() {
            return PSQLReadWriteTx(connection());
        }

        static PSQLFieldType toFieldType(int oid) {
            switch (oid) {
                case 16: return PSQLFieldType::Boolean;
                case 23: return PSQLFieldType::Integer;
                case 20: return PSQLFieldType::BigInt;
                case 21: return PSQLFieldType::SmallInt;
                case 25: return PSQLFieldType::Text;
                case 700: return PSQLFieldType::Real;
                case 701: return PSQLFieldType::Double;
                case 1043: return PSQLFieldType::VarChar;
                case 1042: return PSQLFieldType::CharN;
                case 1700: return PSQLFieldType::Numeric;
                case 1082: return PSQLFieldType::Date;
                case 1114: return PSQLFieldType::Timestamp;
                case 1184: return PSQLFieldType::TimestampZ;
                default: return PSQLFieldType::Undefined;
            }
        }
    };

} // namespace Grain

#endif // GrainPSQL_hpp