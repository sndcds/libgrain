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
        delete m_scroll_area_view;
        delete m_horizontal_scroll_bar;
        delete m_vertical_scroll_bar;
    }


    ScrollView* ScrollView::add(View* view, const Rectd& rect) {
        return (ScrollView*)Component::addComponentToView((Component*)new (std::nothrow) ScrollView(rect), view, AddFlags::kWantsLayer);
    }


    void ScrollView::_init(Component* parent, const Rectd& rect) noexcept {
        m_type = ComponentType::ScrollView;
        m_scroll_wheel_speed = App::scrollWheelSpeed();

        setScrollAreaView(new (std::nothrow) ScrollAreaView());

        m_horizontal_scroll_bar = (ScrollBar*)addComponent(new (std::nothrow) ScrollBar(Rectd(), false), AddFlags::kNone);
        m_horizontal_scroll_bar->setReceiverComponent(this);
        m_horizontal_scroll_bar->setVisibility(false);

        m_vertical_scroll_bar = (ScrollBar*)addComponent(new (std::nothrow) ScrollBar(Rectd(), true), AddFlags::kNone);
        m_vertical_scroll_bar->setReceiverComponent(this);
        m_vertical_scroll_bar->setVisibility(false);

        setRect(rect);
    }


    void ScrollView::setScrollAreaView(ScrollAreaView* view) noexcept {
        if (view != m_scroll_area_view) {
            removeComponent(m_scroll_area_view);
            // delete m_scroll_area_view;    // TODO:!!!!
            addComponent(view, AddFlags::kWantsLayer);
            m_scroll_area_view = view;
            if (view != nullptr) {
                view->setDimension(width(), height());
            }
        }
    }


    void ScrollView::setContentView(View* content_view) noexcept {
        std::cout << "setContentView 1\n";
        if (m_scroll_area_view != nullptr && content_view != m_content_view) {
            std::cout << "setContentView 2\n";
            bool view_in_array_flag = false;

            for (auto view : m_views) {
                if (view == content_view) {
                    view->setVisibility(true);
                    view_in_array_flag = true;
                }
                else {
                    view->setVisibility(false);
                }
            }

            if (!view_in_array_flag) {
                std::cout << "setContentView 3\n";
                m_views.push(content_view);
                m_scroll_area_view->addComponent(content_view, AddFlags::kWantsLayer);
            }

            m_content_view = content_view;
            if (content_view) {
                std::cout << "setContentView 4\n";
                content_view->setPosition(0.0, 0.0);
                m_content_width = content_view->width();
                m_content_height = content_view->height();
                geometryChanged();
                needsDisplay();
            }
        }

        std::cout << "setContentView 5\n";
        setScrollPosition(0, 0);
    }


    void ScrollView::removeContentView(View* content_view) noexcept {
        for (auto view : m_views) {
            if (view == content_view) {
                m_views.removeElement(content_view);
                if (content_view == m_content_view) {
                    m_content_view = nullptr;
                }
                break;
            }
        }
    }


    void ScrollView::setContentDimension(double width, double height) {
        m_content_width = width;
        m_content_height = height;

        if (m_content_view != nullptr) {
            m_content_view->setDimension(width, height);
        }

        geometryChanged();
        needsDisplay();
    }


    void ScrollView::setScrollPosition(double x, double y) {
        Rectd content_rect;

        if (m_content_view != nullptr) {
            content_rect = m_content_view->rect();
            m_content_width = content_rect.m_width;
            m_content_height = content_rect.m_height;
        }

        bool x_changed = false;
        bool y_changed = false;

        if (m_can_scroll_horizontal == true && x < std::numeric_limits<double>::max()) {
            x = std::clamp<double>(x, 0.0, 1.0);

            if (m_horizontal_scroll_bar != nullptr) {
                m_horizontal_scroll_bar->setScrollPosition(x);
            }

            double x_offset = x * (m_content_width - scrollAreaWidth());

            if (x_offset != m_content_x_offset) {
                m_content_x_offset = x_offset;
                x_changed = true;

                if (m_content_view != nullptr) {
                    content_rect.m_x = -m_content_x_offset;
                }
            }
        }

        if (m_can_scroll_vertical == true && y < std::numeric_limits<double>::max()) {
            y = std::clamp<double>(y, 0.0, 1.0);

            if (m_vertical_scroll_bar != nullptr) {
                m_vertical_scroll_bar->setScrollPosition(y);
            }

            double y_offset = y * (m_content_height - scrollAreaHeight());

            if (y_offset != m_content_y_offset) {
                m_content_y_offset = y_offset;
                y_changed = true;

                if (m_content_view != nullptr) {
                    content_rect.m_y = -m_content_y_offset;
                }
            }
        }

        if (m_content_view != nullptr && (x_changed || y_changed)) {
            m_content_view->setRect(content_rect);
        }
        else {
            m_scroll_area_view->needsDisplay();
        }
    }


    void ScrollView::setOffset(double x_offset, double y_offset) {
        double max_x = m_content_width - scrollAreaWidth();
        double max_y = m_content_height - scrollAreaHeight();

        x_offset = std::clamp<double>(x_offset, 0, max_x);
        y_offset = std::clamp<double>(y_offset, 0, max_y);

        double x = canScrollHorizontal() && max_x > FLT_EPSILON ? x_offset / max_x : 0;
        double y = canScrollVertical() && max_y > FLT_EPSILON ? y_offset / max_y : 0;

        setScrollPosition(x, y);
    }


    void ScrollView::setScrollBarColor(const RGB& color) noexcept {
        if (m_horizontal_scroll_bar != nullptr) {
            // m_horizontal_scroll_bar->setUiColor(UIColor::Bg, color); TODO; !!!!
        }

        if (m_vertical_scroll_bar != nullptr) {
            // m_vertical_scroll_bar->setUiColor(UIColor::Bg, color); TODO; !!!!
        }
    }


    void ScrollView::geometryChanged() noexcept {
        if (m_content_view != nullptr) {
            m_content_width = m_content_view->width();
            m_content_height = m_content_view->height();
        }

        Rectd bounds_rect = boundsRect();

        double scroll_area_width = width();
        double scroll_area_height = height();

        bool h_scroll_bar_flag = m_content_width > scroll_area_width && m_can_scroll_horizontal;
        bool v_scroll_bar_flag = m_content_height > scroll_area_height && m_can_scroll_vertical;

        if (h_scroll_bar_flag == true) {
            scroll_area_height -= m_horizontal_scroll_bar->barSize();
            if (m_content_height > scroll_area_height) {
                v_scroll_bar_flag = true;
            }
        }

        if (v_scroll_bar_flag == true) {
            scroll_area_width -= m_horizontal_scroll_bar->barSize();
        }

        m_scroll_area_view->setDimension(scroll_area_width, scroll_area_height);

        double h_bar_size = h_scroll_bar_flag ? m_horizontal_scroll_bar->barSize() : 0;
        double v_bar_size = v_scroll_bar_flag ? m_vertical_scroll_bar->barSize() : 0;

        m_horizontal_scroll_bar->setVisibility(h_scroll_bar_flag);
        if (h_scroll_bar_flag == true) {
            m_horizontal_scroll_bar->setRect({ 0, bounds_rect.m_height - h_bar_size, bounds_rect.m_width - v_bar_size, h_bar_size });
            m_horizontal_scroll_bar->setVisibleFraction(m_content_width, scroll_area_width);
        }

        m_vertical_scroll_bar->setVisibility(v_scroll_bar_flag);
        if (v_scroll_bar_flag == true) {
            m_vertical_scroll_bar->setRect({ bounds_rect.m_width - v_bar_size, 0, v_bar_size, bounds_rect.m_height - h_bar_size });
            m_vertical_scroll_bar->setVisibleFraction(m_content_height, scroll_area_height);
        }


        double max_x_offset = std::clamp<double>(m_content_width - scroll_area_width, 0, m_content_width - 1);
        double max_y_offset = std::clamp<double>(m_content_height - scroll_area_height, 0, m_content_height - 1);

        if (m_content_x_offset > max_x_offset) {
            m_content_x_offset = max_x_offset;
        }

        if (m_content_y_offset > max_y_offset) {
            m_content_y_offset = max_y_offset;
        }

        if (m_content_view != nullptr) {
            m_content_view->setPosition(-m_content_x_offset, -m_content_y_offset);
        }

        m_horizontal_scroll_bar->setScrollPosition(max_x_offset > FLT_EPSILON ? m_content_x_offset / max_x_offset : 0);
        m_vertical_scroll_bar->setScrollPosition(max_y_offset > FLT_EPSILON ? m_content_y_offset / max_y_offset : 0);
    }


    void ScrollView::handleScrollWheel(const Event& event) noexcept {

        Vec3d delta = event.delta() * m_scroll_wheel_speed;
        setOffset(m_content_x_offset - delta.m_x, m_content_y_offset - delta.m_y);
    }


    void ScrollView::setByComponent(Component* component) noexcept {

        if (component != nullptr) {
            if (component->componentType() == ComponentType::ScrollBar) {
                ScrollBar* scroll_bar = (ScrollBar*)component;
                double position = scroll_bar->scrollPosition();
                scroll_bar->isVertical() ? setScrollYPosition(position) : setScrollXPosition(position);
            }
        }
    }


} // End of namespace Grain
