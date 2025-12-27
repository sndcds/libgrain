//
//  CIEXYZ.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/CIEXYZ.hpp"
#include "Color/CIExyY.hpp"
#include "Color/RGB.hpp"
#include "Math/Math.hpp"


namespace Grain {

    CIEXYZ::CIEXYZ(const RGB& rgb) noexcept {

        float r = Color::gamma_to_linear(rgb.m_data[0]);
        float g = Color::gamma_to_linear(rgb.m_data[1]);
        float b = Color::gamma_to_linear(rgb.m_data[2]);
        data_[0] = 0.4124f * r + 0.3576f * g + 0.1805f * b;
        data_[1] = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        data_[2] = 0.0193f * r + 0.1192f * g + 0.9505f * b;
    }


    CIEXYZ::CIEXYZ(const CIExyY& xyY) noexcept {

        float x = xyY.m_pos.x_;
        float y = xyY.m_pos.y_;
        float Y = xyY.y_;

        if (y == 0.0f) {
            data_[0] = data_[1] = data_[2] = 0.0f;
        }
        else {
            data_[0] = (x * Y) / y;
            data_[1] = Y;
            data_[2] = ((1.0f - x - y) * Y) / y;
        }
    }


// From http://www.physics.sfasu.edu/astro/color/blackbodyc.txt

