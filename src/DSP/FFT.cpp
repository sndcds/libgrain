//
//  FFT.hpp
//
//  Created by Roald Christesen on 25.04.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "DSP/FFT.hpp"
#include "Type/Type.hpp"
#include "Math/Math.hpp"
#include "DSP/Partials.hpp"

#include <cmath>


namespace Grain {

    FFT::FFT(int32_t log_n) noexcept {
        if (log_n >= kLogNResolutionFirst && log_n <= kLogNResolutionLast) {
            m_log_n = log_n;
            m_len = 1 << log_n;
            m_half_len = m_len / 2;
            m_io_buffer = static_cast<float*>(fft_alloc(sizeof(float) * m_len));

#if defined(__APPLE__) && defined(__MACH__)
            m_split_complex.realp = static_cast<float*>(fft_alloc(sizeof(float) * m_half_len));
            m_split_complex.imagp = static_cast<float*>(fft_alloc(sizeof(float) * m_half_len));
            vDSP_vclr(m_split_complex.imagp, 1, m_half_len); // Zero imaginary parts
            m_fft_setup = vDSP_create_fftsetup(log_n, kFFTRadix2);
#else
            m_io_buffer = static_cast<float*>(std::malloc(sizeof(float) * m_len));
            m_mag = static_cast<float*>(std::malloc(sizeof(float) * m_half_len));
            m_phase = static_cast<float*>(std::malloc(sizeof(float) * m_half_len));

            // Allocate FFTW output once
            m_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (m_half_len + 1));

            // Create plan once
            m_plan = fftwf_plan_dft_r2c_1d(m_len, m_io_buffer, m_out, FFTW_ESTIMATE);
            m_plan_inv = fftwf_plan_dft_c2r_1d(m_len, m_out, m_io_buffer, FFTW_ESTIMATE);
#endif
        }
    }


    FFT::~FFT() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        if (m_split_complex.realp) { fft_free(m_split_complex.realp); m_split_complex.realp = nullptr; }
        if (m_split_complex.imagp) { fft_free(m_split_complex.imagp); m_split_complex.imagp = nullptr; }
        if (m_fft_setup) { vDSP_destroy_fftsetup(m_fft_setup); m_fft_setup = nullptr; }
        if (m_io_buffer) { fft_free(m_io_buffer); m_io_buffer = nullptr; }
#else
        fftwf_destroy_plan(m_plan);
        fftwf_destroy_plan(m_plan_inv);
        fftwf_free(m_out);
        std::free(m_mag);
        std::free(m_phase);
        fft_free(m_io_buffer);
#endif

    }


    /**
     *  @brief Checks if a given resolution is valid for the FFT.
     *
     *  A valid resolution must satisfy all of the following:
     *  - Be a power of two.
     *  - Be at least 64.
     *  - Be no greater than 16,777,216 (2^24).
     *
     *  @param resolution The FFT resolution to check.
     *  @return true If the resolution is valid.
     *  @return false If the resolution is invalid.
     */
    bool FFT::isValidResolution(int32_t resolution) noexcept {
        return
            (resolution >= (1 << 6)) &&
            (resolution <= (1 << 24)) &&
            ((resolution & (resolution - 1)) == 0);
    }


#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode FFT::fft(float* samples) noexcept {
        if (!samples) {
            return ErrorCode::NullData;
        }

        // Convert real input (samples) into split-complex form
        // Treat samples as interleaved complex (real + imag=0)
        vDSP_ctoz(
                reinterpret_cast<const DSPComplex*>(samples),
                2, // stride through input (real+imag pairs)
                &m_split_complex,
                1, // stride through output split-complex
                m_half_len); // number of complex samples (N/2)

        // Perform in-place forward FFT
        vDSP_fft_zrip(
                m_fft_setup,
                &m_split_complex,
                1,
                m_log_n,
                kFFTDirection_Forward);

        return ErrorCode::None;
    }
