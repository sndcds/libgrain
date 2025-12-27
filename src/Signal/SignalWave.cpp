//
//  SignalWave.cpp
//
//  Created by Roald Christesen on 06.08.2022
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalWave.hpp"
#include "Signal/Signal.hpp"
#include "Signal/Audio.hpp"
#include "Type/Type.hpp"
#include "Type/HiResValue.hpp"
#include "Math/Math.hpp"
#include "Math/Random.hpp"
#include "DSP/Partials.hpp"
#include "DSP/FFT.hpp"


namespace Grain {

    SignalWaveLookUpInfo::SignalWaveLookUpInfo() noexcept {
        setup(44100);
        setFreq(220.0f);
    }


    SignalWaveLookUpInfo::SignalWaveLookUpInfo(int32_t sample_rate) noexcept {
        setup(sample_rate);
        setFreq(220.0f);
    }


    void SignalWaveLookUpInfo::setup(int32_t sample_rate) noexcept {
        m_sample_rate = std::clamp<int32_t>(sample_rate, 1, Audio::kMaxSampleRate);
        m_freq_step_factor = 1.0f / m_sample_rate;
        m_highest_freq = (float)sample_rate / 2;
        m_highest_pitch = Audio::pitchFromFreq(m_highest_freq);
        m_pos = 0.0f;
        setPitch(48);
    }


    void SignalWaveLookUpInfo::setPitch(float pitch) noexcept {
        if (pitch < 0.0f) {
            pitch = 0.0f;
        }
        m_freq = Audio::freqFromPitch(pitch);
        m_pitch = pitch;
        m_step = m_freq_step_factor * m_freq;
    }


    void SignalWaveLookUpInfo::setFreq(float freq) noexcept {
        if (freq < 0.001f) {
            freq = 0.001f;
        }
        m_freq = freq;
        m_pitch = Audio::pitchFromFreq(freq);
        m_step = m_freq_step_factor * freq;
    }


    void SignalWaveLookUpInfo::stepForward() noexcept {
        m_pos += m_step;
        if (m_pos >= 1.0f) {
            m_pos = std::fmod(m_pos, 1.0f);
        }
    }


    void SignalWaveLookUpInfo::setPos(float pos) noexcept {
        m_pos = std::fmod(pos, 1.0f);
        if (m_pos < 0.0f) {
            m_pos += 1.0f;
        }
    }


    void SignalWaveLookUpInfo::addPos(float value) noexcept {
        m_pos += value;
        m_pos = std::fmod(m_pos, 1.0f);
        if (m_pos < 0.0f) {
            m_pos += 1.0f;
        }
    }


    SignalWave::SignalWave(int32_t resolution, int32_t sample_rate) noexcept : Object() {
        m_requested_resolution = resolution;
        m_resolution = std::clamp<int32_t>(resolution, kMinResolution, kMaxResolution);
        m_sample_rate = std::clamp<int32_t>(sample_rate, Audio::kMinSampleRate, Audio::kMaxSampleRate);

        // No waveform data at creation
        for (int32_t i = kMinPitch; i <= kMaxPitch; i++) {
            m_wave_data[i] = nullptr;
        }

        m_wave_data_size = sizeof(float) * m_resolution;
    }


    SignalWave::~SignalWave() noexcept {
        for (int32_t i = kMinPitch; i <= kMaxPitch; i++) {
            std::free(m_wave_data[i]);
        }
    }


    int32_t SignalWave::lowestPitch() const noexcept {
        for (int32_t i = kMinPitch; i <= kMaxPitch; i++) {
            if (m_wave_data[i]) {
                return i;
            }
        }
        return -1;
    }


    bool SignalWave::checkWave(int32_t pitch) noexcept {
        if (!isPitch(pitch)) {
            return false;
        }

        if (!m_wave_data[pitch]) {
            if (allocWave(pitch) != ErrorCode::None) {
                return false;
            }
        }

        return true;
    }


