//
//  LevelCurve.cpp
//
//  Created by Roald Christesen on 08.04.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "DSP/LevelCurve.hpp"
#include "Bezier/Bezier.hpp"


namespace Grain {

    LevelCurve::LevelCurve() {

        reset();
    }


    LevelCurve::LevelCurve(float y1, float x2, float y2, float x3, float y3, float y4) {

        m_y1 = y1;
        m_x2 = x2;
        m_y2 = y2;
        m_x3 = x3;
        m_y3 = y3;
        m_y4 = y4;
        m_amount = 1.0f;
    }


    LevelCurve::~LevelCurve() {
    }


    void LevelCurve::reset() noexcept {

        m_y1 = 0.0f;
        m_x2 = 0.3333f;
        m_y2 = 0.3333f;
        m_x3 = 0.6667f;
        m_y3 = 0.6667f;
        m_y4 = 1.0f;
        m_amount = 1.0f;
    }


    void LevelCurve::setCurve(const LevelCurve* curve) noexcept {

        if (!curve) {
            reset();
        }
        else {
            *this = *curve;
        }
    }


    void LevelCurve::set(float y1, float x2, float y2, float x3, float y3, float y4) noexcept {

        m_y1 = y1;
        m_x2 = x2;
        m_y2 = y2;
        m_x3 = x3;
        m_y3 = y3;
        m_y4 = y4;
    }


    void LevelCurve::setByIntArray(const int32_t* array, int32_t max_level) noexcept {
        if (array) {
            double scale = max_level > 0 ? 1.0 / max_level : 1.0;
            m_y1 = scale * array[0];
            m_x2 = scale * array[1];
            m_y2 = scale * array[2];
            m_x3 = scale * array[3];
            m_y3 = scale * array[4];
            m_y4 = scale * array[5];
        }
    }


    Vec2d LevelCurve::pointAtIndex(int32_t index) const noexcept {

        switch (index) {
            case 0: return Vec2d(0.0f, m_y1);
            case 1: return Vec2d(m_x2, m_y2);
            case 2: return Vec2d(m_x3, m_y2);
            case 3: return Vec2d(1.0f, m_y4);
        }

        return Vec2d(0.0f, 0.0f);
    }


    bool LevelCurve::resetPointAtIndex(int32_t index) noexcept {

        bool modified = false;

        switch (index) {
            case 0:
                modified = m_y1 != 0.0f;
                m_y1 = 0.0f;
                break;
            case 1:
                modified = m_x2 != 0.3333f || m_y2 != 0.3333f;
                m_x2 = m_y2 = 0.3333f;
                break;
            case 2:
                modified = m_x3 != 0.6667f || m_y3 != 0.6667f;
                m_x3 = m_y3 = 0.6667f;
                break;
            case 3:
                modified = m_y4 != 1.0f;
                m_y4 = 1.0f;
                break;
        }

        return modified;
    }


    double LevelCurve::_bezierComponent(double t) const noexcept {

        double u = 1.0 - t;
        return 3.0 * u * u * t * m_x2 + 3.0 * u * t * t * m_x3 + t * t * t;
    }


    double LevelCurve::_bezierComponentDerivative(double t) const noexcept {

        double u = 1.0 - t;
        return 3.0 * u * u * m_x2 + 6.0 * u * t * (m_x3 - m_x2) + 3.0 * t * t * (1.0 - m_x3);
    }


    double LevelCurve::yAtX(double x, int32_t max_iter, double epsilon) const noexcept {

        double t = x;  // good initial guess since x and t are both in [0,1]

        for (int i = 0; i < max_iter; ++i) {
            double _x = _bezierComponent(t);
            double dx = _bezierComponentDerivative(t);
            if (fabs(dx) < epsilon) {
                break;  // Avoid divide-by-zero
            }

            double t_new = t - (_x - x) / dx;
            t = t_new < 0.0 ? 0.0 : t > 1.0 ? 1.0 : t;
            if (fabs(_x - x) < epsilon) {
                break;
            }
        }

        double u = 1.0 - t;
        return u * u * u * m_y1 + 3.0 * u * u * t * m_y2 + 3.0 * u * t * t * m_y3 + t * t * t * m_y4;
    }


} // End of namespace Grain
