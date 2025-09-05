//
//  Event.hpp
//
//  Created by Roald Christesen on from 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Event.hpp"
#include "App/App.hpp"


namespace Grain {

    bool Event::g_mouse_pressed = false;
    bool Event::g_right_mouse_pressed = false;
    Component *Event::g_component = nullptr;
    Component *Event::g_previous_component = nullptr;
    Timestamp Event::g_ts_last_mouse_click;
    Vec2d Event::g_mouse_down_pos;
    int32_t Event::g_mouse_drag_count = 0;


#if defined(__APPLE__) && defined(__MACH__)
    void _macosEvent_release(Event* event);
#endif


    Event::~Event() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        _macosEvent_release(this);
#endif
    }


    void Event::log(Log &l) const {
        l.header(className());
        l << "m_event_type: " << typeName() << Log::endl;
        l << "m_mouse_pos: " << m_mouse_pos << Log::endl;
        l << "m_value: " << m_value << Log::endl;
        l << "m_delta: " << m_delta << Log::endl;
        l << "m_key_mask: " << m_key_mask << Log::endl;
        l << "m_key_unichar_count: " << m_key_unichar_count << Log::endl;
        l << "m_key_unichar: " << m_key_unichar << Log::endl;
        l << "m_key_code: " << m_key_code << Log::endl;
        l << "m_has_precise_scrolling_deltas: " << m_has_precise_scrolling_deltas << Log::endl;
        l << "m_mouse_double_clicked: " << m_mouse_double_clicked << Log::endl;
        l << "m_ignore: " << m_ignore << Log::endl;
        l--;
    }


    double Event::dragZoomX(double step) const noexcept {
        if (step == 0.0) {
            return 1.0;
        }
        else {
            double d = Event::g_mouse_down_pos.m_x - m_mouse_pos.m_x;
            return d >= 0.0 ? 1.0 + d / step : 1.0 / (1.0 - d / step);
        }
    }


    double Event::dragZoomY(double step, bool flipped) const noexcept {
        if (step == 0.0) {
            return 1.0;
        }
        else {
            double d = flipped ? m_mouse_pos.m_y - Event::g_mouse_down_pos.m_y : Event::g_mouse_down_pos.m_y - m_mouse_pos.m_y;
            return d >= 0.0 ? 1.0 + d / step : 1.0 / (1.0 - d / step);
        }
    }


    Event::DragDirection Event::dragDirection() const noexcept {
        if (isShiftPressedOnly()) {
            return std::fabs(mouseDragDeltaX()) > std::fabs(mouseDragDeltaY()) ? DragDirection::Horizontal : DragDirection::Vertical;
        }
        else {
            return DragDirection::Free;
        }
    }


} // End of namespace Grain