    ErrorCode SignalWave::allocWave(int32_t pitch) noexcept {
        if (!isPitch(pitch)) {
            return ErrorCode::BadArgs;
        }

        if (!m_wave_data[pitch]) {
            m_wave_data[pitch] = static_cast<float*>(std::malloc(m_wave_data_size));
            if (!m_wave_data[pitch]) {
                _m_err_alloc_failed_count++;
                return ErrorCode::MemCantAllocate;
            }
        }

        clearSamples(pitch);

        return ErrorCode::None;
    }


    float* SignalWave::mutSamplePtr(int32_t pitch) const noexcept {
        return isPitch(pitch) ? m_wave_data[pitch] : nullptr;
    }


    void SignalWave::clearSamples(int32_t pitch) noexcept {
        if (hasWave(pitch)) {
            Type::clearArray(m_wave_data[pitch], m_resolution);
            _m_must_finalize = true;
        }
    }


    void SignalWave::setSample(int32_t index, float value) noexcept {
        if (checkWave(m_pitch) && index >= 0 && index < m_resolution) {
            m_wave_data[m_pitch][index] = value;
            _m_must_finalize = true;
        }
    }


    void SignalWave::addSamples(float* samples, float level) noexcept {
        if (samples) {
            if (checkWave(m_pitch)) {
                float* s = samples;
                float* d = m_wave_data[m_pitch];

                for (int32_t i = 0; i < m_resolution; i++) {
                    *d++ += *s++ * level;
                }

                _m_must_finalize = true;
            }
        }
    }


    void SignalWave::addSine(int32_t freq, float offset, float level) noexcept {
        if (checkWave(m_pitch)) {
            float* d = m_wave_data[m_pitch];
            double f = 1.0 / m_resolution * freq;

            for (int32_t i = 0; i < m_resolution; i++) {
                *d++ += sinf(Math::kTau * (f * i + offset)) * level;
            }

            _m_must_finalize = true;
        }
    }


    void SignalWave::addTriangle(float offset, float level) noexcept {
        if (checkWave(m_pitch)) {
            float* d = m_wave_data[m_pitch];
            float t = std::clamp<float>(offset, 0.0f, 1.0f);
            float step = 1.0f / static_cast<float>(m_resolution);

            for (int32_t i = 0; i < m_resolution; i++) {
                if (t >= 1.0f) {
                    t -= 1.0f;
                }

                if (t < 0.25f) {
                    *d++ += t * 4.0f;
                }
                else if (t < 0.75f) {
                    *d++ += 1.0f - (t - 0.25f) * 4.0f;
                }
                else {
                    *d++ += -1.0f + (t - 0.75f) * 4.0f;
                }

                t += step;
            }
            _m_must_finalize = true;
        }
    }


    void SignalWave::addSaw(float offs, float level) noexcept {
        if (checkWave(m_pitch)) {
            float* d = m_wave_data[m_pitch];
            float t = offs - std::floor(offs);
            float step = 1.0f / static_cast<float>(m_resolution);
            for (int32_t i = 0; i < m_resolution; i++) {
                if (t >= 1.0f) {
                    t -= 1.0f;
                }
                *d++ += (t * 2.0f - 1.0f) * level;
                t += step;
            }
            _m_must_finalize = true;
        }
    }


    void SignalWave::addSquare(float center, float offset, float level) noexcept {
        if (checkWave(m_pitch)) {
            float* d = m_wave_data[m_pitch];
            float t = std::clamp<float>(offset, 0.0f, 1.0f);
            float step = 1.0f / static_cast<float>(m_resolution);
            center = std::clamp<float>(center, 0.01f, 0.99f);
            float level1 = 1.0f;
            float level2 = -1.0f;

            if (center < 0.5f) {
                level2 = -center;
            }
            else {
                level1 = center;
            }

            for (int32_t i = 0; i < m_resolution; i++) {
                if (t >= 1.0f) {
                    t -= 1.0f;
                }

                *d++ += (t < center ? level1 : level2) * level;
                t += step;
            }

            _m_must_finalize = true;
        }
    }


