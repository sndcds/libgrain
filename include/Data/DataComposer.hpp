//
//  DataComposer.hpp
//
//  Created by Roald Christesen on 19.04.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

/**
 *  Custom Dynamic Object Modeling System
 *
 *  Features:
 *  - DataComposerModel, DataComposerObject, and DataComposerPropDescription (like metadata + value holders)
 *  - A central DataComposerPropPayload union to handle multiple data types
 *  - Support for complex nested structures (ObjectList<DataComposerOb*>)
 */

#ifndef GrainDataComposer_hpp
#define GrainDataComposer_hpp

#include "Grain.hpp"
#include "Core/Log.hpp"
#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "String/String.hpp"

#include <charconv>  // For std::to_chars


/**
 *  @brief Enumerates all supported property types used in DataComposer.
 *
 *  This enumeration defines the various data types that a property can have
 *  Some types are currently placeholders and are not yet implemented.
 *
 *  @note Types marked with "TODO" are reserved for future implementation.
 */
enum class DataComposerPropType : int16_t {
    Unknown = -1,   ///< Invalid or unspecified type
    Bool = 0,       ///< Boolean value (true/false)
    Int32,          ///< 32-bit signed integer
    Int64,          ///< 64-bit signed integer
    Float,          ///< 32-bit floating point number
    Double,         ///< 64-bit floating point number
    Fix,            ///< Fixed-point number (TODO: Not yet implemented)
    Vec2f,          ///< 2D vector, 2 x float (TODO: Not yet implemented)
    RGBA,           ///< 4-component color (R, G, B, A), 4 x uint8_t (TODO: Not yet implemented)
    Rational,       ///< Signed rational number (numerator/denominator) (TODO)
    URational,      ///< Unsigned rational number (TODO)
    String,         ///< UTF-8 encoded text string
    Date,           ///< Calendar date, encoded as YYYYMMDD (e.g., 20250108) (TODO)
    Time,           ///< Time of day, encoded as HHMMSSss (e.g., 10000000) (TODO)
    Timestamp,      ///< Date-time value (TODO)
    Object,         ///< Reference to another object
    List,           ///< List of sub-properties or values

    Count,              ///< Number of defined types
    Last = Count - 1    ///< Alias for the last valid property type
};

namespace Grain {
    class DataComposer;
    class DataComposerModel;
    class DataComposerOb;
    class DataComposerPropDescription;

    struct DataComposerPropValue {
        uint32_t data_size = 0;
        bool is_null = true;

        union {
            bool b;
            int32_t i32;
            int64_t i64;
            float f;
            double d;
            void* data;
            char* str;
            DataComposerOb* ob_ptr;
            ObjectList<DataComposerOb*>* list;
        } value{};

        DataComposerPropValue() = default;
    };

    struct DataComposerPayload {
        DataComposerPropDescription* pd_ = nullptr; ///< Pointer to the property description
        DataComposerPropValue value_{}; ///< The actual value if the property

        [[nodiscard]] inline bool isNull() const noexcept { return value_.is_null; }
        inline void setNull() noexcept { value_.is_null = true; }
    };

    typedef void (*DataComposerPayloadSetStrFunc)(DataComposerPayload* pl, const char* str);
    typedef void (*DataComposerPayloadSetBoolFunc)(DataComposerPayload* pl, bool value);
    typedef void (*DataComposerPayloadSetInt32Func)(DataComposerPayload* pl, int32_t value);
    typedef void (*DataComposerPayloadSetInt64Func)(DataComposerPayload* pl, int64_t value);
    typedef void (*DataComposerPayloadSetFloatFunc)(DataComposerPayload* pl, float value);
    typedef void (*DataComposerPayloadSetDoubleFunc)(DataComposerPayload* pl, double value);
    typedef void (*DataComposerPayloadSetStringFunc)(DataComposerPayload* pl, const char* str);

