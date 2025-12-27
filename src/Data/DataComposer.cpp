//
//  DataComposer.cpp
//
//  Created by Roald Christesen on 19.04.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

// TODO: exceptions, throw

#include "Data/DataComposer.hpp"
#include "Time/Timestamp.hpp"
#include "App/App.hpp"
#include "Scripting/Toml.hpp"


namespace Grain {


const DataComposer::PropTypeName DataComposer::g_prop_type_name_table[] = {
    { DataComposerPropType::Bool, "bool" },
    { DataComposerPropType::Int32, "i32" },
    { DataComposerPropType::Int64, "i64" },
    { DataComposerPropType::Float, "float" },
    { DataComposerPropType::Double, "double" },
    { DataComposerPropType::Fix, "fix" },
    { DataComposerPropType::Vec2f, "vec2f" },
    { DataComposerPropType::RGBA, "rgba" },
    { DataComposerPropType::Rational, "rational" },
    { DataComposerPropType::URational, "urational" },
    { DataComposerPropType::String, "string" },
    { DataComposerPropType::Date, "date" },
    { DataComposerPropType::Time, "time" },
    { DataComposerPropType::Timestamp, "timestamp" },
    { DataComposerPropType::Object, "object" },
    { DataComposerPropType::List, "list" },
    { DataComposerPropType::Unknown, nullptr }
};


DataComposerPropDescription::DataComposerPropDescription(
    const char* name,
    DataComposerPropType type,
    const char* default_value,
    const char* model_name,
    bool is_nullable
) {
    name_ = strdup(name);
    type_ = type;
    default_value_str_ = default_value != nullptr ? strdup(default_value) : nullptr;
    model_name_ = model_name != nullptr ? strdup(model_name) : nullptr;
    is_nullable_ = is_nullable;
    _initFunctions();
}

DataComposerPropDescription::DataComposerPropDescription(const char* name, DataComposerModel* model) {
    name_ = strdup(name);
    type_ = DataComposerPropType::Object;
    _initFunctions();
}

DataComposerPropDescription::DataComposerPropDescription(DataComposerPropDescription* prop) {
    name_ = prop->name_;
    type_ = prop->type_;
    _initFunctions();
}

DataComposerPropDescription::~DataComposerPropDescription() {
    free(default_value_str_);
    free(model_name_);
}

void DataComposerPropDescription::_initFunctions() {
    switch (type_) {
        case DataComposerPropType::Bool:
            _set_b_func = DataComposer::_pl_set_b_by_b;
            _set_i32_func = DataComposer::_pl_set_b_by_i32;
            _set_i64_func = DataComposer::_pl_set_b_by_i64;
            _set_f_func = DataComposer::_pl_set_b_by_f;
            _set_d_func = DataComposer::_pl_set_b_by_d;
            _set_str_func = DataComposer::_pl_set_b_by_str;
            break;

        case DataComposerPropType::Int32:
            _set_b_func = DataComposer::_pl_set_i32_by_b;
            _set_i32_func = DataComposer::_pl_set_i32_by_i32;
            _set_i64_func = DataComposer::_pl_set_i32_by_i64;
            _set_f_func = DataComposer::_pl_set_i32_by_f;
            _set_d_func = DataComposer::_pl_set_i32_by_d;
            _set_str_func = DataComposer::_pl_set_i32_by_str;
            break;

        case DataComposerPropType::Int64:
            _set_b_func = DataComposer::_pl_set_i64_by_b;
            _set_i32_func = DataComposer::_pl_set_i64_by_i32;
            _set_i64_func = DataComposer::_pl_set_i64_by_i64;
            _set_f_func = DataComposer::_pl_set_i64_by_f;
            _set_d_func = DataComposer::_pl_set_i64_by_d;
            _set_str_func = DataComposer::_pl_set_i64_by_str;
            break;

        case DataComposerPropType::Float:
            _set_b_func = DataComposer::_pl_set_f_by_b;
            _set_i32_func = DataComposer::_pl_set_f_by_i32;
            _set_i64_func = DataComposer::_pl_set_f_by_i64;
            _set_f_func = DataComposer::_pl_set_f_by_f;
            _set_d_func = DataComposer::_pl_set_f_by_d;
            _set_str_func = DataComposer::_pl_set_f_by_str;
            break;

        case DataComposerPropType::Double:
            _set_b_func = DataComposer::_pl_set_d_by_b;
            _set_i32_func = DataComposer::_pl_set_d_by_i32;
            _set_i64_func = DataComposer::_pl_set_d_by_i64;
            _set_f_func = DataComposer::_pl_set_d_by_f;
            _set_d_func = DataComposer::_pl_set_d_by_d;
            _set_str_func = DataComposer::_pl_set_d_by_str;
            break;

        case DataComposerPropType::String:
            _set_b_func = DataComposer::_pl_set_str_by_b;
            _set_i32_func = DataComposer::_pl_set_str_by_i32;
            _set_i64_func = DataComposer::_pl_set_str_by_i64;
            _set_f_func = DataComposer::_pl_set_str_by_f;
            _set_d_func = DataComposer::_pl_set_str_by_d;
            _set_str_func = DataComposer::_pl_set_str_by_str;
            break;

        case DataComposerPropType::Object:
        case DataComposerPropType::List:
        default:
            break;
    }
}

void DataComposerPropDescription::log(Log& l) const {
    l << '(' << DataComposer::propTypeName(type_) << ") ";
    l << name_;
    if (type_ != DataComposerPropType::Object) {
        if (default_value_str_ != nullptr) {
            l << ", default: " << default_value_str_;
        }
    }
    if (is_nullable_ == true) {
        l << ", nullable";
    }
    l << ", size: " << sizeOf() << " bytes";
    l << Log::endl;
}


/**
 *
 */
size_t DataComposerPropDescription::sizeOf() {
    // TODO: Check!
    // The property actually consumes 8 more bytes for the pointer to the model!
    return sizeof(DataComposerPropValue) + 8;
}

void DataComposerPropDescription::logPayload(Log& l, DataComposerPayload* payload) {
    l << payload->pd_->name_ << ": ";
    if (payload->value_.is_null) {
        l << "null" << Log::endl;
    }
    else {
        switch (payload->pd_->type_) {
            case DataComposerPropType::Bool:
                l << (payload->value_.value.b ? "true" : "false") << Log::endl;
                break;
            case DataComposerPropType::Int32:
                l << payload->value_.value.i32 << Log::endl;
                break;
            case DataComposerPropType::Int64:
                l << payload->value_.value.i64 << Log::endl;
                break;
            case DataComposerPropType::Float:
                l << Fix(payload->value_.value.f) << Log::endl;
                break;
            case DataComposerPropType::Double:
                l << Fix(payload->value_.value.d) << Log::endl;
                break;
            case DataComposerPropType::String: {
                if (payload->value_.value.str != nullptr) {
                    l << '"' << payload->value_.value.str << '"' << Log::endl;
                }
                break;
            }
            case DataComposerPropType::Object:
                l << Log::endl;
                l++;
                if (payload->value_.value.ob_ptr != nullptr) {
                    payload->value_.value.ob_ptr->log(l);
                }
                else {
                    l << "Error: nullptr in property of type Object" << Log::endl;
                }
                l--;
                break;
            case DataComposerPropType::List: {
                l << "size: " << payload->value_.value.list->size() << Log::endl;
                l++;
                int32_t index = 0;
                for (auto& ob : *payload->value_.value.list) {
                    ob->log(l);
                    index++;
                    if (index > 10) {
                        l << "... " << (payload->value_.value.list->size() - 10) << " more objects" << Log::endl;
                        break;
                    }
                }
                l--;
                break;
            }

            default:
                break;
        }
    }
}

int64_t DataComposerPropDescription::sizeOfPayload(DataComposerPayload* payload) {
    if (!payload) return 0;
    // Prevent overflow
    uint64_t base = sizeof(DataComposerPayload);
    uint64_t extra = payload->value_.data_size;
    if (extra > INT64_MAX - base) {
        return -1;
    }
    return static_cast<int64_t>(base + extra);
}

DataComposerModel::DataComposerModel(const char* name, DataComposerModel* parent) {
    name_ = name;
    parent_ = parent;
}

void DataComposerModel::logClassHierarchy(Log& l) const {
    if (parent_ != nullptr) {
        parent_->logClassHierarchy(l);
        l << "/";
    }
    l << name_;
}

void DataComposerModel::log(Log& l) {
    l.header(className());
    l << "model_name: " << name_ << Log::endl;
    l << "class hierarchy: ";
    logClassHierarchy(l);
    l << Log::endl;
    l << "properties: " << propCount() << Log::endl;
    l << "property bytes: " << propBytes() << Log::endl;
    l++;
    logProperties(l);
    l--;
    l--;
}

void DataComposerModel::logProperties(Log& l) {
    if (parent_ != nullptr) {
        parent_->logProperties(l);
    }
    l << '(' << name() << ')' << Log::endl;
    l++;
    for (auto& pd : pd_list_) {
        pd->log(l);
    }
    l--;
}

void DataComposerModel::addPropChangeOwner(DataComposerPropDescription* prop) {
    if (propDescriptionByName(prop->name_) != nullptr) {
        // TODO: Error code!
        std::cout << "Property with name \"" << prop->name_ << "\" already exists!\n";
    }
    else {
        model_prop_size_ += DataComposerPropDescription::sizeOf();
        pd_list_.push(prop);
    }
}

void DataComposerModel::addProp(
    const char* name,
    DataComposerPropType type,
    const char* default_value,
    const char* model_name,
    bool is_nullable
)
{
    addPropChangeOwner(new DataComposerPropDescription(name, type, default_value, model_name, is_nullable));
}

int32_t DataComposerModel::propCount() {
    if (total_prop_n_ < 0) {
        _updatePropCount();
    }
    return total_prop_n_;
}

DataComposerPropDescription* DataComposerModel::propDescriptionByName(const char* name) {
    for (auto& pd : pd_list_) {
        if (strcmp(pd->name_, name) == 0) {
            return pd;
        }
    }
    if (parent_ != nullptr) {
        return parent_->propDescriptionByName(name);
    }
    return nullptr;
}

void DataComposerModel::_updatePropCount() {
    total_prop_n_ = 0;
    total_prop_size_ = 0;

    auto model = this;
    while (model != nullptr) {
        total_prop_n_ += static_cast<int32_t>(model->pd_list_.size());
        total_prop_size_ += model->model_prop_size_;
        model = model->parent_;
    }
}

void DataComposerModel::log(Log& l, DataComposerModel* ob) {
    if (ob != nullptr) {
        ob->log(l);
    }
    else {
        l << "DataComposerModel model nullptr" << Log::endl;
    }
}

DataComposerOb::DataComposerOb(DataComposerModel* model) {
    model_ = model;
    // Allocate memory for all properties
    payloads_ = (DataComposerPayload*)malloc(sizeof(DataComposerPayload) * propCount());
    _initProperties(model, 0);
}


DataComposerOb::~DataComposerOb() {
    for (int32_t i = 0; i < propCount(); i++) {
        switch (payloads_[i].pd_->type_) {
            case DataComposerPropType::String: {
                free(payloads_[i].value_.value.str);
                break;
            }
            case DataComposerPropType::Object: {
                delete payloads_[i].value_.value.ob_ptr;
                break;
            }
            case DataComposerPropType::List: {
                delete payloads_[i].value_.value.list;
                break;
            }
            default:
                break;
        }
    }

    free(payloads_);
}

void DataComposerOb::log(Log& l) const {
    l << "(model: " << model_->name_ << ", " << sizeOf() << " bytes)" << Log::endl;
    for (int32_t i = 0; i < propCount(); i++) {
        DataComposerPropDescription::logPayload(l, &payloads_[i]);
    }
}

int32_t DataComposerOb::_initProperties(DataComposerModel* model, int32_t index) {
    if (model->parent_ != nullptr) {
        index = _initProperties(model->parent_, index);
    }
    if (payloads_ != nullptr) {
        for (auto& pd : model->pd_list_) {
            auto payload_ptr = &payloads_[index];
            payload_ptr->pd_ = pd;

            switch (pd->type_) {
                case DataComposerPropType::Object: {
                    if (pd->is_nullable_) {
                        payload_ptr->value_.is_null = true;
                    }
                    else {
                        payload_ptr->value_.value.ob_ptr = new(std::nothrow) DataComposerOb(pd->model_);
                        payload_ptr->value_.is_null = false;
                    }
                    break;
                }
                case DataComposerPropType::List: {
                    payload_ptr->value_.value.list = new ObjectList<DataComposerOb*>();
                    payload_ptr->value_.is_null = false;
                    break;
                }
                default:
                    if (pd->default_value_str_ != nullptr) {
                        // TODO: Can possibly be optimized!
                        DataComposerOb::setPropPayloadByStr(payload_ptr, pd->default_value_str_);
                    }
                    else if (pd->is_nullable_) {
                        free(payload_ptr->value_.value.str);
                        payload_ptr->value_.value.str = nullptr;
                        payload_ptr->value_.is_null = true;
                    }
                    else {
                        DataComposerOb::setPropPayloadByStr(payload_ptr, "");
                    }
                    break;
            }
            index++;
        }
    }

    return index;
}

int64_t DataComposerOb::sizeOf() const {
    int64_t size = 0;
    for (int32_t i = 0; i < propCount(); i++) {
        size += DataComposerPropDescription::sizeOfPayload(&payloads_[i]);
    }
    return size;
}

int32_t DataComposerOb::propIndexByName(const char* prop_name) const {
    for (int32_t i = 0; i < propCount(); i++) {
        if (strcmp(payloads_[i].pd_->name_, prop_name) == 0) {
            return i;
        }
    }
    return -1;
}

DataComposerPayload* DataComposerOb::propPayloadByName(const char* prop_name) const noexcept {
    for (int32_t i = 0; i < propCount(); i++) {
        if (strcmp(payloads_[i].pd_->name_, prop_name) == 0) {
            return &payloads_[i];
        }
    }
    return nullptr;
}

DataComposerPayload* DataComposerOb::propPayloadByNameCanThrow(const char* prop_name) const {
    for (int32_t i = 0; i < propCount(); i++) {
        if (strcmp(payloads_[i].pd_->name_, prop_name) == 0) {
            return &payloads_[i];
        }
    }
    Exception::throwStandard(ErrorCode::DataComposerUnknownPropertyType);
    return nullptr;
}

DataComposerPayload* DataComposerOb::propPayloadByNameCheckTypeCanThrow(const char* prop_name, DataComposerPropType prop_type) const {
    auto payload = propPayloadByNameCanThrow(prop_name);
    if (payload->pd_->type_ != prop_type) {
        /* TODO: Build Exception message!
        m_model->m_composer->m_last_errors->writeFormatted(
            "Error: GenOb \"%s\" is of type \"%s\" but \"%s\" is requested",
            m_model->m_name,
            DataComposer::propTypeName(payload->m_pd->m_type),
            DataComposer::propTypeName(payload->m_pd->m_type)
        );
         */
        Exception::throwStandard(ErrorCode::DataComposerPropertyTypeMismatch);
    }
    return payload;
}

DataComposerPayload* DataComposerOb::propPayloadByNameAndType(const char* prop_name, DataComposerPropType type) const {
    for (int32_t i = 0; i < propCount(); i++) {
        if (strcmp(payloads_[i].pd_->name_, prop_name) == 0 && payloads_[i].pd_->type_ == type) {
            return &payloads_[i];
        }
    }
    return nullptr;
}

DataComposerPayload* DataComposerOb::propPayloadAtIndex(int32_t index) const {
    if (index >= 0 && index < propCount()) {
        return &payloads_[index];
    }
    return nullptr;
}

DataComposerOb* DataComposerOb::obByName(const char* prop_name) const {
    if (auto payload = propPayloadByName(prop_name)) {
        if (payload->pd_->type_ == DataComposerPropType::Object) {
            return payload->value_.value.ob_ptr;
        }
    }
    return nullptr;
}

DataComposerOb* DataComposerOb::obByNameGuaranteed(const char* prop_name) const {
    if (auto payload = propPayloadByName(prop_name)) {
        if (payload->pd_->type_ == DataComposerPropType::Object) {
            if (payload->value_.value.ob_ptr != nullptr) {
                return payload->value_.value.ob_ptr;
            }
            else {
                payload->value_.value.ob_ptr = new(std::nothrow) DataComposerOb(payload->pd_->model_);
                payload->value_.is_null = false;
                return payload->value_.value.ob_ptr;
            }
        }
    }
    return nullptr;
}

const char* DataComposerOb::getStr(const char* prop_name) const {
    auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, DataComposerPropType::String);
    return payload->value_.value.str;
}

