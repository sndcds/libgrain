//
//  Object.cpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Type/Object.hpp"
#include <cstring>

namespace Grain {

    /**
     *  @brief Returns the class name as a C-string.
     *
     *  @return A constant character pointer to the class name C-string.
     */
    const char* BaseObject::className() const noexcept {

        return "BaseObject";
    }


    /**
     *  @brief Checks if the object's class name matches the given name.
     *
     *  @param name The name to compare against the object's class name.
     *  @return true if the names match, false otherwise.
     */
    bool BaseObject::isClass(const char* name) const noexcept {

        return name != nullptr && std::strcmp(className(), name) == 0;
    }


    /**
     *  @brief Attempts to set a parameter by name and value.
     *
     *  The default implementation returns ErrorCode::UnknownParameter.
     *
     *  @param name The name of the parameter.
     *  @param value The value to assign to the parameter.
     *  @return An ErrorCode indicating success or the type of error.
     */
    ErrorCode BaseObject::setParam(const String& name, const String& value) noexcept {

        return ErrorCode::ObjectParamSetFailed;
    }


    /**
     *  @brief Handles a message with an associated value.
     *
     *  The default implementation returns ErrorCode::BaseMethodCalled.
     *
     *  @param message The message identifier.
     *  @param value The value associated with the message.
     *  @return An ErrorCode indicating success or the type of error.
     */
    ErrorCode BaseObject::handleMessage(const char* message, const char* value) noexcept {

        return ErrorCode::ObjectMessageFailed;
    }


    Object::Object() {

        m_retain_counter = 1;
    }


    Object::~Object() {
    }


    const char* Object::className() const noexcept {

        return "Object";
    }


    /**
     *  @brief Gets the currentMillis retain count.
     *
     *  @return The currentMillis value of the retain counter.
     */
    int64_t Object::retainCounter() const noexcept {

        return m_retain_counter;
    }


    /**
     *  @brief Increments the retain count of the given object.
     *
     *  @param object A pointer to the object whose retain count is to be
     *                incremented.
     */
    void Object::retain(Object* object) {

        if (object != nullptr) {
            object->m_retain_counter++;
        }
    }

    /**
     *  @brief Decrements the retain count of the given object and deletes it if
     *         the count reaches zero.
     *
     *  @param object A pointer to the object whose retain count is to be
     *                decremented.
     *  @return nullptr if the object was deleted, otherwise returns the
     *          original pointer.
     */
    Object* Object::release(Object* object) {

        if (object != nullptr) {
            object->m_retain_counter--;

            if (object->m_retain_counter < 1) {
                delete object;
                return nullptr;
            }
        }

        return object;
    }

} // namespace Grain