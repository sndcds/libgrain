//
//  DSP.cpp
//
//  Created by Roald Christesen on 25.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "DSP/DSP.hpp"
#include "Type/Type.hpp"
#include "Math/Math.hpp"


namespace Grain {

    bool DSP::hanningWindowSymmetric(int32_t window_size, float *out_data) noexcept {

        if (window_size <= 1 || out_data == nullptr) {
            return false;
        }

        for (int32_t i = 0; i < window_size; i++) {
            out_data[i] = static_cast<float>(0.5 * (1.0 - std::cos((std::numbers::pi * 2 * i) / (window_size - 1))));
        }

        return true;
    }


    bool DSP::hanningWindowPeriodic(int32_t window_size, float* out_data) noexcept {

        if (window_size <= 0 || out_data == nullptr) {
            return false;
        }

        for (int32_t i = 0; i < window_size; ++i) {
            out_data[i] = static_cast<float>(0.5 * (1.0 - std::cos((2.0 * std::numbers::pi * i) / window_size)));
        }

        return true;
    }



    bool DSP::normalizedHanningWindow(int32_t window_size, float* out_data) noexcept {

        // TODO: Check!!!!!

        if (window_size <= 1 || out_data == nullptr) {
            return false;
        }

        auto buffer = new float[window_size * 2];
        if (buffer == nullptr) {
            return false;
        }

        auto hann = buffer;
        auto normalization = &buffer[window_size];

        for (int32_t i = 0; i < window_size; i++) {
            hann[i] = 0.5f * (1.0f - std::cos((2.0f * std::numbers::pi * i) / (window_size - 1)));
        }

        // Compute normalization factor assuming 50% overlap
        for (int32_t i = 0; i < window_size; i++) {
            normalization[i] = 0.0f;
        }

        int32_t hop_size = window_size / 2;

        // Overlap-add the squared window twice (50% overlap)
        for (int32_t i = 0; i < window_size; i++) {
            normalization[i] += hann[i] * hann[i];
            if (i >= hop_size) {
                normalization[i - hop_size] += hann[i] * hann[i];
            }
        }

        // Normalize the window so overlapping segments sum to 1
        for (int32_t i = 0; i < window_size; i++) {
            out_data[i] = hann[i] / std::sqrt(normalization[i] + 1e-6f); // +epsilon avoids division by zero
        }

        delete[] buffer;

        return true;
    }


    ErrorCode DSP::window(
            int32_t window_size,
            WindowType window_type,
            float alpha,
            float beta,
            bool unity_gain,
            float *out_data)
        noexcept {

        constexpr float pi = std::numbers::pi;
        constexpr float tau = pi * 2.0f;

        if (window_type < WindowType::First || window_type > WindowType::Last || window_size < 2) {
            return ErrorCode::BadArgs;
        }

        if (out_data == nullptr) {
            return ErrorCode::NullData;
        }


        if (window_type == WindowType::Kaiser || window_type == WindowType::FlatTop) {
            alpha = 0.0f;
        }
        else {
            alpha = std::clamp(alpha, 0.0f, 1.0f);
        }

        beta = std::clamp(beta, 0.0f, 10.0f);


        float *win_coef = out_data;

        auto top_width = static_cast<int32_t>(alpha * static_cast<float>(window_size));
        if (top_width % 2 != 0) {
            top_width++;
        }
        if (top_width > window_size) {
            top_width = window_size;
        }
        int32_t m = window_size - top_width;
        float dm = static_cast<float>(m) + 1;


        // Calculate the window for width / 2 points, then fold the window over (at the bottom)
        // top_width points will be set to 1

        int32_t j = 0;

        switch (window_type) {

            case WindowType::Kaiser: {
                float arg;
                for (j = 0; j < window_size; j++) {
                    arg = beta * static_cast<float>(std::sqrt(1.0 - std::pow((static_cast<double>(2 * j + 2) - dm) / dm, 2.0)));
                    win_coef[j] = static_cast<float>(Math::bessel(arg) / Math::bessel(beta));
                }
                break;
            }

            case WindowType::Sinc: {    // Lanczos
                for (j = 0; j < window_size; j++) {
                    win_coef[j] = static_cast<float>(Math::sinc(static_cast<double>(2 * j + 1 - m) / dm * std::numbers::pi));  // TODO: Check, if that its correct!
                }
                for (j = 0; j < window_size; j++) {
                    win_coef[j] = std::pow(win_coef[j], beta);
                }
                break;
            }

            case WindowType::Sine: {
                for (j = 0; j < window_size / 2; j++) {
                    win_coef[j] = std::sin(static_cast<float>(j + 1) * pi / dm);
                }
                for (j = 0; j < window_size / 2; j++) {
                    win_coef[j] = std::pow(win_coef[j], beta);
                }
                break;
            }

            case WindowType::Hanning: {
                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = 0.5f - 0.5f * std::cos(static_cast<float>(j + 1) * tau / dm);
                }
                break;
            }

            case WindowType::Hamming:
            {
                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = 0.54f - 0.46f * std::cos(static_cast<float>(j + 1) * tau / dm);
                }
            }
                break;