    void SignalWave::addWhiteNoise(float level) noexcept {
        WhiteNoiseRand rand;

        if (checkWave(m_pitch)) {
            float* d = m_wave_data[m_pitch];

            for (int32_t i = 0; i < m_resolution; i++) {
                *d++ += rand.next() * level;
            }

            _m_must_finalize = true;
        }
    }


    void SignalWave::addSineSeries(int32_t freq_start, int32_t freq_end, int32_t freq_step, float level, float damp) noexcept {
        if (checkWave(m_pitch)) {
            freq_start = std::clamp<int32_t>(freq_start, 1, 1023);
            freq_end = std::clamp<int32_t>(freq_end, freq_start, 1023);

            for (int32_t i = freq_start; i <= freq_end; i += freq_step) {
                float db = -log2f(i) * damp;
                float l = Audio::dbToLinear(db);
                addSine(i, 0.0f, l * level);
            }
        }
    }


    bool SignalWave::addFromFile(const String& file_path, float level) noexcept {
        bool result = false;

        if (checkWave(m_pitch)) {
            ErrorCode err;
            Signal* signal = Signal::createFromFile(file_path, DataType::Float, err);
            if (signal) {
                HiResValue sample_pos(static_cast<double>(signal->sampleCount()) / m_resolution);
                float* d = m_wave_data[m_pitch];
                for (int32_t i = 0; i < m_resolution; i++) {
                    *d++ += signal->readFloatLerp(0, sample_pos) * level;
                    sample_pos.stepForward();
                }

                delete signal;
                result = true;
            }
        }

        return result;
    }


    void SignalWave::flipHorizontal() noexcept {
        if (hasWave(m_pitch)) {
            Type::flipArray(m_wave_data[m_pitch], m_resolution);
        }
    }


