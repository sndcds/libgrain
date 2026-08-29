//
//  JSONWalker.hpp
//
//  Created by Roald Christesen on from 15.02.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  Last viewed: 29.05.2026
//

#ifndef GrainJSONWalker_hpp
#define GrainJSONWalker_hpp

#include <fstream>
#include <iostream>
#include "String/String.hpp"
#include "Extern/nlohmann/json.hpp"


namespace Grain {

using json = nlohmann::json;


class JSONValue {
public:
    explicit JSONValue(const json* j) noexcept : _j(j) {}


    // Type checks

    [[nodiscard]] bool isNull() const noexcept { return !_j || _j->is_null(); }
    [[nodiscard]] bool isString() const noexcept { return _j && _j->is_string(); }
    [[nodiscard]] bool isInt() const noexcept { return _j && _j->is_number_integer(); }
    [[nodiscard]] bool isUInt() const noexcept { return _j && _j->is_number_unsigned(); }
    [[nodiscard]] bool isFloat() const noexcept { return _j && _j->is_number_float(); }
    [[nodiscard]] bool isBoolean() const noexcept { return _j && _j->is_boolean(); }
    [[nodiscard]] bool isNumber() const noexcept { return _j && _j->is_number(); }


    // Value accessors

    [[nodiscard]] String asString() const noexcept {
        if (!_j) return String::emptyString();

        if (_j->is_string()) {
            return { _j->get<std::string>().c_str() };
        }

        if (_j->is_boolean()) {
            return _j->get<bool>() ? String("true") : String("false");
        }

        if (_j->is_null()) {
            return { "null" };
        }

        if (_j->is_number()) {
            return { _j->dump().c_str() };
        }

        return String::emptyString();
    }

    [[nodiscard]] int64_t asInt64() const noexcept {
        if (!_j) return 0;

        if (_j->is_number_integer()) {
            return _j->get<int64_t>();
        }

        if (_j->is_boolean()) {
            return _j->get<bool>() ? 1 : 0;
        }

        if (_j->is_number()) {
            return static_cast<int64_t>(_j->get<double>());
        }

        return 0;
    }

    [[nodiscard]] double asDouble() const noexcept {
        if (!_j) return 0.0;

        if (_j->is_number()) {
            return _j->get<double>();
        }

        if (_j->is_boolean()) {
            return _j->get<bool>() ? 1.0 : 0.0;
        }

        return 0.0;
    }

    [[nodiscard]] int32_t asInt32() const noexcept { return static_cast<int32_t>(asInt64()); }
    [[nodiscard]] float asFloat() const noexcept { return static_cast<float>(asDouble()); }

    [[nodiscard]] bool asBoolean() const noexcept {
        if (!_j) return false;

        if (_j->is_boolean()) {
            return _j->get<bool>();
        }

        if (_j->is_number_integer()) {
            return _j->get<int64_t>() != 0;
        }

        if (_j->is_string()) {
            return !_j->get<std::string>().empty();
        }

        return false;
    }


    // Raw access

    [[nodiscard]] const json* raw() noexcept { return _j; }
    [[nodiscard]] const json* raw() const noexcept { return _j; }

private:
    const json* _j = nullptr;
};

class JSONWalker {
public:
    virtual ~JSONWalker() = default;

    bool loadFromFile(const String& file_path) {
        std::ifstream file(file_path.utf8());
        if (!file.is_open()) return false;

        try {
            json_root_ = json::parse(file);
        } catch (const json::parse_error& e) {
            last_error_ = e.what();
            return false;
        }
        return true;
    }

    void traverse() {
        if (!json_root_.is_null()) _traverseJSON(json_root_);
    }

protected:

    void _traverseJSON(const json& j) {
        walk(j, "");
    }

    void walk(const json& j, const std::string& path) {
        if (j.is_object()) {
            onObjectStart(path.c_str());

            for (auto it = j.begin(); it != j.end(); ++it) {
                std::string childPath = path.empty()
                    ? it.key()
                    : path + "." + it.key();

                walk(it.value(), childPath);
            }

            onObjectEnd(path.c_str());
        }
        else if (j.is_array()) {
            onArrayStart(path.c_str());

            for (size_t i = 0; i < j.size(); ++i) {
                std::string childPath =
                    path + "[" + std::to_string(i) + "]";

                walk(j[i], childPath);
            }

            onArrayEnd(path.c_str());
        }
        else {
            JSONValue jv(&j);
            addProperty(path.c_str(), jv);
        }
    }


    // Hooks which can overridden

    virtual void onObjectStart(const String& path) {}
    virtual void onObjectEnd(const String& path) {}
    virtual void onArrayStart(const String& path) {}
    virtual void onArrayEnd(const String& path) {}
    virtual void addProperty(const String& path, const JSONValue& value);

    const String& lastError() noexcept { return last_error_; }

protected:
    json json_root_;
    String last_error_;
};

} // End of namespace Grain

#endif // GrainJSONWalker