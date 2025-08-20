//
//  ScrollBar.hpp
//
//  Created by Roald Christesen on from 19.12.2015.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 18.08.2025
//

#ifndef GrainScrollView_hpp
#define GrainScrollView_hpp

#include "GUI/Views/View.hpp"


namespace Grain {

    class ScrollBar;


    class ScrollAreaView : public View {
    public:
        ScrollAreaView() noexcept;

        virtual bool update() noexcept { return true; }
        void mustUpdate() noexcept { m_must_update = true; }
        void updateDone() noexcept { m_must_update = false; }
        bool isUpdateNeeded() noexcept { return m_must_update; }
        void forcedUpdate() noexcept { mustUpdate(); update(); }

    protected:
        bool m_must_update = true;
    };


    class ScrollView : public View {
    public:
        ScrollView(const Rectd& rect) noexcept;
        ~ScrollView() noexcept;

        void _init(Component* parent, const Rectd& rect) noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "ScrollView"; }


        [[nodiscard]] static ScrollView* add(View* view) { return add(view, Rectd()); }
        [[nodiscard]] static ScrollView* add(View* view, const Rectd &rect);


        void setScrollAreaView(ScrollAreaView* view) noexcept;

        [[nodiscard]] double scrollAreaWidth() const noexcept { return m_scroll_area_view->width(); }
        [[nodiscard]] double scrollAreaHeight() const noexcept { return m_scroll_area_view->height(); }

        [[nodiscard]] int32_t contentXOffset() { return m_content_x_offset; }
        [[nodiscard]] int32_t contentYOffset() { return m_content_y_offset; }
        [[nodiscard]] Vec2d contentOffset() noexcept { return Vec2d(m_content_x_offset, m_content_y_offset); }

        void setContentView(View *content_view) noexcept;
        void removeContentView(View *content_view) noexcept;

        void setContentDimension(double width, double height);
        void setScrollPosition(double x, double y);
        void setScrollXPosition(double x) { setScrollPosition(x, std::numeric_limits<double>::max()); }
        void setScrollYPosition(double y) { setScrollPosition(std::numeric_limits<double>::max(), y); }

        void setOffset(double x_offset, double y_offset);

        [[nodiscard]] bool canScrollHorizontal() { return m_can_scroll_horizontal; }
        [[nodiscard]] bool canScrollVertical() { return m_can_scroll_vertical; }
        void setCanScrollHorizontal(bool flag) {
            m_can_scroll_horizontal = flag;
            if (m_scroll_area_view != nullptr) {
                m_scroll_area_view->mustUpdate();
            }
            needsDisplay();
        }
        void setCanScrollVertical(bool flag) {
            m_can_scroll_vertical = flag;
            if (m_scroll_area_view != nullptr) {
                m_scroll_area_view->mustUpdate();
            }
            needsDisplay();
        }
        void disableHorizontal() { setCanScrollHorizontal(false); }
        void disableVertical() { setCanScrollVertical(false); }
        void enableHorizontal() { setCanScrollHorizontal(true); }
        void enableVertical() { setCanScrollVertical(true); }

        void setScrollBarColor(const RGB &color) noexcept;

        void geometryChanged() noexcept override;

        void handleScrollWheel(const Event &event) noexcept override;
        void setByComponent(Component *component) noexcept override;

    protected:
        ScrollAreaView* m_scroll_area_view = nullptr;
        View* m_content_view = nullptr;
        ScrollBar* m_horizontal_scroll_bar = nullptr;
        ScrollBar* m_vertical_scroll_bar = nullptr;
        ObjectList<View*> m_views;

        int32_t m_content_width = 640;
        int32_t m_content_height = 480;
        int32_t m_content_x_offset = 0;
        int32_t m_content_y_offset = 0;
        float m_scroll_wheel_speed = 1.0f;
        bool m_can_scroll_horizontal = true;
        bool m_can_scroll_vertical = true;
    };


} // End of namespace Grain

#endif // GrainScrollView_hpp
