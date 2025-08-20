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
        ScrollBar(const Rectd &rect, bool vertical);
        ~ScrollBar();

        const char *className() const noexcept override { return "ScrollBar"; }

        [[nodiscard]] static ScrollBar *add(View *view, const Rectd &rect, bool vertical);

        [[nodiscard]] double barSize() { return m_bar_size; }
        [[nodiscard]] double scrollPosition() { return std::clamp<double>(m_scroll_position, 0.0, 1.0); }
        [[nodiscard]] bool isVertical() { return m_vertical; }

        void setVisibleFraction(double visible_fraction);
        void setVisibleFraction(double total_size, double visible_size);
        void setScrollPosition(double scroll_position);
        void setScrollPosition(double offset, double max_offset);

        void draw(const Rectd &dirty_rect) noexcept override;
        void handleMouseDown(const Event &event) noexcept override;
        void handleMouseDrag(const Event &event) noexcept override;
        void handleMouseUp(const Event &event) noexcept override;

    protected:
        double m_bar_size{};
        double m_visible_fraction{};
        double m_scroll_position{};
        Rectd m_track_rect{};
        Rectd m_handle_rect{};
        double m_remembered_scroll_position{};
        bool m_vertical{};
    };


} // End of namespace Grain

#endif // GrainScrollBar_hpp