    class DataComposerPropDescription : public Object {
    public:
        char* name_;                            ///< Name of property
        DataComposerPropType type_;             ///< Type of data
        DataComposerModel* model_ = nullptr;
        ///< If property is of type `DataComposerPropType::Model`, this model is used for the property
        char* default_value_str_ = nullptr;     ///< Default value in C-String form
        char* model_name_ = nullptr;            ///< Optional model name
        bool is_nullable_ = false;
        bool has_default_ = false;
        bool uses_model_ = false;

        DataComposerPayloadSetBoolFunc _set_b_func{};
        DataComposerPayloadSetInt32Func _set_i32_func{};
        DataComposerPayloadSetInt64Func _set_i64_func{};
        DataComposerPayloadSetFloatFunc _set_f_func{};
        DataComposerPayloadSetDoubleFunc _set_d_func{};
        DataComposerPayloadSetStringFunc _set_str_func{};

    public:
        DataComposerPropDescription(
            const char* name,
            DataComposerPropType type, const char* default_value,
            const char* model_name, bool is_nullable);
        DataComposerPropDescription(const char* name, DataComposerModel* model);
        explicit DataComposerPropDescription(DataComposerPropDescription* prop);
        ~DataComposerPropDescription() override;

        [[nodiscard]] const char* className() const noexcept override {
            return "DataComposerPropDescription";
        }

        void log(Log& l) const;
        void _initFunctions();
        [[nodiscard]] static size_t sizeOf();
        [[nodiscard]] bool isModelType() const { return type_ == DataComposerPropType::Object; }
        static void logPayload(Log& l, DataComposerPayload* payload);
        [[nodiscard]] static int64_t sizeOfPayload(DataComposerPayload* payload);
    };


    class DataComposerModel : public Object {
    public:
        String name_;
        String parent_name_;
        DataComposerModel* parent_ = nullptr;
        DataComposer* composer_ = nullptr;
        ObjectList<DataComposerPropDescription*>pd_list_;
        int32_t total_prop_n_ = -1;
        size_t model_prop_size_ = 0;    ///< The byte size of local properties
        size_t total_prop_size_ = 0;    ///< The byte size of all properties, including them from parent models

    public:
        DataComposerModel(const char* name, DataComposerModel* parent);
        ~DataComposerModel() override = default;

        [[nodiscard]] const char* className() const noexcept override { return "DataComposerModel"; }

        friend std::ostream &operator <<(std::ostream& os, const DataComposerModel* o) {
            o == nullptr ? os << "DataComposerModel nullptr" : os << *o;
            return os;
        }

        friend std::ostream &operator <<(std::ostream& os, const DataComposerModel& o) {
            os << o.name_;
            return os;
        }

        void logClassHierarchy(Log& l) const;
        void log(Log& l);
        void logProperties(Log& l);
        static void log(Log& l, DataComposerModel* ob);

        [[nodiscard]] const char* name() const { return name_.utf8(); }
        [[nodiscard]] const char* parentName() const { return parent_name_.utf8(); }
        [[nodiscard]] int32_t propCount();
        [[nodiscard]] size_t propBytes() const { return total_prop_size_; }

        void addPropChangeOwner(DataComposerPropDescription* prop);
        void addProp(
            const char* name,
            DataComposerPropType type,
            const char* default_value,
            const char* model_name,
            bool is_nullable);

        [[nodiscard]] DataComposerPropDescription* propDescriptionByName(const char* name);

        void _updatePropCount();
    };


    class DataComposerOb : public Object {
    public:
        DataComposerModel* model_ = nullptr;
        DataComposerPayload* payloads_ = nullptr;

    public:
        explicit DataComposerOb(DataComposerModel* model);

        ~DataComposerOb() override;

        [[nodiscard]] const char* className() const noexcept override {
            return "DataComposerObject";
        }

        void log(Log& l) const;

        int32_t _initProperties(DataComposerModel* model, int32_t index);

        [[nodiscard]] bool isOf(const char* name) const {
            return model_->name_.compare(name) == 0;
        }

