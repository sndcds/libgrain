//
//  Toml.cpp
//
//  Created by Roald Christesen on 12.04.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#include "Scripting/Toml.hpp"
#include "Core/Log.hpp"
#include "File/File.hpp"
#include "Graphic/Font.hpp"
#include "String/StringList.hpp"
#include "CSS/CSSColor.hpp"
#include "Time/Timestamp.hpp"

#include <sstream>


namespace Grain {


    void TomlNode::asTable(TomlTable& out_table) const {
        out_table._setTppTablePtr(_m_tpp_node_view.as_table());
    }


    RGB TomlNode::asRGB() const {
        RGB rgb;
        if (isString()) {
            CSSColor::parseColorToRGB(asString(), rgb);
        }
        return rgb;
    }


    TomlPos TomlTable::position() {
        TomlPos pos;
        if (const toml::node* node = _m_tpp_table_ptr) {
            toml::source_region src = node->source();  // just assign directly
            pos.m_line = src.begin.line;
            pos.m_column = src.begin.column;
        }
        return pos;
    }


    bool TomlTable::hasItem(const char* name) const {
        if (_m_tpp_table_ptr == nullptr || name == nullptr) {
            return false;
        }
        else {
            return _m_tpp_table_ptr->contains(name);
        }
    }


    /**
     *  @brief Checks if the TOML table contains an item with the given name
     *         and throws if required and missing.
     *
     *  This method checks whether the table contains a key matching the
     *  provided name. If the key is not found and the `required` flag is set
     *  to `true`, it throws an Exception.
     *
     *  @param name The name of the item to look for. If `nullptr`, the
     *              function returns `false` without throwing.
     *  @param required Indicates whether the item is required. If `true` and
     *                  the item is not found, an exception is thrown.
     *
     *  @return `true` if the item exists in the table, `false` otherwise.
     *
     *  @throws Exception If `required` is `true` and the item is not present
     *          in the table.
     */
    bool TomlTable::hasItemThrowIfRequired(const char* name, bool required) const {
        if (_m_tpp_table_ptr == nullptr || name == nullptr) {
            return false;
        }

        bool contained = _m_tpp_table_ptr->contains(name);
        if (contained == false) {
            if (required == true) {
                Exception::throwFormattedMessage(
                        ErrorCode::TomlParseError,
                        "Expected table item with name \"%s\"",
                        name);
            }
        }

        return contained;
    }


    /**
     *  @brief Retrieves an item from the TOML table by name and stores it in
     *         the provided output parameter.
     *
     *  This function attempts to find a key in the TOML table with the given
     *  name. If found, it populates the provided `TomlTableItem` reference with
     *  the key and corresponding value as a `toml::node_view`.
     *
     *  @param name The name of the item to look for. Must not be `nullptr`.
     *  @param out_table_item A reference to a `TomlTableItem` that will be
     *                        filled with the found key and value.
     *
     *  @return `true` if the item exists and `out_table_item` is successfully
     *          populated; `false` otherwise.
     *
     *  @note The function does not throw. It is the caller's responsibility to
     *        ensure `name` is valid.
     */
    bool TomlTable::itemByName(const char* name, TomlTableItem& out_table_item) {
        if (_m_tpp_table_ptr == nullptr) {
            return false;
        }

        auto item = _tppItemByName(name);

        if (item == _m_tpp_table_ptr->end()) {
            return false;
        }
        else {
            out_table_item.m_key = item->first.str().data();
            out_table_item.m_value = toml::node_view<const toml::node>(item->second);
            return true;
        }
    }


    const char* TomlTable::stringOr(const char* name, const char* fallback, int32_t local_exc_code) const {
        return hasItem(name) ? stringOrThrow(name, local_exc_code) : fallback;
    }


    const char* TomlTable::stringOrThrow(const char* name, int32_t local_exc_code) const {
        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (!item->second.is_string()) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedString,
                    "Expected a string for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }
        return item->second.as_string()->get().c_str();
    }


    bool TomlTable::booleanOr(const char* name, bool fallback, int32_t local_exc_code) const {
        return hasItem(name) ? booleanOrThrow(name, local_exc_code) : fallback;
    }


    bool TomlTable::booleanOrThrow(const char* name, int32_t local_exc_code) const {
        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (!item->second.is_boolean()) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedString,
                    "Expected a boolean for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }
        return item->second.as_boolean()->get();
    }


    int64_t TomlTable::integerOr(const char* name, int64_t fallback, int32_t local_exc_code) const {
        return hasItem(name) ? integerOrThrow(name, local_exc_code) : fallback;
    }


    int64_t TomlTable::integerOrThrow(const char* name, int32_t local_exc_code) const {
        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (!item->second.is_integer()) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedString,
                    "Expected an integer for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }
        return item->second.as_integer()->get();
    }


    double TomlTable::doubleOr(const char* name, double fallback, int32_t local_exc_code) const {
        return hasItem(name) ? doubleOrThrow(name, local_exc_code) : fallback;
    }