            case WindowType::Blackman:
            {
                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = 0.42f
                                  - 0.50f * std::cos(static_cast<float>(j + 1) * tau / dm)
                                  + 0.08f * std::cos(static_cast<float>(j + 1) * tau * 2.0f / dm);
                }
            }
                break;

            case WindowType::FlatTop:
            {
                for (j = 0; j <= m / 2; j++) {
                    win_coef[j] = 1
                                  - 1.93293488969227f * std::cos(static_cast<float>(j + 1) * tau / dm)
                                  + 1.28349769674027f * std::cos(static_cast<float>(j + 1) * tau * 2.0f / dm)
                                  - 0.38130801681619f * std::cos(static_cast<float>(j + 1) * tau * 3.0f / dm)
                                  + 0.02929730258511f * std::cos(static_cast<float>(j + 1) * tau * 4.0f / dm);
                }
            }
                break;

            case WindowType::BlackmanHarris:
            {
                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = 0.35875f
                                  - 0.48829f * std::cos(static_cast<float>(j + 1) * tau / dm)
                                  + 0.14128f * std::cos(static_cast<float>(j + 1) * tau * 2.0f / dm)
                                  - 0.01168f * std::cos(static_cast<float>(j + 1) * tau * 3.0f / dm);
                }
            }
                break;

            case WindowType::BlackmanNuttall:
            {
                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = 0.3535819f
                                  - 0.4891775f * std::cos(static_cast<float>(j + 1) * tau / dm)
                                  + 0.1365995f * std::cos(static_cast<float>(j + 1) * tau * 2.0f / dm)
                                  - 0.0106411f * std::cos(static_cast<float>(j + 1) * tau * 3.0f / dm);
                }
            }
                break;

            case WindowType::Nuttall:
            {
                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = 0.355768f
                                  - 0.487396f * std::cos(static_cast<float>(j + 1) * tau / dm)
                                  + 0.144232f * std::cos(static_cast<float>(j + 1) * tau * 2.0f / dm)
                                  - 0.012604f * std::cos(static_cast<float>(j + 1) * tau * 3.0f / dm);
                }
            }
                break;

            case WindowType::KaiserBessel:
            {
                for (j = 0; j <= m / 2; j++) {
                    win_coef[j] = 0.402f
                                  - 0.498f * std::cos(tau * static_cast<float>(j + 1) / dm)
                                  + 0.098f * std::cos(2.0f * tau * static_cast<float>(j + 1) / dm)
                                  + 0.001f * std::cos(3.0f * tau * static_cast<float>(j + 1) / dm);
                }
            }
                break;

            case WindowType::Trapezoid:
            {
                // Rectangle for alpha = 1, triangle for alpha = 0
                int32_t k = m / 2;
                if (m % 2) {
                    k++;
                }
                for (j = 0; j < k; j++) {
                    win_coef[j] = static_cast<float>(j + 1) / static_cast<float>(k);
                }
            }
                break;

            case WindowType::Gauss:
            {
                // This definition is from http://en.wikipedia.org/wiki/Window_function (Gauss Generalized normal window)
                // We set their p = 2, and use alpha in the numerator, instead of sigma in the denominator, as most others do.
                // alpha = M_E puts the Gauss window response midway between the Hanning and the Flattop (basically what we want).
                // It also gives the same bw as the Gauss window used in the HP 89410A Vector Signal Analyzer.
                // alpha = 1.8 puts it quite close to the Hanning.

                for (j = 0; j < m / 2; j++) {
                    win_coef[j] = (static_cast<float>(j + 1) - static_cast<float>(dm) / 2.0f) / (dm / 2.0f) * static_cast<float>(std::numbers::e);
                    win_coef[j] *= win_coef[j];
                    win_coef[j] = std::exp(-win_coef[j]);
                }
            }
                break;
        }

        // Fold the coefficients over
        for (j = 0; j < m / 2; j++) {
            win_coef[window_size - j - 1] = win_coef[j];
        }

        // This is the flat top if alpha > 0. Cannot be applied to a Kaiser or Flat Top
        if (window_type != WindowType::Kaiser && window_type != WindowType::FlatTop) {
            for (j = m / 2; j < window_size - m / 2; j++) {
                win_coef[j] = 1;
            }
        }

        // This will set the gain of the window to 1. Only the Flattop window has unity gain by design
        if (unity_gain) {

            float sum = 0.0f;

            for (j = 0; j < window_size; j++) {
                sum += win_coef[j];
            }

            sum /= static_cast<float>(window_size);

            if (sum != 0.0) {
                for (j = 0; j < window_size; j++) {
                    win_coef[j] /= sum;
                }
            }
        }

        // The result is return via pointer `out_data`
        return ErrorCode::None;
    }


} // End of namespace Grain