        [[nodiscard]] inline bool isPropIndex(int32_t index) const {
            return index < model_->total_prop_n_ && index >= 0;
        }

        [[nodiscard]] inline int32_t propCount() const {
            return model_->total_prop_n_;
        }

        [[nodiscard]] int64_t sizeOf() const;

        [[nodiscard]] int32_t propIndexByName(const char* prop_name) const;
        [[nodiscard]] DataComposerPayload* propPayloadByName(const char* prop_name) const noexcept;
        [[nodiscard]] DataComposerPayload* propPayloadByNameCanThrow(const char* prop_name) const;
        [[nodiscard]] DataComposerPayload* propPayloadByNameCheckTypeCanThrow(
            const char* prop_name, DataComposerPropType prop_type) const;
        [[nodiscard]] DataComposerPayload* propPayloadByNameAndType(
            const char* prop_name, DataComposerPropType type) const;
        [[nodiscard]] DataComposerPayload* propPayloadAtIndex(int32_t index) const;
        [[nodiscard]] DataComposerOb* obByName(const char* prop_name) const;
        [[nodiscard]] DataComposerOb* obByNameGuaranteed(const char* prop_name) const;

        [[nodiscard]] const char* getStr(const char* prop_name) const;

        void getString(const char* prop_name, String& out_string) const;
        void getInt32(const char* prop_name, int32_t& out_value) const;
        void getInt64(const char* prop_name, int64_t& out_value) const;

        void setStr(const char* prop_name, const char* str) const;
        void setStr(int32_t index, const char* str) const;
        void setInt32(const char* prop_name, int32_t value) const;
        void setInt32(int32_t index, int32_t value) const;
        void setInt64(const char* prop_name, int64_t value) const;
        void setInt64(int32_t index, int64_t value) const;
        void setFloat(const char* prop_name, float value) const;
        void setFloat(int32_t index, float value) const;
        void setDouble(const char* prop_name, double value) const;
        void setDouble(int32_t index, double value) const;

        void setObChangeOwner(const char* prop_name, DataComposerOb* ob) const;

        void setPropAtIndexByStr(int32_t index, const char* str);

        void addToListChangeOwner(const char* prop_name, DataComposerOb* ob) const;

        static void setPropPayloadByStr(DataComposerPayload* payload, const char* str);

        static void setPropPayloadByInt32(DataComposerPayload* payload, int32_t value);
    };


    /**
     *  @class DataComposer
     *  @brief Manages and composes generic data models.
     *
     *  The DataComposer class is responsible for managing a list of generic data
     *  models, allowing for the addition, lookup, and logging of models and their
     *  properties.
     */
    class DataComposer : public Object {
        friend class DataComposerPropDescription;

    public:
        ObjectList<DataComposerModel*> model_list_;

        struct PropTypeName {
            DataComposerPropType type;
            const char* name;
        };

        static const PropTypeName g_prop_type_name_table[];

    public:
        DataComposer() = default;
        ~DataComposer() = default;

        [[nodiscard]] const char* className() const noexcept override { return "DataComposer"; }

        void log(Log& l);

        void initByTomlFile(const String& file_path);
        ErrorCode finalize() { return _updateReferences(); }
        ErrorCode _updateReferences();

        [[nodiscard]] DataComposerModel* addModel(const char* name, const char* parent_model_name);
        [[nodiscard]] static DataComposerPropType propTypeByName(const char* type_name) noexcept;
        [[nodiscard]] static const char* propTypeName(DataComposerPropType type) noexcept;
        void addModelChangeOwner(DataComposerModel* model);
        [[nodiscard]] DataComposerModel* modelByName(const char* name) noexcept;
        [[nodiscard]] DataComposerPropDescription* modelPropByName(
            const char* model_name,
            const char* prop_name) noexcept;

