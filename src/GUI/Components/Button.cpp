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
#include "Graphic/MacCGContext.hpp"
#endif


namespace Grain {

    Button::Button(const Rectd& rect, const char* text, int32_t tag) noexcept : Component(rect, tag) {
        m_type = ComponentType::Button;
        m_radio_group = 0;
        m_radio_value = 0;
        m_draws_as_button = true;

        // setMouseCursor(App::MouseCursor::PointingHand); TODO: !!!!!

        setText(text);
    }


    Button* Button::add(View* view, const Rectd& rect, const char* text, int32_t tag) {
        return (Button*)Component::addComponentToView(new (std::nothrow) Button(rect, text, tag), view, AddFlags::kNone);
    }


    void Button::setSelected(bool selected) noexcept {
        if (m_radio_group > 0) {
            if (selected && m_parent) {
                m_parent->deselectRadioGroup(m_radio_group);
            }
        }
        m_is_selected = selected;
        needsDisplay();
    }


    void Button::draw(const Rectd& dirty_rect) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        MacCGContext gc(this);
#else
        GraphicContext gc(this);
#endif

        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        gc.setFillRGBA(style->backgroundColor());
        switch (style->cornerRadiusMode()) {
            case GUIStyle::CornerRadiusMode::No:
                gc.fillRect(boundsRect());
                break;
            case GUIStyle::CornerRadiusMode::Same:
                gc.fillRoundRect(boundsRect(), style->cornerRadius(0));
                break;
            case GUIStyle::CornerRadiusMode::Different:
                gc.fillRoundRect(
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
            gc.drawTextInRect(m_text->utf8(), text_rect, style->textAlignment(), style->font(), style->foregroundColor());
        }
    }


    void Button::handleMouseDown(const Event& event) noexcept {
        if (hit(event)) {
            highlight();
            if (!isDelayed()) {
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

        deHighlight();
    }

} // End of namespace Grain
