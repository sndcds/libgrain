//
//  ValueComponent.hpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/ValueComponent.hpp"
#include "Color/Gradient.hpp"


namespace Grain {

    ValueComponent::ValueComponent(const Rectd& rect, int32_t tag) noexcept : Component(rect, tag) {
        m_value = 0;
        m_min = 0;
        m_max = 100;
        m_offset = 0;
        m_default = 0;
        m_step = 1;
        m_big_step = 10;
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

        m_value = default_value;
        m_min = min;
        m_max = max;
        m_offset = offset;
        m_default = default_value;
        m_step = step;
        m_big_step = big_step;

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
        m_fractional_digits = std::clamp<int32_t>(fractional_digits, 0, Fix::kDecPrecision);
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
            if (m_value.set(component->value(), m_min, m_max, m_fractional_digits)) {
                updateRepresentations(component);
                needsDisplay();
            }
            fireAction(Component::ActionType::None, component);
        }
    }

}  // End of namespace Grain
