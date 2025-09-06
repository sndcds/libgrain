//
//  FFT.hpp
//
//  Created by Roald Christesen on 25.04.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

/*
 *  TODO: Check these ...
 *  - cblas_scopy is deprecated
 */

#include "DSP/FFT.hpp"
#include "Type/Type.hpp"
#include "Math/Math.hpp"
#include "DSP/Partials.hpp"

#include <cmath>


namespace Grain {

#if defined(__APPLE__) && defined(__MACH__)
    FFTSetup FFT::g_fft_setups[kLogNResolutionCount] = { nullptr };
#endif


    FFT::FFT(int32_t log_n) noexcept {
        if (log_n >= kLogNResolutionFirst && log_n <= kLogNResolutionLast) {
            m_log_n = log_n;
            m_length = 1 << log_n;
            m_half_length = m_length / 2;


#if defined(__APPLE__) && defined(__MACH__)
            m_data = static_cast<float*>(std::malloc(sizeof(float) * m_length * 4));

            // Buffers for real (time-domain) input and output signals
            int32_t offset = 0;
            m_x_buffer = &m_data[offset]; offset += m_length;
            m_y_buffer = &m_data[offset]; offset += m_length;
            m_real_part = &m_data[offset]; offset += m_half_length;
            m_imag_part = &m_data[offset]; offset += m_half_length;
            m_mag = &m_data[offset]; offset += m_half_length;
            m_phase = &m_data[offset]; offset += m_half_length;

            m_fft_setup = _macos_fftSetup(log_n);
            // We need complex buffers in two different formats!
            m_temp_complex = (DSPComplex*)std::malloc(sizeof(DSPComplex) * m_half_length);
            m_valid = m_fft_setup && m_data && m_temp_complex;
#else
            m_x_buffer = static_cast<float*>(std::malloc(sizeof(float) * m_length));
            m_mag = static_cast<float*>(std::malloc(sizeof(float) * m_half_length));
            m_phase = static_cast<float*>(std::malloc(sizeof(float) * m_half_length));

            // Allocate FFTW output once
            m_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (m_half_length + 1));

            // Create plan once
            m_plan = fftwf_plan_dft_r2c_1d(m_length, m_x_buffer, m_out, FFTW_ESTIMATE);
#endif
        }
    }


    FFT::~FFT() noexcept {

#if defined(__APPLE__) && defined(__MACH__)
        std::free(m_data);
        std::free(m_x_buffer);
        std::free(m_mag);
        std::free(m_phase);
        std::free(m_temp_complex);
#else
        fftwf_destroy_plan(m_plan);
        fftwf_free(m_out);
        std::free(m_x_buffer);
        std::free(m_mag);
        std::free(m_phase);
#endif
    }


#if defined(__APPLE__) && defined(__MACH__)
    FFTSetup FFT::_macos_fftSetup(int32_t log_n) noexcept {
        FFTSetup fft_setup = nullptr;

        if (log_n >= kLogNResolutionFirst && log_n <= kLogNResolutionLast) {
            int32_t index = log_n - kLogNResolutionFirst;

            if (!g_fft_setups[index]) {    // Must be initialized.
                g_fft_setups[index] = vDSP_create_fftsetup(log_n, kFFTRadix2);
            }

            fft_setup = g_fft_setups[index];
        }

        return fft_setup;
    }
#endif


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


    /**
     *  @brief Computes the base-2 logarithm of a given FFT resolution.
     *
     *  This function calculates log2(resolution) by counting the number
     *  of times the resolution can be divided by 2 until it reaches 0.
     *
     *  @note The function does not check if the resolution is a power of two.
     *        Use FFT::isValidResolution() before calling this function if necessary.
     *
     *  @param resolution The FFT resolution (number of points).
     *  @return int32_t The base-2 logarithm of the resolution.
     *                  For example, if resolution = 1024, returns 10.
     */
    int32_t FFT::logNFromResolution(int32_t resolution) noexcept {
        // Previous version: return isValidResolution(resolution) ? Math::pow_inverse(resolution) : -1;
        int32_t log_n = 0;
        while (resolution >>= 1) {
            log_n++;
        }
        return log_n;
    }


    /**
     *  @brief Computes the FFT resolution from a base-2 logarithm.
     *
     *  Given the logarithm of the FFT length (log_n), this function returns
     *  the corresponding number of points in the FFT.
     *
     *  @param log_n The base-2 logarithm of the FFT length.
     *  @return int32_t The FFT resolution (number of points), equal to 2^log_n.
     *                  For example, if log_n = 10, returns 1024.
     */
    int32_t FFT::resolutionFromLogN(int32_t log_n) noexcept {
        return 1 << log_n;
    }


