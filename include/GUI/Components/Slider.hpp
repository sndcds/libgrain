//
//  Slider.hpp
//
//  Created by Roald Christesen on from 25.04.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainSlider_hpp
#define GrainSlider_hpp

#include "Grain.hpp"
#include "GUI/Components/Component.hpp"
#include "GUI/Components/ValueComponent.hpp"
#include "Data/ParamConfig.hpp"


namespace Grain {

class ColorWell;


class Slider : public ValueComponent {

public:
    enum class SliderType {
        Normal = 0,
        Kelvin,
        RGBRed,
        RGBGreen,
        RGBBlue,
        Hue
    };

    enum class HandleType {
        Standard = 0,
        ColorCircle,
        Fader
    };

protected:
    SliderType slider_type_ = SliderType::Normal;
    HandleType handle_type_ = HandleType::Standard;
    RGB custom_color_;
    Rectd handle_rect_;
    Rectd slider_rect_;
    bool handle_hit_ = false;

public:
    Slider(const Rectd &rect, int32_t tag = 0) noexcept;
    virtual ~Slider() noexcept;

    const char* className() const noexcept override { return "Slider"; }


    static Slider* add(View* view, const Rectd &rect, int32_t tag = 0);
    static Slider* add(View* view, const Rectd &rect, Fix min, Fix max, Fix offset, Fix default_value, Fix step, Fix big_step, int32_t fractional_digits, ComponentAction action, int32_t tag, void* action_ref);
    static Slider* add(View* view, const Rectd &rect, ParamConfig* config, int32_t tag, ComponentAction action, void* action_ref);

    void setColor(const RGB &color) noexcept override { custom_color_ = color; needsDisplay(); }
    RGB color() const noexcept override;
    void setColorWell(ColorWell* color_well) noexcept override;

    bool hit(const Event &event) noexcept override;
    void draw(GraphicContext* gc, const Rectd &dirty_rect) noexcept override;
    void handleMouseDown(const Event &event) noexcept override;
    void handleMouseDrag(const Event &event) noexcept override;

    bool isNormalSlider() const noexcept { return slider_type_ == SliderType::Normal; }
    bool isKelvinSlider() const noexcept { return slider_type_ == SliderType::Kelvin; }
    bool isHueSlider() const noexcept { return slider_type_ == SliderType::Hue; }

    SliderType sliderType() const noexcept { return slider_type_; }
    HandleType handleType() const noexcept { return handle_type_; }
    Rectd handleRect() const noexcept { return handle_rect_; }
    double sliderLength() const noexcept { return rect_.longSide() - 2 * controller_padding_; }
    Rectd sliderRect() const noexcept { return slider_rect_; }

    double pixelStep() const noexcept;

    void setHandleType(HandleType type) noexcept { handle_type_ = type; needsDisplay(); }
    void setHandleTypeColorCircle() noexcept { handle_type_ = HandleType::ColorCircle; needsDisplay(); }
    void setKelvinSlider(int32_t k0, int32_t k1, int32_t k_default, float saturation = 0.95f, float value = 0.85f) noexcept;
    void setHueSlider(float saturation = 0.95f, float value = 0.85f) noexcept;
    bool setValueByPos(const Vec2d &pos) noexcept;
};

} // End of namespace Grain

#endif // GrainSlider_hpp
