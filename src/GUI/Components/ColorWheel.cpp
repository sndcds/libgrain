//
//  ColorWheel.cpp
//
//  Created by Roald Christesen on 11.12.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "GUI/Components/ColorWheel.hpp"
#include "GUI/Components/ColorWell.hpp"
#include "GUI/Components/Slider.hpp"
#include "GUI/Components/TextField.hpp"
#include "GUI/GUIStyle.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Math/Vec3Fix.hpp"


namespace Grain {

    ColorWheel::ColorWheel(const Rectd& rect) noexcept : ValueComponent(rect) {
        type_ = ComponentType::ColorWheel;

        current_color_.zero();
        mouse_mode_ = ColorWheel::kMouseModeNone;
        // setMouseCursor(App::MouseCursor::Crosshair); // TODO: How?

        _updateDimensions();
    }


    ColorWheel::~ColorWheel() noexcept {
    }


    ColorWheel* ColorWheel::add(View* view, const Rectd& rect) {
        return (ColorWheel*)Component::addComponentToView((Component*)new(std::nothrow) ColorWheel(rect), view);
    }


    Vec2d ColorWheel::colorPos() const noexcept {
        Vec2d pos(0, -current_color_.yDouble() * (inner_radius_ - inner_spacing_));
        pos.rotate(current_color_.xDouble() * 360);
        pos += center_;
        return pos;
    }


    double ColorWheel::hueAngle(const Vec2d& pos) const noexcept {
        return atan2(pos.x_ - center_.x_, center_.y_ - pos.y_) * 180.0 / M_PI + 180.0;
    }


    double ColorWheel::valueAngle(const Vec2d& pos) const noexcept {
        return atan2(pos.x_ - center_.x_, center_.y_ - pos.y_) * 180.0 / M_PI + 180.0;
    }


    double ColorWheel::distance(const Vec2d& pos) const noexcept {
        Vec2d v = pos - center_;
        return std::sqrt(v.x_ * v.x_ + v.y_ * v.y_) / (inner_radius_ - inner_spacing_);
    }


    bool ColorWheel::setEnabled(bool enabled) noexcept {
        bool result = Component::setEnabled(enabled);
        if (result == true) {
            if (color_well_) {
                color_well_->setEnabled(enabled);
            }
            if (hue_textfield_) {
                hue_textfield_->setEnabled(enabled);
            }
            if (saturation_textfield_) {
                saturation_textfield_->setEnabled(enabled);
            }
            if (value_textfield_)
                value_textfield_->setEnabled(enabled);

            if (hue_slider_) {
                hue_slider_->setEnabled(enabled);
            }
            if (saturation_slider_) {
                saturation_slider_->setEnabled(enabled);
            }
            if (value_slider_) {
                value_slider_->setEnabled(enabled);
            }
        }

        if (label_) {
            // label->setEnabled(enabled);
        }

        return result;
    }


    void ColorWheel::setColorPos(const Vec2d& pos) noexcept {
        /*
         double angle = hueAngle(pos) - 180;
         double h = angle / 360;
        if (h < 0) {
            h += 1;
        }
        else if (h > 1) {
            h -= 1;
        }
        color_.x_ = h;
        */

        double s = distance(pos);
        if (s > 1) {
            s = 1;
        }
        current_color_.y_ = s;
    }


    void ColorWheel::setHueTextField(TextField* textfield) noexcept {
        hue_textfield_ = textfield;
        if (textfield) {
            textfield->setReceiverComponent(this);
            textfield->setValueRangeInt32(0, 360);
            textfield->setValue(current_color_.x_);
            textfield->setStepFlipMode(true);
            textfield->setTag(kControlTagHueTextField);
        }
    }


    void ColorWheel::setSaturationTextField(TextField* textfield) noexcept {
        saturation_textfield_ = textfield;
        if (textfield) {
            textfield->setReceiverComponent(this);
            textfield->setValueRangeInt32(0, 100);
            textfield->setValue(current_color_.y_);
            textfield->setTag(kControlTagSaturationTextField);
        }
    }


