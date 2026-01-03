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

    ScrollBar::ScrollBar(const Rectd& rect, bool vertical) : Component(rect) {
        type_ = ComponentType::ScrollBar;
        is_delayed_ = true;
        bar_size_ = 12.0f;
        is_vertical_ = vertical;
        visible_fraction_ = 1.0f;
        scroll_position_ = 0.0f;
        track_rect_.zero();
        handle_rect_.zero();
    }

    ScrollBar::~ScrollBar() = default;

    ScrollBar* ScrollBar::add(View* view, const Rectd& rect, bool vertical) {
        ScrollBar* scroll_bar = nullptr;
        if (view) {
            scroll_bar = new (std::nothrow) ScrollBar(rect, vertical);
            if (scroll_bar) {
                view->addComponent(scroll_bar, AddFlags::kNone);
            }
        }
        return scroll_bar;
    }

    void ScrollBar::setVisibleFraction(double visible_fraction) {
        visible_fraction_ = std::clamp<double>(visible_fraction, 0.00001, 1.0);
        needsDisplay();
    }

    void ScrollBar::setVisibleFraction(double total_size, double visible_size) {
        setVisibleFraction(total_size > DBL_MIN ? visible_size / total_size : 1.0f);
        needsDisplay();
    }

    void ScrollBar::setScrollPosition(double scroll_position) {
        if (scroll_position != scroll_position_) {
            scroll_position_ = std::clamp<double>(scroll_position, 0.0, 1.0);
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

    void ScrollBar::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        gc->save();

        Rectd bounds_rect = boundsRect();
        RGBA bg_color = style->backgroundColor();

        gc->setFillRGB(bg_color);
        gc->fillRect(bounds_rect);

        track_rect_ = bounds_rect;
        track_rect_.inset(style->scrollBarPadding());
        handle_rect_ = track_rect_;

        if (is_vertical_) {
            double min_size = track_rect_.width_;
            handle_rect_.height_ = std::clamp<double>(track_rect_.height_ * visible_fraction_, min_size, track_rect_.height_);
            handle_rect_.y_ = track_rect_.y_ + (track_rect_.height_ - handle_rect_.height_) * scroll_position_;
        }
        else {
            double min_size = track_rect_.height_;
            handle_rect_.width_ = std::clamp<double>(handle_rect_.width_ * visible_fraction_, min_size, track_rect_.width_);
            handle_rect_.x_ = track_rect_.x_ + (track_rect_.width_ - handle_rect_.width_) * scroll_position_;
        }

        RGBA handle_color = style->scrollBarHandleColor();
        gc->setFillRGBA(handle_color);
        gc->fillRoundBar(handle_rect_);

        gc->restore();
    }

    void ScrollBar::handleMouseDown(const Event& event) noexcept {
        if (handle_rect_.contains(event.mouseDownPos())) {
            remembered_scroll_position_ = scroll_position_;
            highlight();
        }
        else {
            event.mousePressedFinished();
        }
    }

    void ScrollBar::handleMouseDrag(const Event& event) noexcept {
        if (visible_fraction_ < 1.0f) {
            if (is_vertical_) {
                double length = (track_rect_.height_ - handle_rect_.height_);
                if (length > DBL_EPSILON) {
                    scroll_position_ = std::clamp<double>(remembered_scroll_position_ + event.mouseDragDeltaY() / length, 0.0, 1.0);
                    fireActionAndDisplay(ActionType::None, nullptr);
                }
            }
            else {
                double length = (track_rect_.width_ - handle_rect_.width_);
                if (length > DBL_EPSILON) {
                    scroll_position_ = std::clamp<double>(remembered_scroll_position_ + event.mouseDragDeltaX() / length, 0.0, 1.0);
                    fireActionAndDisplay(ActionType::None, nullptr);
                }
            }
        }
    }

    void ScrollBar::handleMouseUp(const Event& event) noexcept {
        deHighlight();
    }

} // End of namespace Grain.