void DataComposerOb::getString(const char* prop_name, String& out_string) const {
    auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, DataComposerPropType::String);
    out_string = payload->value_.value.str;
}

void DataComposerOb::getInt32(const char* prop_name, int32_t& out_value) const {
    auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, DataComposerPropType::Int32);
    out_value = payload->value_.value.i32;
}

void DataComposerOb::getInt64(const char* prop_name, int64_t& out_value) const {
    auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, DataComposerPropType::Int64);
    out_value = payload->value_.value.i64;
}

void DataComposerOb::setStr(const char* prop_name, const char* str) const {
    auto payload = propPayloadByNameCanThrow(prop_name);
    payload->pd_->_set_str_func(payload, str);
}

void DataComposerOb::setStr(int32_t index, const char* str) const  {
    if (isPropIndex(index)) {
        auto payload = &payloads_[index];
        payload->pd_->_set_str_func(payload, str);
    }
}

void DataComposerOb::setInt32(const char* prop_name, int32_t value) const {
    auto payload = propPayloadByNameCanThrow(prop_name);
    payload->pd_->_set_i32_func(payload, value);
}

void DataComposerOb::setInt32(int32_t index, int32_t value) const {
    // TODO: Implement!
}

void DataComposerOb::setInt64(const char* prop_name, int64_t value) const {
    auto payload = propPayloadByNameCanThrow(prop_name);
    payload->pd_->_set_i64_func(payload, value);
}