    double TomlTable::doubleOrThrow(const char* name, int32_t local_exc_code) const {
        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (item->second.is_floating_point()) {
            return item->second.as_floating_point()->get();
        }
        else if (item->second.is_integer()) {
            return static_cast<double>(item->second.as_integer()->get());
        }
        else {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedString,
                    "Expected a floating point for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }
        return 0.0;
    }


    void TomlTable::tableOrThrow(const char* name, int32_t local_exc_code, TomlTable& out_table) const {
        if (name == nullptr) {
            Exception::throwMessage(
                    ErrorCode::TomlNoName,
                    "Expected table item name, but a nullptr was found");
        }

        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (!item->second.is_table()) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedString,
                    "Expected a table for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }

        out_table._setTppTablePtr(item->second.as_table());
    }


    void TomlTable::arrayOrThrow(const char* name, int32_t local_exc_code, TomlArray& out_array) const {
        if (name == nullptr) {
            Exception::throwMessage(
                    ErrorCode::TomlNoName,
                    "Expected array item name, but a nullptr was found");
        }

        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (!item->second.is_array()) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedString,
                    "Expected an array for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }

        out_array._setTppArrayPtr(item->second.as_array());
    }


    int32_t TomlTable::doublesOrThrow(const char* name, int32_t local_exc_code, int32_t max_values, double* out_values) {
        if (out_values == nullptr) {
            Toml::throwParserErrorFileLine(__FILE__, __LINE__);
        }
        auto item = _tppItemByNameOrThrow(name, local_exc_code);
        if (!item->second.is_array()) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlExpectedArray,
                    "Expected an array for table item with name \"%s\", code: %d",
                    name,
                    local_exc_code);
        }

