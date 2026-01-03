//
//  Button.cpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "App/App.hpp"
#include "GUI/Components/Button.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include "Graphic/AppleCGContext.hpp"
#endif


namespace Grain {

    Button::Button(const Rectd& rect, const char* text, int32_t tag) noexcept : Component(rect, tag) {
        type_ = ComponentType::Button;
        radio_group_ = 0;
        radio_value_ = 0;
        draws_as_button_ = true;

        // setMouseCursor(App::MouseCursor::PointingHand); TODO: !!!!!

        setText(text);
    }

    Button* Button::add(View* view, const Rectd& rect, const char* text, int32_t tag) {
        return (Button*)Component::addComponentToView(new (std::nothrow) Button(rect, text, tag), view, AddFlags::kNone);
    }

    void Button::setSelected(bool selected) noexcept {
        if (radio_group_ > 0) {
            if (selected && parent_) {
                parent_->deselectRadioGroup(radio_group_);
            }
        }
        is_selected_ = selected;
        needsDisplay();
    }

    void Button::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        gc->setFillRGBA(style->backgroundColor());
        switch (style->cornerRadiusMode()) {
            case GUIStyle::CornerRadiusMode::No:
                gc->fillRect(boundsRect());
                break;
            case GUIStyle::CornerRadiusMode::Same:
                gc->fillRoundRect(boundsRect(), style->cornerRadius(0));
                break;
            case GUIStyle::CornerRadiusMode::Different:
                gc->fillRoundRect(
                    boundsRect(),
                    style->cornerRadius(0),
                    style->cornerRadius(1),
                    style->cornerRadius(2),
                    style->cornerRadius(3));
                break;
        }

        if (hasText()) {
            Rectd text_rect = boundsRect();
            text_rect.inset(style->padding(0), style->padding(1), style->padding(2), style->padding(3));
            gc->drawTextInRect(text_->utf8(), text_rect, style->textAlignment(), style->font(), style->foregroundColor());
        }

        if (isHighlighted()) {
            gc->setFillColor(1, 0, 0, 1);
            gc->fillFrame(boundsRect(), 2);
        }
    }

    void Button::handleMouseDown(const Event& event) noexcept {
        if (hit(event)) {
            highlight();
            if (!isDelayed()) {
                if (radio_group_ > 0) {
                    if (!is_selected_) {
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

                deHighlight();
                event.mousePressedFinished();
            }
        }
    }

    void Button::handleMouseDrag(const Event& event) noexcept {
        setHighlighted(hit(event));
    }

    void Button::handleMouseUp(const Event& event) noexcept {
        if (hit(event)) {
            if (radio_group_ > 0) {
                if (!is_selected_) {
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

        deHighlight();
    }

} // End of namespace Grain