void DataComposerOb::setInt64(int32_t index, int64_t value) const {
    // TODO: Implement!
}

void DataComposerOb::setFloat(const char* prop_name, float value) const {
    auto payload = propPayloadByNameCanThrow(prop_name);
    payload->pd_->_set_f_func(payload, value);
}

void DataComposerOb::setFloat(int32_t index, float value) const {
    // TODO: Implement!
}

void DataComposerOb::setDouble(const char* prop_name, double value) const {
    auto payload = propPayloadByNameCanThrow(prop_name);
    payload->pd_->_set_d_func(payload, value);
}

void DataComposerOb::setDouble(int32_t index, double value) const {
    // TODO: Implement!
}

void DataComposerOb::setObChangeOwner(const char* prop_name, DataComposerOb* ob) const {
    auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, DataComposerPropType::Object);
    payload->value_.value.ob_ptr = ob;
    payload->value_.is_null = (ob == nullptr);
}

void DataComposerOb::setPropAtIndexByStr(int32_t index, const char* str) {
    // TODO: Implement!
}

void DataComposerOb::addToListChangeOwner(const char* prop_name, DataComposerOb* ob) const {
    auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, DataComposerPropType::List);
    payload->value_.value.list->push(ob);
}

void DataComposerOb::setPropPayloadByStr(DataComposerPayload* payload, const char* str) {
    if (!str) {
        payload->value_.is_null = true;
        return;
    }

    switch (payload->pd_->type_) {
        case DataComposerPropType::Bool: {
            if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0) {
                payload->value_.value.b = true;
                payload->value_.is_null = false;
            }
            else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0) {
                payload->value_.value.b = false;
                payload->value_.is_null = false;
            }
            else {
                payload->value_.is_null = true;
            }
            break;
        }
        case DataComposerPropType::Int32: {
            payload->value_.value.i32 = static_cast<int32_t>(String::asInt32(str));
            payload->value_.is_null = false;
            break;
        }
        case DataComposerPropType::Int64: {
            payload->value_.value.i64 = static_cast<int64_t>(String::asInt64(str));
            payload->value_.is_null = false;
            break;
        }
        case DataComposerPropType::Float: {
            payload->value_.value.f = static_cast<float>(String::parseDoubleWithDotOrComma(str));
            payload->value_.is_null = false;
            break;
        }
        case DataComposerPropType::Double: {
            payload->value_.value.d = String::parseDoubleWithDotOrComma(str);
            payload->value_.is_null = false;
            break;
        }
        case DataComposerPropType::String: {
            if (payload->value_.value.str != nullptr) {
                free(payload->value_.value.str);
                payload->value_.value.str = nullptr;
            }
            payload->value_.data_size = 0;
            payload->value_.is_null = true;
            payload->value_.value.str = strdup(str);
            payload->value_.data_size = (int32_t)strlen(str);
            payload->value_.is_null = false;
            break;
        }
        default:
            payload->value_.is_null = true;
            break;
    }
}

