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
        explicit JSONValue(const json* j) noexcept : j_(j) {}

        // Type checks
        [[nodiscard]] bool isNull() const noexcept { return !j_ || j_->is_null(); }
        [[nodiscard]] bool isString() const noexcept { return j_ && j_->is_string(); }
        [[nodiscard]] bool isInt() const noexcept { return j_ && j_->is_number_integer(); }
        [[nodiscard]] bool isUInt() const noexcept { return j_ && j_->is_number_unsigned(); }
        [[nodiscard]] bool isFloat() const noexcept { return j_ && j_->is_number_float(); }
        [[nodiscard]] bool isBoolean() const noexcept { return j_ && j_->is_boolean(); }
        [[nodiscard]] bool isNumber() const noexcept { return j_ && j_->is_number(); }

        // Value accessors
        [[nodiscard]] String asString() const noexcept {
            if (!j_) return String::emptyString();

            if (j_->is_string()) {
                return { j_->get<std::string>().c_str() };
            }

            if (j_->is_boolean()) {
                return j_->get<bool>() ? String("true") : String("false");
            }

            if (j_->is_null()) {
                return { "null" };
            }

            if (j_->is_number()) {
                return { j_->dump().c_str() };
            }

            return String::emptyString();
        }

        [[nodiscard]] int64_t asInt64() const noexcept {
            if (!j_) return 0;

            if (j_->is_number_integer()) {
                return j_->get<int64_t>();
            }

            if (j_->is_boolean()) {
                return j_->get<bool>() ? 1 : 0;
            }

            if (j_->is_number()) {
                return static_cast<int64_t>(j_->get<double>());
            }

            return 0;
        }

        [[nodiscard]] double asDouble() const noexcept {
            if (!j_) return 0.0;

            if (j_->is_number()) {
                return j_->get<double>();
            }

            if (j_->is_boolean()) {
                return j_->get<bool>() ? 1.0 : 0.0;
            }

            return 0.0;
        }

        [[nodiscard]] int32_t asInt32() const noexcept { return static_cast<int32_t>(asInt64()); }
        [[nodiscard]] float asFloat() const noexcept { return static_cast<float>(asDouble()); }

        [[nodiscard]] bool asBoolean() const noexcept {
            if (!j_) return false;

            if (j_->is_boolean()) {
                return j_->get<bool>();
            }

            if (j_->is_number_integer()) {
                return j_->get<int64_t>() != 0;
            }

            if (j_->is_string()) {
                return !j_->get<std::string>().empty();
            }

            return false;
        }

        // Raw access
        [[nodiscard]] const json* raw() noexcept { return j_; }
        [[nodiscard]] const json* raw() const noexcept { return j_; }

    private:
        const json* j_ = nullptr;
    };


    class JSONAccessor {
    public:
        using json = nlohmann::json;

        // Construction

        explicit JSONAccessor(const json& root) noexcept
            : value_(&root) {
        }

        // State

        [[nodiscard]]
        bool isValid() const noexcept {
            return value_ != nullptr;
        }

        [[nodiscard]]
        bool isNull() const noexcept {
            return !value_ || value_->is_null();
        }

        [[nodiscard]]
        bool isObject() const noexcept {
            return value_ && value_->is_object();
        }

        [[nodiscard]]
        bool isArray() const noexcept {
            return value_ && value_->is_array();
        }

        [[nodiscard]]
        bool isString() const noexcept {
            return value_ && value_->is_string();
        }

        [[nodiscard]]
        bool isInt() const noexcept {
            return value_ && value_->is_number_integer();
        }

        [[nodiscard]]
        bool isUInt() const noexcept {
            return value_ && value_->is_number_unsigned();
        }

        [[nodiscard]]
        bool isFloat() const noexcept {
            return value_ && value_->is_number_float();
        }

        [[nodiscard]]
        bool isBoolean() const noexcept {
            return value_ && value_->is_boolean();
        }

        [[nodiscard]]
        bool isNumber() const noexcept {
            return value_ && value_->is_number();
        }

        // Size

        [[nodiscard]]
        size_t size() const noexcept {
            return value_ ? value_->size() : 0;
        }

        // Object access

        [[nodiscard]]
        JSONAccessor at(const char* key) const noexcept
        {
            if (!value_ || !value_->is_object() || !key) {
                return JSONAccessor(nullptr);
            }

            auto it = value_->find(key);

            if (it == value_->end()) {
                return JSONAccessor(nullptr);
            }

            return JSONAccessor(&(*it));
        }

        // Array access

        [[nodiscard]]
        JSONAccessor at(size_t index) const noexcept
        {
            if (!value_ || !value_->is_array()) {
                return JSONAccessor(nullptr);
            }

            if (index >= value_->size()) {
                return JSONAccessor(nullptr);
            }

            return JSONAccessor(&(*value_)[index]);
        }

        // Value access

        [[nodiscard]]
        String asString() const noexcept {
            return JSONValue(value_).asString();
        }

        [[nodiscard]]
        int64_t asInt64() const noexcept {
            return JSONValue(value_).asInt64();
        }

        [[nodiscard]]
        int32_t asInt32() const noexcept {
            return JSONValue(value_).asInt32();
        }

        [[nodiscard]]
        double asDouble() const noexcept {
            return JSONValue(value_).asDouble();
        }

        [[nodiscard]]
        float asFloat() const noexcept {
            return JSONValue(value_).asFloat();
        }

        [[nodiscard]]
        bool asBoolean() const noexcept {
            return JSONValue(value_).asBoolean();
        }

        // Raw access

        [[nodiscard]]
        const json* raw() const noexcept {
            return value_;
        }

    private:
        // Invalid accessor
        explicit JSONAccessor(const json* value) noexcept
            : value_(value) {
        }

        const json* value_ = nullptr;
    };


    class JSONWalker {
    public:
        virtual ~JSONWalker() = default;

        bool loadFromString(const char* json_string) {
            if (!json_string) {
                last_error_ = "JSON string is null";
                return false;
            }

            try {
                json_root_ = json::parse(json_string);
                return true;
            }
            catch (const json::parse_error& e) {
                last_error_ = e.what();
                return false;
            }
        }

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

        [[nodiscard]] const json& jsonRoot() const noexcept {
            return json_root_;
        }

    protected:
        json json_root_;
        String last_error_;
    };

} // End of namespace Grain

#endif // GrainJSONWalker