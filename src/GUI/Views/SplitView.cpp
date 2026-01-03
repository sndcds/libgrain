//
//  SpitView.cpp
//
//  Created by Roald Christesen on from 05.08.2012.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Views/SplitView.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"
#include "App/App.hpp"


namespace Grain {

    SplitView::SplitView(const Rectd& rect, int32_t tag) noexcept : View(rect, tag) {
        type_ = ComponentType::SplitView;

        // setMouseCursor(App::MouseCursor::ResizeLeftRight); TODO: !!!!!
    }


    SplitView::~SplitView() noexcept {
        // TODO: Realease resources
    }


    SplitView* SplitView::add(View* view, const Rectd& rect, int32_t tag) {
        return (SplitView*)Component::addComponentToView((Component*)new (std::nothrow) SplitView(rect, tag), view, AddFlags::kWantsLayer);
    }


    void SplitView::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {

        if (m_must_init) {
            initLayout();
            m_must_init = false;
        }

        Rectd bounds_rect = boundsRect();

        RGB bg_color = { 0.2, 0.2, 0.2 }; // TODO: !!!!!
        gc->setFillRGB(bg_color);
        gc->fillRect(bounds_rect);

        float divider_size = dividerSize();
        Rectd split_rect;

        if (m_vertical) {
            split_rect.set(0, static_cast<int32_t>(divider_size / 2) - 0.5f, bounds_rect.width_, 1);
        }
        else {
            split_rect.set(static_cast<int32_t>(divider_size / 2) - 0.5f, 0, 1, bounds_rect.height_);
        }

        int32_t n = viewCount();

        for (int32_t i = 0; i < n; i++) {
            Rectd view_rect = getViewRect(i);
            m_vertical ? split_rect.y_ += view_rect.height_ : split_rect.x_ += view_rect.width_;
            m_vertical ? split_rect.y_ += divider_size : split_rect.x_ += divider_size;
        }

        callDrawFunction(gc);
    }


    bool SplitView::hasDescendant(const Component* component) noexcept {
        for (int32_t i = 0; i < viewCount(); i++) {
            if (m_items[i].view_->hasDescendant(component)) {
                return true;
            }
        }
        return false;
    }


    void SplitView::parentGeometryChanged() noexcept {
        Component::parentGeometryChanged();
    }


    void SplitView::geometryChanged() noexcept {
        double available_size = availableSize();

        for (int32_t i = 0; i < m_view_count; i++) {
            auto item = itemAtIndex(i);
            item->setSize(item->real_size_ * available_size);
        }

        updateRectOfAllViews();

        available_size = availableSize();
        int32_t total_view_size = totalSizeOfViews();

        int32_t grow_potential;
        int32_t shrink_potential;
        int32_t growable_count;
        int32_t shrinkable_count;

        viewResizePotential(grow_potential, shrink_potential, growable_count, shrinkable_count);

        int32_t rest = available_size - total_view_size;

        if (rest > 0 && growable_count > 0) {
            int32_t loop_index = 0;

            while (rest > 0 && loop_index < 10) {
                if (rest < growable_count) {
                    for (int32_t i = 0; i < m_view_count; i++) {
                        auto item = itemAtIndex(i);
                        if (item->canGrow()) {
                            item->setSize(item->size_ + 1);
                            rest--;
                        }
                        if (rest < 1) {
                            break;
                        }
                    }
                    rest = 0;
                }
                else {
                    total_view_size = 0;
                    for (int32_t i = 0; i < m_view_count; i++) {
                        auto item = itemAtIndex(i);
                        if (item->canGrow()) {
                            item->setSize(item->size_ + rest / growable_count);
                        }
                        total_view_size += item->size_;
                    }
                    rest = available_size - total_view_size;
                }
                loop_index++;
            }

            updateRectOfAllViews();
        }
        else if (rest < 0 && shrinkable_count > 0) {
            rest = -rest;
            int32_t loop_index = 0;

            while (rest > 0 && loop_index < 10) {
                if (rest < shrinkable_count) {
                    for (int32_t i = 0; i < m_view_count; i++) {
                        auto item = itemAtIndex(i);
                        if (item->canShrink()) {
                            item->setSize(item->size_ - 1);
                            rest--;
                        }
                        if (rest < 1) {
                            break;
                        }
                    }
                    rest = 0;
                }
                else {
                    total_view_size = 0;
                    for (int32_t i = 0; i < m_view_count; i++) {
                        auto item = itemAtIndex(i);
                        if (item->canShrink()) {
                            item->setSize(item->size_ - rest / shrinkable_count);
                        }
                        total_view_size += item->size_;
                    }
                    rest = total_view_size - available_size;
                }
                loop_index++;
            }
            updateRectOfAllViews();
        }
    }


