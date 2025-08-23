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
        int32_t m_size = 40;
        int32_t m_min = 40;
        int32_t m_max = 10000;
        View* m_view = nullptr;
        double m_real_pos = 0;
        double m_real_size = 0;

    public:
        int32_t size() const noexcept { return m_size; }
        double realSize() const noexcept { return m_real_size; }
        void limitSize() noexcept { if (m_size < m_min) m_size = m_min; else if (m_size > m_max) m_size = m_max; }
        void setSize(double size) noexcept { m_size = size < m_min ? m_min : (size > m_max ? m_max : size); }
        bool canShrink() noexcept { return m_min < m_size; }
        bool canGrow() noexcept { return m_max > m_size; }
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
        SplitView(const Rectd& rect, int32_t tag = 0) noexcept;
        ~SplitView() noexcept;

        const char* className() const noexcept override { return "SplitView"; }

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
        [[nodiscard]] bool isVertical() const noexcept { return m_vertical; }
        [[nodiscard]] bool isHorizontal() const noexcept { return !m_vertical; }
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