void DataComposerOb::setPropPayloadByInt32(DataComposerPayload* payload, int32_t value) {
}

void DataComposer::initByTomlFile(const String& file_path) {
    Log l;

    try {
        Toml toml;
        toml.parseFile(file_path);

        TomlTable my_table;
        toml.asTable(my_table);

        l << my_table.position() << Log::endl;
        auto toml_models = toml.arrayByName("model");

        for (const TomlArrayItem& toml_model : toml_models) {
            auto toml_table = toml_model.asTableOrThrow();
            auto model_name = toml_table.asStringThrow("model_name");
            auto parent_name = toml_table.asString("model_parent", "");
            auto model = addModel(model_name, parent_name);

            for (const TomlTableItem& item : toml_table) {
                auto key = item.key();
                auto value = item.value();
                if (value.isTable()) {
                    TomlTable prop_table;
                    value.asTable(prop_table);

                    // Property type
                    auto prop_type_name = prop_table.asStringThrow("type");
                    auto prop_type = propTypeByName(prop_type_name);
                    if (prop_type == DataComposerPropType::Unknown) {
                        Exception::throwFormattedMessage(ErrorCode::DataComposerUnknownPropertyType, "'%s'", prop_type_name);
                    }

                    // Optional fields
                    String prop_default;
                    String prop_model_name;
                    bool prop_has_default = false;
                    bool prop_uses_model = false;
                    bool prop_is_nullable = false;

                    TomlTableItem prop_table_item;
                    if (prop_table.itemByName("default", prop_table_item) == true) {
                        prop_has_default = prop_table_item.value().asStringForced(prop_default);
                    }

                    if (prop_table.itemByName("model", prop_table_item) == true) {
                        prop_uses_model = prop_table_item.value().asStringForced(prop_model_name);
                    }

                    if (prop_table.itemByName("nullable", prop_table_item) == true) {
                        prop_is_nullable = prop_table_item.value().asBoolean();
                    }

                    const char* default_value = prop_has_default ? prop_default.utf8() : nullptr;
                    const char* used_model_name = prop_uses_model ? prop_model_name.utf8() : nullptr;
                    model->addProp(key, prop_type, default_value, used_model_name, prop_is_nullable);
                }
            }
        }

        _updateReferences();
        log(l);
    }
    catch (const toml::parse_error& err) {
        Exception::throwFormattedMessage(ErrorCode::TomlParseError, "%s", err.what());
    }
    catch (ErrorCode err) {
        Exception::throwFormattedMessage(err, "Raised in Grain::DataComposer::initByTomlFile()");
    }
    catch (const std::exception& e) {
        Exception::throwFormattedMessage(ErrorCode::Unknown, e.what());
    }
    catch (...) {
        Exception::throwFormattedMessage(ErrorCode::Unknown, "Unknown exception in Grain::DataComposer::initByTomlFile()");
    }
}