    void CIEXYZ::setKelvin(float temperature) noexcept {

        constexpr double min_temp = g_min_temperature;
        constexpr int band_count = 81;
        constexpr double start_wavelength = 380.0;
        constexpr double step = 5.0;

        static const double color_match[band_count][3] = {
                { 0.0014, 0.0000, 0.0065 }, { 0.0022, 0.0001, 0.0105 }, { 0.0042, 0.0001, 0.0201 },
                { 0.0076, 0.0002, 0.0362 }, { 0.0143, 0.0004, 0.0679 }, { 0.0232, 0.0006, 0.1102 },
                { 0.0435, 0.0012, 0.2074 }, { 0.0776, 0.0022, 0.3713 }, { 0.1344, 0.0040, 0.6456 },
                { 0.2148, 0.0073, 1.0391 }, { 0.2839, 0.0116, 1.3856 }, { 0.3285, 0.0168, 1.6230 },
                { 0.3483, 0.0230, 1.7471 }, { 0.3481, 0.0298, 1.7826 }, { 0.3362, 0.0380, 1.7721 },
                { 0.3187, 0.0480, 1.7441 }, { 0.2908, 0.0600, 1.6692 }, { 0.2511, 0.0739, 1.5281 },
                { 0.1954, 0.0910, 1.2876 }, { 0.1421, 0.1126, 1.0419 }, { 0.0956, 0.1390, 0.8130 },
                { 0.0580, 0.1693, 0.6162 }, { 0.0320, 0.2080, 0.4652 }, { 0.0147, 0.2586, 0.3533 },
                { 0.0049, 0.3230, 0.2720 }, { 0.0024, 0.4073, 0.2123 }, { 0.0093, 0.5030, 0.1582 },
                { 0.0291, 0.6082, 0.1117 }, { 0.0633, 0.7100, 0.0782 }, { 0.1096, 0.7932, 0.0573 },
                { 0.1655, 0.8620, 0.0422 }, { 0.2257, 0.9149, 0.0298 }, { 0.2904, 0.9540, 0.0203 },
                { 0.3597, 0.9803, 0.0134 }, { 0.4334, 0.9950, 0.0087 }, { 0.5121, 1.0000, 0.0057 },
                { 0.5945, 0.9950, 0.0039 }, { 0.6784, 0.9786, 0.0027 }, { 0.7621, 0.9520, 0.0021 },
                { 0.8425, 0.9154, 0.0018 }, { 0.9163, 0.8700, 0.0017 }, { 0.9786, 0.8163, 0.0014 },
                { 1.0263, 0.7570, 0.0011 }, { 1.0567, 0.6949, 0.0010 }, { 1.0622, 0.6310, 0.0008 },
                { 1.0456, 0.5668, 0.0006 }, { 1.0026, 0.5030, 0.0003 }, { 0.9384, 0.4412, 0.0002 },
                { 0.8544, 0.3810, 0.0002 }, { 0.7514, 0.3210, 0.0001 }, { 0.6424, 0.2650, 0.0000 },
                { 0.5419, 0.2170, 0.0000 }, { 0.4479, 0.1750, 0.0000 }, { 0.3608, 0.1382, 0.0000 },
                { 0.2835, 0.1070, 0.0000 }, { 0.2187, 0.0816, 0.0000 }, { 0.1649, 0.0610, 0.0000 },
                { 0.1212, 0.0446, 0.0000 }, { 0.0874, 0.0320, 0.0000 }, { 0.0636, 0.0232, 0.0000 },
                { 0.0468, 0.0170, 0.0000 }, { 0.0329, 0.0119, 0.0000 }, { 0.0227, 0.0082, 0.0000 },
                { 0.0158, 0.0057, 0.0000 }, { 0.0114, 0.0041, 0.0000 }, { 0.0081, 0.0029, 0.0000 },
                { 0.0058, 0.0021, 0.0000 }, { 0.0041, 0.0015, 0.0000 }, { 0.0029, 0.0010, 0.0000 },
                { 0.0020, 0.0007, 0.0000 }, { 0.0014, 0.0005, 0.0000 }, { 0.0010, 0.0004, 0.0000 },
                { 0.0007, 0.0002, 0.0000 }, { 0.0005, 0.0002, 0.0000 }, { 0.0003, 0.0001, 0.0000 },
                { 0.0002, 0.0001, 0.0000 }, { 0.0002, 0.0001, 0.0000 }, { 0.0001, 0.0000, 0.0000 },
                { 0.0001, 0.0000, 0.0000 }, { 0.0001, 0.0000, 0.0000 }, { 0.0000, 0.0000, 0.0000 }
        };

        if (temperature < min_temp) {
            temperature = min_temp;
        }

        double ax = 0.0, ay = 0.0, az = 0.0;

        constexpr double planck_const = 6.62607015e-34;
        constexpr double speed_of_light = 2.99792458e8;
        constexpr double boltzmann = 1.380649e-23;

        constexpr double c1 = 2.0 * planck_const * speed_of_light * speed_of_light;  // 3.74183e-16
        constexpr double c2 = planck_const * speed_of_light / boltzmann;  // 1.438776877...

        for (int i = 0; i < band_count; ++i) {
            double lambda = start_wavelength + i * step;       // nm
            double lambda_m = lambda * 1e-9;                   // convert to meters
            double weight = (i == 0 || i == band_count - 1) ? 0.5 : 1.0;

            double spectral_radiance = c1 / (std::pow(lambda_m, 5.0) * (std::exp(c2 / (lambda_m * temperature)) - 1.0));

            ax += weight * spectral_radiance * color_match[i][0];
            ay += weight * spectral_radiance * color_match[i][1];
            az += weight * spectral_radiance * color_match[i][2];
        }

        double norm = std::max({ ax, ay, az });
        if (norm > 0.0) {
            data_[0] = static_cast<float>(ax / norm);
            data_[1] = static_cast<float>(ay / norm);
            data_[2] = static_cast<float>(az / norm);
        }
    }


    Vec2f CIEXYZ::CIExy() const noexcept {

        /**
         *  Formula:http://de.wikipedia.org/wiki/CIE-Normvalenzsystem
         *
         *  |X|   |0.4124  0.3576  0.1805|   |R|
         *  |Y| = |0.2126  0.7152  0.0722| x |G|
         *  |Z|   |0.0193  0.1192  0.9505|   |B|
         *
         *  x = X / (X + Y + Z)
         *  y = Y / (X + Y + Z)
         */

        float sum = data_[0] + data_[1] + data_[2];

        if (sum != 0.0f) {
            return Vec2f(data_[0] / sum, data_[1] / sum);
        }
        else {
            return Vec2f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        }
    }


