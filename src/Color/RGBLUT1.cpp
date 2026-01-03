//
//  RGBLUT1.cpp
//
//  Created by Roald Christesen on from 05.02.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/RGBLUT1.hpp"
#include "Color/Gradient.hpp"
#include "Math/Math.hpp"


namespace Grain {

    const RGBLUT1* RGBLUT1::g_kelvin_lut = RGBLUT1::kelvinLUT();


    RGBLUT1::RGBLUT1(int32_t resolution) noexcept {

        resolution = std::clamp<int32_t>(resolution, kMinResolution, kMaxResolution);

        m_resolution = m_max_resolution = 0;
        setResolution(resolution);
    }


    RGBLUT1::~RGBLUT1() noexcept {

        std::free(m_samples);
    }


    bool RGBLUT1::setResolution(int32_t resolution) noexcept {

        resolution = std::clamp<int32_t>(resolution, kMinResolution, kMaxResolution);

        if (resolution <= m_max_resolution) {
            m_resolution = resolution;
            m_max_index = resolution - 1;
        }
        else {
            // Reallocation necessary
            int32_t new_max_resolution = std::clamp<int32_t>(resolution, kMinResolution, kMaxResolution);
            RGB* new_samples = (RGB*)std::realloc(m_samples, sizeof(RGB) * new_max_resolution);

            if (!new_samples) {
                return false;
            }

            for (int32_t i = 0; i < new_max_resolution; i++) {
                new (&new_samples[i])RGB();
            }

            m_samples = new_samples;
            m_resolution = resolution;
            m_max_resolution = new_max_resolution;
            m_max_index = resolution - 1;
        }

        return true;
    }


    void RGBLUT1::setColorAtIndex(int32_t index, const RGB& color) noexcept {

        if (index >= 0 && index < m_resolution) {
            m_samples[index] = color;
        }
    }


    bool RGBLUT1::shrink() noexcept {
        RGB* new_samples = (RGB*)std::realloc(m_samples, sizeof(RGB) * m_resolution);
        if (!new_samples) {
            return false;
        }

        m_samples = new_samples;
        m_max_resolution = m_resolution;

        return true;
    }


    void RGBLUT1::updateByGradient(Gradient* gradient) noexcept {
        if (!gradient) {
            return;
        }

        if (gradient->stopCount() == 1) {
            RGBA color;
            gradient->colorAtIndex(0, 0, color);    // TODO: Gradient!!!!
            RGB* d = m_samples;
            for (int32_t i = 0; i < m_resolution; i++) {
                *d = color;
                d++;
            }
        }
        else if (gradient->stopCount() > 1) {
            int32_t sub_resolution = 10;

            RGB color;
            RGB* d = m_samples;

            float lut_pos = 0;
            float lut_step = 1.0f / static_cast<float>(m_max_index);
            float lut_sub_step = lut_step / static_cast<float>(sub_resolution);

            for (int32_t i = 0; i < m_resolution; i++) {
                lut_pos = static_cast<float>(i) / static_cast<float>(m_max_index);
                color.black();

                for (int32_t j = 0; j < sub_resolution; j++) {
                    RGBA sample_color;
                    gradient->lookupColor(lut_pos, sample_color);
                    color += sample_color;
                    lut_pos += lut_sub_step;
                }

                color *= 1.0f / static_cast<float>(sub_resolution);
                *d = color;
                d++;
            }
        }

        smooth();
    }


    void RGBLUT1::smooth() noexcept {

        if (m_resolution < 3) {
            return;
        }

        RGB* c0 = &m_samples[1];
        RGB* c1 = &m_samples[2];
        RGB temp0 = m_samples[0];

        for (int32_t i = 1; i < m_resolution - 1; i++) {
            RGB temp1 = *c1;

            c0->data_[0] = (temp0.data_[0] + c0->data_[0] + c1->data_[0]) / 3;
            c0->data_[1] = (temp0.data_[1] + c0->data_[1] + c1->data_[1]) / 3;
            c0->data_[2] = (temp0.data_[2] + c0->data_[2] + c1->data_[2]) / 3;

            temp0 = temp1;
            c0++; c1++;
        }
    }


    void RGBLUT1::lookup(float pos, RGB& out_color) const noexcept {

        lookup(pos, out_color.data_);
    }


    void RGBLUT1::lookup(float pos, float* out_color) const noexcept {

        if (pos < 0) {
            m_samples[0].values(out_color);
            return;
        }

        if (pos > 1) {
            m_samples[m_resolution - 1].values(out_color);
            return;
        }

        int32_t index = static_cast<int32_t>(pos * m_resolution);
        if (index >= m_resolution - 1) {
            m_samples[m_resolution - 1].values(out_color);
            return;
        }

        float t = pos*  m_resolution - index;
        float f2 = std::clamp<float>(t, 0.0f, 1.0f);
        float f1 = 1.0f - f2;
        float* c1 = m_samples[index].data_;
        float* c2 = m_samples[index + 1].data_;
        out_color[0] = c1[0] * f1 + c2[0] * f2;
        out_color[1] = c1[1] * f1 + c2[1] * f2;
        out_color[2] = c1[2] * f1 + c2[2] * f2;
    }


    RGBLUT1 const *RGBLUT1::_initKelvinLUT() noexcept {
        int32_t resolution = 128;
        auto lut = new (std::nothrow) RGBLUT1(resolution);
        if (lut) {
            for (int32_t i = 0; i < resolution; i++) {
                RGB rgb;
                rgb.setKelvin(Math::remap(0, resolution - 1, Color::kKelvinMin, Color::kKelvinMax, i));
                lut->setColorAtIndex(i, rgb);
            }
            std::cout << (long)lut << std::endl;
            g_kelvin_lut = lut;
            return g_kelvin_lut;
        }
        else {
            return nullptr;
        }
    }


} // End of namespace Grain.