    void SplitView::handleMouseDown(const Event& event) noexcept {
        m_prev_mouse_pos = isVertical() ? event.mouseY() : event.mouseX();
        m_divider_index = -1;

        for (int32_t i = 1; i < m_view_count && m_divider_index < 0; i++) {

            auto item = itemAtIndex(i);
            Rectd view_rect = item->view_->rect();

            if (isVertical()) {
                if (event.mouseY() < view_rect.y()) {
                    m_divider_index = i - 1;
                }
            }
            else {
                if (event.mouseX() < view_rect.x()) {
                    m_divider_index = i - 1;
                }
            }
        }

        if (m_divider_index >= 0) {
            m_item_a = itemAtIndex(m_divider_index);
            m_item_b = itemAtIndex(m_divider_index + 1);

            int32_t delta_min_a = m_item_a->min_ - m_item_a->size_;
            int32_t delta_max_a = m_item_a->max_ - m_item_a->size_;
            int32_t delta_min_b = m_item_b->size_ - m_item_b->max_;
            int32_t delta_max_b = m_item_b->size_ - m_item_b->min_;
            m_divider_delta_min = std::max(delta_min_a, delta_min_b);
            m_divider_delta_max = std::min(delta_max_a, delta_max_b);

            // Remember sizes and positions.
            m_item_a_size = m_item_a->size_;
            m_item_b_size = m_item_b->size_;
        }
    }


    void SplitView::handleMouseDrag(const Event& event) noexcept {
        if (m_divider_index >= 0 && m_divider_index < m_view_count) {
            double new_mouse_pos = isVertical() ? event.mouseY() : event.mouseX();
            int32_t delta = static_cast<int32_t>(std::round(new_mouse_pos - m_prev_mouse_pos));
            delta = std::clamp<int32_t>(delta, m_divider_delta_min, m_divider_delta_max);

            m_item_a->size_ = m_item_a_size + delta;
            m_item_b->size_ = m_item_b_size - delta;

            updateRectOfAllViews();
            updateRealPositions();
        }
    }


    void SplitView::handleMouseUp(const Event& event) noexcept {
        m_divider_index = -1;
    }


    void SplitView::handleMouseEntered(const Event& event) noexcept {
    }


    void SplitView::handleMouseExited(const Event& event) noexcept {
    }


    SplitViewItem* SplitView::itemByView(Component* component) noexcept {
        for (int32_t i = 0; i < m_view_count; i++) {
            auto item = itemAtIndex(i);
            if ((Component*)item->view_ == component) {
                return item;
            }
        }
        return nullptr;
    }


    View* SplitView::viewAtIndex(int32_t index) const noexcept {
        return isViewIndex(index) ? m_items[index].view_ : nullptr;
    }


    Rectd SplitView::getViewRect(int32_t index) const noexcept {
        return isViewIndex(index) ? m_items[index].view_->rect() : Rectd();
    }


    int32_t SplitView::totalSizeOfViews() const noexcept {
        double total_size = 0.0;

        for (int32_t i = 0; i < m_view_count; i++) {
            total_size += isVertical() ? m_items[i].view_->height() : m_items[i].view_->width();
        }

        return static_cast<int32_t>(total_size);
    }


    void SplitView::setVertical(bool vertical) noexcept {
        if (vertical != m_vertical) {
            m_vertical = vertical;
            updateRectOfAllViews();
            if (m_vertical == true) {
                // setMouseCursor(App::MouseCursor::ResizeUpDown); TODO: !!!!!
            }
            else {
                // setMouseCursor(App::MouseCursor::ResizeLeftRight); TODO: !!!!!
            }
        }
    }