    bool CIEXYZ::colorTemp(float* temperature) const noexcept {

        // From: XYZtoCorColorTemp.c by Bruce Justin Lindbloom

        static const double rt[31] = {
                std::numeric_limits<double>::epsilon(),  10.0e-6,  20.0e-6,  30.0e-6,  40.0e-6,  50.0e-6,
                60.0e-6,  70.0e-6,  80.0e-6,  90.0e-6, 100.0e-6, 125.0e-6,
                150.0e-6, 175.0e-6, 200.0e-6, 225.0e-6, 250.0e-6, 275.0e-6,
                300.0e-6, 325.0e-6, 350.0e-6, 375.0e-6, 400.0e-6, 425.0e-6,
                450.0e-6, 475.0e-6, 500.0e-6, 525.0e-6, 550.0e-6, 575.0e-6,
                600.0e-6
        };

        static const double uvt[31][3] = {
                { 0.18006, 0.26352, -0.24341 },
                { 0.18066, 0.26589, -0.25479 },
                { 0.18133, 0.26846, -0.26876 },
                { 0.18208, 0.27119, -0.28539 },
                { 0.18293, 0.27407, -0.30470 },
                { 0.18388, 0.27709, -0.32675 },
                { 0.18494, 0.28021, -0.35156 },
                { 0.18611, 0.28342, -0.37915 },
                { 0.18740, 0.28668, -0.40955 },
                { 0.18880, 0.28997, -0.44278 },
                { 0.19032, 0.29326, -0.47888 },
                { 0.19462, 0.30141, -0.58204 },
                { 0.19962, 0.30921, -0.70471 },
                { 0.20525, 0.31647, -0.84901 },
                { 0.21142, 0.32312, -1.0182 },
                { 0.21807, 0.32909, -1.2168 },
                { 0.22511, 0.33439, -1.4512 },
                { 0.23247, 0.33904, -1.7298 },
                { 0.24010, 0.34308, -2.0637 },
                { 0.24792, 0.34655, -2.4681 },
                { 0.25591, 0.34951, -2.9641 },
                { 0.26400, 0.35200, -3.5814 },
                { 0.27218, 0.35407, -4.3633 },
                { 0.28039, 0.35577, -5.3762 },
                { 0.28863, 0.35714, -6.7262 },
                { 0.29685, 0.35823, -8.5955 },
                { 0.30505, 0.35907, -11.324 },
                { 0.31320, 0.35968, -15.628 },
                { 0.32129, 0.36011, -23.325 },
                { 0.32931, 0.36038, -40.770 },
                { 0.33724, 0.36051, -116.45 }
        };

        double us, vs, p, di = 0.0, dm;
        int32_t i;


        if ((data_[0] < std::numeric_limits<float>::min()) &&
            (data_[1] < std::numeric_limits<float>::min()) &&
            (data_[2] < std::numeric_limits<float>::min())) {
            // Protect against possible divide-by-zero failure
            return false;
        }

        us = (4.0 * data_[0]) / (data_[0] + 15.0 * data_[1] + 3.0 * data_[2]);
        vs = (6.0 * data_[1]) / (data_[0] + 15.0 * data_[1] + 3.0 * data_[2]);
        dm = 0.0;

        for (i = 0; i < 31; i++) {
            di = (vs - uvt[i][1]) - uvt[i][2] * (us - uvt[i][0]);
            if ((i > 0) && (((di < 0.0) && (dm >= 0.0)) || ((di >= 0.0) && (dm < 0.0)))) {
                // Found lines bounding (us, vs) : i - 1 and i
                break;
            }
            dm = di;
        }

        if (i == 31) {
            // Bad XYZFile input, color temp would be less than minimum of 1666.7 degree, or too far towards blue
            *temperature = g_min_temperature;
            return false;
        }

        di = di / std::sqrt(1.0 + uvt[i][2] * uvt[i][2]);
        dm = dm / std::sqrt(1.0 + uvt[i - 1][2] * uvt[i - 1][2]);
        p = dm / (dm - di);  // p = interpolation parameter, 0.0 : i - 1, 1.0 : i

        double a = Math::lerp(rt[i - 1], rt[i], p);
        if (a != 0.0) {
            p = 1.0 / a;
        }
        else {
            p = std::numeric_limits<float>::max();
        }

        *temperature = static_cast<float>(p);

        return true;
    }



    void CIEXYZ::transform(const Mat3f& matrix, RGB& out_rgb) const noexcept {

        matrix.transform3(data_, out_rgb.mutValuePtr());
    }


}  // End of namespace Grain