        auto array = item->second.as_array();
        int32_t n = static_cast<int32_t>(array->size());
        if (n > max_values) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlParseError,
                    "Array to big for item with name \"%s\", max array size is: %d, code: %d",
                    name,
                    max_values,
                    local_exc_code);
        }
        int32_t value_index = 0;
        for (const auto& elem : *array) {
            if (elem.is_floating_point() == true) {
                out_values[value_index] = elem.as_floating_point()->get();
            }
            else if (elem.is_integer() == true) {
                out_values[value_index] = static_cast<int32_t>(elem.as_integer()->get());
            }
            else {
                Exception::throwFormattedMessage(
                        ErrorCode::TomlParseError,
                        "Array to big for item with name \"%s\", accepts integers or doubles only, code: %d",
                        name,
                        local_exc_code);
            }
            value_index++;
        }
        return value_index;
    }


    const RGB TomlTable::rgbOr(const char* name, const RGB& fallback, int32_t local_exc_code) {
        return hasItem(name) ? rgbOrThrow(name, local_exc_code) : fallback;
    }


    const RGB TomlTable::rgbOrThrow(const char* name, int32_t local_exc_code) {
        RGB rgb;
        auto s = stringOrThrow(name, local_exc_code);
        auto err = CSSColor::parseColorToRGB(s, rgb);
        if (err != ErrorCode::None) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlParseError,
                    "Invalid CSS color string for item with name \"%s\", CSS: \"%s\", code: %d",
                    name,
                    s,
                    local_exc_code);
        }
        return rgb;
    }


    Toml::Toml() {
    }


    Toml::~Toml() {
    }


    void Toml::parseFile(const String& file_path, Option options) {
        constexpr const char* include_token = "[[include]]";
        auto result = ErrorCode::None;
        uint8_t* data_buffer = nullptr;

        try {
            String dir_path = file_path.fileDirPath();
            ObjectList<FileEntry*> files_list;
            FileEntry toml_file_entry;
            File::fileEntryByPath(file_path, toml_file_entry);

            // Include external TOML files
            if (static_cast<int32_t>(options) & static_cast<int32_t>(Option::FileIncludes)) {
                File file(file_path);
                file.startReadAscii();

                while (file.skipUntilLineWithText(include_token) == true) {
                    String key, external_file_path;
                    file.readTomlKeyValue(key, external_file_path);

                    ErrorCode err;
                    auto file_entry = new FileEntry();
                    if (external_file_path.firstAsciiChar() == '/') {  // Absolute path
                        err = File::fileEntryByPath(external_file_path, *file_entry);
                    }
                    else {  // Relative path
                        String abs_file_path = dir_path + '/' + external_file_path;
                        err = File::fileEntryByPath(abs_file_path, *file_entry);
                    }

                    if (err != ErrorCode::None) {
                        Exception::throwSpecific(kErrFailedToIncludeFile);
                    }

                    files_list.push(file_entry);
                }

                m_included_files_count = static_cast<int32_t>(files_list.size());
                m_included_files_total_size = 0;
                for (auto f : files_list) {
                    m_included_files_total_size += f->m_file_size;
                }

                if (m_included_files_count == 0 || m_included_files_total_size < 1) {
                    _m_tpp_parse_result = toml::parse_file(file_path.utf8());
                }
                else {
                    int64_t bytes_needed = m_included_files_total_size + toml_file_entry.m_file_size;

                    data_buffer = (uint8_t*)malloc(bytes_needed);
                    throwIfNull(data_buffer, ErrorCode::MemCantAllocate);
                    auto data_rest = bytes_needed;
                    auto data_ptr = data_buffer;

                    String toml_data(bytes_needed);
                    file.rewind();

                    int32_t included_file_index = 0;
                    String line;
                    while (file.readLine(line)) {
                        line.trim();

                        if (line == include_token) {
                            file.skipLine();  // Skip the line defining the included file path

                            auto file_entry = files_list.elementAtIndex(included_file_index);

                            throwIfNull(file_entry, ErrorCode::NullData);
                            if (file_entry->m_file_size > 0) {
                                auto err = File::readToBuffer(file_entry->m_path, data_rest, data_ptr);
                                throwIfError(err);
                                data_ptr += file_entry->m_file_size;
                                data_rest -= file_entry->m_file_size;
                                *data_ptr++ = '\n';
                                data_rest--;
                            }

                            included_file_index++;
                        }
                        else {
                            auto line_bytes = line.byteLength();
                            memcpy(data_ptr, line.utf8(), line_bytes);
                            data_ptr += line_bytes;
                            data_rest -= line_bytes;
                            *data_ptr++ = '\n';
                            data_rest--;
                        }
                    }

                    *data_ptr = '\0';  // End of string, important!

                    /* TODO: Refactor for debugging */
                    /*
                    File wf("/Users/roaldchristesen/Desktop/test.toml");
                    wf.startWriteAsciiOverwrite();
                    wf.writeStr((const char*)data_buffer);
                    // wf.writeData(1, data_ptr, data_ptr - data_buffer);
                    wf.close();
                    */

                    _m_tpp_parse_result = toml::parse(std::string_view((const char*)data_buffer));
                }
            }
            else {
                _m_tpp_parse_result = toml::parse_file(file_path.utf8());
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (const toml::parse_error& err) {
            _tppParserError(err);
            result = m_last_err_code = ErrorCode::TomlParseError;
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Cleanup
        free(data_buffer);

        throwIfError(result);
    }


    void Toml::parse(const char* str) {
        auto result = ErrorCode::None;

        try {
            _m_tpp_parse_result = toml::parse(std::string_view(str));
        }
        catch (const toml::parse_error& err) {
            _tppParserError(err);
            result = m_last_err_code = ErrorCode::TomlParseError;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        throwIfError(result);
    }


    void Toml::_tppParserError(const toml::parse_error& err) {
        const auto& region = err.source();
        m_line = region.begin.line;
        m_column = region.begin.column;

        char buffer[1024];
        snprintf(
                buffer,
                1024,
                "Toml parse error at line %" PRId32 ", column %" PRId32 ": %s",
                m_line,
                m_column,
                std::string(err.description()).c_str());

        m_last_err_message = buffer;
    }


    void Toml::logError(Log& l) {
        l << "Toml error: " << m_last_err_message;
        l << ", code: " << static_cast<int32_t>(m_last_err_code) << l.endl;
    }


    TomlArray Toml::arrayByName(const char* name) noexcept {
        TomlArray array;
        if (name != nullptr) {
            array._m_tpp_array_ptr = _m_tpp_parse_result[name].as_array();
        }
        return array;
    }


    TomlArray Toml::arrayByNameOrThrow(const char* name, int32_t local_exc_code) {
        if (name == nullptr) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlNoName,
                    "Expected a name, but a nullptr was found, code: %d",
                    local_exc_code);
        }
        else {
            auto tpp_array = _m_tpp_parse_result[name].as_array();
            if (!tpp_array) {
                Exception::throwFormattedMessage(
                        ErrorCode::TomlExpectedNode,
                        "Expected array with name \"%s\" not found, code: %d",
                        name,
                        local_exc_code);
            }

            return TomlArray(_m_tpp_parse_result[name].as_array());
        }
        return TomlArray();
    }


    void Toml::throwParserError(const char* str) {
        Exception::throwFormattedMessage(
                ErrorCode::TomlParseError,
                "Toml parser exception: %s",
                str != nullptr ? str : "");
    }


    void Toml::throwParserErrorFileLine(const char* file, int32_t line) {
        Exception::throwFormattedMessage(
                ErrorCode::TomlParseError,
                "Toml parser exception in file: %s, line: %d",
                file,
                line);
    }


    void throwTomlParseError(const toml::source_region& region) {
        Exception::throwFormattedMessage(
                ErrorCode::TomlParseError,
                "Toml parser exception: %s, at line %d, column %d",
                region.begin.line,
                region.begin.column);
    }


    ErrorCode Toml::toJson(String& out_string) {
        try {
            toml::json_formatter formatter{ _m_tpp_parse_result };

            std::stringstream stream;
            stream << toml::json_formatter{ _m_tpp_parse_result };
            out_string = stream.str().c_str();
        }
        catch (...) {
            throwIfError(ErrorCode::Fatal);
        }

        return ErrorCode::None;
    }


} // End of namespace Grain
