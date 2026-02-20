//
//  AngleDial.cpp
//
//  Created by Roald Christesen on 11.12.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "GUI/Components/AngleDial.hpp"
#include "GUI/Components/ColorWell.hpp"
#include "GUI/Components/Slider.hpp"
#include "GUI/Components/TextField.hpp"
#include "GUI/GUIStyle.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Math/Vec3Fix.hpp"


namespace Grain {

    AngleDial::AngleDial(const Rectd& rect) noexcept : ValueComponent(rect) {
        type_ = ComponentType::AngleDial;
        can_get_focus_ = true;
    }


    AngleDial* AngleDial::add(View* view, const Rectd& rect) {
        return (AngleDial*)Component::addComponentToView((Component*)new(std::nothrow) AngleDial(rect), view);
    }

    void AngleDial::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto style = guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        // Recalculate geometry-dependent properties
        auto bounds_rect = boundsRect();
        ring_.center_ = bounds_rect.center();
        ring_.outer_radius_ = (bounds_rect.longSide() - (controller_padding_ * 2)) * 0.5;
        ring_.inner_radius_ = (bounds_rect.longSide() - (controller_padding_ * 2)) * 0.4;

        std::cout << ring_ << std::endl;

        bool enabled = isEnabled();

        RGBA view_color = { 1, 1, 0, 1 };   // component->uiViewColor();
        RGBA bg_color = { 1, 0, 0, 1 };     // filled ? component->uiColor(UIColor::Bg) : view_color;
        RGBA track_color = { 1, 0, 1, 1 };  // component->uiTrackColor();
        RGBA indicator_color = { 0, 1, 1, 1 };  // component->uiIndicatorColor();
        RGBA handle_color = { 1, 0.2, 0.4, 1 }; // component->uiHandleColor();


        gc->fillHueRing(ring_);

        gc->resetClip();
        if (isKeyComponent()) {
            gc->setFillColor(1, 0, 0, 0.5);
            gc->fillFrame(boundsRect(), 1);
        }
    }

    void AngleDial::handleMouseDown(const Event& event) noexcept {
        // if (event.mousePos())

        /*        mouse_down_color_ = current_color_ = color_;

        Vec2d v = event.mouseDownPos() - center_;
        double distance = v.length();

        mouse_mode_ = AngleDial::kMouseModeNone;
        if (distance < inner_radius_) {
            mouse_mode_ = AngleDial::kMouseModeSaturation;
        }
        else if (distance < hue_outer_radius_ + 2 && distance > hue_inner_radius_ - 2) {
            mouse_mode_ = AngleDial::kMouseModeHue;
        }
        else if (distance < value_outer_radius_ + 2 && distance > value_inner_radius_ - 2) {
            mouse_mode_ = AngleDial::kMouseModeValue;
        }
        else {
            return;
        }

        rem_color_pos_ = colorPos();

        if (_mousePointerAction(event)) {
            _checkModified();
        }
        */
    }


    void AngleDial::handleMouseDrag(const Event& event) noexcept {
/*        if (_mousePointerAction(event)) {
            _checkModified();
        }
        */
    }


    void AngleDial::handleMouseUp(const Event& event) noexcept {
        /*
        if (_mousePointerAction(event)) {
            mouse_mode_ = AngleDial::kMouseModeNone;
        }
        needsDisplay();
        */
    }


    void AngleDial::handleScrollWheel(const Event& event) noexcept {
        // TODO: Implement
    }


    void AngleDial::updateRepresentations(const Component* excluded_component) noexcept {
        HSV hsv;
        // color_.hsv(hsv); TODO: !!!!
    }

} // End of namespace