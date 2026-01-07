//
//  ColorWheel.hpp
//
//  Created by Roald Christesen on 11.12.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainColorWheel_hpp
#define GrainColorWheel_hpp

#include "GUI/Components/ValueComponent.hpp"
#include "Math/Vec3Fix.hpp"
#include "Color/RGB.hpp"
#include "Color/HSV.hpp"


namespace Grain {
    class ColorWell;
    class Slider;
    class TextField;
    class GraphicContext;


    class ColorWheel : public ValueComponent {
    public:
        enum {
            kMouseModeNone,
            kMouseModeValue,
            kMouseModeHue,
            kMouseModeSaturation
        };

        enum {
            kControlTagUndefined,
            kControlTagHueTextField,
            kControlTagSaturationTextField,
            kControlTagValueTextField,
            kControlTagHueSlider,
            kControlTagSaturationSlider,
            kControlTagValueSlider
        };

    protected:
        double track_size_ = 6;
        double indicator_size_ = 10;
        double spacer_size_ = 6;
        double hue_size_ = 12;
        double inner_spacing_ = 2;
        double value_slider_offset_angle_ = 20;
        double saturation_precision_ = 6.0;

        Slider* hue_slider_ = nullptr;
        Slider* saturation_slider_ = nullptr;
        Slider* value_slider_ = nullptr;
        TextField* hue_textfield_ = nullptr;
        TextField* saturation_textfield_ = nullptr;
        TextField* value_textfield_ = nullptr;
        ColorWell* color_well_ = nullptr;

        // Computed properties
        Vec2d center_;
        double value_outer_radius_;
        double value_inner_radius_;
        double hue_outer_radius_;
        double hue_inner_radius_;
        double inner_radius_;
        Rectd value_rect_;
        Rectd hue_rect_;
        Rectd inner_rect_;

        //
        Vec2d remembered_color_pos_;
        Vec3Fix mouse_down_color_;
        Vec3Fix current_color_;

    public:
        ColorWheel(const Rectd& rect) noexcept;

        virtual ~ColorWheel() noexcept;

        const char* className() const noexcept override { return "ColorWheel"; }

        static ColorWheel* add(View* view, const Rectd& rect);

        Vec2d center() const noexcept override { return center_; }

        RGB color() const noexcept override { return rgb(); }

        virtual void setColor(RGB& color) noexcept {
            HSV hsv(color);
            current_color_.set(hsv.hue(), hsv.saturation(), hsv.value());
            updateRepresentations(nullptr);
            needsDisplay();
        }

        RGB rgb() const noexcept {
            RGB rgb = { current_color_.xFloat(), current_color_.yFloat(), current_color_.zFloat() };
            return rgb;
        }

        HSV hsv() const noexcept {
            HSV hsv = rgb();
            return hsv;
        }

        double colorHue() const noexcept { return current_color_.xDouble(); }
        double colorSaturation() const noexcept { return current_color_.yDouble(); }
        double colorValue() const noexcept { return current_color_.zDouble(); }

        Vec2d hueVectorNorm() const noexcept {
            Vec2d v(0.0, -1.0);
            v.rotate(colorHue() * 360);
            return v;
        }

        Vec2d hueVector() const noexcept {
            Vec2d v(0, -colorSaturation());
            v.rotate(colorHue() * 360);
            return v;
        }

        void setColorHueByAngle(double angle) noexcept {
            current_color_.x_ = Grain::Type::wrappedValue<double>(angle / 360.0f, 0, 1);
        }

        void setColorHue(double hue) noexcept { current_color_.x_ = std::clamp<double>(hue, 0, 1); }
        void setColorSaturation(double saturation) noexcept { current_color_.y_ = std::clamp<double>(saturation, 0, 1); }
        void setColorValue(double value) noexcept { current_color_.z_ = std::clamp<double>(value, 0, 1); }

        Vec2d colorPos() const noexcept;

        double hueAngle(const Vec2d& pos) const noexcept;

        double valueAngle(const Vec2d& pos) const noexcept;

        double distance(const Vec2d& pos) const noexcept;

        double valueStartAngle() const noexcept { return 90.0f + value_slider_offset_angle_; }
        double valueAngleSpan() const noexcept { return 360.0f - value_slider_offset_angle_ * 2; }
        double valueTrackSize() const noexcept { return track_size_; }
        double indicatorSize() const noexcept { return indicator_size_; }
        double trackOuterRadius() const noexcept { return value_outer_radius_; }
        double trackInnerRadius() const noexcept { return value_inner_radius_; }
        double hueOuterRadius() const noexcept { return hue_outer_radius_; }
        double hueInnerRadius() const noexcept { return hue_inner_radius_; }
        double innerRadius() const noexcept { return inner_radius_; }

        bool setEnabled(bool enabled) noexcept override;

        void setTrackSize(double track_size) noexcept {
            track_size_ = track_size;
            needsDisplay();
        }

        void setIndicatorSize(double indicator_size) noexcept {
            indicator_size_ = indicator_size;
            needsDisplay();
        }

        void setSpacerSize(double spacer_size) noexcept {
            spacer_size_ = spacer_size;
            needsDisplay();
        }

        void setHueSize(double hue_size) noexcept {
            hue_size_ = hue_size;
            needsDisplay();
        }

        void setInnerSpace(double inner_space) noexcept {
            inner_spacing_ = inner_space;
            needsDisplay();
        }

        void setColorPos(const Vec2d& pos) noexcept;

        void setHueTextField(TextField* textfield) noexcept;

        void setSaturationTextField(TextField* textfield) noexcept;

        void setValueTextField(TextField* textfield) noexcept;

        void setHueSlider(Slider* slider) noexcept;

        void setSaturationSlider(Slider* slider) noexcept;

        void setValueSlider(Slider* slider) noexcept;

        void setColorWell(ColorWell* color_well) noexcept override;

        void setByComponent(Component* component) noexcept override;

        void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept override;
        void drawColorWheel(GraphicContext* gc, GUIStyle* style) const noexcept;

        virtual void drawHueRing(GraphicContext* gc, const Rectd& dirty_rect) noexcept;

        virtual void drawCrossLine(GraphicContext* gc, const Rectd& dirty_rect) noexcept;

        void handleMouseDown(const Event& event) noexcept override;

        void handleMouseDrag(const Event& event) noexcept override;

        void handleMouseUp(const Event& event) noexcept override;

        void handleRightMouseDown(const Event& event) noexcept override {
        }

        void handleRightMouseDrag(const Event& event) noexcept override {
        }

        void handleRightMouseUp(const Event& event) noexcept override {
        }

        void handleScrollWheel(const Event& event) noexcept override;

        void handleMagnification(const Event& event) noexcept override;

        void handleRotation(const Event& event) noexcept override;

        bool _mousePointerAction(const Event& event) noexcept;

        void _checkModified() noexcept;

        void _updateDimensions() noexcept;

        void updateRepresentations(const Component* excluded_component) noexcept override;
    };
} // End of namespace

#endif // GrainColorWheel_hpp