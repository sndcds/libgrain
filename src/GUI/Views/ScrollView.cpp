//
//  ScrollBar.cpp
//
//  Created by Roald Christesen on from 19.12.2015.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Views/ScrollView.hpp"
#include "GUI/Components/ScrollBar.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"
#include "App/App.hpp"


namespace Grain {

    ScrollAreaView::ScrollAreaView() noexcept : View(Rectd(0, 0, 100, 100)) {
    }

    ScrollView::ScrollView(const Rectd& rect) noexcept : View(rect) {
        _init(nullptr, rect);
    }

    ScrollView::~ScrollView() noexcept {
        delete scroll_area_view_;
        delete h_scroll_bar_;
        delete v_scroll_bar_;
    }

    ScrollView* ScrollView::add(View* view, const Rectd& rect) {
        return (ScrollView*)Component::addComponentToView((Component*)new (std::nothrow) ScrollView(rect), view, AddFlags::kWantsLayer);
    }

    void ScrollView::_init(Component* parent, const Rectd& rect) noexcept {
        type_ = ComponentType::ScrollView;
        scroll_wheel_speed_ = App::scrollWheelSpeed();

        setScrollAreaView(new (std::nothrow) ScrollAreaView());

        h_scroll_bar_ = (ScrollBar*)addComponent(new (std::nothrow) ScrollBar(Rectd(), false), AddFlags::kNone);
        h_scroll_bar_->setReceiverComponent(this);
        h_scroll_bar_->setVisibility(false);

        v_scroll_bar_ = (ScrollBar*)addComponent(new (std::nothrow) ScrollBar(Rectd(), true), AddFlags::kNone);
        v_scroll_bar_->setReceiverComponent(this);
        v_scroll_bar_->setVisibility(false);

        setRect(rect);
    }


    void ScrollView::setScrollAreaView(ScrollAreaView* view) noexcept {
        if (view != scroll_area_view_) {
            removeComponent(scroll_area_view_);
            // delete m_scroll_area_view;    // TODO:!!!!
            addComponent(view, AddFlags::kWantsLayer);
            scroll_area_view_ = view;
            if (view != nullptr) {
                view->setDimension(width(), height());
            }
        }
    }

    void ScrollView::setContentView(View* content_view) noexcept {
        if (scroll_area_view_ != nullptr && content_view != content_view_) {
            bool view_in_array_flag = false;

            for (auto view : views_) {
                if (view == content_view) {
                    view->setVisibility(true);
                    view_in_array_flag = true;
                }
                else {
                    view->setVisibility(false);
                }
            }

            if (!view_in_array_flag) {
                views_.push(content_view);
                scroll_area_view_->addComponent(content_view, AddFlags::kWantsLayer);
            }

            content_view_ = content_view;
            if (content_view) {
                content_view->setPosition(0.0, 0.0);
                content_width_ = content_view->width();
                content_height_ = content_view->height();
                geometryChanged();
                needsDisplay();
            }
        }

        setScrollPosition(0, 0);
    }

    void ScrollView::removeContentView(View* content_view) noexcept {
        for (auto view : views_) {
            if (view == content_view) {
                views_.removeElement(content_view);
                if (content_view == content_view_) {
                    content_view_ = nullptr;
                }
                break;
            }
        }
    }


    void ScrollView::setContentDimension(double width, double height) {
        content_width_ = width;
        content_height_ = height;

        if (content_view_ != nullptr) {
            content_view_->setDimension(width, height);
        }

        geometryChanged();
        needsDisplay();
    }


    void ScrollView::setScrollPosition(double x, double y) {
        Rectd content_rect;

        if (content_view_ != nullptr) {
            content_rect = content_view_->rect();
            content_width_ = content_rect.width_;
            content_height_ = content_rect.height_;
        }

        bool x_changed = false;
        bool y_changed = false;

        if (can_h_scroll_ == true && x < std::numeric_limits<double>::max()) {
            x = std::clamp<double>(x, 0.0, 1.0);

            if (h_scroll_bar_ != nullptr) {
                h_scroll_bar_->setScrollPosition(x);
            }

            double x_offset = x * (content_width_ - scrollAreaWidth());

            if (x_offset != content_x_offset_) {
                content_x_offset_ = x_offset;
                x_changed = true;

                if (content_view_ != nullptr) {
                    content_rect.x_ = -content_x_offset_;
                }
            }
        }

        if (can_v_scroll_ == true && y < std::numeric_limits<double>::max()) {
            y = std::clamp<double>(y, 0.0, 1.0);

            if (v_scroll_bar_ != nullptr) {
                v_scroll_bar_->setScrollPosition(y);
            }

            double y_offset = y * (content_height_ - scrollAreaHeight());

            if (y_offset != content_y_offset_) {
                content_y_offset_ = y_offset;
                y_changed = true;

                if (content_view_ != nullptr) {
                    content_rect.y_ = -content_y_offset_;
                }
            }
        }

        if (content_view_ != nullptr && (x_changed || y_changed)) {
            content_view_->setRect(content_rect);
        }
        else {
            scroll_area_view_->needsDisplay();
        }
    }


