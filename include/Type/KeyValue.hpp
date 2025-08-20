//
//  KeyValue.hpp
//
//  Created by Roald Christesen on 06.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 03.06.2025
//

#ifndef GrainKeyValue_hpp
#define GrainKeyValue_hpp

#include <limits>
#include <cstring>


namespace Grain {

    /**
     *  @struct KeyValuePair
     *  @brief A templated key-value pair structure for storing string keys and associated values.
     *
     *  This struct is designed to store values associated with keys. The special value
     *  `kUnknownValue` indicates that there is no value associated with the key when performing lookups.
     *
     *  @tparam V The type of the value associated with the key.
     */
    template <typename V>
    struct KeyValuePair {
        static constexpr V kUnknownValue = std::numeric_limits<V>::min();

        const char* m_key;  ///< Key string, a name for the value
        V m_value;          ///< Value for the key

        /**
         *  @brief Finds the value associated with a given key.
         *
         *  @param key The key string to search for.
         *  @param items Pointer to an array of `KeyValuePair<V>`, terminated with a `nullptr` key.
         *  @return The corresponding value if found, otherwise `kUnknownValue`.
         */
        static V lookupValue(const char* key, const KeyValuePair<V>* items) noexcept {
            if (items != nullptr && key != nullptr) {
                const KeyValuePair<V>* item = items;
                while (item->m_key != nullptr) {
                    // Check for sentinel value indicating the end of the array
                    if (std::strcmp(key, item->m_key) == 0) {
                        return item->m_value;
                    }
                    ++item;
                }
            }
            return kUnknownValue;
        }

        /**
         *  @brief Finds the value associated with a given key (case-insensitive).
         *
         *  @param key The key string to search for.
         *  @param items Pointer to an array of `KeyValuePair<V>`, terminated with a `nullptr` key.
         *  @return The corresponding value if found, otherwise `kUnknownValue`.
         */
        static V lookupValueIgnoreCase(const char* key, const KeyValuePair<V>* items) noexcept {
            if (items != nullptr && key != nullptr) {
                const KeyValuePair<V>* item = items;
                while (item->m_key != nullptr) {
                    // Check for sentinel value indicating the end of the array
                    if (strcasecmp(key, item->m_key) == 0) {
                        return item->m_value;
                    }
                    ++item;
                }
            }
            return kUnknownValue;
        }

        /**
         *  @brief Finds the key associated with a given value.
         *
         *  @param value The value to search for.
         *  @param items Pointer to an array of `KeyValuePair<V>`, terminated with a `nullptr` key.
         *  @param unknown Optional parameter specifying a default string if the key is not found.
         *  @return The corresponding key string if found, otherwise `unknown` or an empty string.
         */
        static const char* lookupKey(V value, const KeyValuePair<V>* items, const char* unknown = nullptr) noexcept {
            static const char* default_unknown = "";
            if (items != nullptr) {
                const KeyValuePair<V>* item = items;
                while (item->m_key != nullptr) {
                    // Check for sentinel value indicating the end of the array
                    if (item->m_value == value) {
                        return item->m_key;
                    }
                    ++item;
                }
            }
            return unknown != nullptr ? unknown : default_unknown;
        }
    };


    // Standard types
    using KeyBytePair = KeyValuePair<int8_t>;
    using KeyIntPair = KeyValuePair<int32_t>;
    using KeyLongPair = KeyValuePair<int64_t>;
    using KeyFloatPair = KeyValuePair<float>;
    using KeyDoublePair = KeyValuePair<double>;


}  // End of namespace Grain

#endif // GrainKeyValue_hpp