#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode FFT::fft(float* data, Partials* out_partials) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!data || !out_partials) {
                throw ErrorCode::NullData;
            }

            if (FFT::logNFromResolution(out_partials->resolution() * 2) != m_log_n) {
                throw ErrorCode::BadArgs;
            }

            DSPSplitComplex temp_split_complex;
            float* mag = m_mag;
            float* phase = m_phase;

            float* s = data;
            float* d = m_x_buffer;
            for (int32_t i = 0; i < m_length; i++) {
                *d++ = *s++;
            }

            // Scramble-pack the real data into complex buffer in just the way that's
            // required by the real-to-complex FFT function that follows
            temp_split_complex.realp = m_real_part;
            temp_split_complex.imagp = m_imag_part;
            vDSP_ctoz((DSPComplex*)m_x_buffer, 2, &temp_split_complex, 1, m_half_length);

            // Do real->complex forward FFT
            vDSP_fft_zrip(m_fft_setup, &temp_split_complex, 1, m_log_n, kFFTDirection_Forward);

            // Convert from complex/rectangular (real, imaginary) coordinates
            // to polar (magnitude and phase) coordinates

            // Compute magnitude and phase. Can also be done using vDSP_polar
            vDSP_zvabs(&temp_split_complex, 1, mag, 1, m_half_length);
            vDSP_zvphas(&temp_split_complex, 1, phase, 1, m_half_length);

            // Write result to 'outPartials'
            float fft_norm_factor = 1.0f / std::pow(2.0f, m_log_n);  // Scaling factor
            for (int32_t i = 0; i < m_half_length; i++) {
                out_partials->setPartialAtIndex(i, mag[i] * fft_norm_factor, phase[i]);
            }

            return ErrorCode::None;
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }
#else
    ErrorCode FFT::fft(float* data, Partials* out_partials) noexcept {
        if (!data || !out_partials) return ErrorCode::NullData;

        // Copy input to internal buffer
        std::memcpy(m_x_buffer, data, sizeof(float) * m_length);

        // Execute FFT
        fftwf_execute(m_plan);

        // Normalize factor (same as vDSP)
        float fft_norm_factor = 1.0f / m_length;

        // Fill Partials
        for (int32_t k = 0; k <= m_half_length; ++k) {
            float re = m_out[k][0];
            float im = m_out[k][1];
            float mag = std::sqrt(re * re + im * im) * fft_norm_factor;
            float phase = std::atan2(im, re);
            out_partials->setPartialAtIndex(k, mag, phase);
        }

        return ErrorCode::None;
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode FFT::ifft(Partials* partials, float* out_data) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!partials || !out_data) {
                throw ErrorCode::NullData;
            }

            if (FFT::logNFromResolution(partials->resolution() * 2) != m_log_n) {
                std::cout << "partials->resolution() * 2: " << (partials->resolution() * 2) << std::endl;
                std::cout << "FFT::logNFromResolution(partials->resolution() * 2): " << FFT::logNFromResolution(partials->resolution() * 2) << std::endl;
                std::cout << "m_log_n: " << m_log_n << std::endl;
                throw ErrorCode::BadArgs;
            }

            DSPSplitComplex temp_split_complex;
            float* mag = m_mag;
            float* phase = m_phase;

            float f = std::pow(2, m_log_n);  // Scaling factor
            for (int32_t i = 0; i < m_half_length; i++) {
                partials->partialAtIndex(i, mag[i], phase[i]);
                mag[i] *= f;
            }

            // Convert from polar coordinates back to rectangular coordinates
            temp_split_complex.realp = mag;
            temp_split_complex.imagp = phase;
            vDSP_ztoc(&temp_split_complex, 1, m_temp_complex, 2, m_half_length);
            vDSP_rect((float*)m_temp_complex, 2, (float*)m_temp_complex, 2, m_half_length);
            vDSP_ctoz(m_temp_complex, 2, &temp_split_complex, 1, m_half_length);

            // Do complex->real inverse FFT
            vDSP_fft_zrip(m_fft_setup, &temp_split_complex, 1, m_log_n, kFFTDirection_Inverse);

            // This leaves result in packed format. Here we unpack it into a real vector
            vDSP_ztoc(&temp_split_complex, 1, (DSPComplex*)m_y_buffer, 2, m_half_length);

            // Neither the forward nor inverse FFT does any scaling, here we compensate for that
            float scale = 0.5f / m_length;
            vDSP_vsmul(m_y_buffer, 1, &scale, m_y_buffer, 1, m_length);

            // Write result to 'outData'
            float* s = m_y_buffer;
            float* d = out_data;
            for (int32_t i = 0; i < m_length; i++) {
                *d++ = *s++;
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }
#else
    ErrorCode FFT::ifft(Partials* partials, float* out_data) noexcept {
        if (!partials || !out_data) return ErrorCode::NullData;

        // Fill FFTW input buffer from Partials
        for (int32_t k = 0; k <= m_half_length; ++k) {
            float mag = partials->amplitudeAtIndex(k);
            float phase = partials->phaseAtIndex(k);
            m_out[k][0] = mag * std::cos(phase) * m_length; // scale back
            m_out[k][1] = mag * std::sin(phase) * m_length;
        }

        // Create inverse plan on the fly or reuse one if needed
        fftwf_plan plan_inv = fftwf_plan_dft_c2r_1d(m_length, m_out, m_x_buffer, FFTW_ESTIMATE);
        fftwf_execute(plan_inv);
        fftwf_destroy_plan(plan_inv);

        // Copy result
        std::memcpy(out_data, m_x_buffer, sizeof(float) * m_length);

        return ErrorCode::None;
    }