        void logModelByName(Log& l, const char* model_name) noexcept;
        [[nodiscard]] int32_t modelCount() const noexcept {
            return static_cast<int32_t>(model_list_.size());
        }
        [[nodiscard]] static DataComposerOb* addOb(DataComposerModel* model);
        [[nodiscard]] DataComposerOb* addOb(const char* model_name);

    private:
        static inline void _pl_set_str(DataComposerPayload* pl, const char* str) {
            if (pl->value_.value.str != nullptr) {
                free(pl->value_.value.str);
                pl->value_.value.str = nullptr;
                pl->value_.data_size = 0;
                pl->value_.is_null = true;
            }
            if (str != nullptr) {
                pl->value_.value.str = strdup(str);
                pl->value_.is_null = false;
                pl->value_.data_size = (int32_t) strlen(str);
            }
            else {
                pl->value_.is_null = true;
            }
        }

        static void _pl_set_by_b_dummy(DataComposerPayload* pl, bool value) {
        };

        static void _pl_set_b_by_b(DataComposerPayload* pl, bool value) {
            pl->value_.value.b = value;
        }

        static void _pl_set_i32_by_b(DataComposerPayload* pl, bool value) {
            pl->value_.value.i32 = value ? 0 : 1;
        }

        static void _pl_set_i64_by_b(DataComposerPayload* pl, bool value) {
            pl->value_.value.i64 = value ? 0 : 1;
        }

        static void _pl_set_f_by_b(DataComposerPayload* pl, bool value) {
            pl->value_.value.f = value ? 0.0f : 1.0f;
        }

        static void _pl_set_d_by_b(DataComposerPayload* pl, bool value) {
            pl->value_.value.d = value ? 0.0 : 1.0;
        }

        static void _pl_set_str_by_b(DataComposerPayload* pl, bool value) {
            _pl_set_str(pl, value == true ? "true" : "false");
        }

        static void _pl_set_by_i32_dummy(DataComposerPayload* pl, int32_t value) {
        }

        static void _pl_set_b_by_i32(DataComposerPayload* pl, int32_t value) {
            pl->value_.value.b = value != 0;
        }

        static void _pl_set_i32_by_i32(DataComposerPayload* pl, int32_t value) {
            pl->value_.value.i32 = value;
        }

        static void _pl_set_i64_by_i32(DataComposerPayload* pl, int32_t value) {
            pl->value_.value.i64 = static_cast<int64_t>(value);
        }

        static void _pl_set_f_by_i32(DataComposerPayload* pl, int32_t value) {
            pl->value_.value.f = static_cast<float>(value);
        }

        static void _pl_set_d_by_i32(DataComposerPayload* pl, int32_t value) {
            pl->value_.value.d = static_cast<double>(value);
        }

        static void _pl_set_str_by_i32(DataComposerPayload* pl, int32_t value) {
            char buffer[12]; // Safe size for int32_t: 11 digits + null if needed
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            if (ec == std::errc()) {
                *ptr = '\0'; // null-terminate if you want a C-string
                _pl_set_str(pl, buffer);
            }
        }

        static void _pl_set_by_i64_dummy(DataComposerPayload* pl, int64_t value) {
        }

        static void _pl_set_b_by_i64(DataComposerPayload* pl, int64_t value) {
            pl->value_.value.b = value != 0;
        }

        static void _pl_set_i32_by_i64(DataComposerPayload* pl, int64_t value) {
            pl->value_.value.i32 = static_cast<int32_t>(value);
        }

        static void _pl_set_i64_by_i64(DataComposerPayload* pl, int64_t value) {
            pl->value_.value.i64 = value;
        }

        static void _pl_set_f_by_i64(DataComposerPayload* pl, int64_t value) {
            pl->value_.value.f = static_cast<float>(value);
        }

        static void _pl_set_d_by_i64(DataComposerPayload* pl, int64_t value) {
            pl->value_.value.d = static_cast<double>(value);
        }

        static void _pl_set_str_by_i64(DataComposerPayload* pl, int64_t value) {
            char buffer[21]; // Safe size for int64_t: 20 digits + null if needed
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            if (ec == std::errc()) {
                *ptr = '\0'; // null-terminate if you want a C-string
                _pl_set_str(pl, buffer);
            }
        }

