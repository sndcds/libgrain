//
//  Checkbox.cpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/Checkbox.hpp"
#include "Graphic/GraphicContext.hpp"
#include "App/App.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include "Graphic/AppleCGContext.hpp"
#endif


namespace Grain {

    Checkbox::Checkbox(const Rectd& rect, const char* text, int32_t tag) noexcept : Button(rect, text, tag) {
        type_ = ComponentType::CheckBox;
        is_toggle_mode_ = true;
        m_check_size = 12;
        radio_group_ = 0;
        radio_value_ = 0;
        draws_as_button_ = false;

        setText(text);
    }


    int32_t Checkbox::selectedRadioValue() const noexcept {
        Component* parent_component = parent_;

        if (parent_ && parent_component->componentType() != ComponentType::View) {
            View* parent_view = (View*)parent_;

            for (auto component : parent_view->components()) {

                if (component->componentType() == ComponentType::CheckBox) {
                    Checkbox* check_box = (Checkbox*)component;

                    if (check_box->radioGroup() == radio_group_ && check_box->isSelected()) {
                        return check_box->radioValue();
                    }
                }
            }
        }

        return -1;
    }


    Checkbox* Checkbox::add(View* view, const Rectd& rect, int32_t tag) {
        return add(view, rect, "", 0, 0, tag);
    }


    Checkbox* Checkbox::add(View* view, const Rectd& rect, const char* text, int32_t tag) {
        return add(view, rect, text, 0, 0, tag);
    }


    Checkbox* Checkbox::add(View* view, const Rectd& rect, const char* text, int32_t radio_group, int32_t radio_value, int32_t tag) {
        auto check_box = (Checkbox*)Component::addComponentToView(new (std::nothrow) Checkbox(rect, text, tag), view, AddFlags::kNone);

        if (check_box) {
            check_box->setRadioGroup(radio_group);
            check_box->setRadioValue(radio_value);
        }

        return check_box;
    }


    void Checkbox::draw(const Rectd& dirty_rect) noexcept {
        auto gc = graphicContextPtr();

        auto style= guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        auto bounds_rect = boundsRect();
        float radius = style->checkboxRadius();
        float check_size = style->checkboxSize();
        // Borderd padding = component->padding();

        bool enabled = isEnabled();
        bool selected = isSelected();
        bool highlighted = isHighlighted();
        int32_t radio_group = radioGroup();

        // RGB view_color = component->uiViewColor();
        RGBA background_color = style->backgroundColor();
        RGBA boder_color = style->backgroundColor();
        // RGB mark_color = component->Component::uiColor(UIColor::BgSelected);
        // RGB status_color = RGB::statusColor(selected, highlighted, bg_color, mark_color);

        if (!enabled) {
            // disableColorMixed(status_color, bg_color, view_color);
            // disableColor(bg_color, view_color);
        }

        if (hasText()) {
            Rectd text_rect = bounds_rect;
            text_rect.x_ += check_size + style->labelGap();
            text_rect.width_ -= check_size + style->labelGap();
            gc->drawTextInRect(text_->utf8(), text_rect, Alignment::Left, style->font(), style->labelColor());
        }

        Rectd check_rect(0, (bounds_rect.height_ - check_size) / 2, check_size, check_size);

        if (radio_group != 0) {
            radius = check_rect.width_ / 2;
        }

        gc->setFillRGBA(background_color);

        if (radio_group != 0) {
            gc->fillEllipse(check_rect);
        }
        else {
            gc->fillRoundRect(check_rect, radius);
        }

        if (selected || highlighted) {
            Rectd status_rect = check_rect;
            float inset_size = highlighted ? 5 : 3;
            if (radio_group != 0) {
                inset_size += 1;
            }
            status_rect.inset(inset_size);

            gc->setFillRGBA(style->foregroundColor());

            if (radio_group != 0) {
                gc->fillEllipse(status_rect);
            }
            else {
                gc->fillRect(status_rect);
            }
        }
    }


} // End of namespace Grain.
