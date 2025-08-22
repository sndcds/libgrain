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
#include "Core/Log.hpp"

#include "cairo/cairo.h"


namespace Grain {

    CairoContext::CairoContext() noexcept : GraphicContext() {
    }


    CairoContext::~CairoContext() noexcept {
        _freeCairoResources();
    }


    void CairoContext::log(Log& l) const noexcept {
        l << "Cairo version: "
            << CAIRO_VERSION_MAJOR << "."
            << CAIRO_VERSION_MINOR << "."
            << CAIRO_VERSION_MICRO
            << l.endl;

        cairo_status_t s = cairo_status((::cairo_t *) _m_cairo_cr);
        if (s != CAIRO_STATUS_SUCCESS) {
            l << "Cairo error: " << cairo_status_to_string(s) << l.endl;
        }
        if (!_m_cairo_cr) {
            l << "Cairo error: _m_cairo_cr is nullptr!" << l.endl;
            return;
        }
        if (!_m_cairo_surface) {
            l << "Cairo error: _m_cairo_surface is nullptr!" << l.endl;
            return;
        }

        if (cairo_surface_status((::cairo_surface_t *) _m_cairo_surface) != CAIRO_STATUS_SUCCESS) {
            l << "Cairo surface error: "
                << cairo_status_to_string(cairo_surface_status((::cairo_surface_t *) _m_cairo_surface))
                << l.endl;
        }

        if (cairo_status((::cairo_t*)_m_cairo_cr) != CAIRO_STATUS_SUCCESS) {
            l << "Cairo context error: "
                << cairo_status_to_string(cairo_status((::cairo_t *) _m_cairo_cr))
                << l.endl;
        }
    }


    void CairoContext::_freeCairoResources() noexcept {
        if (_m_cairo_cr) {
            cairo_destroy((::cairo_t*)_m_cairo_cr);
            _m_cairo_cr = nullptr;
        }
        if (_m_cairo_surface) {
            cairo_surface_destroy((::cairo_surface_t*)_m_cairo_surface);
            _m_cairo_surface = nullptr;
        }
    }


    void CairoContext::setImage(Image* image) noexcept {
        _freeImage();
        if (!image) {
            return;
        }

        GRAIN_RETAIN(image);
        m_image = image;

        _freeCairoResources();

        if (image->colorModel() == Color::Model::RGBA && image->isFloat()) {

            _m_cairo_surface = cairo_image_surface_create_for_data(
                    image->mutPixelDataPtr(),
                    CAIRO_FORMAT_RGBA128F, // Cairo's pixel format
                    image->width(),
                    image->height(),
                    image->bytesPerRow());
            _m_cairo_cr = cairo_create((::cairo_surface_t*)_m_cairo_surface);
        }
    }


    void CairoContext::setFillRGB(const RGB& rgb) noexcept {
        m_fill_color.setRGB(rgb);
        // cairo_set_source_rgb((::cairo_t*)_m_cairo_cr, rgb.red(), rgb.green(), rgb.blue());
    }


    void CairoContext::setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept {
        m_fill_color.setRGBA(rgb, alpha);
        // cairo_set_source_rgba((::cairo_t*)_m_cairo_cr, rgb.red(), rgb.green(), rgb.blue(), alpha);
    }


    void CairoContext::fillRect(const Rectd& rect) noexcept {
        cairo_set_source_rgba(
                (::cairo_t*)_m_cairo_cr,
                m_fill_color.red(),
                m_fill_color.green(),
                m_fill_color.blue(),
                m_fill_color.alpha());
        cairo_rectangle((::cairo_t*)_m_cairo_cr, rect.m_x, rect.m_y, rect.m_width, rect.m_height);
        cairo_fill((::cairo_t*)_m_cairo_cr);
    }
}