//
//  Freq.hpp
//
//  Created by Roald Christesen on 30.01.2018
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainFreq_hpp
#define GrainFreq_hpp

#include "Grain.hpp"


namespace Grain {

    class Freq {
    public:
        enum class Scale {
            Linear = 0,
            Logarithmic,
            Mel,
            Bark,
            ERB,

            Count,
            First = 0,
            Last = Count - 1
        };

    public:
        static float freqToPos(float freq, float low_freq, float high_freq, float low_pos, float high_pos) noexcept;
        static float posToFreq(float pos, float low_freq, float high_freq, float low_pos, float high_pos) noexcept;
    };


    class FreqRange {
    public:
        FreqRange() noexcept {
            set(20, 20000);
        }

        explicit FreqRange(float min_freq, float max_freq) noexcept {
            set(min_freq, max_freq);
        }

        [[nodiscard]] float minFreq() const noexcept { return m_min_freq; }
        [[nodiscard]] float maxFreq() const noexcept { return m_max_freq; }
        [[nodiscard]] float freqRange() const noexcept { return m_freq_range; }

        void set(float min_freq, float max_freq) noexcept;

        [[nodiscard]] float lerpFreq(float t) const noexcept;
        [[nodiscard]] float freqToLogNrmScale(float freq) const;
        [[nodiscard]] float freqToLinNrmScale(float freq) const;
        [[nodiscard]] float freqToMel(float freq) const noexcept;
        [[nodiscard]] float melToFreq(float mel_nrm) const noexcept;
        [[nodiscard]] float freqToBark(float freq) const noexcept;
        [[nodiscard]] float barkToFreq(float bark_nrm) const noexcept;
        [[nodiscard]] float freqToERB(float freq) const noexcept;
        [[nodiscard]] float erbToFreq(float erb_nrm) const noexcept;
        [[nodiscard]] float freqToScale(float freq, Freq::Scale scale) const noexcept;
        [[nodiscard]] float logToLinFreq(float log_nrm) const noexcept;
        [[nodiscard]] float linToLog(float lin_nrm) const noexcept;
        [[nodiscard]] float linToMel(float lin_nrm) const noexcept;
        [[nodiscard]] float linToBark(float lin_nrm) const noexcept;
        [[nodiscard]] float linToERB(float lin_nrm) const noexcept;
        [[nodiscard]] float tToFreq(float t, Freq::Scale scale) const;

    protected:
        float m_min_freq{};
        float m_max_freq{};
        float m_freq_range{};
        float m_max_div_min{};
        float m_log_min_freq{};
        float m_log_max_freq{};
        float m_freq_range_f{};
    };


    class FreqBands {
    public:
        FreqBands() noexcept = default;
        FreqBands(float left_freq, float center_freq, float right_freq) noexcept {
            m_left_freq = left_freq;
            m_center_freq = center_freq;
            m_right_freq = right_freq;
        }

        [[nodiscard]] int32_t bandCount() const noexcept { return m_band_count; }
        [[nodiscard]] float leftFreq() const noexcept { return m_left_freq; }
        [[nodiscard]] float centerFreq() const noexcept { return m_center_freq; }
        [[nodiscard]] float rightFreq() const noexcept { return m_right_freq; }

        void set(float left_freq, float center_freq, float right_freq) noexcept;
        void setupBands(float bands_per_octave, float start_freq, float end_freq) noexcept;
        bool setBand(int32_t index) noexcept;

        void setBandsPerOctave(float bands_per_octave) noexcept { m_bands_per_octave = bands_per_octave; }
        void setCenterFreq(float center_freq, float octave_range) noexcept;

    protected:
        float m_bands_per_octave = 2.0f;
        int32_t m_band_count = 11;
        float m_band_start_freq = 20.0f;
        float m_band_end_freq = 20000.0f;

        float m_left_freq = 100.0f;
        float m_center_freq = 1000.0f;
        float m_right_freq = 5000.0f;
    };


} // End of namespace Grain

#endif // GrainFreq_hpp