    void ScrollView::setOffset(double x_offset, double y_offset) {
        double max_x = content_width_ - scrollAreaWidth();
        double max_y = content_height_ - scrollAreaHeight();

        x_offset = std::clamp<double>(x_offset, 0, max_x);
        y_offset = std::clamp<double>(y_offset, 0, max_y);

        double x = canScrollHorizontal() && max_x > FLT_EPSILON ? x_offset / max_x : 0;
        double y = canScrollVertical() && max_y > FLT_EPSILON ? y_offset / max_y : 0;

        setScrollPosition(x, y);
    }


    void ScrollView::setScrollBarColor(const RGB& color) noexcept {
        if (h_scroll_bar_ != nullptr) {
            // m_horizontal_scroll_bar->setUiColor(UIColor::Bg, color); TODO; !!!!
        }

        if (v_scroll_bar_ != nullptr) {
            // m_vertical_scroll_bar->setUiColor(UIColor::Bg, color); TODO; !!!!
        }
    }


    void ScrollView::geometryChanged() noexcept {
        if (content_view_ != nullptr) {
            content_width_ = content_view_->width();
            content_height_ = content_view_->height();
        }

        Rectd bounds_rect = boundsRect();

        double scroll_area_width = width();
        double scroll_area_height = height();

        bool h_scroll_bar_flag = content_width_ > scroll_area_width && can_h_scroll_;
        bool v_scroll_bar_flag = content_height_ > scroll_area_height && can_v_scroll_;

        if (h_scroll_bar_flag == true) {
            scroll_area_height -= h_scroll_bar_->barSize();
            if (content_height_ > scroll_area_height) {
                v_scroll_bar_flag = true;
            }
        }

        if (v_scroll_bar_flag == true) {
            scroll_area_width -= h_scroll_bar_->barSize();
        }

        scroll_area_view_->setDimension(scroll_area_width, scroll_area_height);

        double h_bar_size = h_scroll_bar_flag ? h_scroll_bar_->barSize() : 0;
        double v_bar_size = v_scroll_bar_flag ? v_scroll_bar_->barSize() : 0;

        h_scroll_bar_->setVisibility(h_scroll_bar_flag);
        if (h_scroll_bar_flag == true) {
            h_scroll_bar_->setRect({ 0, bounds_rect.height_ - h_bar_size, bounds_rect.width_ - v_bar_size, h_bar_size });
            h_scroll_bar_->setVisibleFraction(content_width_, scroll_area_width);
        }

        v_scroll_bar_->setVisibility(v_scroll_bar_flag);
        if (v_scroll_bar_flag == true) {
            v_scroll_bar_->setRect({ bounds_rect.width_ - v_bar_size, 0, v_bar_size, bounds_rect.height_ - h_bar_size });
            v_scroll_bar_->setVisibleFraction(content_height_, scroll_area_height);
        }


        double max_x_offset = std::clamp<double>(content_width_ - scroll_area_width, 0, content_width_ - 1);
        double max_y_offset = std::clamp<double>(content_height_ - scroll_area_height, 0, content_height_ - 1);

        if (content_x_offset_ > max_x_offset) {
            content_x_offset_ = max_x_offset;
        }

        if (content_y_offset_ > max_y_offset) {
            content_y_offset_ = max_y_offset;
        }

        if (content_view_ != nullptr) {
            content_view_->setPosition(-content_x_offset_, -content_y_offset_);
        }

        h_scroll_bar_->setScrollPosition(max_x_offset > FLT_EPSILON ? content_x_offset_ / max_x_offset : 0);
        v_scroll_bar_->setScrollPosition(max_y_offset > FLT_EPSILON ? content_y_offset_ / max_y_offset : 0);
    }

    void ScrollView::handleScrollWheel(const Event& event) noexcept {

        Vec3d delta = event.delta() * scroll_wheel_speed_;
        setOffset(content_x_offset_ - delta.x_, content_y_offset_ - delta.y_);
    }

    void ScrollView::setByComponent(Component* component) noexcept {
        if (component && component->componentType() == ComponentType::ScrollBar) {
            auto scroll_bar = (ScrollBar*)component;
            double position = scroll_bar->scrollPosition();
            scroll_bar->isVertical() ? setScrollYPosition(position) : setScrollXPosition(position);
        }
    }

} // End of namespace Grain
