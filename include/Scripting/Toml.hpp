//
//  Toml.hpp
//
//  Created by Roald Christesen on 12.04.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#ifndef GrainToml_hpp
#define GrainToml_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "String/String.hpp"
#include "Color/RGB.hpp"

#include "Extern/toml.hpp"


namespace Grain {


    class TomlTable;
    class TomlArray;


/**
 *  @brief Represents a position in a TOML file.
 *
 *  This structure stores the line and column number corresponding to a node's
 *  location within a TOML file. It is useful for error reporting and debugging.
 */
    struct TomlPos {
        int32_t m_line;
        int32_t m_column;

        friend std::ostream& operator << (std::ostream& os, const TomlPos* o) {
            o == nullptr ? os << "TomlPos nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const TomlPos& o) {
            os << "line: " << o.m_line << ", column: " << o.m_column;
            return os;
        }
    };


    class TomlNode {
    public:
        /**
         *  @brief Node types.
         *
         *  @note Must correspond with the types in toml++
         */
        enum class Type {
            None = 0,
            Table,
            Array,
            String,
            Integer,
            FloatingPoint,
            Boolean,
            Date,
            Time,
            DateTime,
            Count,
            Last = Count -1
        };
    protected:
        toml::node_view<const toml::node> _m_tpp_node_view;

    public:
        TomlNode() {
        }

        TomlNode(toml::node_view<const toml::node> ttp_node_view) {
            _m_tpp_node_view = ttp_node_view;
        }

        friend std::ostream& operator << (std::ostream& os, const TomlNode* o) {
            o == nullptr ? os << "TomlNode nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const TomlNode& o) {
            os << "TomlNode type: " << o.typeName();
            return os;
        }

        void _setTtpNodeView(toml::node_view<const toml::node> ttp_node_view) {
            _m_tpp_node_view = ttp_node_view;
        }

        [[nodiscard]] toml::node_view<const toml::node> _ttpNodeView() const {
            return _m_tpp_node_view;
        }

        [[nodiscard]] Type type() const {
            return _m_tpp_node_view ? (Type)_m_tpp_node_view.type() : Type::None;
        }

        [[nodiscard]] const char* typeName() const {
            constexpr const char* names[] = {
                    "None", "Table", "Array", "String", "Integer", "FloatingPoint",
                    "Boolean", "Date", "Time", "DateTime"
            };
            auto t = type();
            return t >= Type::None && t < Type::Count ? names[(int32_t)t] : names[0];
        }

        [[nodiscard]] TomlPos position() {
            TomlPos pos;
            if (const toml::node* node = _m_tpp_node_view.node()) {
                toml::source_region src = node->source();  // just assign directly
                pos.m_line = src.begin.line;
                pos.m_column = src.begin.column;
            }
            return pos;
        }

        [[nodiscard]] bool isBoolean() const { return _m_tpp_node_view.is_boolean(); }
        [[nodiscard]] bool asBoolean() const { return _m_tpp_node_view.as_boolean(); }

        [[nodiscard]] bool isString() const { return _m_tpp_node_view.is_string(); }
        [[nodiscard]] const char* asString () const { return _m_tpp_node_view.as_string()->get().c_str(); };
        [[nodiscard]] bool stringIsEqualTo(const char* str) const { return strcmp(asString(), str) == 0; }

        [[nodiscard]] bool isTable() const { return _m_tpp_node_view.is_table(); }
        void asTable(TomlTable& out_table) const;

        bool asStringForced(String& out_string) const {
            if (!_m_tpp_node_view) {
                out_string = "";
            }
            else if (auto str = _m_tpp_node_view.value<std::string>()) {
                out_string = str->data();
            }
            else if (auto i = _m_tpp_node_view.value<int64_t>()) {
                out_string = std::to_string(*i).data();
            }
            else if (auto f = _m_tpp_node_view.value<double>()) {
                out_string = std::to_string(*f).data();
            }
            else if (auto b = _m_tpp_node_view.value<bool>()) {
                out_string = *b ? "true" : "false";
            }
            else {
                // TODO: Implement Date, Time, DateTime ...
                return false;
            }

            return true;
        }

        RGB asRGB() const;
    };


    class TomlTableItem {
        friend class TomlTable;
        friend class TomlTableIterator;

    protected:
        String m_key;
        TomlNode m_value;

    public:
        TomlTableItem() = default;

        const char* key() const { return m_key.utf8(); }
        const TomlNode& value() const { return m_value; }
    };


    class TomlTableIterator {
        toml::table::const_iterator _it;
        toml::table::const_iterator _end;
        TomlTableItem _current_item;

