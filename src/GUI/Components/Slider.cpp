//
//  Slider.hpp
//
//  Created by Roald Christesen on from 25.04.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/Slider.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Color/Gradient.hpp"


namespace Grain {

Slider::Slider(const Rectd& rect, int32_t tag) noexcept : ValueComponent(rect, tag) {
    type_ = ComponentType::Slider;
    can_get_focus_ = true;
    handle_size_ = 8;
    track_size_ = 4;
    custom_color_.setValue(-1);
}


Slider::~Slider() noexcept {
    delete gradient_;
}


Slider* Slider::add(View* view, const Rectd& rect, int32_t tag) {
    return (Slider*)Component::addComponentToView((Component*)new(std::nothrow) Slider(rect, tag), view);
}


Slider* Slider::add(View* view, const Rectd& rect, Fix min, Fix max, Fix offset, Fix default_value, Fix step, Fix big_step, int32_t fractional_digits, ComponentAction action, int32_t tag, void* action_ref) {
    auto slider = (Slider*)Component::addComponentToView((Component*)new(std::nothrow) Slider(rect, tag), view);
    if (slider != nullptr) {
        slider->setup(min, max, offset, default_value, step, big_step);
        slider->setAction(action, action_ref);
        slider->setFractionalDigits(fractional_digits);
    }

    return slider;
}


Slider* Slider::add(View* view, const Rectd& rect, ParamConfig* config, int32_t tag, ComponentAction action, void* action_ref) {
    if (config != nullptr) {
        return Slider::add(view, rect, config->min_, config->max_, config->offset_, config->default_, config->step_, config->big_step_, config->precision_, action, tag, action_ref);
    }

    return nullptr;
}


RGB Slider::color() const noexcept {
    switch (slider_type_) {
        case SliderType::Kelvin: {
            RGB color;
            color.setKelvin(valueAsDouble());
            return color;
        }
        case SliderType::Hue: {
            RGB color;
            color.setHSV(valueAsDouble(), 1, 1);
            return color;
        }
        case SliderType::RGBRed:
            return RGB(0.9f, 0.1f, 0.1f);
        case SliderType::RGBGreen:
            return RGB(0.1f, 0.9f, 0.1f);
        case SliderType::RGBBlue:
            return RGB(0.1f, 0.1f, 0.9f);
        case SliderType::Normal:
        default:
            return custom_color_;
    }
}


bool Slider::hit(const Event& event) noexcept {
    return boundsRect().contains(event.mousePos());
}


void Slider::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
    if (!gc) { return; }
    auto style = guiStyle();
    if (!style) {
        drawDummy(gc);
        return;
    }

    slider_rect_ = boundsRect();
    if (isHorizontal()) {
        slider_rect_.insetHorizontal(controller_padding_);
    }
    else {
        slider_rect_.insetVertical(controller_padding_);
    }

    Rectd track_rect;
    Rectd indicator_rect;

    Slider::SliderType slider_type = sliderType();
    // Slider::HandleType handle_type = handleType();

    bool is_enabled = isEnabled();
    bool is_horizontal = isHorizontal();

    double offset_value = offsetValue().asDouble();
    double min_value = minValue().asDouble();
    double max_value = maxValue().asDouble();

    double offset_pos = normalizedOffsetValue();
    double handle_pos = normalizedValue();

    bool indicator_visibility = isIndicatorVisible();

    bool has_gradient = false; // TODO: hasGradient();
    Gradient* gradient = nullptr;   // TODO: gradient();

    float handle_size = handleSize();
    float track_size = trackSize();


    float handle_radius;
    float track_radius = 0;
    float track_margin = 0;
    float min, max, length;

