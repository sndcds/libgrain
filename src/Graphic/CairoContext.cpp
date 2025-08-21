//
//  CairoContext.hpp
//
//  Created by Roald Christesen on from 15.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Graphic/CairoContext.hpp"
#include "Image/Image.hpp"

#include "cairo/cairo.h"


namespace Grain {

    CairoContext::CairoContext(Image* image) noexcept : GraphicContext(image) {

        std::cout << "init CairoContext\n";
        std::cout << "Cairo version: "
                  << CAIRO_VERSION_MAJOR << "."
                  << CAIRO_VERSION_MINOR << "."
                  << CAIRO_VERSION_MICRO
                  << std::endl;

        _setImage(image);

        cairo_status_t s = cairo_status((::cairo_t *) _m_cairo_cr);
        if (s != CAIRO_STATUS_SUCCESS) {
            std::cout << "Cairo error: " << cairo_status_to_string(s) << std::endl;
        }
        if (!_m_cairo_cr) {
            std::cout << "error: _m_cairo_cr is nullptr!" << std::endl;
            return;
        }
        if (!_m_cairo_surface) {
            std::cout << "error: _m_cairo_surface is nullptr!" << std::endl;
            return;
        }

        if (cairo_surface_status((::cairo_surface_t *) _m_cairo_surface) != CAIRO_STATUS_SUCCESS) {
            std::cerr
                << "Surface error: "
                << cairo_status_to_string(cairo_surface_status((::cairo_surface_t *) _m_cairo_surface))
                << std::endl;
        }

        if (cairo_status((::cairo_t*)_m_cairo_cr) != CAIRO_STATUS_SUCCESS) {
            std::cerr
                << "Context error: "
                << cairo_status_to_string(cairo_status((::cairo_t *) _m_cairo_cr))
                << std::endl;
        }
    }


    void CairoContext::_setImage(Image* image) noexcept {
        _freeResources();

        if (!image) {
            return;
        }

        GRAIN_RETAIN(image);
        m_image = image;

        if (image->colorModel() == Color::Model::RGBA && image->isFloat()) {
            _m_cairo_surface = cairo_image_surface_create_for_data(
                    reinterpret_cast<unsigned char*>(image->mutPixelDataPtr()),
                    CAIRO_FORMAT_RGBA128F, // Cairo's pixel format
                    static_cast<int>(m_width),
                    static_cast<int>(m_height),
                    image->bytesPerRow());
            _m_cairo_cr = cairo_create((::cairo_surface_t*)_m_cairo_surface);
        }
    }


    void CairoContext::setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept {
        m_fill_color.setRGBA(rgb, alpha);
        cairo_set_source_rgba((::cairo_t*)_m_cairo_cr, rgb.red(), rgb.green(), rgb.blue(), alpha);
    }

    void CairoContext::fillRect(const Rectd& rect) noexcept {
        std::cout << "fillRect: " << rect << std::endl;
        cairo_set_source_rgba((::cairo_t*)_m_cairo_cr, m_fill_color.red(), m_fill_color.green(), m_fill_color.blue(), m_fill_color.alpha());
        cairo_rectangle((::cairo_t*)_m_cairo_cr, rect.m_x, rect.m_y, rect.m_width, rect.m_height);
        cairo_fill_preserve((::cairo_t*)_m_cairo_cr);
        cairo_stroke((::cairo_t*)_m_cairo_cr);
        std::cout << "done" << std::endl;
    }

}