        void updateCurrentItem() {
            if (_it != _end) {
                _current_item.m_key = _it->first.str().data();
                _current_item.m_value._setTtpNodeView(toml::node_view<const toml::node>(_it->second));
            }
        }

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = TomlTableItem;
        using difference_type = std::ptrdiff_t;
        using pointer = const TomlTableItem*;
        using reference = const TomlTableItem&;

        TomlTableIterator(toml::table::const_iterator it, toml::table::const_iterator end)
                : _it(it), _end(end) {
            updateCurrentItem();
        }

        reference operator * () const { return _current_item; }

        TomlTableIterator& operator ++() {
            ++_it;
            updateCurrentItem();
            return *this;
        }

        bool operator != (const TomlTableIterator& other) const {
            return _it != other._it;
        }
    };


    class TomlTable {
        friend class Toml;
        friend class TomlTableItem;

    protected:
        const toml::table *_m_tpp_table_ptr = nullptr;    ///< Pointer to toml++ table

    public:
        TomlTable() {
            _m_tpp_table_ptr = nullptr;
        }

        TomlTable(const toml::table* tpp_table) {
            _m_tpp_table_ptr = tpp_table;
        }

        void _setTppTablePtr(const toml::table *ttp_table_ptr) { _m_tpp_table_ptr = ttp_table_ptr; }

        TomlTableIterator begin() const {
            return TomlTableIterator(
                    _m_tpp_table_ptr ? _m_tpp_table_ptr->begin() : toml::table::const_iterator{},
                    _m_tpp_table_ptr ? _m_tpp_table_ptr->end() : toml::table::const_iterator{}
            );
        }

        TomlTableIterator end() const {
            return TomlTableIterator(
                    _m_tpp_table_ptr ? _m_tpp_table_ptr->end() : toml::table::const_iterator{},
                    _m_tpp_table_ptr ? _m_tpp_table_ptr->end() : toml::table::const_iterator{}
            );
        }

        [[nodiscard]] const toml::table::const_iterator _tppItemByName(const char* name) const {
            if (name == nullptr || _m_tpp_table_ptr == nullptr) {
                return _m_tpp_table_ptr->end();
            }
            else {
                return _m_tpp_table_ptr->find(name);
            }
        }

        [[nodiscard]] const toml::table::const_iterator _tppItemByNameOrThrow(const char* name, int32_t local_exc_code) const {
            if (name == nullptr) {
                throw Exception(ErrorCode::TomlNoName, "Expected a table item name, but a nullptr was found");
            }
            auto item = _m_tpp_table_ptr->find(name);
            if (item == _m_tpp_table_ptr->end()) {

                String message;
                message.setFormatted(202)
                throw Exception(ErrorCode::TomlExpectedTableItem, "Expected table item with name \"%s\"");
            }
            return item;
        }


        [[nodiscard]] TomlPos position();
        [[nodiscard]] bool hasItem(const char* name) const;
        [[nodiscard]] bool hasItemThrowIfRequired(const char* name, bool required) const;
        [[nodiscard]] bool itemByName(const char* name, TomlTableItem& out_table_item);

        void tableOrThrow(const char* name, int32_t local_exc_code, TomlTable& out_table) const;
        void arrayOrThrow(const char* name, int32_t local_exc_code, TomlArray& out_array) const;

        [[nodiscard]] const char* stringOr(const char* name, const char* fallback, int32_t local_exc_code) const;
        [[nodiscard]] const char* stringOrThrow(const char* name, int32_t local_exc_code) const;

        [[nodiscard]] bool booleanOr(const char* name, bool fallback, int32_t local_exc_code) const;
        [[nodiscard]] bool booleanOrThrow(const char* name, int32_t local_exc_code) const;

        [[nodiscard]] int64_t integerOr(const char* name, int64_t fallback, int32_t local_exc_code) const;
        [[nodiscard]] int64_t integerOrThrow(const char* name, int32_t local_exc_code) const;

        [[nodiscard]] double doubleOr(const char* name, int64_t fallback, int32_t local_exc_code) const;
        [[nodiscard]] double doubleOrThrow(const char* name, int32_t local_exc_code) const;

        [[nodiscard]] int32_t doublesOrThrow(const char* name, int32_t local_exc_code, int32_t max_values, double* out_values);

        const RGB rgbOr(const char* name, const RGB& fallback, int32_t local_exc_code);
        const RGB rgbOrThrow(const char* name, int32_t local_exc_code);

    };


    class TomlArrayItem {
        TomlNode m_value;

    public:
        TomlArrayItem() = default;

        explicit TomlArrayItem(toml::node_view<const toml::node> view) {
            m_value._setTtpNodeView(view);
        }

