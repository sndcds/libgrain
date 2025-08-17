//
//  Checkbox.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/Checkbox.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    Checkbox::Checkbox(const Rectd& rect, const char* text, int32_t tag) noexcept : Button(rect, text, tag) {
        m_type = ComponentType::CheckBox;
        m_is_toggle_mode = true;
        m_check_size = 12;
        m_radio_group = 0;
        m_radio_value = 0;
        m_draws_as_button = false;
        m_text_alignment = Alignment::Left;
        m_text_y_offset = -1;

        setText(text);
    }


    int32_t Checkbox::selectedRadioValue() const noexcept {
        Component* parent_component = m_parent;
        if (m_parent && parent_component->componentType() != ComponentType::View) {
            View* parent_view = (View*)m_parent;
            for (auto component : parent_view->components()) {
                if (component->componentType() == ComponentType::CheckBox) {
                    Checkbox* check_box = (Checkbox*)component;
                    if (check_box->radioGroup() == m_radio_group && check_box->isSelected()) {
                        return check_box->radioValue();
                    }
                }
            }
        }

        return -1;
    }


    Checkbox *Checkbox::add(View *view, const Rectd &rect, int32_t tag) {
        return add(view, rect, "", 0, 0, tag);
    }


    Checkbox *Checkbox::add(View *view, const Rectd &rect, const char *text, int32_t tag) {
        return add(view, rect, text, 0, 0, tag);
    }


    Checkbox *Checkbox::add(View *view, const Rectd &rect, const char *text, int32_t radio_group, int32_t radio_value, int32_t tag) {
        auto check_box = (Checkbox*)Component::addComponentToView(new(std::nothrow) Checkbox(rect, text, tag), view, AddFlags::kNone);
        if (check_box) {
            check_box->setRadioGroup(radio_group);
            check_box->setRadioValue(radio_value);
        }

        return check_box;
    }


    void Checkbox::draw(const Rectd &dirty_rect) noexcept {
        GraphicContext gc(this);
        gc.setFillColor(RGBA(1, 0, 0, 1));
        gc.fillRect(boundsRect());
        // drawRect(gc, boundsRect());
    }


} // End of namespace Grain.