    void ColorWheel::setValueTextField(TextField* textfield) noexcept {
        value_textfield_ = textfield;
        if (textfield) {
            textfield->setReceiverComponent(this);
            textfield->setValueRangeInt32(0, 100);
            textfield->setValue(current_color_.z_);
            textfield->setTag(kControlTagValueTextField);
        }
    }


    void ColorWheel::setHueSlider(Slider* slider) noexcept {
        hue_slider_= slider;
        if (slider) {
            slider->setReceiverComponent(this);
            slider->setValue(current_color_.x_);
            slider->setTag(kControlTagHueSlider);
        }
    }


    void ColorWheel::setSaturationSlider(Slider* slider) noexcept {
        saturation_slider_ = slider;
        if (slider) {
            slider->setReceiverComponent(this);
            slider->setValue(current_color_.y_);
            slider->setTag(kControlTagSaturationSlider);
        }
    }


    void ColorWheel::setValueSlider(Slider* slider) noexcept {
        value_slider_ = slider;
        if (slider) {
            slider->setReceiverComponent(this);
            slider->setValue(current_color_.z_);
            slider->setTag(kControlTagValueSlider);
        }
    }


    void ColorWheel::setColorWell(ColorWell* color_well) noexcept {
        color_well_ = color_well;
        if (color_well) {
            HSV hsv = { current_color_.xFloat(), current_color_.yFloat(), current_color_.zFloat() };
            // current_color_.hsv(hsv); // TODO: !!!!
            color_well->setHSVColor(hsv);
            color_well->setReceiverComponent(this);
        }
    }


    void ColorWheel::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        _updateDimensions();

        drawColorWheel(gc, style);
        drawCrossLine(gc, dirty_rect);

        RGBA handle_color = style->backgroundColor();

        // Circle at hue on the ring
        if (is_enabled_) {
            Vec2d pos(0, -0.5f * (hue_outer_radius_ + hue_inner_radius_));
            pos.rotate(current_color_.xDouble() * 360);
            pos += center_;
            gc->setFillColor(1, 1, 1, 0.7f);
            gc->fillCircle(pos, 0.3f * (hue_outer_radius_ - hue_inner_radius_));
        }

        // Hue/saturation point
        if (mouse_mode_ == ColorWheel::kMouseModeSaturation) {
            // Draw old position during mouse drag
            gc->setFillRGBAndAlpha(handle_color, 0.25f);
            gc->fillCircle(remembered_color_pos_, 3);
        }

