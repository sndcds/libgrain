//
//  Button.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "App/App.hpp"
#include "GUI/Components/Button.hpp"
#include "Graphic/GraphicContext.hpp"
#include "GUI/Event.hpp"
// #include "Look.hpp"


namespace Grain {

    Button::Button(const Rectd& rect, const char* text, int32_t tag) noexcept : Component(rect, tag) {
        m_type = ComponentType::Button;
        m_text_y_offset = -1;
        m_radio_group = 0;
        m_radio_value = 0;
        m_draws_as_button = true;

        // setMouseCursor(App::MouseCursor::PointingHand);

        setText(text);
    }


    Button* Button::add(View* view, const Rectd& rect, const char* text, int32_t tag) {
        return (Button*)Component::addComponentToView(new(std::nothrow) Button(rect, text, tag), view, AddFlags::kNone);
    }


    void Button::setSelected(bool selected) noexcept {
        if (m_radio_group > 0) {
            if (selected == true && m_parent != nullptr) {
                m_parent->deselectRadioGroup(m_radio_group);
            }
        }
        m_is_selected = selected;
        needsDisplay();
    }



    void Button::draw(const Rectd& dirty_rect) noexcept {
        GraphicContext gc(this);

        if (m_must_compute_style) {
            auto app_style_set = App::styleSet();
            auto button_style_class = app_style_set->buttonStyleClass();

            if (auto property = button_style_class->propertyAtType(StylePropertyType::Color)) {
                m_color[0] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::ColorHighlighted)) {
                m_color[1] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::ColorSelected)) {
                m_color[2] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::BackgroundColor)) {
                m_background_color[0] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::BackgroundColorHighlighted)) {
                m_background_color[1] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::BackgroundColorSelected)) {
                m_background_color[2] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::BorderColor)) {
                m_border_color[0] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::BorderColorHighlighted)) {
                m_border_color[1] = property->valueOrDefault<RGBA>().rgba32bit();
            }
            if (auto property = button_style_class->propertyAtType(StylePropertyType::BorderColorSelected)) {
                m_border_color[2] = property->valueOrDefault<RGBA>().rgba32bit();
            }

            for (auto& property : *m_style_list.properties()) {
                if (property.type() == StylePropertyType::Color) {
                    m_color[0] = property.valueOrDefault<RGBA>().rgba32bit();
                }
                if (property.type() == StylePropertyType::BackgroundColor) {
                    m_background_color[0] = property.valueOrDefault<RGBA>().rgba32bit();
                }
                if (property.type() == StylePropertyType::BorderColor) {
                    m_border_color[0] = property.valueOrDefault<RGBA>().rgba32bit();
                }
                if (property.type() == StylePropertyType::BorderWidth) {
                    m_border_width[0] = property.valueOrDefault<float>();
                }
            }

            m_must_compute_style = false;
        }

        drawRect(gc, boundsRect());

        RectEdgesf style_padding(10.0f, 0.0f, 4.0f, 4.0f);
        Alignment style_text_alignment = Alignment::Center;
        Font* style_font = App::uiFont();

        if (hasText()) {
            Rectd text_rect = boundsRect();
            text_rect.inset(style_padding);
            gc.drawTextInRect(m_text->utf8(), text_rect, style_text_alignment, style_font, RGBA(m_color[0]));
        }
    }


    void Button::handleMouseDown(const Event& event) noexcept {
        if (hit(event)) {
            highlight();
            if (isDelayed() == false) {
                if (m_radio_group > 0) {
                    if (!m_is_selected) {
                        select();
                        forcedDisplay();
                        fireAction(Component::ActionType::None, nullptr);
                    }
                }
                else {
                    if (isToggleMode()) {
                        toggleSelection();
                    }

                    if (hasAction()) {
                        forcedDisplay();
                        fireAction(Component::ActionType::None, nullptr);
                    }
                }

                dehighlight();
                event.mousePressedFinished();
            }
        }
    }


    void Button::handleMouseDrag(const Event& event) noexcept {
        setHighlighted(hit(event));
    }


    void Button::handleMouseUp(const Event& event) noexcept {
        if (hit(event)) {
            if (m_radio_group > 0) {
                if (!m_is_selected) {
                    select();
                    fireAction(Component::ActionType::None, nullptr);
                }
            }
            else {
                if (isToggleMode()) {
                    toggleSelection();
                }

                if (isDelayed()) {
                    fireAction(Component::ActionType::None, nullptr);
                }
            }
        }

        dehighlight();
    }


} // End of namespace Grain
