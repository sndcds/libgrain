//
//  ScrollBar.cpp
//
//  Created by Roald Christesen on from 05.06.15.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/ScrollBar.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    ScrollBar::ScrollBar(const Rectd &rect, bool vertical) : Component(rect) {
        m_type = ComponentType::ScrollBar;
        m_is_delayed = true;
        m_bar_size = 12.0f;
        m_vertical = vertical;
        m_visible_fraction = 1.0f;
        m_scroll_position = 0.0f;
        m_track_rect.zero();
        m_handle_rect.zero();
    }


    ScrollBar::~ScrollBar() {
    }


    ScrollBar *ScrollBar::add(View *view, const Rectd &rect, bool vertical) {
        ScrollBar *scroll_bar = nullptr;
        if (view) {
            scroll_bar = new (std::nothrow) ScrollBar(rect, vertical);
            if (scroll_bar) {
                view->addComponent(scroll_bar, AddFlags::kNone);
            }
        }
        return scroll_bar;
    }


    void ScrollBar::setVisibleFraction(double visible_fraction) {
        m_visible_fraction = std::clamp<double>(visible_fraction, 0.00001, 1.0);
        needsDisplay();
    }


    void ScrollBar::setVisibleFraction(double total_size, double visible_size) {
        setVisibleFraction(total_size > DBL_MIN ? visible_size / total_size : 1.0f);
        needsDisplay();
    }


    void ScrollBar::setScrollPosition(double scroll_position) {
        if (scroll_position != m_scroll_position) {
            m_scroll_position = std::clamp<double>(scroll_position, 0.0, 1.0);
            needsDisplay();
        }
    }


    void ScrollBar::setScrollPosition(double offset, double max_offset) {
        if (max_offset < 0.00001) {
            setScrollPosition(0.0f);
        }
        else {
            setScrollPosition(offset / max_offset);
        }
    }


    void ScrollBar::draw(const Rectd &dirty_rect) noexcept {
        GraphicContext gc(this);

        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        gc.save();

        Rectd bounds_rect = boundsRect();
        RGB bg_color = style->backgroundColor();

        gc.setFillRGB(bg_color);
        gc.fillRect(bounds_rect);

        m_track_rect = bounds_rect;
        m_track_rect.inset(style->scrollBarPadding());
        m_handle_rect = m_track_rect;

        double radius = 0.0f;

        if (m_vertical) {
            double min_size = m_track_rect.m_width;
            m_handle_rect.m_height = std::clamp<double>(m_track_rect.m_height * m_visible_fraction, min_size, m_track_rect.m_height);
            m_handle_rect.m_y = m_track_rect.m_y + (m_track_rect.m_height - m_handle_rect.m_height) * m_scroll_position;
            radius = m_track_rect.m_width / 2.0f;
        }
        else {
            double min_size = m_track_rect.m_height;
            m_handle_rect.m_width = std::clamp<double>(m_handle_rect.m_width * m_visible_fraction, min_size, m_track_rect.m_width);
            m_handle_rect.m_x = m_track_rect.m_x + (m_track_rect.m_width - m_handle_rect.m_width) * m_scroll_position;
            radius = m_handle_rect.m_height / 2.0f;
        }

        RGB handle_color = style->scrollBarColor();
        gc.setFillRGB(handle_color);
        gc.fillRoundBar(m_handle_rect);

        gc.restore();
    }


    void ScrollBar::handleMouseDown(const Event &event) noexcept {
        if (m_handle_rect.contains(event.mouseDownPos())) {
            m_remembered_scroll_position = m_scroll_position;
            highlight();
        }
        else {
            event.mousePressedFinished();
        }
    }


    void ScrollBar::handleMouseDrag(const Event &event) noexcept {
        if (m_visible_fraction < 1.0f) {
            if (m_vertical) {
                double length = (m_track_rect.m_height - m_handle_rect.m_height);
                if (length > DBL_EPSILON) {
                    m_scroll_position = std::clamp<double>(m_remembered_scroll_position + event.mouseDragDeltaY() / length, 0.0, 1.0);
                    fireActionAndDisplay(ActionType::None, nullptr);
                }
            }
            else {
                double length = (m_track_rect.m_width - m_handle_rect.m_width);
                if (length > DBL_EPSILON) {
                    m_scroll_position = std::clamp<double>(m_remembered_scroll_position + event.mouseDragDeltaX() / length, 0.0, 1.0);
                    fireActionAndDisplay(ActionType::None, nullptr);
                }
            }
        }
    }


    void ScrollBar::handleMouseUp(const Event &event) noexcept {
        deHighlight();
    }


} // End of namespace Grain.
