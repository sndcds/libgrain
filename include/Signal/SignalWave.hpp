//
//  SignalWave.hpp
//
//  Created by Roald Christesen on 06.08.2022
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainSignalWave_hpp
#define GrainSignalWave_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"


namespace Grain {

    class Partials;
    class FFT;


    class SignalWaveLookUpInfo {
    public:
        SignalWaveLookUpInfo() noexcept;
        explicit SignalWaveLookUpInfo(int32_t sample_rate) noexcept;

        void setup(int32_t sample_rate) noexcept;
        void setPitch(float pitch) noexcept;
        void setFreq(float freq) noexcept;
        void setInvertWave(bool value) noexcept { m_invert_wave = value; }
        void setInvertPolarity(bool value) noexcept { m_invert_polarity = value; }
        void stepForward() noexcept;
        void setPos(float pos) noexcept;
        void addPos(float value) noexcept;

    public:
        float m_sample_rate;
        float m_highest_freq;
        float m_highest_pitch;
        float m_pitch;
        float m_freq;
        float m_pos = 0.0f;
        float m_step;
        float m_freq_step_factor;
        bool m_invert_wave = false;
        bool m_invert_polarity = false;
    };


    /**
     *  @brief One cycle waveform.
     */
    class SignalWave : public Object {
    public:
        enum {
            kMinResolution = 2,
            kMaxResolution = INT32_MAX,
            kMinPitch = 0,
            kMaxPitch = 127,
            kPitchCount = 128
        };

        enum {
            kErrNoWaveData = 0,
            kErrCheckWaveFailed,
            kErrUnsupportedPitch,
            kErrNoFFTInstance,
            kErrNoPartialsInstance,
        };

    public:
        SignalWave(int32_t resolution, int32_t sample_rate) noexcept;
        ~SignalWave() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "SignalWave"; }


        void setFreqRollOff(float rolloff) { m_freq_rolloff = std::clamp<float>(rolloff, 0.01f, 1.0f); }
        [[nodiscard]] int32_t resolution() const noexcept { return m_resolution; }

        [[nodiscard]] int32_t pitch() const noexcept { return m_pitch; }
        void setPitch(int32_t pitch) noexcept { m_pitch = std::clamp<int32_t>(pitch, kMinPitch, kMaxPitch); }
        [[nodiscard]] static bool isPitch(int32_t pitch) noexcept { return pitch >= kMinPitch && pitch <= kMaxPitch; }
        [[nodiscard]] int32_t lowestPitch() const noexcept;

        [[nodiscard]] bool hasWave(int32_t pitch) const noexcept {
            return isPitch(pitch) ? m_wave_data[pitch] != nullptr : false;
        }
        bool checkWave(int32_t pitch) noexcept;
        ErrorCode allocWave(int32_t pitch) noexcept;

        [[nodiscard]] float* mutSamplePtr(int32_t pitch) const noexcept;
        [[nodiscard]] float* mutLowestPitchSamplePtr() const noexcept { return mutSamplePtr(lowestPitch()); }

        void clearSamples() noexcept { clearSamples(m_pitch); }
        void clearSamples(int32_t pitch) noexcept;
        void setSample(int32_t index, float value) noexcept;
        void addSamples(float* samples, float level = 1.0f) noexcept;
        void addTriangle(float offset = 0.0f, float level = 1.0f) noexcept;
        void addSaw(float offs = 0.0f, float level = 1.0f) noexcept;
        void addSquare(float center = 0.5f, float offset = 0.0f, float level = 1.0f) noexcept;
        void addSine(int32_t freq = 1, float offset = 0.0f, float level = 1.0f) noexcept;
        void addWhiteNoise(float level = 1.0f) noexcept;
        void addSineSeries(int32_t freq_start, int32_t freq_end, int32_t freq_step, float level = 1.0f, float damp = 12.0f) noexcept;
        bool addFromFile(const String& file_path, float level = 1.0f) noexcept;

        void flipHorizontal() noexcept;
        void flipVertical() noexcept;
        void absolute() noexcept;

        void normalize(float level = 1.0f) noexcept;
        void normalizeAll(float level = 1.0f) noexcept;


        ErrorCode highVersion(int32_t pitch, int32_t src_pitch) noexcept;
        ErrorCode highVersions(int32_t src_pitch, int32_t last_pitch, int32_t pitch_step) noexcept;

        bool finalize() noexcept;

        [[nodiscard]] float lookup(SignalWaveLookUpInfo& info) noexcept;
        ErrorCode partials(FFT* fft, Partials* out_partials) const noexcept;

    protected:
        int32_t m_requested_resolution;
        int32_t m_resolution;
        int32_t m_sample_rate;
        float m_freq_rolloff = 5.0f / 6.0f;

        float* m_wave_data[kPitchCount];    ///< Pointers to waveform data for each pitch
        size_t m_wave_data_size;            ///< Size of memory for a single waveform in bytes

        int32_t m_pitch_index_table[kPitchCount][2];
        int32_t m_pitch = 10;               ///< Is been used in methods working with the wave data

        bool _m_must_finalize = true;
        int32_t _m_err_alloc_failed_count = 0;
        int32_t _m_err_loop_index_count = 0;
    };


} // End of namespace Grain

#endif // GrainSignalWave_hpp
