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
#include "Database/PSQL.hpp"


namespace Grain::DataComposer {

    const Schema::PropTypeName Schema::g_prop_type_name_table[] = {
        { PropType::Bool, "bool" },
        { PropType::Int32, "i32" },
        { PropType::Int64, "i64" },
        { PropType::Float, "float" },
        { PropType::Double, "double" },
        { PropType::Fix, "fix" },
        { PropType::Vec2f, "vec2f" },
        { PropType::RGBA, "rgba" },
        { PropType::Rational, "rational" },
        { PropType::URational, "urational" },
        { PropType::String, "string" },
        { PropType::Date, "date" },
        { PropType::Time, "time" },
        { PropType::Timestamp, "timestamp" },
        { PropType::Object, "object" },
        { PropType::List, "list" },
        { PropType::Unknown, nullptr }
    };


    PropDescription::PropDescription(
        const char* name,
        PropType type,
        const char* default_value,
        const char* model_name,
        bool is_nullable
    ) {
        name_ = strdup(name);
        type_ = type;
        default_value_str_ = default_value != nullptr ? strdup(default_value) : nullptr;
        referenced_model_name_ = model_name != nullptr ? strdup(model_name) : nullptr;
        is_nullable_ = is_nullable;
        _initFunctions();
    }

    /*
    PropDescription::PropDescription(const char* name, Model* model) {
        name_ = strdup(name);
        type_ = PropType::Object;
        _initFunctions();
    }

    PropDescription::PropDescription(PropDescription* prop) {
        name_ = prop->name_;
        type_ = prop->type_;
        _initFunctions();
    }
    */
    PropDescription::~PropDescription() {
        free(name_);
        free(default_value_str_);
        free(referenced_model_name_);
    }

    void PropDescription::_initFunctions() {
        switch (type_) {
            case PropType::Bool:
                _set_b_func = Schema::_pl_set_b_by_b;
                _set_i32_func = Schema::_pl_set_b_by_i32;
                _set_i64_func = Schema::_pl_set_b_by_i64;
                _set_f_func = Schema::_pl_set_b_by_f;
                _set_d_func = Schema::_pl_set_b_by_d;
                _set_str_func = Schema::_pl_set_b_by_str;
                break;

            case PropType::Int32:
                _set_b_func = Schema::_pl_set_i32_by_b;
                _set_i32_func = Schema::_pl_set_i32_by_i32;
                _set_i64_func = Schema::_pl_set_i32_by_i64;
                _set_f_func = Schema::_pl_set_i32_by_f;
                _set_d_func = Schema::_pl_set_i32_by_d;
                _set_str_func = Schema::_pl_set_i32_by_str;
                break;

            case PropType::Int64:
                _set_b_func = Schema::_pl_set_i64_by_b;
                _set_i32_func = Schema::_pl_set_i64_by_i32;
                _set_i64_func = Schema::_pl_set_i64_by_i64;
                _set_f_func = Schema::_pl_set_i64_by_f;
                _set_d_func = Schema::_pl_set_i64_by_d;
                _set_str_func = Schema::_pl_set_i64_by_str;
                break;

            case PropType::Float:
                _set_b_func = Schema::_pl_set_f_by_b;
                _set_i32_func = Schema::_pl_set_f_by_i32;
                _set_i64_func = Schema::_pl_set_f_by_i64;
                _set_f_func = Schema::_pl_set_f_by_f;
                _set_d_func = Schema::_pl_set_f_by_d;
                _set_str_func = Schema::_pl_set_f_by_str;
                break;

            case PropType::Double:
                _set_b_func = Schema::_pl_set_d_by_b;
                _set_i32_func = Schema::_pl_set_d_by_i32;
                _set_i64_func = Schema::_pl_set_d_by_i64;
                _set_f_func = Schema::_pl_set_d_by_f;
                _set_d_func = Schema::_pl_set_d_by_d;
                _set_str_func = Schema::_pl_set_d_by_str;
                break;

            case PropType::String:
                _set_b_func = Schema::_pl_set_str_by_b;
                _set_i32_func = Schema::_pl_set_str_by_i32;
                _set_i64_func = Schema::_pl_set_str_by_i64;
                _set_f_func = Schema::_pl_set_str_by_f;
                _set_d_func = Schema::_pl_set_str_by_d;
                _set_str_func = Schema::_pl_set_str_by_str;
                break;

            case PropType::Object:
            case PropType::List:
            default:
                break;
        }
    }

