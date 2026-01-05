//
//  Toggle.cpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/Toggle.hpp"
#include "2d/Rect.hpp"
#include "GUI/Views/View.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include "Graphic/AppleCGContext.hpp"
#endif


namespace Grain {

    Toggle::Toggle(const Rectd& rect, int32_t tag) noexcept : Checkbox(rect, nullptr, tag) {
        type_ = ComponentType::Toggle;
    }


    Toggle::~Toggle() noexcept {
    }


    Toggle* Toggle::add(View* view, const Rectd& rect, int32_t tag) {
        return (Toggle*)Component::addComponentToView((Component*)new(std::nothrow) Toggle(rect, tag), view);
    }


    void Toggle::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto style= guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        Rectd bounds_rect = boundsRect();
        Rectd toggle_rect(24, 14);
        toggle_rect.alignInRect(Alignment::Center, bounds_rect);

        bool enabled = isEnabled();
        bool selected = isSelected();
        bool highligted = isHighlighted();

        RGBA bg_color = style->backgroundColor();
        RGBA fg_color = style->foregroundColor();
        RGBA border_color = style->backgroundColor();

        if (!enabled) {
            // TODO:
        }

        gc->setFillRGBA(bg_color);
        gc->fillRoundBar(toggle_rect);

        // Indicator
        Vec2d indicator_pos = toggle_rect.center();
        double indicator_radius = (toggle_rect.height_- 6) / 2;
        if (selected) {
            indicator_pos.x_ = toggle_rect.x_ + toggle_rect.height_ / 2;
        }
        else {
            indicator_pos.x_ = toggle_rect.x2() - toggle_rect.height_ / 2;
        }

        gc->setFillRGBA(fg_color);
        gc->fillCircle(indicator_pos, indicator_radius);

        if (isKeyComponent()) {
            gc->setFillColor(1, 0, 0, 0.5);
            gc->fillFrame(boundsRect(), 1);
        }
    }

} // End of namespace Grain