#endif


    FFT_FIR::FFT_FIR(int32_t log_n) noexcept {
        // Clamp log_n
        log_n = std::clamp<int32_t>(log_n, FFT::kLogNResolutionFirst, FFT::kLogNResolutionLast);

        m_log_n = log_n;
        m_step_length = 1 << log_n;

        m_filter_length = m_step_length;
        m_overlap_length = m_step_length;
        m_signal_length = m_step_length + m_overlap_length;

        // FFT length: next power of 2 of signal + overlap
        m_fft_length = static_cast<int32_t>(Math::next_pow2(m_signal_length));
        m_fft_half_length = m_fft_length / 2;

        // Allocate basic buffers
        m_filter_samples = (float*)std::malloc(sizeof(float) * m_filter_length);
        m_signal_samples = (float*)std::malloc(sizeof(float) * m_signal_length);
        m_convolved_samples = (float*)std::malloc(sizeof(float) * m_signal_length);

#if defined(__APPLE__) && defined(__MACH__)
        // macOS vDSP buffers
        m_filter_padded = (float*)std::malloc(sizeof(float) * m_fft_length);
        m_signal_padded = (float*)std::malloc(sizeof(float) * m_fft_length);
        m_filter_result = (float*)std::malloc(sizeof(float) * m_fft_length);

        m_signal_real = (float*)std::malloc(sizeof(float) * m_fft_half_length);
        m_signal_imag = (float*)std::malloc(sizeof(float) * m_fft_half_length);

        m_fft_setup = FFT::_macos_fftSetup(std::log2f(static_cast<float>(m_fft_length)));

        m_filter_real = (float*)std::malloc(sizeof(float) * m_fft_length);
        m_filter_imag = &m_filter_real[m_fft_half_length];
        m_filter_split_complex.realp = m_filter_real;
        m_filter_split_complex.imagp = m_filter_imag;

#else
        // FFTW buffers
        // For in-place r2c, FFTW requires N + 2 floats
        m_signal_padded = fftwf_alloc_real(m_fft_length + 2);
        m_filter_padded = fftwf_alloc_real(m_fft_length); // filter padding is out-of-place
        m_filter_result = fftwf_alloc_real(m_fft_length); // for convolution result
        m_filter_fft = fftwf_alloc_complex(m_fft_half_length + 1);

        m_plan_fwd_signal = fftwf_plan_dft_r2c_1d(
                m_fft_length,
                m_signal_padded,
                reinterpret_cast<fftwf_complex*>(m_signal_padded),
                FFTW_ESTIMATE
        );

        m_plan_fwd_filter = fftwf_plan_dft_r2c_1d(
                m_fft_length,
                m_filter_padded,
                m_filter_fft,
                FFTW_ESTIMATE
        );

        m_plan_inv_signal = fftwf_plan_dft_c2r_1d(
                m_fft_length,
                reinterpret_cast<fftwf_complex*>(m_signal_padded),
                m_signal_padded,
                FFTW_ESTIMATE
        );
#endif
    }


    FFT_FIR::~FFT_FIR() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        if (m_fft_setup) { vDSP_destroy_fftsetup(m_fft_setup); }

        std::free(m_filter_real);
        std::free(m_signal_real);
        std::free(m_signal_imag);

        std::free(m_filter_padded);
        std::free(m_signal_padded);
        std::free(m_filter_result);
