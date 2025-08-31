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
            m_data = (float*)std::malloc(sizeof(float) * m_length * 4);


            // Buffers for real (time-domain) input and output signals
            int32_t offset = 0;
            m_x_buffer = &m_data[offset]; offset += m_length;
            m_y_buffer = &m_data[offset]; offset += m_length;
            m_real_part = &m_data[offset]; offset += m_half_length;
            m_imag_part = &m_data[offset]; offset += m_half_length;
            m_mag = &m_data[offset]; offset += m_half_length;
            m_phase = &m_data[offset]; offset += m_half_length;

            #if defined(__APPLE__) && defined(__MACH__)
                m_fft_setup = _macos_fftSetup(log_n);
                // We need complex buffers in two different formats!
                m_temp_complex = (DSPComplex*)std::malloc(sizeof(DSPComplex) * m_half_length);
                m_valid = m_fft_setup && m_data && m_temp_complex;
            #endif
        }
    }


    FFT::~FFT() noexcept {

        std::free(m_data);

        #if defined(__APPLE__) && defined(__MACH__)
            std::free(m_temp_complex);
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

// Returns true, if resolution is FFT compatible
    bool FFT::isValidResolution(int32_t resolution) noexcept {

        return (resolution >= (1 << 6)) && (resolution <= (1 << 24)) &&
               ((resolution & (resolution - 1)) == 0);
    }


    int32_t FFT::logNFromResolution(int32_t resolution) noexcept {

        return isValidResolution(resolution) ? Math::pow_inverse(resolution) : -1;
    }


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
        // TODO: Reuse settings and memory!
        int32_t N = m_length;
        int32_t half_n = N / 2;

        // Create FFTW plan (real -> complex)
        fftwf_complex* out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (half_n + 1));
        fftwf_plan plan = fftwf_plan_dft_r2c_1d(N, data, out, FFTW_ESTIMATE);

        fftwf_execute(plan);

        // Fill Partials
        for (int k = 0; k <= half_n; ++k) {
            float re = out[k][0];
            float im = out[k][1];
            for (int32_t i = 0; i < m_half_length; i++) {
                out_partials->setPartialAtIndex(i, std::sqrt(re * re + im * im), std::atan2(im, re));
            }
        }

        fftwf_destroy_plan(plan);
        fftwf_free(out);

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
        // TODO: Reuse settings and memory!
        int32_t N = m_length;
        int32_t half_n = N / 2;

        fftwf_complex* in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (half_n + 1));

        // Convert amplitudes + phases back to complex numbers
        for (int k = 0; k <= half_n; ++k) {
            float amp;
            float phase;
            partials->partialAtIndex(k, amp, phase);
            in[k][0] = amp * std::cosf(phase); // real
            in[k][1] = amp * std::sinf(phase); // imaginary
        }

        // Create FFTW plan (complex -> real)
        fftwf_plan plan = fftwf_plan_dft_c2r_1d(N, in, out_data, FFTW_ESTIMATE);

        fftwf_execute(plan);

        // Normalize output
        for (int i = 0; i < N; ++i) {
            out_data[i] /= N;
        }

        fftwf_destroy_plan(plan);
        fftwf_free(in);

        return ErrorCode::OK;
    }
#endif


    FFT_FIR::FFT_FIR(int32_t log_n) noexcept {
        log_n = std::clamp(log_n, static_cast<int32_t>(FFT::kLogNResolutionFirst), static_cast<int32_t>(FFT::kLogNResolutionLast));

        m_log_n = log_n;
        m_step_length = 1 << log_n;

        m_filter_length = m_step_length;
        m_overlap_length = m_step_length;
        m_signal_length = m_step_length + m_overlap_length;

        m_fft_length = static_cast<int32_t>(Math::next_pow2(m_signal_length));  // TODO: next_pow2 or pad_two?
        m_fft_half_length = m_fft_length / 2;

        m_filter_samples = (float*)std::malloc(sizeof(float) * m_filter_length);
        m_signal_samples = (float*)std::malloc(sizeof(float) * m_signal_length);
        m_convolved_samples = (float*)std::malloc(sizeof(float) * m_signal_length);
        m_filter_padded = (float*)std::malloc(sizeof(float) * m_fft_length);
        m_signal_padded = (float*)std::malloc(sizeof(float) * m_fft_length);
        m_filter_result = (float*)std::malloc(sizeof(float) * m_fft_length);
        m_signal_real = (float*)std::malloc(sizeof(float) * m_fft_half_length);
        m_signal_imag = (float*)std::malloc(sizeof(float) * m_fft_half_length);

        // TODO: Check all buffers againt nullptr

        #if defined(__APPLE__) && defined(__MACH__)
            m_fft_setup = FFT::_macos_fftSetup(std::log2(static_cast<float>(m_fft_length)));
            m_filter_real = (float*)std::malloc(sizeof(float) * m_fft_length);
            m_filter_imag = &m_filter_real[m_fft_half_length];
            m_filter_split_complex.realp = m_filter_real;
            m_filter_split_complex.imagp = m_filter_imag;
        #endif
    }


    FFT_FIR::~FFT_FIR() noexcept {
        std::free(m_filter_real);
        std::free(m_filter_samples);
        std::free(m_signal_samples);
        std::free(m_convolved_samples);
        std::free(m_filter_padded);
        std::free(m_signal_padded);
        std::free(m_filter_result);
        std::free(m_signal_real);
        std::free(m_signal_imag);
    }


#if defined(__APPLE__) && defined(__MACH__)
    void FFT_FIR::setFilter() noexcept {
        float zero = 0;
        vDSP_vfill(&zero, m_filter_padded, 1, m_fft_length);
        cblas_scopy(m_filter_length, m_filter_samples, 1, m_filter_padded, 1);

        vDSP_ctoz((DSPComplex*)m_filter_padded, 2, &m_filter_split_complex, 1, m_fft_half_length);
        vDSP_fft_zrip(m_fft_setup, &m_filter_split_complex, 1, std::log2f(m_fft_length), FFT_FORWARD);
    }
#else
    void FFT_FIR::setFilter() noexcept {
        // TODO: Implement linux version
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    void FFT_FIR::filter() noexcept {

        float zero = 0;
        vDSP_vfill(&zero, m_signal_padded, 1, m_fft_length);
        cblas_scopy(m_signal_length, m_signal_samples, 1, m_signal_padded, 1);

        DSPSplitComplex signal_split_complex;
        signal_split_complex.realp = m_signal_real;
        signal_split_complex.imagp = m_signal_imag;

        vDSP_ctoz((DSPComplex*)m_signal_padded, 2, &signal_split_complex, 1, m_fft_half_length);
        vDSP_fft_zrip(m_fft_setup, &signal_split_complex, 1, std::log2f(m_fft_length), FFT_FORWARD);

        // This gets a bit strange. The vDSP FFT stores the real value at nyquist in the
        // first element in the imaginary array. The first imaginary element is always
        // zero, so no information is lost by doing this. The only issue is that we are
        // going to use a complex vector multiply function from vDSP and it doesn't
        // handle this format very well. We calculate this multiplication ourselves and
        // add it into our result later.

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
        // TODO: Implement linux version
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
