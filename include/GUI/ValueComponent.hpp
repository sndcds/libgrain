//
//  ValueComponent.hpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 26.07.2025
//

#ifndef GrainValueComponent_hpp
#define GrainValueComponent_hpp

#include "Grain.hpp"
#include "GUI/Component.hpp"


namespace Grain {

    class ValueComponent : public Component {

    public:
        ValueComponent(const Rectd& rect) noexcept : ValueComponent(rect, 0) {}
        ValueComponent(const Rectd& rect, int32_t tag) noexcept;
        virtual ~ValueComponent() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "ValueComponent"; }


        [[nodiscard]] Fix value() const noexcept override { return m_value; }

        bool setValue(const Fix& value) noexcept override {
            if (m_value.set(value, m_min, m_max, m_fractional_digits)) {
                updateRepresentations(nullptr);
                needsDisplay();
                return true;
            }
            return false;
        }

        virtual void setRange(Fix min, Fix max) noexcept {
            m_min = min;
            m_max = max;
        }

        virtual void setRange(int32_t min, int32_t max) noexcept {
            m_min = Fix(min);
            m_max = Fix(max);
        }


        void setDisplayPrecision(int32_t value) noexcept { m_display_precision = value; needsDisplay(); }


        [[nodiscard]] int32_t valueAsInt32() const noexcept override { return m_value.asInt32(); }
        [[nodiscard]]  double valueAsDouble() const noexcept override { return m_value.asDouble(); }


        [[nodiscard]] Fix minValue() const noexcept { return m_min; }
        [[nodiscard]] Fix maxValue() const noexcept { return m_max; }
        [[nodiscard]] Fix offsetValue() const noexcept { return m_offset; }
        [[nodiscard]] Fix defaultValue() const noexcept { return m_default; }
        [[nodiscard]] Fix valueStep() const noexcept { return m_step; }
        [[nodiscard]] Fix valueBigStep() const noexcept { return m_big_step; }
        [[nodiscard]] int32_t fractionalDigits() const noexcept { return m_fractional_digits; }
        [[nodiscard]] int32_t displayPrecision() const noexcept { return m_display_precision; }

        [[nodiscard]] Fix rememberedValue() const noexcept { return m_remembered_value; }

        [[nodiscard]] double normalizedValue() const noexcept {
            double value = m_value.asDouble();
            double min = m_min.asDouble();
            double max = m_max.asDouble();
            return ((max - min) == 0.0) ? 0.0 : (value - min) / (max - min);
        }

        [[nodiscard]] double normalizedOffsetValue() const noexcept {
            double offset = m_offset.asDouble();
            double min = m_min.asDouble();
            double max = m_max.asDouble();
            return ((max - min) == 0.0) ? 0.0 : (offset - min) / (max - min);
        }

        [[nodiscard]] bool shouldDisplayValue() const noexcept { return m_displays_value; }
        [[nodiscard]] bool isIndicatorVisible() const noexcept { return m_indicator_visibility; }
        [[nodiscard]] float trackSize() const noexcept { return m_track_size; }
        [[nodiscard]] float handleSize() const noexcept { return m_handle_size; }


        void setTrackSize(float track_size) noexcept { m_track_size = track_size; needsDisplay(); }
        void setHandleSize(float handle_size) noexcept { m_handle_size = handle_size; needsDisplay(); }
        void enableValueDisplay() noexcept { setDisplaysValue(true); }
        void disableValueDisplay() noexcept { setDisplaysValue(false); }
        void setDisplaysValue(bool value) noexcept { m_displays_value = value; needsDisplay(); }

        void setup(Fix min, Fix max, Fix offset, Fix default_value, Fix step, Fix big_step) noexcept;
        void setupInt(int32_t min, int32_t max, int32_t offset, int32_t def, int32_t step, int32_t big_step) noexcept;
        void setupReal(double min, double max, double offset, double def, double step, double big_step) noexcept;
        void setFractionalDigits(int32_t fractional_digits) noexcept;

        // void setTextfield(Textfield* textfield) noexcept override; !!!!!
        void setByComponent(Component* component) noexcept override;

        void setIndicatorVisibility(bool indicator_visibility) noexcept { m_indicator_visibility = indicator_visibility; needsDisplay(); }
        void hideIndicator() noexcept { setIndicatorVisibility(false); }
        void showIndicator() noexcept { setIndicatorVisibility(true); }

        void removeGradient() noexcept;


        void handleMouseDown(const Event& event) noexcept override {
            m_remembered_value = m_value;
        }

    protected:
        Fix m_value;
        Fix m_min;
        Fix m_max;
        Fix m_offset;
        Fix m_default;
        Fix m_step;
        Fix m_big_step;

        int32_t m_fractional_digits = 2;
        int32_t m_display_precision = 2;
        bool m_displays_value = false;
        bool m_indicator_visibility = true;
        float m_track_size = 4.0;
        float m_handle_size = 8.0;

        Fix m_remembered_value;
    };


} // End of namespace Grain

#endif // GrainValueComponent_hpp