    // Compute the positions and dimensions
    if (is_horizontal) {
        if (handle_size > slider_rect_.height_) {
            handle_size = slider_rect_.height_;
        }

        handle_radius = handle_size / 2;

        if (track_size > slider_rect_.height_) {
            track_size = slider_rect_.height_;
        }

        track_radius = track_size / 2;
        track_margin = handle_radius - track_radius;

        if (track_margin > 0) {
            track_rect.set(
                track_margin + controller_padding_,
                slider_rect_.height_ / 2 - track_radius,
                slider_rect_.width_ - 2 * track_margin,
                track_size);
            min = 0;
            max = slider_rect_.width_ - handle_size;
        }
        else {
            track_margin = 0;
            track_rect.set(
                controller_padding_,
                slider_rect_.height_ / 2 - track_radius,
                slider_rect_.width_,
                track_size);
            min = track_radius - handle_radius;
            max = slider_rect_.width_ - (track_radius - handle_radius) - handle_size;
        }

        length = max - min;

        handle_rect_.set(
            min + handle_pos * length + controller_padding_,
            (slider_rect_.height_ - handle_size) / 2,
            handle_size,
            handle_size);

        indicator_rect.set(
            track_rect.x_,
            track_rect.y_,
            handle_rect_.centerX() - track_rect.x_,
            track_rect.height_);
    }
    else { // Vertical
        if (handle_size > slider_rect_.width_) {
            handle_size = slider_rect_.width_;
        }
        handle_radius = handle_size / 2;

        if (track_size > slider_rect_.width_) {
            track_size = slider_rect_.width_;
        }
        track_radius = track_size / 2;

        track_margin = handle_radius - track_radius;

        if (track_margin > 0) {
            track_rect.set(
                slider_rect_.width_ / 2 - track_radius,
                track_margin + controller_padding_,
                track_size,
                slider_rect_.height_ - 2 * track_margin);
            min = 0;
            max = slider_rect_.height_ - handle_size;
        }
        else {
            track_margin = 0;
            track_rect.set(
                slider_rect_.width_ / 2 - track_radius,
                controller_padding_,
                track_size,
                slider_rect_.height_);
            min = track_radius - handle_radius;
            max = slider_rect_.height_ - (track_radius - handle_radius) - handle_size;
        }

        length = max - min;

        handle_rect_.set(
            (slider_rect_.width_ - handle_size) / 2,
            max - handle_pos * length + controller_padding_,
            handle_size,
            handle_size);

        indicator_rect.set(
            track_rect.x_,
            handle_rect_.centerY(),
            track_rect.width_,
            track_rect.height() + track_rect.y() - handle_rect_.centerY());
    }

    // Collect colors
    RGBA view_color = style->viewColor();
    RGBA track_color = style->controllerTrackColor();
    RGBA indicator_color = { 1, 0, 0, 1 };
    RGBA handle_color = style->controllerHandleColor();
    RGBA custom_color = { -1, -1, -1, -1 };

    if (slider_type == Slider::SliderType::Normal && custom_color.isValidUiColor()) {
        track_color.setBlend(custom_color, view_color, 0.7f);
        indicator_color = custom_color;
    }

    if (!is_enabled) {
        track_color.setBlend(view_color, 0.5f);
        indicator_color.setBlend(view_color, 0.5f);
        handle_color.setBlend(view_color, 0.5f);
    }

    bool handle_inside = handle_radius < track_radius;
    if (handle_inside && has_gradient) {
        if (is_enabled) {
            handle_color.black();
        }
        if (has_gradient) {
            indicator_visibility = false;
        }
    }

    // Draw track
    if (track_size >= 0.5f) {
        Vec2d start_pos;
        Vec2d end_pos;
        if (is_horizontal) {
            start_pos.set(min, 0);
            end_pos.set(max, 0);
        }
        else {
            start_pos.set(0, max);
            end_pos.set(0, min);
        }

        gc->save();

        gc->beginPath();
        gc->addRoundBarPath(track_rect);
        gc->clipPath();

        if (slider_type == Slider::SliderType::Kelvin) {
            // App::kelvinGradient()->draw(gc, start_pos, end_pos);
        }
        else if (is_enabled && has_gradient) {
            gradient->draw(gc, start_pos, end_pos);
        }
        else {
            gc->setFillRGBA(track_color);
            gc->fillRect(track_rect);

            if (indicator_visibility) {
                gc->setFillRGBA(indicator_color);
                gc->fillRect(indicator_rect);
            }
        }

        gc->restore();
    }

