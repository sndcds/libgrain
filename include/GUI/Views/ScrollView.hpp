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

    protected:
        bool must_update_ = true;
    public:
        ScrollAreaView() noexcept;

        virtual bool update() noexcept { return true; }
        void mustUpdate() noexcept { must_update_ = true; }
        void updateDone() noexcept { must_update_ = false; }
        [[nodiscard]] bool isUpdateNeeded() noexcept { return must_update_; }
        void forcedUpdate() noexcept { mustUpdate(); update(); }

    };


    class ScrollView : public View {
    public:
        explicit ScrollView(const Rectd& rect) noexcept;
        ~ScrollView() noexcept override;

        void _init(Component* parent, const Rectd& rect) noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "ScrollView"; }

        [[nodiscard]] static ScrollView* add(View* view) { return add(view, Rectd()); }
        [[nodiscard]] static ScrollView* add(View* view, const Rectd &rect);

        void setScrollAreaView(ScrollAreaView* view) noexcept;

        [[nodiscard]] double scrollAreaWidth() const { return scroll_area_view_->width(); }
        [[nodiscard]] double scrollAreaHeight() const { return scroll_area_view_->height(); }
        [[nodiscard]] View* scrollAreaViewPtr() const { return scroll_area_view_; }

        [[nodiscard]] int32_t contentXOffset() const { return content_x_offset_; }
        [[nodiscard]] int32_t contentYOffset() const { return content_y_offset_; }
        [[nodiscard]] Vec2d contentOffset() const { return Vec2d(content_x_offset_, content_y_offset_); }

        void setContentView(View *content_view) noexcept;
        void removeContentView(View *content_view) noexcept;

        void setContentDimension(double width, double height);
        void setScrollPosition(double x, double y);
        void setScrollXPosition(double x) { setScrollPosition(x, std::numeric_limits<double>::max()); }
        void setScrollYPosition(double y) { setScrollPosition(std::numeric_limits<double>::max(), y); }

        void setOffset(double x_offset, double y_offset);

        [[nodiscard]] bool canScrollHorizontal() const { return can_h_scroll_; }
        [[nodiscard]] bool canScrollVertical() const { return can_v_scroll_; }
        void setCanScrollHorizontal(bool flag) {
            can_h_scroll_ = flag;
            if (scroll_area_view_ != nullptr) {
                scroll_area_view_->mustUpdate();
            }
            needsDisplay();
        }
        void setCanScrollVertical(bool flag) {
            can_v_scroll_ = flag;
            if (scroll_area_view_ != nullptr) {
                scroll_area_view_->mustUpdate();
            }
            needsDisplay();
        }
        void disableHorizontal() { setCanScrollHorizontal(false); }
        void disableVertical() { setCanScrollVertical(false); }
        void enableHorizontal() { setCanScrollHorizontal(true); }
        void enableVertical() { setCanScrollVertical(true); }

        void setScrollBarColor(const RGB& color) noexcept;

        void geometryChanged() noexcept override;

        void handleScrollWheel(const Event &event) noexcept override;
        void setByComponent(Component *component) noexcept override;

    protected:
        ScrollAreaView* scroll_area_view_ = nullptr;
        View* content_view_ = nullptr;
        ScrollBar* h_scroll_bar_ = nullptr;
        ScrollBar* v_scroll_bar_ = nullptr;
        ObjectList<View*> views_;

        int32_t content_width_ = 640;
        int32_t content_height_ = 480;
        int32_t content_x_offset_ = 0;
        int32_t content_y_offset_ = 0;
        float scroll_wheel_speed_ = 1.0;
        bool can_h_scroll_ = true;
        bool can_v_scroll_ = true;
    };

} // End of namespace Grain

#endif // GrainScrollView_hpp