#else
    ErrorCode FFT::fft(float* data, Partials* out_partials) noexcept {
        if (!data || !out_partials) {
            return ErrorCode::NullData;
        }
        if (Math::log2IfPowerOfTwo(out_partials->resolution() * 2) != m_log_n) {
            return ErrorCode::BadArgs;
        }

        // Copy input into internal buffer used by plan
        std::memcpy(m_io_buffer, data, sizeof(float) * m_len);

        // Execute the pre-created r2c plan: m_io_buffer -> m_out
        fftwf_execute(m_plan);

        // Fill out_partials in CARTESIAN form (no scaling)
        out_partials->setCartesian();
        float* out_re = out_partials->mutRealData();
        float* out_im = out_partials->mutImagData();
        for (int32_t i = 0; i <= m_half_len; i++) {
            out_re[i] = m_out[i][0];
            out_im[i] = m_out[i][1];
        }
        return ErrorCode::None;
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode FFT::ifft(float* out_samples) noexcept {
        if (!out_samples) {
            return ErrorCode::NullData;
        }

        // Inverse FFT in-place on split-complex
        vDSP_fft_zrip(m_fft_setup,
                      &m_split_complex,
                      1,
                      m_log_n,
                      kFFTDirection_Inverse);

        // Scale the result by 1/N (vDSP does not scale automatically)
        float scale = 1.0f / static_cast<float>(m_len);
        vDSP_vsmul(m_split_complex.realp, 1, &scale, m_split_complex.realp, 1, m_half_len);
        vDSP_vsmul(m_split_complex.imagp, 1, &scale, m_split_complex.imagp, 1, m_half_len);

        // Convert split-complex back into interleaved complex
        // This will produce {real, imag} pairs
        vDSP_ztoc(
                &m_split_complex,
                1,
                reinterpret_cast<DSPComplex*>(out_samples),
                2,
                m_half_len);

        // Now out_samples contains interleaved real/imag data,
        // but since we started with purely real input,
        // the real values are in out_samples[0], out_samples[2], ...
        // and the imaginary parts should be ~0 in out_samples[1], out_samples[3], ...

        return ErrorCode::None;
    }
#else
    ErrorCode FFT::ifft(Partials* partials, float* out_data) noexcept {
        if (!partials || !out_data) {
            return ErrorCode::NullData;
        }

        if (!partials->isCartesian()) {
            return ErrorCode::BadArgs; // or your custom error
        }

        if (Math::log2IfPowerOfTwo(partials->resolution() * 2) != m_log_n) {
            return ErrorCode::BadArgs;
        }

        float* real = partials->mutRealData();
        float* imag = partials->mutImagData();

        // Fill FFTW complex input (m_out), same layout as FFT produced
        for (int32_t i = 0; i <= m_half_len; i++) {
            m_out[i][0] = real[i];
            m_out[i][1] = imag[i];
        }

        // Execute c2r plan producing N real samples in m_io_buffer (unscaled: sum)
        // reuse a plan cached for (m_out -> m_io_buffer) created via:
        // fftwf_plan_dft_c2r_1d(m_len, m_out, m_io_buffer, FFTW_ESTIMATE)
        fftwf_execute(m_plan_inv);

        // Normalize: FFTW's inverse returns sum; divide by N
        const float invN = 1.0f / static_cast<float>(m_len);
        for (int i = 0; i < m_len; i++) {
            m_io_buffer[i] *= invN;
        }

        // Copy result to out_data (you can also write directly)
        std::memcpy(out_data, m_io_buffer, sizeof(float) * m_len);

        return ErrorCode::None;
    }
#endif


    ErrorCode FFT::filter(const Partials* partials) noexcept {
        m_split_complex.realp[0] *= partials->dc();
        m_split_complex.imagp[0] *= partials->mag(partials->resolution() - 1);

        auto m = partials->mutMagPtr();
        for (int k = 1; k < m_half_len; k++) {
            m_split_complex.realp[k] *= *m;
            m_split_complex.imagp[k] *= *m;
            m++;
        }

        return ErrorCode::None;
    }


#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode FFT::setPartials(const Partials* partials) noexcept {
        if (!partials) {
            return ErrorCode::NullPointer;
        }

        if (partials->resolution() != m_half_len) {
            return ErrorCode::UnsupportedSettings;
        }

        auto scale = static_cast<float>(m_len) * 0.5f;

        // DC component: purely real, no phase
        m_split_complex.realp[0] = partials->dc() * scale;

        // Nyquist component: purely real in FFT representation
        m_split_complex.imagp[0] = partials->mag(m_half_len - 1) * scale;

        auto m = partials->mutMagPtr();
        auto p = partials->mutPhasePtr();

        for (int k = 1; k < m_half_len; k++) {
            float mag = m[k - 1];
            float phase = p[k - 1];
            float re = mag * std::cos(phase);
            float im = mag * std::sin(phase);
            m_split_complex.realp[k] = re * scale;
            m_split_complex.imagp[k] = im * scale;
        }

        return ErrorCode::None;
    }
#else
    ErrorCode FFT::setPartials(const Partials* partials) noexcept {
        #pragma message("FFT::setFilter() must be implemented for Linux")
    }