    void SignalWave::flipVertical() noexcept {
        if (hasWave(m_pitch)) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_wave_data[m_pitch][i] = 1.0f - m_wave_data[m_pitch][i];
            }
        }
    }


    void SignalWave::absolute() noexcept {
        if (hasWave(m_pitch)) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_wave_data[m_pitch][i] = std::fabs(m_wave_data[m_pitch][i]);
            }
        }
    }


    void SignalWave::normalize(float level) noexcept {
        // TODO: Implementation isnt right!!!!
        if (hasWave(m_pitch)) {
            Type::normalizeArrayToUnitRange(m_wave_data[m_pitch], m_resolution);
        }
    }


    void SignalWave::normalizeAll(float level) noexcept {
        // TODO: Implementation isnt right!!!!
        for (int32_t i = 0; i < kPitchCount; i++) {
            if (hasWave(i)) {
                Type::normalizeArrayToUnitRange(m_wave_data[m_pitch], m_resolution);
            }
        }
    }


    ErrorCode SignalWave::highVersion(int32_t pitch, int32_t src_pitch) noexcept {
        auto result = ErrorCode::None;
        FFT* fft = nullptr;
        Partials* partials = nullptr;

        try {
            if (pitch < kMinPitch || pitch > kMaxPitch || pitch <= src_pitch) {
                Exception::throwSpecific(kErrUnsupportedPitch);
            }
            if (!hasWave(src_pitch)) {
                Exception::throwSpecific(kErrNoWaveData);
            }
            if (!checkWave(pitch)) {
                Exception::throwSpecific(kErrCheckWaveFailed);
            }

            int32_t partial_resolution = m_resolution / 2;
            partials = new(std::nothrow) Partials(partial_resolution);
            if (!partials) {
                Exception::throwStandard(ErrorCode::ClassInstantiationFailed);
            }

            fft = new(std::nothrow) FFT(Math::nextLog2(m_resolution));
            if (!fft) {
                Exception::throwStandard(ErrorCode::ClassInstantiationFailed);
            }

            float freq = Audio::freqFromPitch(static_cast<float>(pitch));
            float base_freq = freq;
            float max_freq = static_cast<float>(m_sample_rate) / 2.0f; // Nyquist frequency

            fft->fft(m_wave_data[src_pitch]);
            fft->getPartials(partials);

            partials->setDC(0.0f);
            float bin_freq = base_freq;
            for (int32_t bin_index = 0; bin_index < partial_resolution; bin_index++) {
                float bin_scale = 1.0f;
                if (bin_freq >= max_freq) {
                    // Above Nyquist → remove
                    bin_scale = 0.0f;
                }
                else if (bin_freq >= m_freq_rolloff) {
                    // Smooth roll-off between roll_off_freq and Nyquist
                    float t = Math::remapclampedf(m_freq_rolloff, max_freq, 1.0f, 0.0f, bin_freq);
                    bin_scale = Math::ease(Math::EaseMode::InOutSine, t);
                }
                partials->scaleMag(bin_index, bin_scale);
                bin_freq += base_freq;
            }

            fft->setPartials(partials);
            fft->ifft(m_wave_data[pitch]);
        }
        catch (const Exception& e) {
            result = e.code();
        }

        delete fft;
        delete partials;

        return result;
    }


    ErrorCode SignalWave::highVersions(int32_t src_pitch, int32_t last_pitch, int32_t pitch_step) noexcept {
        last_pitch = std::max<int32_t>(last_pitch, kMaxPitch);
        pitch_step = std::max<int32_t>(pitch_step, 1); // Must be at least 1
        for (int32_t pitch = src_pitch + pitch_step; pitch < last_pitch; pitch += pitch_step) {
            auto err = highVersion(pitch, src_pitch);
            if (Error::isError(err)) {
                return err;
            }
        }
        return highVersion(last_pitch, src_pitch);
    }


    /**
     *  @brief Finalizes the waveform generation process.
     *
     *  This function is responsible for performing any necessary operations to conclude the waveform generation.
     *  It may involve post-processing steps, clean-up tasks, or final adjustments before the waveform is considered
     *  complete.
     *
     *  @return Returns true if the waveform finalization is successful, otherwise returns false.
     */
    bool SignalWave::finalize() noexcept {
        if (_m_must_finalize) {
            for (int32_t pitch = kMinPitch; pitch <= kMaxPitch; pitch++) {
                m_pitch_index_table[pitch][0] = -1;
                m_pitch_index_table[pitch][1] = -1;
            }

            int32_t last_pitch = kPitchCount;
            for (int32_t pitch = kMaxPitch; pitch >= 0; pitch--) {
                if (m_wave_data[pitch]) {
                    for (int32_t i = pitch; i < last_pitch; i++) {
                        m_pitch_index_table[i][0] = pitch;
                    }
                    last_pitch = pitch;
                }
            }

            if (last_pitch < kPitchCount) {
                for (int32_t pitch = 0; pitch < last_pitch; pitch++) {
                    m_pitch_index_table[pitch][0] = last_pitch;
                }
            }

            last_pitch = -1;

            for (int32_t pitch = 0; pitch < kPitchCount; pitch++) {
                if (m_wave_data[pitch]) {
                    for (int32_t i = pitch; i > last_pitch; i--) {
                        m_pitch_index_table[i][1] = pitch;
                    }
                    last_pitch = pitch - 1;
                }
            }

            if (last_pitch >= 0) {
                for (int32_t pitch = last_pitch; pitch < kPitchCount; pitch++) {
                    m_pitch_index_table[pitch][1] = last_pitch + 1;
                }
            }

            for (int32_t i = kMinPitch; i < kPitchCount; i++) {
                if (m_pitch_index_table[i][0] < 0 || m_pitch_index_table[i][1] < 0) {
                    return false;
                }
            }

            for (int32_t i = kMinPitch; i < kPitchCount; i++) {
                if (m_pitch_index_table[i][0] < 0 || m_pitch_index_table[i][1] < 0) {
                    return false;
                }
            }

            _m_must_finalize = false;
        }

        return true;
    }



    float SignalWave::lookup(SignalWaveLookUpInfo& info) noexcept {
        if (_m_must_finalize) {
            finalize();
        }

        float step = info.m_step;
        if (step > 0.5f) {
            return 0.0f;
        }

        float result = 0.0f;

        float real_pitch = info.m_pitch;
        if (real_pitch < 0.0001f) {
            real_pitch = 0.0001f;
        }
        else if (real_pitch > static_cast<float>(kMaxPitch)) {
            real_pitch = kMaxPitch;
        }

        auto int_pitch = static_cast<int32_t>(real_pitch);
        int32_t low_pitch_index = m_pitch_index_table[int_pitch][0];
        int32_t high_pitch_index = m_pitch_index_table[int_pitch][1];

        // Pitch related interpolation factors
        float pf1 = static_cast<float>(real_pitch - low_pitch_index) / (high_pitch_index - low_pitch_index);
        float pf0 = 1.0f - pf1;

        // Interpolation
        float sample_real_index = info.m_pos * m_resolution;
        auto sample_index0 = static_cast<int32_t>(sample_real_index);
        if (sample_index0 >= m_resolution) {
            sample_index0 -= m_resolution;
        }

        int32_t sample_index1 = sample_index0 + 1;
        if (sample_index1 >= m_resolution) {
            sample_index1 -= m_resolution;
        }

        if (sample_index0 < 0 || sample_index0 >= m_resolution ||
            sample_index1 < 0 || sample_index1 >= m_resolution) {
            _m_err_loop_index_count++;
            return 0.0f;
        }

        float sf1 = sample_real_index - sample_index0;
        float sf0 = 1.0f - sf1;

        if (info.m_invert_wave) {
            sample_index0 = (m_resolution - 1) - sample_index0;
            sample_index1 = (m_resolution - 1) - sample_index1;
        }

        if (low_pitch_index == high_pitch_index) {
            float* w = m_wave_data[low_pitch_index];
            result = w[sample_index0] * sf0 + w[sample_index1] * sf1;
        }
        else {
            float* w0 = m_wave_data[low_pitch_index];
            float* w1 = m_wave_data[high_pitch_index];

            result =
                    (w0[sample_index0] * sf0 + w0[sample_index1] * sf1) * pf0 +
                    (w1[sample_index0] * sf0 + w1[sample_index1] * sf1) * pf1;
        }

        if (step > 0.25f) {
            result *= 1.0f - (step - 0.25f) / 0.25f;
        }

        if (info.m_invert_polarity) {
            result = -result;
        }

        return result;
    }


    ErrorCode SignalWave::partials(FFT* fft, Partials* out_partials) const noexcept {
        if (!fft) { return Error::specific(kErrNoFFTInstance); }
        if (!out_partials) { return Error::specific(kErrNoPartialsInstance); }

        auto samples = mutLowestPitchSamplePtr();
        if (!samples) { return Error::specific(kErrNoWaveData); }

        fft->fft(samples);
        fft->getPartials(out_partials);

        return ErrorCode::None;
    }


/* TODO: !!!!!
    void SignalWave::draw(View* view, GraphicContext& gc, const Rectd& rect, int32_t sample_count) noexcept {

        if (view) {

            int32_t w = (int32_t)rect.width_;
            if (w > 2) {

                float x_step = 1.0f;
                int32_t n = 0;

                SignalWaveLookUpInfo info(44100);

                if (sample_count > 1) {
                    n = sample_count;
                    x_step = rect.width_ / sample_count;
                }
                else {
                    n = w;
                }

                info.setFreq((float)m_resolution / n * (44100 / m_resolution) / 2);
                float step = 1.0f / (n - 1);

                float y_center = rect.centerY();
                float y_scale = rect.height_ / 2;

                Vec2d pos(rect.x_, y_center - lookup(info) * y_scale);
                gc.moveTo(pos);
                n--;

                while (n--) {
                    info.addPos(step);
                    pos.x_ += x_step;
                    pos.m_y = y_center - lookup(info) * y_scale;
                    gc.lineTo(pos);
                }

                gc.strokePath();
            }
        }
    }
    */

} // End of namespace Grain