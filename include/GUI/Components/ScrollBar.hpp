//
//  ScrollBar.hpp
//
//  Created by Roald Christesen on from 05.06.2015.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 18.08.2025
//

#ifndef GrainScrollBar_hpp
#define GrainScrollBar_hpp

#include "GUI/Components/Component.hpp"


namespace Grain {

    class View;

    class ScrollBar : public Component {

    public:
        ScrollBar(const Rectd& rect, bool vertical);
        ~ScrollBar() override;

        [[nodiscard]] const char* className() const noexcept override { return "ScrollBar"; }

        [[nodiscard]] static ScrollBar* add(View* view, const Rectd& rect, bool vertical);

        [[nodiscard]] double barSize() const { return bar_size_; }
        [[nodiscard]] double scrollPosition() const { return std::clamp<double>(scroll_position_, 0.0, 1.0); }
        [[nodiscard]] bool isVertical() const noexcept override { return is_vertical_; }

        void setVisibleFraction(double visible_fraction);
        void setVisibleFraction(double total_size, double visible_size);
        void setScrollPosition(double scroll_position);
        void setScrollPosition(double offset, double max_offset);

        void draw(const Rectd& dirty_rect) noexcept override;
        void handleMouseDown(const Event& event) noexcept override;
        void handleMouseDrag(const Event& event) noexcept override;
        void handleMouseUp(const Event& event) noexcept override;

    protected:
        double bar_size_{};
        double visible_fraction_{};
        double scroll_position_{};
        Rectd track_rect_{};
        Rectd handle_rect_{};
        double remembered_scroll_position_{};
        bool is_vertical_{};
    };

} // End of namespace Grain

#endif // GrainScrollBar_hpp
