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
#include <cblas.h>
#endif


namespace Grain {

    class Partials;

    class FFTComplexSplit : Object {
    public:
        int32_t m_half_len = 0;
        float* m_real = nullptr;
        float* m_imag = nullptr;

        explicit FFTComplexSplit(int32_t half_len) {
            m_half_len = half_len;
            if (half_len > 0) {
                m_real = static_cast<float *>(std::calloc(half_len, sizeof(float)));
                m_imag = static_cast<float *>(std::calloc(half_len, sizeof(float)));
            }
        }

        ~FFTComplexSplit() override {
            std::free(m_real);
            std::free(m_imag);
        }

        void clear() {
            std::fill(m_real, &m_real[m_half_len], 0.0f);
            std::fill(m_imag, &m_imag[m_half_len], 0.0f);
        }
    };

    class FFTComplexSplitArray {
    public:
        int32_t m_split_count = 0;
        bool valid_ = true;
        FFTComplexSplit** m_split_array = nullptr;

        explicit FFTComplexSplitArray(int32_t n, int32_t half_len) {
            m_split_count = n;
            if (n > 0) {
                m_split_array = static_cast<FFTComplexSplit **>(std::calloc(n, sizeof(FFTComplexSplit *)));
                if (m_split_array) {
                    for (int32_t i = 0; i < m_split_count; i++) {
                        m_split_array[i] = new (std::nothrow) FFTComplexSplit(half_len);
                        if (!m_split_array[i]) {
                            valid_ = false;
                            break;
                        }
                    }
                }
            }
        }

        ~FFTComplexSplitArray() {
            if (m_split_array) {
                for (int32_t i = 0; i < m_split_count; i++) {
                    std::free(m_split_array[i]);
                }
                std::free(m_split_array);
            }
        }

       [[nodiscard]] FFTComplexSplit* splitAtIndex(int32_t index) const {
            return m_split_array[index];
        }
    };


    class FFT : public Object {
    public:
        enum {
            kErrPartialsMustBeCartesian
        };
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
            kLogNResolutionCount = kLogNResolutionLast - kLogNResolutionFirst + 1,

            kMinLogN = kLogNResolutionFirst,
            kMaxLogN = kLogNResolutionLast
        };

    protected:
        int32_t m_log_n;
        int32_t m_len;
        int32_t m_half_len;
        float* m_io_buffer = nullptr;         ///< Input/output buffer for samples, used in fft() and ifft()

#if defined(__APPLE__) && defined(__MACH__)
        DSPSplitComplex m_split_complex;
        FFTSetup m_fft_setup;
#else
        fftwf_complex* m_out{};
        fftwf_plan m_plan{};
        fftwf_plan m_plan_inv{};
        float* m_mag;
        float* m_phase;
#endif

    public:
        explicit FFT(int32_t log_n) noexcept;
        ~FFT() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "FFT"; }

        friend std::ostream &operator<<(std::ostream& os, const FFT* o) {
            o == nullptr ? os << "FFT nullptr" : os << *o;
            return os;
        }

        friend std::ostream &operator<<(std::ostream& os, const FFT& o) {
            os << "m_log_n: " << o.m_log_n << ", m_len: " << o.m_len;
            os << ", m_half_len: " << o.m_half_len << std::endl;
            return os;
        }

#if defined(__APPLE__) && defined(__MACH__)
        static FFTSetup _macos_fftSetup(int32_t log_n) noexcept;
#endif

        static bool isValidResolution(int32_t resolution) noexcept;
        [[nodiscard]] int32_t logN() const noexcept { return m_log_n; }
        [[nodiscard]] int32_t len() const noexcept { return m_len; }
        [[nodiscard]] int32_t partialResolution() const noexcept { return m_len / 2; }

        ErrorCode fft(float* samples) noexcept;
        ErrorCode ifft(float* out_samples) noexcept;

        ErrorCode filter(const Partials* partials) noexcept;

        ErrorCode setPartials(const Partials* partials) noexcept;
        ErrorCode getPartials(Partials* out_partials) noexcept;

        void shiftPhase(int32_t bin_index, float delta) noexcept;


#if defined(__APPLE__) && defined(__MACH__)
        // On macOS/iOS: use posix_memalign for alignment
        inline void* fft_alloc(std::size_t n_bytes) {
            void* ptr = nullptr;
            // vDSP likes 16-byte alignment (safe for NEON/AVX)
            if (posix_memalign(&ptr, 16, n_bytes) != 0) {
                return nullptr;
            }
            return ptr;
        }

        inline void fft_free(void* ptr) {
            free(ptr);
        }
#else
        // On other platforms: use FFTW's allocator
        inline void* fft_alloc(std::size_t n_bytes) {
            return fftwf_malloc(n_bytes); // properly aligned for FFTW
        }

        inline void fft_free(void* ptr) {
            fftwf_free(ptr);
        }
#endif
    };


    class SlidingDFT : public Object {

        // TODO: This class is not tested!

    private:
        int32_t m_dft_len = 0;

        // Are the frequency domain values valid? e.g. have at elast `dft_len_` data points been seen?
        bool _m_data_valid = false;

        // Index of the next item in the buffer to be used. Equivalently, the number
        // of samples that have been seen so far modulo `dft_len_`
        int32_t _m_x_index = 0;

        double* _m_x;        ///< Time domain samples are stored in this circular buffer

        std::complex<double>* _m_twiddle;   ///< Twiddle factors for the update algorithm
        std::complex<double>* _m_s;         ///< Frequency domain values (unwindowed!)
        std::complex<double>* _m_dft;       ///< Frequency domain values (windowed)

        double _m_damping_factor;


    public:
        explicit SlidingDFT(int32_t dft_len) noexcept;
        ~SlidingDFT() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "SlidingDFT"; }


        [[nodiscard]] bool isDataValid() const noexcept { return _m_data_valid; }
        double binFreq(uint32_t bin_index, double sample_rate);

        bool push(float value) noexcept;

        /**
         *  @brief Returns a pointer to the current DFT output (windowed values).
         *  @return Pointer to an array of std::complex<double> of size `dft_len`.
         */
        [[nodiscard]] const std::complex<double>* dft() const noexcept { return _m_dft; }

        /**
         *  @brief Returns the length of the DFT.
         *  @return Number of frequency bins.
         */
        [[nodiscard]] int32_t len() const noexcept { return m_dft_len; }
    };


} // End of namespace Grain

#endif // GrainFFT_hpp
