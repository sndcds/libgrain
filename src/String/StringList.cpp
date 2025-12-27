//
//  StringList.cpp
//
//  Created by Roald Christesen on 14.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "String/StringList.hpp"
#include "Core/Log.hpp"


namespace Grain {

    void StringList::log(Log& l) {
        l << className() << l.endl;
        l++;
        l << "size_: " << size_ << l.endl;
        l << "m_capacity: " << capacity_ << l.endl;
        l++;
        int64_t index = 0;
        for (auto string : *this) {
            if (index > 10) {
                l << "..." << l.endl;
                break;
            }
            l << index << ": " << string << l.endl;
            index++;
        }
        if (index < size_) {
            l << (size_ - 1) << ": " << last() << l.endl;
        }
        l--;
        l--;
    }


    bool StringList::contains(const String& string) const noexcept {
        for (auto s : *this) {
            if (s) {
                if (s->compare(string) == 0) {
                    return true;
                }
            }
        }
        return false;
    }


    int32_t StringList::countOccurrences(const String& string) const noexcept {
        int32_t result = 0;
        for (auto s : *this) {
            if (s) {
                if (s->compare(string) == 0) {
                    result++;
                }
            }
        }
        return result;
    }


    bool StringList::pushStr(const char* str) noexcept {
        if (str) {
            auto s = new (std::nothrow) String(str);
            if (s) {
                bool result = List::push(s);
                return result;
            }
        }
        return false;
    }


    /**
     *  @brief Appends a string to the end of the list.
     *
     *  This function copies the content of the given string and adds it to the end
     *  of the list.
     *
     *  @param string The string to be appended to the list.
     *  @return true if the operation succeeds, false otherwise.
     */
    bool StringList::pushString(const String& string) noexcept {
        return pushString(&string);
    }


    /**
     *  @brief Appends a string to the end of the list.
     *
     *  This function copies the content of the given string and adds it to the end
     *  of the list.
     *
     *  @param string A pointer to the string to be appended to the list.
     *  @return true if the operation succeeds, false otherwise.
     */
    bool StringList::pushString(const String* string) noexcept {
        if (string) {
            auto s = new (std::nothrow) String(*string);
            if (s) {
                bool result = List::push(s);
                return result;
            }
        }
        return false;
    }


    /**
     *  @brief Appends a string to the end of the list if it is not already present.
     *
     *  This function copies the content of the given string and adds it to the end
     *  of the list, but only if the string is not already in the list.
     *
     *  @param string The string to be appended to the list.
     *  @return true if the operation succeeds (string added) or the string
     *          is already present in the list, false otherwise.
     */
    bool StringList::pushUnique(const String& string) noexcept {
        if (countOccurrences(string) < 1) {
            return pushString(string);
        }
        return false;
    }


    bool StringList::removeStr(const char* str) noexcept {
        for (int64_t i = 0; i < size_; ++i) {
            auto s = data_[i];
            if (s && s->compare(str) == 0) {
                auto err = removeAtIndex(i);  // safe: i is still valid
                return err == ErrorCode::None;
            }
        }
        return false;
    }


    bool StringList::removeString(const String& string) noexcept {
        return removeStr(string.utf8());
    }


    int32_t StringList::_sortAsc(const void* a, const void* b) {
        auto string_a = (const String**)(a);
        auto string_b = (const String**)(b);
        return (**string_a).compareIgnoreCase(**string_b);
    }


    int32_t StringList::_sortDesc(const void* a, const void* b) {
        auto string_a = (const String**)(a);
        auto string_b = (const String**)(b);
        return (**string_b).compareIgnoreCase(**string_a);
    }


} // End of namespace Grain