#endif


    ErrorCode FFT::getPartials(Partials* out_partials) noexcept {
        if (!out_partials) {
            return ErrorCode::NullPointer;
        }

        if (out_partials->resolution() != m_half_len) {
            return ErrorCode::UnsupportedSettings;
        }

        float scale = 1.0f / static_cast<float>(m_len);

        // DC component: purely real, no phase
        out_partials->setDC(m_split_complex.realp[0] * scale);

        // In vDSP, imagp[0] is actually the Nyquist component
        out_partials->setMagNyquist(std::fabs(m_split_complex.imagp[0]) * scale);

        auto m = out_partials->mutMagPtr();
        auto p = out_partials->mutPhasePtr();

        for (int k = 1; k < m_half_len; k++) {
            float re = m_split_complex.realp[k] * scale;
            float im = m_split_complex.imagp[k] * scale;
            *m++ = std::sqrt(re * re + im * im);  // magnitude
            *p++ = std::atan2(im, re);            // phase in radians (-π .. π)
        }

        return ErrorCode::None;
    }


#if defined(__APPLE__) && defined(__MACH__)
    void FFT::shiftPhase(int32_t bin_index, float delta) noexcept {
        if (bin_index >= 0 && bin_index < m_half_len - 1) {
            float scale = 1.0f / static_cast<float>(m_len);
            int32_t bi = bin_index + 1;
            float re = m_split_complex.realp[bi] * scale;
            float im = m_split_complex.imagp[bi] * scale;
            float mag = std::sqrt(re * re + im * im);
            float phase = std::atan2(im, re);
            phase += delta;
            m_split_complex.realp[bi] = mag * std::cos(phase) * m_len;
            m_split_complex.imagp[bi] = mag * std::sin(phase) * m_len;
        }
    }
#else
    ErrorCode FFT::setPartials(const Partials* partials) noexcept {
        #pragma message("FFT::shiftPhase() must be implemented for Linux")
    }
#endif


    SlidingDFT::SlidingDFT(int32_t dft_len) noexcept {
        m_dft_len = std::clamp(dft_len, 128, 100000);

        _m_x = (double*)std::malloc(sizeof(double) * m_dft_len);
        _m_twiddle = (std::complex<double>*)std::malloc(sizeof(std::complex<double>) * m_dft_len);
        _m_s = (std::complex<double>*)std::malloc(sizeof(std::complex<double>) * m_dft_len);
        _m_dft = (std::complex<double>*)std::malloc(sizeof(std::complex<double>) * m_dft_len);
        _m_damping_factor = std::nexttoward(1.0, 0.0);

        const std::complex<double> j(0.0, 1.0);
        const double N = m_dft_len;

        // Compute the twiddle factors, and zero the x and S arrays
        for (int32_t k = 0; k < m_dft_len; k++) {
            double factor = std::numbers::pi * 2 * k / N;
            _m_twiddle[k] = std::exp(j * factor);
            _m_s[k] = 0;
            _m_x[k] = 0;
        }
    }


    SlidingDFT::~SlidingDFT() noexcept {
        std::free(_m_x);
        std::free(_m_twiddle);
        std::free(_m_s);
        std::free(_m_dft);
    }


    /**
     *  @brief Get the frequency of a DFT bin.
     *
     *  @param bin_index The index of the DFT bin
     *  @param sample_rate The sampling rate in Hz (e.g., 48000)
     *  @return Frequency of the bin in Hz
     */
    double SlidingDFT::binFreq(uint32_t bin_index, double sample_rate) {
        return (sample_rate * bin_index) / m_dft_len;
    }


    bool SlidingDFT::push(float value) noexcept {
        // Update the calculation with a new sample
        // Returns true if the data are valid (because enough samples have been presented),
        // or false if the data are invalid

        // Update the storage of the time domain values
        const double new_x = value;
        const double old_x = _m_x[_m_x_index];
        _m_x[_m_x_index] = value;

        // Update the DFT
        const double r = _m_damping_factor;
        const double r_to_n = std::pow(r, static_cast<double>(m_dft_len));
        for (int32_t k = 0; k < m_dft_len; k++) {
            _m_s[k] = _m_twiddle[k] * (r * _m_s[k] - r_to_n * old_x + new_x);
        }

        // Apply the Hanning window
        _m_dft[0] = 0.5 * _m_s[0] - 0.25 * (_m_s[m_dft_len - 1] + _m_s[1]);
        for (int32_t k = 1; k < (m_dft_len - 1); k++) {
            _m_dft[k] = 0.5 * _m_s[k] - 0.25 * (_m_s[k - 1] + _m_s[k + 1]);
        }

        _m_dft[m_dft_len - 1] = 0.5 * _m_s[m_dft_len - 1] - 0.25 * (_m_s[m_dft_len - 2] + _m_s[0]);

        // Increment the counter
        _m_x_index++;
        if (_m_x_index >= m_dft_len) {
            _m_data_valid = true;
            _m_x_index = 0;
        }

        return _m_data_valid;
    }


}  // End of namespace Grain
