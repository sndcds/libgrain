//
//  StringList.hpp
//
//  Created by Roald Christesen on 14.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 20.06.2025
//

#ifndef GrainStringList_hpp
#define GrainStringList_hpp


#include "Grain.hpp"
#include "String.hpp"


namespace Grain {

    /**
     *  @class StringList
     *  @brief Dynamic list of strings with memory management.
     *
     *  The StringList class provides a dynamic list to store strings, managing the
     *  memory allocation and deallocation for the stored strings. It offers various
     *  utility methods for manipulating and working with the list of strings.
     *
     *  ## Usage Example:
     *  @code
     *      StringList string_list;
     *
     *      // Adding strings to the list
     *      string_list.add("Apple");
     *      string_list.add("Banana");
     *      string_list.add("Orange");
     *
     *      // Sorting the strings
     *      string_list.sort();
     *
     *      // Removing a string
     *      string_list.remove("Banana");
     *
     *      // Counting the number of strings
     *      i32_t count = string_list.length();
     *  @endcode
     *
     *  The class ensures proper memory management for the stored strings, including allocation
     *  and deallocation, providing a convenient interface for working with dynamic lists of strings.
     */
    class StringList : public List<String*> {

    public:
        StringList() : List(List<String*>::kMinStepSize) {
            setSortCompareFunc(_sortAsc);
        }

        StringList(int64_t capacity) : List(capacity) {
            setSortCompareFunc(_sortAsc);
        }

        ~StringList() override {
            for (auto string : *this) {
                GRAIN_RELEASE(string);
            }
        }

        [[nodiscard]] const char* className() const noexcept override { return "StringList"; }

        friend std::ostream& operator << (std::ostream& os, const StringList* o) {
            o == nullptr ? os << "StringList nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const StringList& o) {
            return os << "StringList size: " << o.size() << std::endl;
        }

        /* TODO: !!!!!
        virtual void log(Log& l) override {
            l.header(className());
            l << "m_size: " << m_size << Log::endl;
            l << "m_capacity: " << m_capacity << Log::endl;
            l++;
            int64_t index = 0;
            for (auto string : *this) {
                if (index > 10) {
                    l << "..." << Log::endl;
                    break;
                }
                l << index << ": " << string << Log::endl;
                index++;
            }

            if (index < m_size) {
                l << (m_size - 1) << ": " << last() << Log::endl;
            }

            l--;
            l--;
        }
         */

        void clear() noexcept override {
            for (auto string : *this) {
                GRAIN_RELEASE(string);
            }
            List::clear();
        }

        bool resize(int64_t new_size, const String& string) noexcept {
            return resize(new_size, (String*)&string);
        }

        bool resize(int64_t new_size, String* string) noexcept override {
            if (new_size > size()) {
                int64_t new_n = new_size - size();
                if (!reserve(new_size)) {
                    return false;
                }
                for (int64_t i = 0; i < new_n; i++) {
                    pushString(string);
                }
            }
            else if (new_size < size()) {
                m_size = new_size;
            }
            return true;
        }

        ErrorCode removeAtIndex(int64_t index) noexcept override {
            if (!hasIndex(index)) {
                return ErrorCode::IndexOutOfRange;
            }
            else {
                auto string_to_remove = elementAtIndex(index);
                GRAIN_RELEASE(string_to_remove);

                m_size--;

                // Reorganize data if necessary
                if (index < m_size) {
                    for (int64_t i = index; i < m_size; i++) {
                        m_data[i] = m_data[i + 1];
                    }
                }

                return ErrorCode::None;
            }
    }

        /**
         *  @brief Inserts a String at a specific index in the list.
         *
         *  @param string The string to be inserted.
         *  @param index The index at which to insert the String.
         *  @return An `ErrorCode` representing the result of the insertion.
         */
        ErrorCode insertAtIndex(int64_t index, const String& string) noexcept {
            if (index < 0 || index > m_size) {
                return ErrorCode::BadArgs;
            }
            if (m_size >= m_capacity) {
                auto flag = reserve(m_capacity + m_grow_step);
                if (!flag) {
                    return ErrorCode::MemCantGrow;
                }
            }

            auto string_to_insert = new (std::nothrow) String(string);
            if (!string_to_insert) {
                return ErrorCode::MemCantAllocate;
            }

            auto data = mutDataPtr();

            // Move all references behind index position to the right
            for (int64_t i = m_size; i > index; i--) {
                data[i] = data[i - 1];
            }

            // Insert the new reference
            data[index] = string_to_insert;
            m_size++;

            return ErrorCode::None;
        }

        void sortAsc() noexcept {
            if (size() > 1) {
                qsort(mutDataPtr(), size(), elementSize(), _sortAsc);
            }
        }

        void sortDesc() noexcept {
            if (size() > 1) {
                qsort(mutDataPtr(), size(), elementSize(), _sortDesc);
            }
        }

        [[nodiscard]] const String& stringAtIndex(int64_t index) const noexcept {
            auto string_ptr = elementAtIndex(index);
            if (string_ptr) {
                return *string_ptr;
            }
            else {
                return String::emptyString();
            }
        }

        [[nodiscard]] const String& last() const noexcept {
            return size() > 0 ? stringAtIndex(lastIndex()) : String::emptyString();
        }

        [[nodiscard]] bool contains(const String& string) const noexcept;
        [[nodiscard]] int32_t countOccurrences(const String& string) const noexcept;

        bool pushStr(const char* str) noexcept;
        bool pushString(const String& string) noexcept;
        bool pushString(const String* string) noexcept;
        bool pushUnique(const String& string) noexcept;

        bool removeStr(const char* str) noexcept;
        bool removeString(const String& string) noexcept;

        [[nodiscard]] static int32_t _sortAsc(const void* a, const void* b);
        [[nodiscard]] static int32_t _sortDesc(const void* a, const void* b);
    };


} // End of namespace Grain

#endif // GrainStringList_hpp
