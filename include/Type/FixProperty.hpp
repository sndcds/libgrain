//
//  FixProperty.hpp
//
//  Created by Roald Christesen on 22.09.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 14.08.2025
//

#ifndef GrainFixProperty_hpp
#define GrainFixProperty_hpp

#include "Object.hpp"
#include "Fix.hpp"
#include "List.hpp"


namespace Grain {

    /**
     *  @struct FixProperty
     *  @brief Represents a property with a name, minimum, maximum, default and
     *         current value.
     */
    struct FixProperty {
        static constexpr int32_t kMaxNameLength = 31;
        static constexpr int32_t kMaxNameBufferSize = kMaxNameLength + 1;

        char m_name[kMaxNameBufferSize];    ///< Property name (up to 31 characters, NULL-terminated)
        int64_t m_min;      ///< Minimum acceptable value for the property
        int64_t m_max;      ///< Maximum acceptable value for the property
        int64_t m_default;  ///< Default value of the property (used when resetting)
        int64_t m_value;    ///< Current value of the property


        void set(const char* name, const Fix& min, const Fix& max, const Fix& default_value, const Fix& value) noexcept {
            strncpy(m_name, name, kMaxNameLength);
            m_name[kMaxNameLength] = '\0';
            m_min = min.raw();
            m_max = max.raw();
            m_default = default_value.raw();
            m_value = value.raw();
        }

        bool setValue(int64_t value) {
            if (value < m_min) {
                value = m_min;
            }
            else if (value > m_max) {
                value = m_max;
            }
            if (value != m_value) {
                m_value = value;
                return true;
            }
            return false;
        }

        bool resetValue() {
            bool changed = m_value != m_default;
            m_value = m_default;
            return changed;
        }
    };


    /**
     *  @class FixProperties
     *  @brief A container class that manages a list of Fix properties with
     *         validation against min/max ranges.
     *
     *  Provides access, modification, and iteration over a list of Fix properties.
     */
    class FixPropertyList : protected List<FixProperty> {
    public:
        FixPropertyList(int32_t capacity = 8);
        virtual ~FixPropertyList();

        const char* className() const noexcept override { return "FixProperties"; }

        friend std::ostream& operator << (std::ostream& os, const FixPropertyList& o) {
            os << o.className() << ": " << o.size();
            return os;
        }

        void addProperty(const char* name, const Fix& min, const Fix& max, const Fix& default_value, const Fix& value) noexcept;
        void removePropertyAtIndex(int32_t index) noexcept;

        [[nodiscard]] bool isIndex(int32_t index) const noexcept;
        bool propertyAtIndex(int32_t index, FixProperty& out_property) const noexcept;
        [[nodiscard]] FixProperty* mutPropertyAtIndex(int32_t index) noexcept;
        [[nodiscard]] FixProperty* mutPropertyPtrByName(const char* name) noexcept;


        [[nodiscard]] Fix valueAtIndex(int32_t index) const noexcept;
        bool setValueAtIndex(int32_t index, const Fix& value) noexcept;
        bool resetValueAtIndex(int32_t index) noexcept;

        bool setValueByName(const char* name, const Fix& value) noexcept;
        bool resetValueByName(const char* name) noexcept;
    };


} // End of namespace Grain

#endif // GrainFixProperty_hpp