    void SplitView::setViewLimits(int32_t index, int32_t min, int32_t max) noexcept {
        if (SplitViewItem* item = itemAtIndex(index)) {
            m_items[index].min_ = min;
            m_items[index].max_ = max;
            updateRectOfAllViews();
        }
    }


    void SplitView::setViewSize(int32_t index, int32_t size) noexcept {
        if (SplitViewItem* item = itemAtIndex(index)) {
            m_items[index].size_ = size;
            updateRectOfAllViews();
        }
    }


    View* SplitView::addView() noexcept {
        View* view = nullptr;

        if (m_view_count < kMaxViewCount) {
            view = View::addView(Rectd(0, 0, 100, 100));

            if (view != nullptr) {
                view->setSplitViewFlag(true);
                m_items[m_view_count].view_ = view;
                m_view_count++;
                updateRectOfAllViews();
            }
        }

        return view;
    }

/* TODO: !!!!
    void SplitView::removeView(View* view) noexcept {
         int32_t index = 0;

        while (auto v = viewAtIndex(index)) {
            if (v == view) {
                // [view->getNSView() removeFromSuperview];
                m_views->removeRef(view);
                delete view;
                break;
            }
            index++;
        }
    }
*/

    void SplitView::initLayout() noexcept {
        int32_t available_size = availableSize();
        int32_t total_view_size = totalSizeOfViews();

        int32_t grow_potential;
        int32_t shrink_potential;
        int32_t growable_count;
        int32_t shrinkable_count;

        viewResizePotential(grow_potential, shrink_potential, growable_count, shrinkable_count);

        int32_t rest = available_size - total_view_size;

        if (growable_count > 1) {

            int32_t loop_index = 0;
            while (rest > 0 && loop_index < 10) {
                if (rest < growable_count) {
                    for (int32_t i = 0; i < m_view_count; i++) {
                        auto item = itemAtIndex(i);
                        if (item->canGrow()) {
                            item->setSize(item->size_ + 1);
                            rest--;
                        }
                        if (rest < 1) {
                            break;
                        }
                    }
                }
                else {
                    total_view_size = 0;
                    for (int32_t i = 0; i < m_view_count; i++) {
                        auto item = itemAtIndex(i);
                        if (item->canGrow()) {
                            item->setSize(item->size_ + rest / growable_count);
                        }
                        total_view_size += item->size_;
                    }

                    rest = available_size - total_view_size;
                }
                loop_index++;
            }
        }

        updateRealPositions();
        updateRectOfAllViews();
    }


    void SplitView::updateRectOfAllViews() noexcept {
        int32_t pos = 0;

        for (int32_t i = 0; i < m_view_count; i++) {
            auto item = itemAtIndex(i);

            if (isVertical()) {
                item->view_->setRect(Rectd(0, pos, width(), item->size_));
            }
            else {
                item->view_->setRect(Rectd(pos, 0, item->size_, height()));
            }

            pos += item->size_ + m_divider_size;
        }
    }


    void SplitView::viewResizePotential(int32_t& out_grow_potential, int32_t& out_shrink_potential, int32_t& out_growable, int32_t& out_shrinkable) noexcept {
        out_grow_potential = 0;
        out_shrink_potential = 0;
        out_growable = 0;
        out_shrinkable = 0;

        for (int32_t i = 0; i < m_view_count; i++) {
            auto item = itemAtIndex(i);

            if (item->canGrow()) {
                out_growable++;
            }

            if (item->canShrink()) {
                out_shrinkable++;
            }

            out_grow_potential += item->max_ - item->size_;
            out_shrink_potential += item->size_ - item->min_;
        }
    }


    void SplitView::updateRealPositions() noexcept {
        int32_t available_size = availableSize();
        double p = 0.0;

        for (int32_t i = 0; i < m_view_count; i++) {
            auto item = itemAtIndex(i);

            item->real_pos_ = p / available_size;
            item->real_size_ = static_cast<double>(item->size_) / available_size;
            item->view_->needsDisplay();

            p += item->size_;
        }
    }


} // End of namespace Grain.
