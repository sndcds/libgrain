//
//  ColorWell.cpp
//
//  Created by Roald Christesen on 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "GUI/Components/ColorWell.hpp"
#include "GUI/GUIStyle.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    ColorWell::ColorWell(const Rectd& rect) noexcept : Component(rect, 0) {
        // m_ns_view = [[GrNSColorWell alloc] initForUI:this rect:rect];

        type_ = ComponentType::ColorWell;
        color_ = RGB::kBlack;
    }


    ColorWell::~ColorWell() noexcept {
    }


    ColorWell* ColorWell::add(View* view, const Rectd& rect) {
        return (ColorWell*)Component::addComponentToView((Component*)new(std::nothrow) ColorWell(rect), view);
    }


    void ColorWell::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        Rectd bounds_rect = boundsRect();

        float alpha = 1.0f;
        RGBA bg_color;

        if (!is_enabled_ == false) {
            bg_color = style->backgroundColor();
            alpha = 0.5f;
        }
        else if (drag_entered_flag_) {
            bg_color= { 1, 0, 0, 1 }; // TODO: Get from style
        }
        else {
            bg_color = style->backgroundColor();
        }

        float radius = style->cornerRadius(0); // TODO: Which radius to get?

        gc->setFillRGBA(bg_color);
        gc->fillRect(bounds_rect, radius);

        bg_color = style->viewColor();
        Rectd rect = bounds_rect;
        rect.inset(3);
        gc->setFillRGBA(bg_color);
        gc->fillRect(rect);

        rect.inset(1);
        gc->setFillRGBAndAlpha(color_, alpha);
        gc->fillRect(rect);
    }


    void ColorWell::setByComponent(Component* component) noexcept {
        if (component) {
            setColor(component->color());
        }
    }


} // End of namespace