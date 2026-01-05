//
//  ValueComponent.hpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/ValueComponent.hpp"
#include "GUI/Event.hpp"
#include "Color/Gradient.hpp"


namespace Grain {

    ValueComponent::ValueComponent(const Rectd& rect, int32_t tag) noexcept : Component(rect, tag) {
        value_ = 0;
        min_ = 0;
        max_ = 100;
        offs_ = 0;
        default_ = 0;
        step_ = 1;
        big_step_ = 10;
    }


    ValueComponent::~ValueComponent() noexcept {
    }


    void ValueComponent::setup(Fix min, Fix max, Fix offset, Fix default_value, Fix step, Fix big_step) noexcept {
        if (min > max) {
            Fix temp = min;
            min = max;
            max = temp;
        }

        default_value.clamp(min, max);
        offset.clamp(min, max);

        value_ = default_value;
        min_ = min;
        max_ = max;
        offs_ = offset;
        default_ = default_value;
        step_ = step;
        big_step_ = big_step;

        /* !!!!!
        if (m_textfield) {
            m_textfield->setVariable(&m_value, Grain::Type::Class::Fix);
            m_textfield->setValueRange(min, max);
            m_textfield->setStep(step, big_step);
            m_textfield->setValue(m_value);
        }
         */

        needsDisplay();
    }


    void ValueComponent::setupInt(int32_t min, int32_t max, int32_t offset, int32_t def, int32_t step, int32_t big_step) noexcept {
        setup(Fix(min), Fix(max), Fix(offset), Fix(def), Fix(step), Fix(big_step));
    }


    void ValueComponent::setupReal(double min, double max, double offset, double def, double step, double big_step) noexcept {
        setup(Fix(min), Fix(max), Fix(offset), Fix(def), Fix(step), Fix(big_step));
    }


    void ValueComponent::setFractionalDigits(int32_t fractional_digits) noexcept {
        fractional_digits_ = std::clamp<int32_t>(fractional_digits, 0, Fix::kDecPrecision);
    }

    /* !!!!!
    void ValueComponent::setTextField(TextField* textfield) noexcept {

        if (textfield != m_textfield) {
            m_textfield = textfield;

            if (m_textfield) {
                m_textfield->setVariable(&m_value, Grain::Type::Class::Fix);
                m_textfield->setValueRange(m_min, m_max);
                m_textfield->setStep(m_step, m_big_step);
                m_textfield->setValue(m_value);
                m_textfield->setFractionalDigits(m_fractional_digits);
                m_textfield->setTextAlignment(Alignment::Right);
                m_textfield->setReceiverComponent(this);
            }
        }
    }
    */

    void ValueComponent::setByComponent(Component* component) noexcept {
        if (component) {
            if (value_.set(component->value(), min_, max_, fractional_digits_)) {
                updateRepresentations(component);
                needsDisplay();
            }
            fireAction(Component::ActionType::None, component);
        }
    }


    void ValueComponent::handleKeyDown(const Event& event) noexcept {
        if (event.keyCharCount() == 1) {
            switch (event.keyChar()) {
                case KeyCode::FunctionDownArrow:
                case KeyCode::FunctionLeftArrow:
                    if (event.noModifiersPressed()) { decValue(); }
                    else if (event.isShiftPressedOnly()) { decValueBig(); }
                    break;
                case KeyCode::FunctionUpArrow:
                case KeyCode::FunctionRightArrow:
                    if (event.noModifiersPressed()) { incValue(); }
                    else if (event.isShiftPressedOnly()) { incValueBig(); }
                    break;
                case KeyCode::CarriageReturn:
                    if (event.isAltPressedOnly()) { setValue(default_); }
                    break;
                default:
                    break;
            }
        }
    }

} // End of namespace Grain