        [[nodiscard]] toml::node_view<const toml::node> _ttpNodeView() const { return m_value._ttpNodeView(); }

        [[nodiscard]] bool isString() const { return m_value.isString(); }
        [[nodiscard]] const char* asString() const { return m_value.asString(); }

        [[nodiscard]] const TomlTable asTableOrThrow(int32_t local_exc_code) const {
            if (m_value.isTable()) {
                TomlTable table;
                m_value.asTable(table);
                return table;
            }
            else {
                throw Exception(ErrorCode::TomlExpectedTable, local_exc_code, "Expected a TOML table but found something else");
            }
        }
    };


    class TomlArrayIterator {
        toml::array::const_iterator _it;
        TomlArrayItem _current_item;

        void updateCurrentItem() {
            if (_it != toml::array::const_iterator{}) {
                _current_item = TomlArrayItem(toml::node_view<const toml::node>{ *_it });
            }
        }

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = TomlArrayItem;
        using difference_type = std::ptrdiff_t;

        TomlArrayIterator(toml::array::const_iterator it) : _it(it) {
            updateCurrentItem();
        }

        const TomlArrayItem& operator * () const {
            return _current_item;
        }

        TomlArrayIterator& operator ++() {
            ++_it;
            updateCurrentItem();
            return *this;
        }

        bool operator != (const TomlArrayIterator& other) const {
            return _it != other._it;
        }
    };


    class TomlArray : public TomlNode {
        friend class Toml;

    protected:
        const toml::array *_m_tpp_array_ptr = nullptr;    ///< Pointer to toml++ array

    public:
        TomlArray() {}
        TomlArray(const toml::array *tpp_array) {
            _m_tpp_array_ptr = tpp_array;
        }

        TomlArrayIterator begin() const {
            return TomlArrayIterator(_m_tpp_array_ptr->begin());
        }

        TomlArrayIterator end() const {
            return TomlArrayIterator(_m_tpp_array_ptr->end());
        }

        [[nodiscard]] int32_t size() const {
            return static_cast<int32_t>(_m_tpp_array_ptr ? _m_tpp_array_ptr->size() : 0);
        }

        [[nodiscard]] const toml::array* _tppArray() const { return _m_tpp_array_ptr; }
        void _setTppArrayPtr(const toml::array* array_ptr) { _m_tpp_array_ptr = array_ptr; }
    };


    class Toml : public Object {

    public:
        enum class DataType {
            None = 0,
            Boolean,
            Integer,
            FloatingPoint,
            String,
            Table,
            Array,
            Date,
            Time,
            DateTime
        };

        enum class Option {
            None = 0x0,
            FileIncludes = 0x1
        };

        enum {
            kErrFailedToIncludeFile = 0
        };

        static constexpr int32_t kErrMessageMaxLength = 2000;
        static constexpr bool kOptional = false;
        static constexpr bool kRequired = true;

    protected:
        toml::parse_result _m_tpp_parse_result;  ///< toml++ parse result

        int32_t m_included_files_count = 0; ///< Number of includes in TOML file ([[include]])
        int64_t m_included_files_total_size = 0;  ///< Number of bytes in all included files

    public:
        Toml();
        ~Toml();

        void parseFile(const String& file_path, Option options = Option::None);
        void parse(const char* str);


        toml::parse_result _ttpParseResult() { return _m_tpp_parse_result; }

        TomlArray arrayByName(const char* name) noexcept;
        TomlArray arrayByNameOrThrow(const char* name, int32_t local_exc_code);

        void asTable(TomlTable& out_table) {
            out_table._m_tpp_table_ptr = _m_tpp_parse_result.as_table();
        }

        void throwIfError(ErrorCode err) {
            if (err != ErrorCode::None) {
                throw err;
            }
        }

        static void throwParserError(const char *str) {
            throw Exception::formatted(ErrorCode::TomlParseError, 0, "Toml parser exception: %s", str != nullptr ? str : "");
        }

        static void throwParserErrorFileLine(const char *file, int32_t line) {
            throw Exception::formatted(ErrorCode::TomlParseError, 0, "Toml parser exception in file: %s, line: %d", file, line);
        }

        [[noreturn]]
        void throwTomlParseError(std::ostringstream* oss, const toml::source_region& region) {
            if (!oss) {
                throw std::runtime_error("throwTomlParseError(): oss pointer is null");
            }

            *oss << " (at line " << region.begin.line
                 << ", column " << region.begin.column << ")";

            // Assuming you have your own Exception class (customize below):
            throw Exception(ErrorCode::TomlParseError, 0, oss->str().c_str());
        }

        ErrorCode toJson(String &out_string);
    };


} // End of namespace Grain

#endif // GrainToml_hpp
