//
//  CairoContext.hpp
//
//  Created by Roald Christesen on from 15.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Graphic/CairoContext.hpp"


namespace Grain {

    CairoContext::CairoContext(Image* image) noexcept : GraphicContext(image) {
        std::cout << "init CairoContext\n";
        std::cout << "Cairo version: "
                  << CAIRO_VERSION_MAJOR << "."
                  << CAIRO_VERSION_MINOR << "."
                  << CAIRO_VERSION_MICRO
                  << std::endl;

        cairo_status_t s = cairo_status(_m_cairo_cr);
        if (s != CAIRO_STATUS_SUCCESS) {
            std::cout << "Cairo error: " << cairo_status_to_string(s) << std::endl;
        }
        if (_m_cairo_cr == nullptr) {
            std::cout << "error: _m_cairo_cr is nullptr!" << std::endl;
            return;
        }
        if (_m_cairo_surface == nullptr) {
            std::cout << "error: _m_cairo_surface is nullptr!" << std::endl;
            return;
        }

        if (cairo_surface_status(_m_cairo_surface) != CAIRO_STATUS_SUCCESS)
            std::cerr << "Surface error: "
                      << cairo_status_to_string(cairo_surface_status(_m_cairo_surface))
                      << std::endl;

        if (cairo_status(_m_cairo_cr) != CAIRO_STATUS_SUCCESS)
            std::cerr << "Context error: "
                      << cairo_status_to_string(cairo_status(_m_cairo_cr))
                      << std::endl;
    }

    void CairoContext::setFillColor(const RGB& color, float alpha) noexcept {
        m_fill_color.setRGBA(color, alpha);
        cairo_set_source_rgba(_m_cairo_cr, color.red(), color.green(), color.blue(), alpha);
    }

    void CairoContext::fillRect(const Rectd& rect) noexcept {
        std::cout << "fillRect: " << rect << std::endl;
        cairo_set_source_rgba(_m_cairo_cr, m_fill_color.red(), m_fill_color.green(), m_fill_color.blue(), m_fill_color.alpha());
        cairo_rectangle(_m_cairo_cr, rect.m_x, rect.m_y, rect.m_width, rect.m_height);
        cairo_fill_preserve(_m_cairo_cr);
        cairo_stroke(_m_cairo_cr);
        std::cout << "done" << std::endl;
    }

}