#else
        // Destroy FFTW plans before freeing buffers
        if (m_plan_fwd_filter) { fftwf_destroy_plan(m_plan_fwd_filter); }
        if (m_plan_fwd_signal) { fftwf_destroy_plan(m_plan_fwd_signal); }
        if (m_plan_inv_signal) { fftwf_destroy_plan(m_plan_inv_signal); }

        if (m_filter_fft) { fftwf_free(m_filter_fft); }
        if (m_filter_padded) { fftwf_free(m_filter_padded); }
        if (m_signal_padded) { fftwf_free(m_signal_padded); }
        if (m_filter_result) { fftwf_free(m_filter_result); }
#endif

        std::free(m_filter_samples);
        std::free(m_signal_samples);
        std::free(m_convolved_samples);
    }


#if defined(__APPLE__) && defined(__MACH__)
    void FFT_FIR::setFilter() noexcept {
        std::cout << m_log_n << " ... " << (std::log2f(m_fft_length)) << std::endl;
        float zero = 0;
        vDSP_vfill(&zero, m_filter_padded, 1, m_fft_length);
        cblas_scopy(m_filter_length, m_filter_samples, 1, m_filter_padded, 1);

        vDSP_ctoz((DSPComplex*)m_filter_padded, 2, &m_filter_split_complex, 1, m_fft_half_length);
        vDSP_fft_zrip(m_fft_setup, &m_filter_split_complex, 1, std::log2f(m_fft_length), FFT_FORWARD);
    }