ErrorCode DataComposer::_updateReferences() {
    auto result = ErrorCode::None;

    for (auto& model : model_list_) {
        model->composer_ = this;

        // Connect model to parent if necessary
        if (model->parent_name_.isNotEmpty()) {
            auto parent_model = modelByName(model->parentName());
            if (!parent_model) {
                Exception::throwFormattedMessage(ErrorCode::DataComposerNoModelForName, "No model found by name \"%s\"", model->parentName());
            }
            model->parent_ = parent_model;
        }

        // Connect properties to models, if property is of type `GenPropType::Object`
        for (auto& pd : model->pd_list_) {
            if (pd->type_ == DataComposerPropType::Object) {
                auto property_model = modelByName(pd->model_name_);
                if (!property_model) {
                    Exception::throwFormattedMessage(
                        ErrorCode::DataComposerNoModelForName,
                        "No model with name '%s' found for property '%s' in model '%s'",
                        pd->model_name_,
                        pd->name_,
                        model->name());
                }
                if (!model) {
                    Exception::throwStandard(ErrorCode::DataComposerNoModelForName);
                }
                pd->model_ = property_model;
            }
        }

        model->_updatePropCount();
    }

    return result;
}


/**
 *  @brief Adds a new model with the given name, optionally based on an
 *         existing model.
 *
 *  @param name The name of the new model to create.
 *  @param parent_model_name Optional name of the base model to inherit from.
 *                           May be nullptr.
 *  @return Pointer to the newly created model.
 *
 *  @note If @p parent_model_name is nullptr, the new model is created without a parent model.
 *  @note Returns a raw pointer. Ownership is transferred to the internal model list.
 */