        Vec2d color_pos = colorPos();
        gc->setFillRGBAndAlpha(handle_color, is_enabled_ ? 1.0 : 0.5f);
        gc->fillCircle(color_pos, 0.5 * indicator_size_);
    }


    void ColorWheel::drawColorWheel(GraphicContext* gc, GUIStyle* style) const noexcept {
        bool enabled = isEnabled();
        Vec2d center = this->center();

        RGBA view_color = { 1, 1, 0, 1 };   // component->uiViewColor();
        RGBA bg_color = { 1, 0, 0, 1 };     // filled ? component->uiColor(UIColor::Bg) : view_color;
        RGBA track_color = { 1, 0, 1, 1 };  // component->uiTrackColor();
        RGBA indicator_color = { 0, 1, 1, 1 };  // component->uiIndicatorColor();
        RGBA handle_color = { 1, 0.2, 0.4, 1 }; // component->uiHandleColor();

        if (enabled == false) {
            // disableColorMixed(track_color, bg_color, view_color);
            // disableColorMixed(indicator_color, bg_color, view_color);
            // disableColorMixed(handle_color, bg_color, view_color);
            // disableColor(bg_color, view_color);
        }

        gc->drawCircleSlider(
            0, colorValue(), center,
            trackOuterRadius(), valueTrackSize(), indicatorSize(),
            valueStartAngle(), valueAngleSpan(),
            track_color, track_color, handle_color, isEnabled());
        if (!enabled) {
            gc->setAlpha(style->disabledAlpha());
        }

        gc->fillColorWheel(center, hueOuterRadius(), hueInnerRadius());
        gc->setAlpha(1);
    }

    void ColorWheel::drawHueRing(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        gc->fillColorWheel(center_, hue_inner_radius_, hue_outer_radius_);
    }


    void ColorWheel::drawCrossLine(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        gc->disableAliasing();

        // gc->setStrokeColor(uiColor(UIColor::Line + (!is_enabled_))); // TODO: !!!!

        double length = inner_radius_ * 2 - 4;

        Lined h_line(center_, length, 0);
        gc->strokeLine(h_line);

        Lined v_line(center_, length, 90);
        gc->strokeLine(v_line);

        gc->enableAliasing();
    }


    void ColorWheel::handleMouseDown(const Event& event) noexcept {
        mouse_down_color_ = current_color_;    // TODO: !!!!
        // mouse_down_color_ = current_color_ = current_color_;    // TODO: !!!!

        Vec2d v = event.mouseDownPos() - center_;
        double distance = v.length();

        mouse_mode_ = ColorWheel::kMouseModeNone;
        if (distance < inner_radius_) {
            mouse_mode_ = ColorWheel::kMouseModeSaturation;
        }
        else if (distance < hue_outer_radius_ + 2 && distance > hue_inner_radius_ - 2) {
            mouse_mode_ = ColorWheel::kMouseModeHue;
        }
        else if (distance < value_outer_radius_ + 2 && distance > value_inner_radius_ - 2) {
            mouse_mode_ = ColorWheel::kMouseModeValue;
        }
        else {
            return;
        }

        remembered_color_pos_ = colorPos();

        if (_mousePointerAction(event)) {
            _checkModified();
        }
    }


    void ColorWheel::handleMouseDrag(const Event& event) noexcept {
        if (_mousePointerAction(event)) {
            _checkModified();
        }
    }


    void ColorWheel::handleMouseUp(const Event& event) noexcept {
        if (_mousePointerAction(event)) {
            mouse_mode_ = ColorWheel::kMouseModeNone;
        }
        needsDisplay();
    }


    void ColorWheel::handleScrollWheel(const Event& event) noexcept {
        double h = current_color_.xDouble();
        double s = current_color_.yDouble();

        double new_h = h - event.deltaX() / 400;
        if (new_h < 0.0) {
            new_h += 1.0;
        }
        else if (new_h > 1.0) {
            new_h -= 1.0;
        }

        double new_s = s + event.deltaY() / 200;
        if (new_s < 0.0) {
            new_s = 0.0;
        }
        else if (new_s > 1.0) {
            new_s = 1.0;
        }

        if (new_h != h || new_s != s) {
            current_color_.x_ = h;
            current_color_.y_ = s;
            fireActionAndDisplay(Component::ActionType::None, nullptr);
        }
    }


    void ColorWheel::handleMagnification(const Event& event) noexcept {
        if (is_enabled_) {
            double v = current_color_.zDouble();
            double new_v = v + (event.value() - 1) / 10;

            if (new_v != v) {
                current_color_.z_ = v;
                fireActionAndDisplay(Component::ActionType::None, nullptr);
            }
        }
    }


    void ColorWheel::handleRotation(const Event& event) noexcept {
        // TODO: Implement!
    }


    void ColorWheel::setByComponent(Component* component) noexcept {
        if (component) {
            Vec3Fix new_color = current_color_;
            Fix value;

            if (component->isTextField()) {
                TextField* textfield = (TextField*)component;
                value = textfield->value();
            }
            else if (component->isSlider()) {
                Slider* slider = (Slider*)component;
                value = slider->value();
            }
            else if (component->isColorWell()) {
                ColorWell* color_well = (ColorWell*)component;
                HSV hsv = color_well->hsvColor();
                // new_color.setHSV(hsv); TODO: !!!!
            }

            switch (component->tag()) {
                case kControlTagHueTextField: new_color.x_ = value / 360; break;
                case kControlTagHueSlider: new_color.x_ = value; break;
                case kControlTagSaturationTextField: new_color.y_ = value / 100; break;
                case kControlTagSaturationSlider: new_color.y_ = value; break;
                case kControlTagValueTextField: new_color.z_ = value / 100; break;
                case kControlTagValueSlider: new_color.z_ = value; break;
                default: break;
            }

            if (new_color != current_color_) {
                current_color_ = new_color;
                fireActionAndDisplay(Component::ActionType::None, component);
            }
        }
    }


    bool ColorWheel::_mousePointerAction(const Event& event) noexcept {
        switch (mouse_mode_) {
            case ColorWheel::kMouseModeNone:
                return false;

            case ColorWheel::kMouseModeHue:
                {
                    auto angle = event.mousePos().angleToPos(center());
                    setColorHueByAngle(angle);
                }
                break;

            case ColorWheel::kMouseModeSaturation:
                {
                    Vec2d dir = hueVectorNorm();
                    Vec2d target = remembered_color_pos_ + (event.mouseDragDelta() / saturation_precision_) - center();
                    double length = target.dot(dir);
                    setColorSaturation(length / inner_radius_);
                }
                break;

            case ColorWheel::kMouseModeValue:
                {
                    double new_angle = valueAngle(event.mousePos());
                    current_color_.z_ = std::clamp<double>((new_angle - value_slider_offset_angle_) / (360.0f - value_slider_offset_angle_ * 2), 0, 1);
                }
                break;
        }

        return true;
    }


    void ColorWheel::_checkModified() noexcept {
        bool modified = false;

        if (mouse_down_color_ != current_color_) {
            is_modified_while_mouse_drag_ = true;
        }

        /* TODO: !!!!
        if (current_color_ != current_color_) {
            is_modified_while_mouse_drag_ = true;
            modified = true;
        }

        current_color_ = color_;
        */
        if (modified) {
            fireActionAndDisplay(Component::ActionType::None, nullptr);
        }
    }


    void ColorWheel::_updateDimensions() noexcept {
        Rectd bounds_rect = boundsRect();
        Rectd content_rect = contentRect();
        Rectd rect = content_rect.centeredSquare();

        center_ = bounds_rect.center();
        value_outer_radius_ = rect.shortSide() / 2;
        value_inner_radius_ = value_outer_radius_ - indicator_size_;
        hue_outer_radius_ = value_inner_radius_ - spacer_size_;
        hue_inner_radius_ = hue_outer_radius_ - hue_size_;
        inner_radius_ = hue_inner_radius_ - 0.5 * indicator_size_ - spacer_size_;
        value_rect_.set(center_, value_outer_radius_);
        hue_rect_.set(center_, hue_outer_radius_);
        inner_rect_.set(center_, inner_radius_);
    }


    void ColorWheel::updateRepresentations(const Component* excluded_component) noexcept {
        HSV hsv;
        // color_.hsv(hsv); TODO: !!!!

        Fix value;

        if (color_well_ && color_well_ != excluded_component) {
            color_well_->setHSVColor(hsv);
        }

        if (hue_textfield_ && hue_textfield_ != excluded_component) {
            value = current_color_.x_;
            value *= 360;
            hue_textfield_->setValue(value);
        }

        if (saturation_textfield_ && saturation_textfield_ != excluded_component) {
            value = current_color_.y_;
            value *= 100;
            saturation_textfield_->setValue(value);
        }

        if (value_textfield_ && value_textfield_ != excluded_component) {
            value = current_color_.z_;
            value *= 100;
            value_textfield_->setValue(value);
        }

        if (hue_slider_ && hue_slider_ != excluded_component) {
            hue_slider_->setValue(current_color_.x_);
        }

        if (saturation_slider_ && saturation_slider_ != excluded_component) {
            saturation_slider_->setValue(current_color_.y_);
        }

        if (value_slider_ && value_slider_ != excluded_component) {
            value_slider_->setValue(current_color_.z_);
        }

        if (receiver_component_ && receiver_component_ != excluded_component) {
            receiver_component_->setByComponent(this);
        }
    }

} // End of namespace