#else
    void FFT_FIR::setFilter() noexcept {
        // Zero-pad filter and copy taps
        std::memset(m_filter_padded, 0, sizeof(float) * m_fft_length);
        cblas_scopy(m_filter_length, m_filter_samples, 1, m_filter_padded, 1);

        // Compute H[k] = FFT{h[n]} into m_filter_fft
        fftwf_execute(m_plan_fwd_filter);
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    void FFT_FIR::filter() noexcept {
        std::cout << m_log_n << " ... " << (std::log2f(m_fft_length)) << std::endl;
        float zero = 0;
        vDSP_vfill(&zero, m_signal_padded, 1, m_fft_length);
        cblas_scopy(m_signal_length, m_signal_samples, 1, m_signal_padded, 1);

        DSPSplitComplex signal_split_complex;
        signal_split_complex.realp = m_signal_real;
        signal_split_complex.imagp = m_signal_imag;

        vDSP_ctoz((DSPComplex*)m_signal_padded, 2, &signal_split_complex, 1, m_fft_half_length);
        vDSP_fft_zrip(m_fft_setup, &signal_split_complex, 1, std::log2f(m_fft_length), FFT_FORWARD);

        // The vDSP FFT stores the real value at nyquist in the first element in the
        // imaginary array. The first imaginary element is always zero, so no information
        // is lost by doing this. The only issue is that we are going to use a complex
        // vector multiply function from vDSP and it doesn't handle this format very well.
        // We calculate this multiplication ourselves and add it into our result later.

        // We'll need this later
        float nyquist_multiplied = m_filter_split_complex.imagp[0] * signal_split_complex.imagp[0];

        // Set the values in the arrays to 0 so they don't affect the multiplication
        m_filter_split_complex.imagp[0] = 0.0;
        signal_split_complex.imagp[0] = 0.0;

        // Complex multiply x_DFT by h_DFT and store the result in x_DFT
        vDSP_zvmul(&signal_split_complex, 1, &m_filter_split_complex, 1, &signal_split_complex,1, m_fft_half_length, 1);

        // Now we put our hand-calculated nyquist value back
        signal_split_complex.imagp[0] = nyquist_multiplied;

        // Do the inverse FFT of our result
        vDSP_fft_zrip(m_fft_setup, &signal_split_complex, 1, std::log2f(m_fft_length), FFT_INVERSE);

        // Now we have to scale our result by 1 / (4 * fft_length)
        // (This is Apple's convention) to get our correct result
        float scale = 1.0f / (128.0f * m_fft_length);
        vDSP_vsmul(signal_split_complex.realp, 1, &scale, signal_split_complex.realp, 1, m_fft_half_length);
        vDSP_vsmul(signal_split_complex.imagp, 1, &scale, signal_split_complex.imagp, 1, m_fft_half_length);

        // And convert split-complex format to real-valued
        vDSP_ztoc(&signal_split_complex, 1, (DSPComplex*)m_filter_result, 2, m_fft_half_length);

        /* TODO: !!!!!
        // Now we have to scale our result by 1 / (4 * fft_length)
        // (This is Apple's convention) to get our correct result
        float scale = (1.0 / (4.0 * m_fft_length));
        vDSP_vsmul(tempResult, 1, &scale, tempResult, 1, m_fft_length);  // m_fft_half_length
         */

        // write the final result to the output. use BLAS copy instead of loop
        cblas_scopy(m_signal_length, m_filter_result, 1, m_convolved_samples, 1);
    }
#else
    void FFT_FIR::filter() noexcept {
        // Zero-pad the input block
        std::memset(m_signal_padded, 0, sizeof(float) * m_fft_length);
        cblas_scopy(m_signal_length, m_signal_samples, 1, m_signal_padded, 1);

        // Forward FFT (r2c, in-place)
        std::cout
                << "plan_fwd_filter = " << m_plan_fwd_filter
                << ", m_filter_padded = " << m_filter_padded
                << ", m_filter_fft = " << m_filter_fft << std::endl;
        fftwf_execute(m_plan_fwd_signal);
        std::cout << "... 1\n";
        fftwf_complex* X = reinterpret_cast<fftwf_complex*>(m_signal_padded);
        std::cout << "... 2\n";

        // Multiply by filter spectrum H[k]
        const fftwf_complex* H = m_filter_fft;
        for (int k = 0; k <= m_fft_half_length; ++k) {
            float xr = X[k][0];
            float xi = X[k][1];
            float hr = H[k][0];
            float hi = H[k][1];

            // Complex multiplication: (xr + j xi)*(hr + j hi)
            X[k][0] = xr * hr - xi * hi; // real part
            X[k][1] = xr * hi + xi * hr; // imag part
        }
        std::cout << "... 3\n";

        // Inverse FFT (c2r, in-place)
        fftwf_execute(m_plan_inv_signal);
        std::cout << "... 4\n";

        // Scale the result
        // FFTW inverse does NOT scale by 1/N
        const float scale = 1.0f / static_cast<float>(m_fft_length);
        cblas_sscal(m_fft_length, scale, m_signal_padded, 1);
        std::cout << "... 5\n";

        // Copy valid samples to output
        cblas_scopy(m_signal_length, m_signal_padded, 1, m_convolved_samples, 1);
        std::cout << "... 6\n";
    }
#endif


    SlidingDFT::SlidingDFT(int32_t dft_length) noexcept {
        m_dft_length = std::clamp(dft_length, 128, 100000);

        _m_x = (double*)std::malloc(sizeof(double) * m_dft_length);
        _m_twiddle = (std::complex<double>*)std::malloc(sizeof(std::complex<double>) * m_dft_length);
        _m_s = (std::complex<double>*)std::malloc(sizeof(std::complex<double>) * m_dft_length);
        _m_dft = (std::complex<double>*)std::malloc(sizeof(std::complex<double>) * m_dft_length);
        _m_damping_factor = std::nexttoward(1.0, 0.0);

        const std::complex<double> j(0.0, 1.0);
        const double N = m_dft_length;

        // Compute the twiddle factors, and zero the x and S arrays
        for (int32_t k = 0; k < m_dft_length; k++) {
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
        return (sample_rate * bin_index) / m_dft_length;
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
        const double r_to_n = std::pow(r, static_cast<double>(m_dft_length));
        for (int32_t k = 0; k < m_dft_length; k++) {
            _m_s[k] = _m_twiddle[k] * (r * _m_s[k] - r_to_n * old_x + new_x);
        }

        // Apply the Hanning window
        _m_dft[0] = 0.5 * _m_s[0] - 0.25 * (_m_s[m_dft_length - 1] + _m_s[1]);
        for (int32_t k = 1; k < (m_dft_length - 1); k++) {
            _m_dft[k] = 0.5 * _m_s[k] - 0.25 * (_m_s[k - 1] + _m_s[k + 1]);
        }

        _m_dft[m_dft_length - 1] = 0.5 * _m_s[m_dft_length - 1] - 0.25 * (_m_s[m_dft_length - 2] + _m_s[0]);

        // Increment the counter
        _m_x_index++;
        if (_m_x_index >= m_dft_length) {
            _m_data_valid = true;
            _m_x_index = 0;
        }

        return _m_data_valid;
    }


}  // End of namespace Grain
