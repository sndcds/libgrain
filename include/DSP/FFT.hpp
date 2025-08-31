//
//  FFT.hpp
//
//  Created by Roald Christesen on 25.04.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainFFT_hpp
#define GrainFFT_hpp


#include "Grain.hpp"
#include "Type/Object.hpp"

#include <complex>

#if defined(__APPLE__) && defined(__MACH__)
#define ACCELERATE_NEW_LAPACK   // Use the updated LAPACK version in Accelerate
#define ACCELERATE_LAPACK_ILP64 // Use 64-bit integers in LAPACK functions
#include <Accelerate/Accelerate.h>
#else
#include <fftw3.h>
#endif


namespace Grain {

    class Partials;

    typedef struct {
        int32_t m_log_n;
        int32_t m_n;
        float* m_data;
        float* m_x_buffer;
        float* m_y_buffer;
        float* m_realp;
        float* m_imagp;
        float* m_mag;
        float *m_phase;
        #if defined(__APPLE__) && defined(__MACH__)
            FFTSetup m_fft_setup;
            DSPComplex* m_temp_complex;
        #endif
    } FFTConfig;


    class FFT : public Object {
    public:
        enum {
            kLogNResolution64 = 6,
            kLogNResolution128,
            kLogNResolution256,
            kLogNResolution512,
            kLogNResolution1024,
            kLogNResolution2048,
            kLogNResolution4096,
            kLogNResolution8192,
            kLogNResolution16384,
            kLogNResolution32768,
            kLogNResolution65536,
            kLogNResolution131072,
            kLogNResolution262144,
            kLogNResolution524288,

            kLogNResolutionFirst = kLogNResolution64,
            kLogNResolutionLast = kLogNResolution524288,
            kLogNResolutionCount = kLogNResolutionLast - kLogNResolutionFirst + 1
        };

    protected:
        #if defined(__APPLE__) && defined(__MACH__)
            static FFTSetup g_fft_setups[kLogNResolutionCount];
        #endif

        bool m_valid = false;
        int32_t m_log_n;
        int32_t m_length;
        int32_t m_half_length;
        float* m_data;
        float* m_x_buffer;
        float* m_y_buffer;
        float* m_real_part;
        float* m_imag_part;
        float* m_mag;
        float* m_phase;

        #if defined(__APPLE__) && defined(__MACH__)
            FFTSetup m_fft_setup;
            DSPComplex* m_temp_complex;
        #endif

    public:
        explicit FFT(int32_t log_n) noexcept;
        ~FFT() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "FFT"; }

        friend std::ostream& operator << (std::ostream& os, const FFT* o) {
            o == nullptr ? os << "FFT nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const FFT& o) {
            os << "m_valid: " << o.m_valid << ", m_log_n: " << o.m_log_n << ", m_length: " << o.m_length << ", m_half_length: " << o.m_half_length << std::endl;
            return os;
        }

        #if defined(__APPLE__) && defined(__MACH__)
            static FFTSetup _macos_fftSetup(int32_t log_n) noexcept;
        #endif

        static bool isValidResolution(int32_t resolution) noexcept;
        static int32_t logNFromResolution(int32_t resolution) noexcept;
        static int32_t resolutionFromLogN(int32_t log_n) noexcept;

        [[nodiscard]] int32_t logN() const noexcept { return m_log_n; }
        [[nodiscard]] int32_t length() const noexcept { return m_length; }
        [[nodiscard]] int32_t partialResolution() const noexcept { return m_length / 2; }

        ErrorCode fft(float* data, Partials* out_partials) noexcept;
        ErrorCode ifft(Partials* partials, float* out_data) noexcept;
    };


    class FFT_FIR : public Object {
    public:
        int32_t m_log_n;
        int32_t m_step_length;
        int32_t m_signal_length;
        int32_t m_filter_length;
        int32_t m_fft_length;
        int32_t m_fft_half_length;
        int32_t m_overlap_length;

        float* m_filter_real;
        float* m_filter_imag;
        float* m_filter_samples = nullptr;
        float* m_signal_samples = nullptr;
        float* m_convolved_samples = nullptr;
        float* m_filter_padded = nullptr;
        float* m_signal_padded = nullptr;
        float* m_filter_result = nullptr;
        float* m_signal_real = nullptr;
        float* m_signal_imag = nullptr;

#if defined(__APPLE__) && defined(__MACH__)
            FFTSetup m_fft_setup;
            DSPSplitComplex m_filter_split_complex;
        #endif

    public:
        explicit FFT_FIR(int32_t log_n) noexcept;
        ~FFT_FIR() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "FFT_FIR"; }

        friend std::ostream& operator << (std::ostream& os, const FFT_FIR* o) {
            o == nullptr ? os << "FFT_FIR nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const FFT_FIR& o) {
            os << "m_log_n: " << o.m_log_n << ", m_step_length: " << o.m_step_length << ", m_signal_length: " << o.m_signal_length << ", m_filter_length: " << o.m_filter_length << ", m_fft_length: " << o.m_filter_length << ", m_overlap_length: " << o.m_overlap_length;
            return os;
        }

        [[nodiscard]] bool isValid() const noexcept {
            #if defined(__APPLE__) && defined(__MACH__)
                return m_filter_real && m_fft_setup;
            #else
                return m_filter_real;
            #endif
        }

        void setFilter() noexcept;
        void filter() noexcept;

        [[nodiscard]] int32_t fftLength() const noexcept { return m_fft_length; }
        [[nodiscard]] int32_t signalLength() const noexcept { return m_signal_length; }
        [[nodiscard]] int32_t filterLength() const noexcept { return m_filter_length; }
        [[nodiscard]] int32_t overlapLength() const noexcept { return m_overlap_length; }

        [[nodiscard]] float* filterSamplesPtr() const noexcept { return m_filter_samples; }
        [[nodiscard]] float* signalSamplesPtr() const noexcept { return m_signal_samples; }
        [[nodiscard]] float* convolvedSamplesPtr() const noexcept { return m_convolved_samples; }
    };



    class SlidingDFT : public Object {

        // TODO: This class is not tested!

    private:
        int32_t m_dft_length = 0;

        // Are the frequency domain values valid? e.g. have at elast DFT_Length data points been seen?
        bool _m_data_valid = false;

        // Index of the next item in the buffer to be used. Equivalently, the number
        // of samples that have been seen so far modulo DFT_Length
        int32_t _m_x_index = 0;

        double* _m_x;        ///< Time domain samples are stored in this circular buffer

        std::complex<double>* _m_twiddle;   ///< Twiddle factors for the update algorithm
        std::complex<double>* _m_s;         ///< Frequency domain values (unwindowed!)
        std::complex<double>* _m_dft;       ///< Frequency domain values (windowed)

        double _m_damping_factor;


    public:
        explicit SlidingDFT(int32_t dft_length) noexcept;
        ~SlidingDFT() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "SlidingDFT"; }


        [[nodiscard]] bool isDataValid() const noexcept { return _m_data_valid; }
        double binFreq(uint32_t bin_index, double sample_rate);

        bool push(float value) noexcept;

        /**
         *  @brief Returns a pointer to the current DFT output (windowed values).
         *  @return Pointer to an array of std::complex<double> of size `dft_length`.
         */
        [[nodiscard]] const std::complex<double>* dft() const noexcept { return _m_dft; }

        /**
         *  @brief Returns the length of the DFT.
         *  @return Number of frequency bins.
         */
        [[nodiscard]] int32_t length() const noexcept { return m_dft_length; }
    };


} // End of namespace Grain

#endif // GrainFFT_hpp