    void PropDescription::log(Log& l) const {
        l << '(' << Schema::propTypeName(type_) << ") ";
        l << name_;
        if (type_ != PropType::Object) {
            if (default_value_str_) {
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
    size_t PropDescription::sizeOf() {
        // TODO: Check!
        // The property actually consumes 8 more bytes for the pointer to the model!
        return sizeof(PropValue) + 8;
    }

    void PropDescription::logPayload(Log& l, Payload* payload) {
        l << payload->pd_->name_ << ": ";
        if (payload->value_.is_null) {
            l << "null" << Log::endl;
        }
        else {
            switch (payload->pd_->type_) {
                case PropType::Bool:
                    l << (payload->value_.value.b ? "true" : "false") << Log::endl;
                    break;
                case PropType::Int32:
                    l << payload->value_.value.i32 << Log::endl;
                    break;
                case PropType::Int64:
                    l << payload->value_.value.i64 << Log::endl;
                    break;
                case PropType::Float:
                    l << Fix(payload->value_.value.f) << Log::endl;
                    break;
                case PropType::Double:
                    l << Fix(payload->value_.value.d) << Log::endl;
                    break;
                case PropType::String: {
                    if (payload->value_.value.str) {
                        l << '"' << payload->value_.value.str << '"' << Log::endl;
                    }
                    break;
                }
                case PropType::Object:
                    l << Log::endl;
                    l++;
                    if (payload->value_.value.ob_ptr) {
                        payload->value_.value.ob_ptr->log(l);
                    }
                    else {
                        l << "Error: nullptr in property of type Object" << Log::endl;
                    }
                    l--;
                    break;
                case PropType::List: {
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

    int64_t PropDescription::sizeOfPayload(Payload* payload) {
        if (!payload) return 0;
        // Prevent overflow
        uint64_t base = sizeof(Payload);
        uint64_t extra = payload->value_.data_size;
        if (extra > INT64_MAX - base) {
            return -1;
        }
        return static_cast<int64_t>(base + extra);
    }

    Model::Model(const char* name, Model* parent) {
        name_ = name;
        parent_ = parent;
    }

    void Model::logClassHierarchy(Log& l) const {
        if (parent_) {
            parent_->logClassHierarchy(l);
            l << "/";
        }
        l << name_;
    }

    void Model::log(Log& l) {
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

    void Model::logProperties(Log& l) {
        if (parent_) {
            parent_->logProperties(l);
        }
        l << '(' << name() << ')' << Log::endl;
        l++;
        for (auto& pd : pd_list_) {
            pd->log(l);
        }
        l--;
    }

    void Model::addPropChangeOwner(PropDescription* prop) {
        if (propDescriptionByName(prop->name_)) {
            // TODO: Error code!
            std::cout << "Property with name \"" << prop->name_ << "\" already exists!\n";
        }
        else {
            model_prop_size_ += PropDescription::sizeOf();
            pd_list_.push(prop);
        }
    }

    void Model::addProp(
        const char* name,
        PropType type,
        const char* default_value,
        const char* model_name,
        bool is_nullable
    )
    {
        addPropChangeOwner(new PropDescription(name, type, default_value, model_name, is_nullable));
    }

    int32_t Model::propCount() {
        if (total_prop_n_ < 0) {
            _updatePropCount();
        }
        return total_prop_n_;
    }

    PropDescription* Model::propDescriptionByName(const char* name) {
        for (auto& pd : pd_list_) {
            if (strcmp(pd->name_, name) == 0) {
                return pd;
            }
        }
        if (parent_) {
            return parent_->propDescriptionByName(name);
        }
        return nullptr;
    }

    void Model::_updatePropCount() {
        total_prop_n_ = 0;
        total_prop_size_ = 0;

        auto model = this;
        while (model) {
            total_prop_n_ += static_cast<int32_t>(model->pd_list_.size());
            total_prop_size_ += model->model_prop_size_;
            model = model->parent_;
        }
    }

    void Model::log(Log& l, Model* ob) {
        if (ob) {
            ob->log(l);
        }
        else {
            l << "Model model nullptr" << Log::endl;
        }
    }

    Object::Object(Model* model) {
        model_ = model;
        payloads_ = (Payload*)calloc(propCount(), sizeof(Payload));
        _initProperties(model, 0);
    }


    Object::~Object() {
        for (int32_t i = 0; i < propCount(); i++) {
            auto p = &payloads_[i];
            switch (p->pd_->type_) {
                case PropType::String: {
                    free(p->value_.value.str);
                    break;
                }
                case PropType::Object: {
                    delete p->value_.value.ob_ptr;
                    break;
                }
                case PropType::List: {
                    delete p->value_.value.list;
                    break;
                }
                default:
                    break;
            }
        }

        free(payloads_);
    }

    void Object::log(Log& l) const {
        l << "(model: " << model_->name_ << ", " << sizeOf() << " bytes)" << Log::endl;
        for (int32_t i = 0; i < propCount(); i++) {
            PropDescription::logPayload(l, &payloads_[i]);
        }
    }

    int32_t Object::_initProperties(Model* model, int32_t index) {
        if (model->parent_) {
            index = _initProperties(model->parent_, index);
        }

        if (payloads_) {
            for (auto& pd : model->pd_list_) {
                auto payload_ptr = &payloads_[index];
                payload_ptr->pd_ = pd;
                switch (pd->type_) {
                    case PropType::Object: {
                        if (pd->is_nullable_) {
                            payload_ptr->value_.is_null = true;
                            payload_ptr->value_.value.ob_ptr = nullptr;
                        }
                        else {
                            payload_ptr->value_.value.ob_ptr = new(std::nothrow) Object(pd->referenced_model_);
                            payload_ptr->value_.is_null = false;
                        }
                        break;
                    }
                    case PropType::List: {
                        payload_ptr->value_.value.list = new ObjectList<Object*>();
                        payload_ptr->value_.is_null = false;
                        break;
                    }
                    default:
                        if (pd->default_value_str_) {
                            // TODO: Can possibly be optimized!
                            setPropPayloadByStr(payload_ptr, pd->default_value_str_);
                        }
                        else if (pd->is_nullable_) {
                            payload_ptr->value_.is_null = true;
                            payload_ptr->value_.value.ob_ptr = nullptr;
                        }
                        else {
                            setPropPayloadByStr(payload_ptr, "");
                        }
                        break;
                }
                index++;
            }
        }

        return index;
    }

    int64_t Object::sizeOf() const {
        int64_t size = 0;
        for (int32_t i = 0; i < propCount(); i++) {
            size += PropDescription::sizeOfPayload(&payloads_[i]);
        }
        return size;
    }

    int32_t Object::propIndexByName(const char* prop_name) const {
        for (int32_t i = 0; i < propCount(); i++) {
            if (strcmp(payloads_[i].pd_->name_, prop_name) == 0) {
                return i;
            }
        }
        return -1;
    }

    Payload* Object::propPayloadByName(const char* prop_name) const noexcept {
        for (int32_t i = 0; i < propCount(); i++) {
            if (strcmp(payloads_[i].pd_->name_, prop_name) == 0) {
                return &payloads_[i];
            }
        }
        return nullptr;
    }

    Payload* Object::propPayloadByNameCanThrow(const char* prop_name) const {
        for (int32_t i = 0; i < propCount(); i++) {
            if (strcmp(payloads_[i].pd_->name_, prop_name) == 0) {
                return &payloads_[i];
            }
        }
        Exception::throwStandard(ErrorCode::DataComposerUnknownPropertyType);
        return nullptr;
    }

    Payload* Object::propPayloadByNameCheckTypeCanThrow(const char* prop_name, PropType prop_type) const {
        auto payload = propPayloadByNameCanThrow(prop_name);
        if (payload->pd_->type_ != prop_type) {
            /* TODO: Build Exception message!
            m_model->m_composer->m_last_errors->writeFormatted(
                "Error: GenOb \"%s\" is of type \"%s\" but \"%s\" is requested",
                m_model->m_name,
                Schema::propTypeName(payload->m_pd->m_type),
                Schema::propTypeName(payload->m_pd->m_type)
            );
             */
            Exception::throwStandard(ErrorCode::DataComposerPropertyTypeMismatch);
        }
        return payload;
    }

    Payload* Object::propPayloadByNameAndType(const char* prop_name, PropType type) const {
        for (int32_t i = 0; i < propCount(); i++) {
            if (strcmp(payloads_[i].pd_->name_, prop_name) == 0 && payloads_[i].pd_->type_ == type) {
                return &payloads_[i];
            }
        }
        return nullptr;
    }

    Payload* Object::propPayloadAtIndex(int32_t index) const {
        if (index >= 0 && index < propCount()) {
            return &payloads_[index];
        }
        return nullptr;
    }

    Object* Object::obByName(const char* prop_name) const {
        if (auto payload = propPayloadByName(prop_name)) {
            if (payload->pd_->type_ == PropType::Object) {
                return payload->value_.value.ob_ptr;
            }
        }
        return nullptr;
    }

    Object* Object::obByNameGuaranteed(const char* prop_name) const {
        if (auto payload = propPayloadByName(prop_name)) {
            if (payload->pd_->type_ == PropType::Object) {
                if (payload->value_.value.ob_ptr) {
                    return payload->value_.value.ob_ptr;
                }
                else {
                    payload->value_.value.ob_ptr = new(std::nothrow) Object(payload->pd_->referenced_model_);
                    payload->value_.is_null = false;
                    return payload->value_.value.ob_ptr;
                }
            }
        }
        return nullptr;
    }

    const char* Object::getStr(const char* prop_name) const {
        auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, PropType::String);
        return payload->value_.value.str;
    }

    void Object::getString(const char* prop_name, String& out_string) const {
        auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, PropType::String);
        out_string = payload->value_.value.str;
    }

    void Object::getInt32(const char* prop_name, int32_t& out_value) const {
        auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, PropType::Int32);
        out_value = payload->value_.value.i32;
    }

    void Object::getInt64(const char* prop_name, int64_t& out_value) const {
        auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, PropType::Int64);
        out_value = payload->value_.value.i64;
    }

    void Object::setStr(const char* prop_name, const char* str) const {
        auto payload = propPayloadByNameCanThrow(prop_name);
        payload->pd_->_set_str_func(payload, str);
    }

    void Object::setStr(int32_t index, const char* str) const  {
        if (isPropIndex(index)) {
            auto payload = &payloads_[index];
            payload->pd_->_set_str_func(payload, str);
        }
    }

    void Object::setInt32(const char* prop_name, int32_t value) const {
        auto payload = propPayloadByNameCanThrow(prop_name);
        payload->pd_->_set_i32_func(payload, value);
    }

    void Object::setInt32(int32_t index, int32_t value) const {
        // TODO: Implement!
    }

    void Object::setInt64(const char* prop_name, int64_t value) const {
        auto payload = propPayloadByNameCanThrow(prop_name);
        payload->pd_->_set_i64_func(payload, value);
    }

    void Object::setInt64(int32_t index, int64_t value) const {
        // TODO: Implement!
    }

    void Object::setFloat(const char* prop_name, float value) const {
        auto payload = propPayloadByNameCanThrow(prop_name);
        payload->pd_->_set_f_func(payload, value);
    }

    void Object::setFloat(int32_t index, float value) const {
        // TODO: Implement!
    }

    void Object::setDouble(const char* prop_name, double value) const {
        auto payload = propPayloadByNameCanThrow(prop_name);
        payload->pd_->_set_d_func(payload, value);
    }

    void Object::setDouble(int32_t index, double value) const {
        // TODO: Implement!
    }

    void Object::setFix(const char* prop_name, Fix& value) const {
        // TODO: Implement!
        std::cout << "Object::setFix(): " << value << std::endl;
    }


    void Object::setObChangeOwner(const char* prop_name, Object* ob) const {
        auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, PropType::Object);
        payload->value_.value.ob_ptr = ob;
        payload->value_.is_null = (ob == nullptr);
    }

    void Object::setPropAtIndexByStr(int32_t index, const char* str) {
        // TODO: Implement!
    }

    void Object::addToListChangeOwner(const char* prop_name, Object* ob) const {
        auto payload = propPayloadByNameCheckTypeCanThrow(prop_name, PropType::List);
        payload->value_.value.list->push(ob);
    }

    void Object::setPropPayloadByStr(Payload* payload, const char* str) {
        if (!str) {
            payload->value_.is_null = true;
            return;
        }

        switch (payload->pd_->type_) {
            case PropType::Bool: {
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
            case PropType::Int32: {
                payload->value_.value.i32 = static_cast<int32_t>(String::asInt32(str));
                payload->value_.is_null = false;
                break;
            }
            case PropType::Int64: {
                payload->value_.value.i64 = static_cast<int64_t>(String::asInt64(str));
                payload->value_.is_null = false;
                break;
            }
            case PropType::Float: {
                payload->value_.value.f = static_cast<float>(String::parseDoubleWithDotOrComma(str));
                payload->value_.is_null = false;
                break;
            }
            case PropType::Double: {
                payload->value_.value.d = String::parseDoubleWithDotOrComma(str);
                payload->value_.is_null = false;
                break;
            }
            case PropType::String: {
                if (payload->value_.value.str) {
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

    void Object::setPropPayloadByInt32(Payload* payload, int32_t value) {
    }

    void Schema::initByTomlFile(const String& file_path) {
        Log l;
        const char* stage = "starting";

        try {
            stage = "checking TOML file exists";
            if (!File::isFile(file_path)) {
                Exception::throwStandard(ErrorCode::FileNotFound);
            }

            stage = "creating Toml object";
            Toml toml;

            stage = "parsing TOML file";
            toml.parseFile(file_path);

            stage = "converting TOML root to table";
            TomlTable my_table;
            toml.asTable(my_table);

            stage = "getting model array";
            auto toml_models = toml.arrayByName("model");

            std::cout << "Found models: " << toml_models.size() << std::endl;

            int model_index = 0;

            for (const TomlArrayItem& toml_model : toml_models) {
                std::cout
                    << "\n=== MODEL "
                    << model_index
                    << " ==="
                    << std::endl;

                stage = "converting model TOML item to table";
                auto toml_table = toml_model.asTableOrThrow();

                stage = "reading model_name";
                auto model_name = toml_table.asStringThrow("model_name");

                stage = "reading model_parent";
                auto parent_name = toml_table.asString("model_parent", "");

                std::cout
                    << "model_name: " << model_name
                    << ", parent_name: " << parent_name
                    << std::endl;

                stage = "creating model";

                std::cout << "\n=== MODEL " << model_index << " ===" << std::endl;
                std::cout << "model_name: [" << model_name << "]" << std::endl;
                std::cout << "parent_name: [" << parent_name << "]" << std::endl;

                std::cout << "schema model count before create: "
                          << model_list_.size() << std::endl;

                if (parent_name) {
                    std::cout << "looking for parent model: ["
                              << parent_name << "]" << std::endl;

                    auto parent_model = modelByName(parent_name);

                    std::cout << "parent lookup result: "
                              << (parent_model ? parent_model->name() : "NOT FOUND")
                              << std::endl;
                }

                std::cout << "creating model..." << std::endl;

                auto model = createAndRegisterModel(model_name, parent_name);

                std::cout << "model created: "
                          << (model ? model->name() : "NULL")
                          << std::endl;

                int property_index = 0;

                for (const TomlTableItem& item : toml_table) {
                    auto key = item.key();

                    std::cout
                        << "  property["
                        << property_index
                        << "]: "
                        << key
                        << std::endl;

                    stage = "getting property value";

                    auto value = item.value();

                    std::cout
                        << "    value: "
                        << value
                        << std::endl;

                    if (value.isTable()) {
                        stage = "converting property to table";

                        TomlTable prop_table;
                        value.asTable(prop_table);

                        stage = "reading property type";

                        auto prop_type_name =
                            prop_table.asStringThrow("type");

                        std::cout
                            << "    type: "
                            << prop_type_name
                            << std::endl;

                        stage = "converting property type";

                        auto prop_type =
                            propTypeByName(prop_type_name);

                        if (prop_type == PropType::Unknown) {
                            Exception::throwFormattedMessage(
                                ErrorCode::DataComposerUnknownPropertyType,
                                "'%s'",
                                prop_type_name
                            );
                        }

                        stage = "reading property optional fields";

                        String prop_default;
                        String prop_model_name;

                        bool prop_has_default = false;
                        bool prop_uses_model = false;
                        bool prop_is_nullable = false;

                        TomlTableItem prop_table_item;

                        if (prop_table.itemByName(
                                "default",
                                prop_table_item)) {

                            prop_has_default =
                                prop_table_item.value()
                                    .asStringForced(prop_default);

                            std::cout
                                << "    default: "
                                << prop_default
                                << std::endl;
                        }

                        if (prop_table.itemByName(
                                "model",
                                prop_table_item)) {

                            prop_uses_model =
                                prop_table_item.value()
                                    .asStringForced(prop_model_name);

                            std::cout
                                << "    model: "
                                << prop_model_name
                                << std::endl;
                        }

                        if (prop_table.itemByName(
                                "nullable",
                                prop_table_item)) {

                            prop_is_nullable =
                                prop_table_item.value()
                                    .asBoolean();

                            std::cout
                                << "    nullable: "
                                << (prop_is_nullable ? "true" : "false")
                                << std::endl;
                        }

                        stage = "adding property to model";

                        const char* default_value =
                            prop_has_default
                                ? prop_default.utf8()
                                : nullptr;

                        const char* used_model_name =
                            prop_uses_model
                                ? prop_model_name.utf8()
                                : nullptr;

                        model->addProp(
                            key,
                            prop_type,
                            default_value,
                            used_model_name,
                            prop_is_nullable
                        );
                    }

                    property_index++;
                }

                std::cout
                    << "=== END MODEL "
                    << model_index
                    << " ==="
                    << std::endl;

                model_index++;
            }

            stage = "updating model/property references";

            std::cout << "Calling _updateReferences()" << std::endl;

            _updateReferences();

            stage = "logging schema";

            log(l);

            std::cout << "Schema successfully initialized"
                      << std::endl;
        }
        catch (const toml::parse_error& err) {
            std::cerr
                << "TOML PARSE ERROR"
                << "\n  stage: " << stage
                << "\n  file:  " << file_path
                << "\n  error: " << err.what()
                << std::endl;

            Exception::throwFormattedMessage(
                ErrorCode::TomlParseError,
                "TOML parse error in file '%s': %s",
                file_path.utf8(),
                err.what()
            );
        }
        catch (ErrorCode err) {
            std::cerr
                << "GRAIN ERROR"
                << "\n  stage: " << stage
                << "\n  file:  " << file_path
                << "\n  code:  " << int(err)
                << std::endl;

            Exception::throwFormattedMessage(
                err,
                "Raised in Grain::DataComposer::Schema::initByTomlFile()"
            );
        }
        catch (const std::exception& e) {
            std::cerr
                << "STD::EXCEPTION"
                << "\n  stage: " << stage
                << "\n  file:  " << file_path
                << "\n  type:  " << typeid(e).name()
                << "\n  what:  " << e.what()
                << std::endl;

            Exception::throwFormattedMessage(
                ErrorCode::Unknown,
                "Exception in stage '%s': %s",
                stage,
                e.what()
            );
        }
        catch (...) {
            std::cerr
                << "UNKNOWN EXCEPTION"
                << "\n  stage: " << stage
                << "\n  file:  " << file_path
                << std::endl;

            Exception::throwFormattedMessage(
                ErrorCode::Unknown,
                "Unknown exception in stage '%s'",
                stage
            );
        }
    }

    ErrorCode Schema::_updateReferences() {
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
                if (pd->type_ == PropType::Object || pd->type_ == PropType::List) {
                    auto property_model = modelByName(pd->referenced_model_name_);
                    if (!property_model) {
                        Exception::throwFormattedMessage(
                            ErrorCode::DataComposerNoModelForName,
                            "No model with name '%s' found for property '%s' in model '%s'",
                            pd->referenced_model_name_,
                            pd->name_,
                            model->name());
                    }
                    if (!model) {
                        Exception::throwStandard(ErrorCode::DataComposerNoModelForName);
                    }
                    pd->referenced_model_ = property_model;
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
    Model* Schema::createModel(const char* name, const char* parent_model_name) {
        Model* parent_model = nullptr;
        if (parent_model_name && parent_model_name[0] != '\0') {
            parent_model = modelByName(parent_model_name);
            if (!parent_model) {
                Exception::throwStandard(ErrorCode::DataComposerModelCreationNoBaseModel);
            }
        }
        auto model = new(std::nothrow) Model(name, parent_model);
        if (!model) {
            Exception::throwStandard(ErrorCode::DataComposerNoModelForName);
            return nullptr;
        }

        return model;
    }

    Model* Schema::createAndRegisterModel(const char* name, const char* parent_model_name) {
        Model* model = Schema::createModel(name, parent_model_name);
        registerModel(model);
        return model;
    }


    Model* Schema::createAndRegisterModelFromPSQLQueryResult(const char* name, Grain::PSQLTxResult& psql_result, const char* parent_model_name) {
        Model* model = Schema::createModel(name, parent_model_name);
        if (model) {
            for (std::size_t i = 0; i < psql_result.columns(); ++i) {
                auto prop_name = psql_result.columnName(i);
                auto psql_type = psql_result.columnType(i);
                switch (psql_type) {
                    case PSQLFieldType::Boolean:
                        model->addProp(prop_name, PropType::Bool, nullptr, nullptr, true);
                        break;
                    case PSQLFieldType::Integer:
                        model->addProp(prop_name, PropType::Int32, nullptr, nullptr, true);
                        break;
                    case PSQLFieldType::BigInt:
                        model->addProp(prop_name, PropType::Int64, nullptr, nullptr, true);
                        break;
                    case PSQLFieldType::Real:
                    case PSQLFieldType::Double:
                        model->addProp(prop_name, PropType::Double, nullptr, nullptr, true);
                        break;
                    case PSQLFieldType::Numeric:  // TODO: Fix type!
                    case PSQLFieldType::Text:
                    case PSQLFieldType::CharN:
                    case PSQLFieldType::VarChar:
                    default:
                        model->addProp(prop_name, PropType::String, nullptr, nullptr, true);
                        break;
                }
            }

            registerModel(model);
        }

        return model;
    }

    void Schema::registerModel(Model* model) {
        if (model) {
            model->_updatePropCount();
            model_list_.push(model);
        }
    }

    PropType Schema::propTypeByName(const char* type_name) noexcept {
        int32_t index = 0;
        while (g_prop_type_name_table[index].type != PropType::Unknown) {
            if (strcmp(type_name, g_prop_type_name_table[index].name) == 0) {
                return g_prop_type_name_table[index].type;
            }
            index++;
        }
        return PropType::Unknown;
    }

    const char* Schema::propTypeName(PropType type) noexcept {
        if ((int32_t)type >= 0 && type <= PropType::Last) {
            return g_prop_type_name_table[(int32_t)type].name;
        }
        else {
            return "unknown";
        }
    }

    void Schema::log(Log& l) {
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

    void Schema::addModelChangeOwner(Model* model) {
        model->_updatePropCount();
        model_list_.push(model);
    }

    Model* Schema::modelByName(const char* name) noexcept {
        for (auto& model : model_list_) {
            if (std::strcmp(model->name(), name) == 0) {
                return model;
            }
        }
        return nullptr;
    }

    PropDescription* Schema::modelPropByName(const char* model_name, const char* prop_name) noexcept {
        auto model = modelByName(model_name);
        if (model) {
            return model->propDescriptionByName(prop_name);
        }
        return nullptr;
    }

    void Schema::logModelByName(Log& l, const char* model_name) noexcept {
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
    Object* Schema::createObject(Model* model) {
        if (!model) {
            Exception::throwStandard(ErrorCode::DataComposerObjectCreationModelIsNull);
        }
        auto ob = new(std::nothrow) Grain::DataComposer::Object(model);
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
     *        calls the corresponding `addOb(ModelModel*)` method.
     *  @note Returns a raw pointer. The caller is responsible for managing
     *        ownership.
     */
    Object* Schema::createObject(const char* model_name) {
        return createObject(modelByName(model_name));
    }


} // End of namespace Grain
