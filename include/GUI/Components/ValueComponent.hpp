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
#include "GUI/Components/Component.hpp"


namespace Grain {

    class ColorWell;

    class ValueComponent : public Component {

    public:
        explicit ValueComponent(const Rectd& rect) noexcept : ValueComponent(rect, 0) {}
        ValueComponent(const Rectd& rect, int32_t tag) noexcept;
        ~ValueComponent() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "ValueComponent"; }


        [[nodiscard]] Fix value() const noexcept override { return value_; }

        bool setValue(const Fix& value) noexcept override {
            if (value_.set(value, min_, max_, fractional_digits_)) {
                updateRepresentations(nullptr);
                needsDisplay();
                return true;
            }
            return false;
        }
        virtual void incValue() noexcept { setValue(value_ + step_); }
        virtual void decValue() noexcept { setValue(value_ - step_); }
        virtual void incValueBig() noexcept { setValue(value_ + big_step_); }
        virtual void decValueBig() noexcept { setValue(value_ - big_step_); }


        virtual void setRange(Fix min, Fix max) noexcept {
            min_ = min;
            max_ = max;
        }

        virtual void setRange(int32_t min, int32_t max) noexcept {
            min_ = Fix(min);
            max_ = Fix(max);
        }


        void setDisplayPrecision(int32_t value) noexcept { display_precisio_n = value; needsDisplay(); }


        [[nodiscard]] int32_t valueAsInt32() const noexcept override { return value_.asInt32(); }
        [[nodiscard]]  double valueAsDouble() const noexcept override { return value_.asDouble(); }


        [[nodiscard]] Fix minValue() const noexcept { return min_; }
        [[nodiscard]] Fix maxValue() const noexcept { return max_; }
        [[nodiscard]] Fix offsetValue() const noexcept { return offs_; }
        [[nodiscard]] Fix defaultValue() const noexcept { return default_; }
        [[nodiscard]] Fix valueStep() const noexcept { return step_; }
        [[nodiscard]] Fix valueBigStep() const noexcept { return big_step_; }
        [[nodiscard]] int32_t fractionalDigits() const noexcept { return fractional_digits_; }
        [[nodiscard]] int32_t displayPrecision() const noexcept { return display_precisio_n; }

        [[nodiscard]] Fix rememberedValue() const noexcept { return remembered_value_; }

        [[nodiscard]] double normalizedValue() const noexcept {
            double value = value_.asDouble();
            double min = min_.asDouble();
            double max = max_.asDouble();
            return ((max - min) == 0.0) ? 0.0 : (value - min) / (max - min);
        }

        [[nodiscard]] double normalizedOffsetValue() const noexcept {
            double offset = offs_.asDouble();
            double min = min_.asDouble();
            double max = max_.asDouble();
            return ((max - min) == 0.0) ? 0.0 : (offset - min) / (max - min);
        }

        [[nodiscard]] bool shouldDisplayValue() const noexcept { return displays_value_; }
        [[nodiscard]] bool isIndicatorVisible() const noexcept { return indicator_visibility_; }
        [[nodiscard]] float trackSize() const noexcept { return track_size_; }
        [[nodiscard]] float handleSize() const noexcept { return handle_size_; }


        void setTrackSize(float track_size) noexcept { track_size_ = track_size; needsDisplay(); }
        void setHandleSize(float handle_size) noexcept { handle_size_ = handle_size; needsDisplay(); }
        void enableValueDisplay() noexcept { setDisplaysValue(true); }
        void disableValueDisplay() noexcept { setDisplaysValue(false); }
        void setDisplaysValue(bool value) noexcept { displays_value_ = value; needsDisplay(); }

        void setup(Fix min, Fix max, Fix offset, Fix default_value, Fix step, Fix big_step) noexcept;
        void setupInt(int32_t min, int32_t max, int32_t offset, int32_t def, int32_t step, int32_t big_step) noexcept;
        void setupReal(double min, double max, double offset, double def, double step, double big_step) noexcept;
        void setFractionalDigits(int32_t fractional_digits) noexcept;

        // void setTextField(TextField* textfield) noexcept override; !!!!!
        void setByComponent(Component* component) noexcept override;

        void setIndicatorVisibility(bool indicator_visibility) noexcept { indicator_visibility_ = indicator_visibility; needsDisplay(); }
        void hideIndicator() noexcept { setIndicatorVisibility(false); }
        void showIndicator() noexcept { setIndicatorVisibility(true); }

        virtual void setColorWell(ColorWell* color_well) noexcept {}


        virtual void removeGradient() noexcept {}

        void handleKeyDown(const Event& event) noexcept override;

        void handleMouseDown(const Event& event) noexcept override {
            remembered_value_ = value_;
        }

    protected:
        Fix value_;
        Fix min_;
        Fix max_;
        Fix offs_;
        Fix default_;
        Fix step_;
        Fix big_step_;
        Fix remembered_value_;

        int32_t fractional_digits_ = 2;
        int32_t display_precisio_n = 2;
        bool displays_value_ = false;
        bool indicator_visibility_ = true;
        float track_size_ = 4.0;
        float handle_size_ = 10.0;

        ColorWell* color_well_{};
        Gradient* gradient_{};
    };


} // End of namespace Grain

#endif // GrainValueComponent_hpp
