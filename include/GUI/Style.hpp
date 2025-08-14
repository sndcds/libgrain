//
//  Style.hpp
//
//  Created by Roald Christesen on from 28.07.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 28.07.2025
//

#ifndef GrainComponentStyle_hpp
#define GrainComponentStyle_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Color/RGBA.hpp"
#include "2d/RectEdges.hpp"


namespace Grain {

    class Font;

    enum class StylePropertyType {
        Undefined = -1,
        Color,
        ColorHighlighted,
        ColorSelected,
        BackgroundColor,
        BackgroundColorHighlighted,
        BackgroundColorSelected,
        BorderColor,
        BorderColorHighlighted,
        BorderColorSelected,
        BorderWidth,
        BorderWidthHighlighted,
        BorderWidthSelected,

        Count,
        First = 0
    };

    class StyleProperty {
        friend class StyleClass;

        using Value = std::variant<
                int32_t,
                uint32_t,
                int64_t,
                Fix,
                float,
                double,
                RGB,
                RGBA,
                Font*
        >;

    public:
        StyleProperty() = default;

        // Copy constructor and assignment operator are auto-generated safely
        StyleProperty(const StyleProperty&) = default;
        StyleProperty& operator=(const StyleProperty&) = default;

        // Optional: a constructor to initialize value and type
        StyleProperty(StylePropertyType type, Value value)
                : m_active(true), m_type(type), m_value(std::move(value)) {}

        [[nodiscard]] StylePropertyType type() const { return m_type; }
        [[nodiscard]] const Value& value() const { return m_value; }
        void setValue(Value&& value) { m_value = std::move(value); }
        void setValue(const Value& value) { m_value = value; }

        template<typename T>
        [[nodiscard]] T valueOrDefault() const {
            if (const T* ptr = std::get_if<T>(&m_value)) {
                return *ptr;
            } else {
                return T{}; // Requires T to be default-constructible
            }
        }

    protected:
        bool m_active = false;
        StylePropertyType m_type = StylePropertyType::Undefined;
        Value m_value;
    };


    class StyleList : public Object {
    public:
        void addProperty(const StyleProperty& property) {
            m_properties.push(property);
        }
        template<typename T>
        void addProperty(StylePropertyType type, T&& value) {
            StyleProperty property(type, std::forward<T>(value));
            addProperty(property);
        }

        [[nodiscard]] const List<StyleProperty>* properties() { return &m_properties; }

    protected:
        List<StyleProperty> m_properties;
    };


    class StyleClass {
    public:
        enum class PropertyType {
            Undefined = -1,
            Color,
            ColorHighlighted,
            ColorSelected,
            BackgroundColor,
            BackgroundColorHighlighted,
            BackgroundColorSelected,
            BorderColor,
            BorderColorHighlighted,
            BorderColorSelected,
            BorderWidth,
            BorderWidthHighlighted,
            BorderWidthSelected,

            Count,
            First = 0
        };

        static constexpr int32_t kPropertyCount = static_cast<int32_t>(PropertyType::Count);

    public:
        StyleClass();

        void build(const StyleClass* parent_style_class, StyleList* style_list) noexcept;
        StyleProperty* properties() noexcept { return m_properties; };
        StyleProperty* propertyAtIndex(int32_t index) noexcept {
            return index >= 0 && index < kPropertyCount ? &m_properties[index] : nullptr;
        }
        StyleProperty* propertyAtType(StylePropertyType type) noexcept {
            auto index = typeIndex(type);
            return index >= 0 ? &m_properties[index] : nullptr;
        }
        template<typename T>
        void setPropertyAtType(StylePropertyType type, T&& value) noexcept {
            auto index = typeIndex(type);
            if (index >= 0) {
                m_properties[index].setValue(value);
            }
        }
        [[nodiscard]] static inline int32_t typeIndex(StylePropertyType type) noexcept {
            auto index = static_cast<int32_t>(type);
            return index >= 0 && index < kPropertyCount ? index : -1;
        }

    public:
        Flags m_state_flags{};
        StyleProperty m_properties[static_cast<int>(kPropertyCount)];
    };


    class StyleSet {
    public:
        void build(const StyleSet* parent_style_set) noexcept;
        StyleClass* viewStyleClass() noexcept { return &m_view_style_class; }
        StyleClass* buttonStyleClass() noexcept { return &m_button_style_class; }

    protected:
        StyleClass m_view_style_class;
        StyleClass m_button_style_class;
    };


} // End of namespace Grain

#endif // GrainComponentStyle_hpp