DataComposerModel* DataComposer::addModel(const char* name, const char* parent_model_name) {
    DataComposerModel* parent_model = nullptr;
    if (parent_model_name != nullptr && parent_model_name[0] != '\0') {
        parent_model = modelByName(parent_model_name);
        if (!parent_model) {
            Exception::throwStandard(ErrorCode::DataComposerModelCreationNoBaseModel);
        }
    }
    auto model = new(std::nothrow) DataComposerModel(name, parent_model);
    if (!model) {
        Exception::throwStandard(ErrorCode::DataComposerNoModelForName);
        return nullptr;
    }
    else {
        model->_updatePropCount();
        model_list_.push(model);
        return model;
    }
}

DataComposerPropType DataComposer::propTypeByName(const char* type_name) noexcept {
    int32_t index = 0;
    while (g_prop_type_name_table[index].type != DataComposerPropType::Unknown) {
        if (strcmp(type_name, g_prop_type_name_table[index].name) == 0) {
            return g_prop_type_name_table[index].type;
        }
        index++;
    }
    return DataComposerPropType::Unknown;
}

const char* DataComposer::propTypeName(DataComposerPropType type) noexcept {
    if ((int32_t)type >= 0 && type <= DataComposerPropType::Last) {
        return g_prop_type_name_table[(int32_t)type].name;
    }
    else {
        return "unknown";
    }
}