        static void _pl_set_by_f_dummy(DataComposerPayload* pl, float value) {
        }

        static void _pl_set_b_by_f(DataComposerPayload* pl, float value) {
            pl->value_.value.b = value != 0.0f;
        }

        static void _pl_set_i32_by_f(DataComposerPayload* pl, float value) {
            pl->value_.value.i32 = static_cast<int32_t>(roundf(value));
        }

        static void _pl_set_i64_by_f(DataComposerPayload* pl, float value) {
            pl->value_.value.i64 = static_cast<int64_t>(roundf(value));
        }

        static void _pl_set_f_by_f(DataComposerPayload* pl, float value) {
            pl->value_.value.f = value;
        }

        static void _pl_set_d_by_f(DataComposerPayload* pl, float value) {
            pl->value_.value.d = static_cast<double>(value);
        }

        static void _pl_set_str_by_f(DataComposerPayload* pl, float value) {
            char buffer[32]; // Safe size for float (up to 9 digits + sign + decimal + exponent)
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            if (ec == std::errc()) {
                *ptr = '\0'; // null-terminate if needed
                _pl_set_str(pl, buffer);
            }
        }

        static void _pl_set_by_d_dummy(DataComposerPayload* pl, double value) {
        }

        static void _pl_set_b_by_d(DataComposerPayload* pl, double value) {
            pl->value_.value.b = value != 0.0;
        }

        static void _pl_set_i32_by_d(DataComposerPayload* pl, double value) {
            pl->value_.value.i32 = static_cast<int32_t>(round(value));
        }

        static void _pl_set_i64_by_d(DataComposerPayload* pl, double value) {
            pl->value_.value.i64 = static_cast<int64_t>(round(value));
        }

        static void _pl_set_f_by_d(DataComposerPayload* pl, double value) {
            pl->value_.value.f = static_cast<float>(value);
        }

        static void _pl_set_d_by_d(DataComposerPayload* pl, double value) {
            pl->value_.value.d = value;
        }

        static void _pl_set_str_by_d(DataComposerPayload* pl, double value) {
            char buffer[64]; // Safe size for double: digits + decimal + sign + exponent
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            if (ec == std::errc()) {
                *ptr = '\0'; // null-terminate if needed
                _pl_set_str(pl, buffer);
            }
        }

        static void _pl_set_by_str_dummy(DataComposerPayload* pl, const char* str) {
        }

        static void _pl_set_b_by_str(DataComposerPayload* pl, const char* str) {
            if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0) {
                pl->value_.value.b = true;
                pl->value_.is_null = false;
            }
            else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0) {
                pl->value_.value.b = false;
                pl->value_.is_null = false;
            }
            else {
                pl->value_.is_null = true;
            }
        }

        static void _pl_set_i32_by_str(DataComposerPayload* pl, const char* str) {
            pl->value_.value.i32 = static_cast<int32_t>(String::asInt32(str));
            pl->value_.is_null = false;
        }

        static void _pl_set_i64_by_str(DataComposerPayload* pl, const char* str) {
            pl->value_.value.i64 = static_cast<int64_t>(String::asInt64(str));
            pl->value_.is_null = false;
        }

        static void _pl_set_f_by_str(DataComposerPayload* pl, const char* str) {
            pl->value_.value.f = static_cast<float>(String::parseDoubleWithDotOrComma(str));
            pl->value_.is_null = false;
        }

        static void _pl_set_d_by_str(DataComposerPayload* pl, const char* str) {
            pl->value_.value.d = String::parseDoubleWithDotOrComma(str);
            pl->value_.is_null = false;
        }

        static void _pl_set_str_by_str(DataComposerPayload* pl, const char* str) {
            _pl_set_str(pl, str);
        }
    };
} // End of namespace Grain

#endif // GrainDataComposer_hpp
