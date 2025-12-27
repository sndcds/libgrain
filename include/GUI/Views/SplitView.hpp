//
//  SpitView.hpp
//
//  Created by Roald Christesen on from 05.08.2012.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 18.08.2025
//

#ifndef GrSplitView_hpp
#define GrSplitView_hpp

#include "Grain.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Components/Component.hpp"


namespace Grain {

    class GrRGB;


    class SplitViewItem {
        friend class SplitView;

    protected:
        int32_t size_ = 40;
        int32_t min_ = 40;
        int32_t max_ = 10000;
        View* view_ = nullptr;
        double real_pos_ = 0.0;
        double real_size_ = 0.0;

    public:
        int32_t size() const noexcept { return size_; }
        double realSize() const noexcept { return real_size_; }
        void limitSize() noexcept { if (size_ < min_) size_ = min_; else if (size_ > max_) size_ = max_; }
        void setSize(double size) noexcept { size_ = size < min_ ? min_ : (size > max_ ? max_ : size); }
        bool canShrink() noexcept { return min_ < size_; }
        bool canGrow() noexcept { return max_ > size_; }
    };


    class SplitView : public View {

    public:
        enum {
            kMaxViewCount = 10,
            kMaxDividerCount = kMaxViewCount - 1
        };

    protected:
        int32_t m_view_count = 0;               ///< Number of views
        bool m_vertical = false;                ///< Direction, vertical or horizontal
        int32_t m_divider_size = 8;             ///< Size of the divider, between the views

        SplitViewItem m_items[kMaxViewCount];   ///< Item data
        SplitViewItem* m_item_a = nullptr;
        SplitViewItem* m_item_b = nullptr;

        int32_t m_divider_index = -1;           ///< Index of active divider, for dragging etc.
        double m_prev_mouse_pos;                ///< The previous position of mouse, for dragging etc.
        int32_t m_divider_delta = 0;
        int32_t m_divider_delta_min = 0;
        int32_t m_divider_delta_max = 0;
        int32_t m_item_a_size = 0;
        int32_t m_item_b_size = 0;

        bool m_must_init = true;

    public:
        explicit SplitView(const Rectd& rect, int32_t tag = 0) noexcept;
        virtual ~SplitView() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "SplitView"; }

        static SplitView* add(View* view, int32_t tag = 0) { return add(view, Rectd(), tag); }
        static SplitView* add(View* view, const Rectd& rect, int32_t tag = 0);

        void draw(const Rectd& dirty_rect) noexcept override;

        [[nodiscard]] bool hasDescendant(const Component* component) noexcept override;

        void parentGeometryChanged() noexcept override;
        void geometryChanged() noexcept override;

        void handleMouseDown(const Event& event) noexcept override;
        void handleMouseDrag(const Event& event) noexcept override;
        void handleMouseUp(const Event& event) noexcept override;
        void handleMouseEntered(const Event& event) noexcept override;
        void handleMouseExited(const Event& event) noexcept override;

        [[nodiscard]] SplitViewItem* itemAtIndex(int32_t index) noexcept { return isViewIndex(index) ? &m_items[index] : nullptr; }
        [[nodiscard]] SplitViewItem* itemByView(Component* component) noexcept;
        [[nodiscard]] int32_t viewCount() const noexcept { return m_view_count; }
        [[nodiscard]] bool isViewIndex(int32_t index) const noexcept { return index >= 0 && index < viewCount(); }
        [[nodiscard]] bool isVertical() const noexcept override { return m_vertical; }
        [[nodiscard]] bool isHorizontal() const noexcept override { return !m_vertical; }
        [[nodiscard]] View* viewAtIndex(int32_t index) const noexcept;
        [[nodiscard]] Rectd getViewRect(int32_t index) const noexcept;
        [[nodiscard]] int32_t dividerSize() const noexcept { return m_divider_size; }
        [[nodiscard]] int32_t availableSize() const noexcept { return size(isVertical()) - (m_view_count - 1) * m_divider_size; }
        [[nodiscard]] int32_t splitViewSize() const noexcept { return size(isVertical()); }
        [[nodiscard]] int32_t totalSizeOfViews() const noexcept;

        void setVertical(bool vertical) noexcept;
        void setVertical() noexcept { setVertical(true); }
        void setHorizontal() noexcept { setVertical(false); }
        void setViewLimits(int32_t index, int32_t limit) noexcept { setViewLimits(index, limit, limit); }
        void setViewLimits(int32_t index, int32_t min, int32_t max) noexcept;
        void setViewSize(int32_t index, int32_t size) noexcept;

        View* addView() noexcept override;
        // void removeView(View* view) noexcept override; TODO: !!!!!

        void initLayout() noexcept;
        void updateRectOfAllViews() noexcept;

        void viewResizePotential(int32_t& out_grow_potential, int32_t& out_shrink_potential, int32_t& out_growable, int32_t& out_shrinkable) noexcept;
        void updateRealPositions() noexcept;
    };

} // End of namespace Grain

#endif // GrSplitView_hpp
