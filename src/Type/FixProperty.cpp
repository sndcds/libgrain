//
//  FixProperty.cpp
//
//  Created by Roald Christesen on 22.09.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Type/FixProperty.hpp"
#include "String/String.hpp"


namespace Grain {

    FixPropertyList::FixPropertyList(int32_t capacity) : List(capacity) {
    }


    FixPropertyList::~FixPropertyList() {
    }


    void FixPropertyList::addProperty(const char* name, const Fix& min, const Fix& max, const Fix& default_value, const Fix& value) noexcept {
        FixProperty property;
        property.set(name, min, max, default_value, value);
        push(property);
    }


    void FixPropertyList::removePropertyAtIndex(int32_t index) noexcept {
    }


    bool FixPropertyList::isIndex(int32_t index) const noexcept {
        return index >= 0 && index < size();
    }


    bool FixPropertyList::propertyAtIndex(int32_t index, FixProperty& out_property) const noexcept {
        return elementAtIndex(index, out_property);
    }


    FixProperty* FixPropertyList::mutablePropertyAtIndex(int32_t index) noexcept {
        return mutElementPtrAtIndex(index);
    }


    FixProperty* FixPropertyList::mutablePropertyPtrByName(const char* name) noexcept {
        for (auto& property : *this) {
            if (strcmp(name, property.m_name) == 0) {
                return &property;
            }
        }

        return nullptr;
    }


    Fix FixPropertyList::valueAtIndex(int32_t index) const noexcept {
        FixProperty property;
        if (elementAtIndex(index, property)) {
            Fix result;
            result.setRaw(property.m_value);
            return result;
        }
        else {
            return Fix();
        }
    }


    bool FixPropertyList::setValueAtIndex(int32_t index, const Fix& value) noexcept {
        if (auto property_ptr = mutablePropertyAtIndex(index)) {
            return property_ptr->setValue(value.raw());
        }
        else {
            return false;
        }
    }


    bool FixPropertyList::resetValueAtIndex(int32_t index) noexcept {
        if (auto property_ptr = mutablePropertyAtIndex(index)) {
            return property_ptr->resetValue();
        }
        else {
            return false;
        }
    }


    bool FixPropertyList::setValueByName(const char* name, const Fix& value) noexcept {
        if (auto property_ptr = mutablePropertyPtrByName(name)) {
            return property_ptr->setValue(value.raw());
        }
        else {
            return false;
        }
    }


    bool FixPropertyList::resetValueByName(const char* name) noexcept {
        if (auto property_ptr = mutablePropertyPtrByName(name)) {
            return property_ptr->resetValue();
        }
        else {
            return false;
        }
    }

} // End of namespace Grain