    // Draw handle
    if (has_gradient && handleType() == Slider::HandleType::ColorCircle) {
        RGBA color = { 0.3, 0.4, 0.5, 1.0 }; // uiColor(UIColor::View);

        gc->setFillRGBA(color);
        gc->fillEllipse(handle_rect_);

        gradient->lookupColor(handle_pos, color);
        gc->setFillRGBA(color);

        Rectd rect = handle_rect_;
        rect.inset(2);

        gc->fillEllipse(rect);
    }
    else if (handleType() == Slider::HandleType::Fader) {
        gc->setFillRGBA(handle_color);
        Rectd rect = handle_rect_;

        if (isVertical()) {
            double w = width();
            rect.x_ = centerX() - w / 2;
            rect.width_ = w;
        }
        else {
            double h = height();
            rect.y_ = centerY() - h / 2;
            rect.height_ = h;
        }
        gc->fillRoundRect(rect, 4); // TODO:
    }
    else {
        gc->setFillRGBA(handle_color);
        gc->fillEllipse(handle_rect_);
    }

    if (isKeyComponent()) {
        gc->setFillColor(1, 0, 0, 0.5);
        gc->fillFrame(boundsRect(), 1);
    }
}


void Slider::handleMouseDown(const Event& event) noexcept {
    if (hit(event)) {
        ValueComponent::handleMouseDown(event);

        handle_hit_ = event.distanceFromMouse(handle_rect_.center()) < (handle_size_ / 2 + 2);
        if (event.isMouseDoubleClicked()) {
            if (value_ != default_) {
                is_modified_since_mouse_down_ = true;
                setValue(default_);
                fireAction(ActionType::None, nullptr);
            }
            event.mousePressedFinished();
        }
        else if (mouse_precision_mode_) {
            remembered_value_ = value_;
        }
        else if (!handle_hit_) {
            setValueByPos(event.mouseDownPos());
            remembered_value_ = value_;
            handle_hit_ = true;
            is_modified_since_mouse_down_ = is_modified_while_mouse_drag_ = true;
        }
    }
}


void Slider::handleMouseDrag(const Event& event) noexcept {
    double delta = isHorizontal() ? event.mouseDragDeltaX() : -event.mouseDragDeltaY();
    if (mouse_precision_mode_) {
        delta *= 0.1f;
    }

    Fix new_value = remembered_value_;
    new_value += delta * pixelStep();
    new_value.clamp(min_, max_);

    if (new_value != value_) {
        setValue(new_value);
        is_modified_while_mouse_drag_ = true;
        fireAction(ActionType::None, nullptr);
    }

    if (remembered_value_ != value_) {
        is_modified_since_mouse_down_ = true;
    }
}


void Slider::setColorWell(ColorWell* color_well) noexcept {
    color_well_ = color_well;
    if (color_well) {
        // color_well->setColor(color()); // TODO: !!!
    }
}


double Slider::pixelStep() const noexcept {
    return (float)((max_.asDouble() - min_.asDouble()) / (sliderLength() - handle_size_));
}


void Slider::setKelvinSlider(int32_t k0, int32_t k1, int32_t k_default, float saturation, float value) noexcept {
    slider_type_ = SliderType::Kelvin;

    removeGradient();
    gradient_ = new(std::nothrow) Gradient();
    if (gradient_) {
        gradient_->buildKelvinGradient(k0, k1, saturation, value);
    }
    setupReal(k0, k1, 0, k_default, 10, 100);
}


void Slider::setHueSlider(float saturation, float value) noexcept {
    slider_type_ = SliderType::Hue;

    removeGradient();
    gradient_ = new(std::nothrow) Gradient();
    if (gradient_) {
        gradient_->buildHueGradient(saturation, value);
    }
    setupReal(0, 360, 0, 0, 1, 10);
}


bool Slider::setValueByPos(const Vec2d& pos) noexcept {
    auto slider_rect = sliderRect();
    double min = min_.asDouble();
    double max = max_.asDouble();
    Fix new_value;

    if (isHorizontal()) {
        new_value.setDouble(Math::remapclamped(slider_rect.x_, slider_rect.x2(), min, max, pos.x_));
    }
    else if (view_is_flipped_) {
        new_value.setDouble(Math::remapclamped(slider_rect.y2(), slider_rect.y_, min, max, pos.y_));
    }
    else {
        new_value.setDouble(Math::remapclamped(slider_rect.y_, slider_rect.y2(), min, max, pos.y_));
    }

    if (new_value == value_) {
        return false;
    }

    setValue(new_value);
    fireAction(ActionType::None, nullptr);
    return true;
}

} // End of namespace Grain