void DataComposer::log(Log& l) {
    l.header(className());
    l << "models: " << modelCount() << Log::endl;
    l++;
    int64_t index = 0;
    for (auto& model : model_list_) {
        if (index >= 10) {
            l << "... and " << (model_list_.size() - 10) << " more" << Log::endl;
            break;
        }
        model->log(l);
        index++;
    }
    l--;
    l--;
}

void DataComposer::addModelChangeOwner(DataComposerModel* model) {
    model->_updatePropCount();
    model_list_.push(model);
}

DataComposerModel* DataComposer::modelByName(const char* name) noexcept {
    for (auto& model : model_list_) {
        if (std::strcmp(model->name(), name) == 0) {
            return model;
        }
    }
    return nullptr;
}

DataComposerPropDescription* DataComposer::modelPropByName(const char* model_name, const char* prop_name) noexcept {
    auto model = modelByName(model_name);
    if (model != nullptr) {
        return model->propDescriptionByName(prop_name);
    }
    return nullptr;
}

void DataComposer::logModelByName(Log& l, const char* model_name) noexcept {
    if (auto model = modelByName(model_name)) {
        model->log(l);
    }
    else {
        l << "No model for name '" << model_name << "'" << Log::endl;
    }
}


/**
 *  @brief Adds a new object based on the given model.
 *
 *  @param model Pointer to the model on which the object will be based.
 *  @return Pointer to the newly created object.
 *
 *  @note Returns a raw pointer. The caller is responsible for managing
 *        ownership.
 */
DataComposerOb* DataComposer::addOb(DataComposerModel* model) {
    if (!model) {
        Exception::throwStandard(ErrorCode::DataComposerObjectCreationModelIsNull);
    }
    auto ob = new(std::nothrow) DataComposerOb(model);
    if (!ob) {
        Exception::throwStandard(ErrorCode::DataComposerObjectCreationFailed);
    }
    return ob;
}


/**
 *  @brief Adds a new object based on a model identified by name.
 *
 *  @param model_name Name of the model to base the object on.
 *  @return Pointer to the newly created object.
 *
 *  @note This is a convenience wrapper that resolves the model by name and
 *        calls the corresponding `addOb(DataComposerModel*)` method.
 *  @note Returns a raw pointer. The caller is responsible for managing
 *        ownership.
 */
DataComposerOb* DataComposer::addOb(const char* model_name) {
    return addOb(modelByName(model_name));
}


} // End of namespace Grain
