//
//  Audio.cpp
//
//  Created by Roald Christesen on 27.03.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/Audio.hpp"
#include "Type/Type.hpp"
#include "DSP/LUT1.hpp"
#include "Signal/Signal.hpp"
#include "Core/Log.hpp"


namespace Grain {

    const double Audio::g_envelope_min_level = 0.00001;
    LUT1* Audio::g_amplitude_from_level_lut = nullptr;
    LUT1* Audio::g_release_lut = nullptr;
    LUT1* Audio::g_release_duration_lut = nullptr;


    const InstrumentCharacteristics Audio::g_instrument_characteristics[] = {
        { "Piano", 21, 108 },           // A0 - C8
        { "Guitar", 40, 88 },           // E2 - 88 E6
        { "Bass Guitar", 28, 67 },      //  E1 - G4
        { "Violin", 55 - 103 },         // G3 - G7
        { "Viola", 48, 88 },            // C3 - E6
        { "Cello", 36, 76 },            // (C2) - 76 (E6)
        { "Double Bass", 28, 67 },      // (E1) - 67 (G4)
        { "Flute", 60, 91 },            // (C4) - 91 (G6)
        { "Clarinet", 55, 86 },         // (G3) - 86 (D6)
        { "Saxophone", 45, 89 },        // (A2) - 89 (F#6)
        { "Trumpet", 55, 81 },          // (G3) - 81 (A#5)
        { "Trombone", 40, 81 },         // (E2) - 81 (A#5)
        { "French Horn", 33, 77 },      // (A#1) - 77 (F#5)
        { "Tuba", 29, 58 },             // (D#1) - 58 (A#3)
        { "Voice (Male)", 41, 67 },     // (F2) - 67 (G4)
        { "Voice (Female)", 57, 86 },   // (A3) - 86 (D6)
        { "Voice (Child)", 48, 80 },    // (C3) - 80 (G5)
        { "Drums/Percussion", 36, 81 }, // (C2) - 81 (A#5)
    };


    void Audio::_init() noexcept {

        {
            // Compute `g_amplitude_from_levellut`
            int32_t resolution = 1024;
            g_amplitude_from_level_lut = new(std::nothrow) LUT1(resolution);
            if (g_amplitude_from_level_lut) {
                for (int32_t lut_index = 0; lut_index < resolution; lut_index++) {
                    g_amplitude_from_level_lut->setValueAtIndex(lut_index, Audio::amplitudeFromLevel(static_cast<float>(lut_index) / (resolution - 1)));
                }
            }
        }

        {
            // Compute `g_release_lut`
            int32_t resolution = 4096;
            g_release_lut = new(std::nothrow) LUT1(resolution);
            auto f = static_cast<float>(Signal::releaseCoef(1, 0, Audio::g_envelope_min_level, resolution, 1));
            float v = 1.0f;
            for (int32_t lut_index = 0; lut_index < resolution; lut_index++) {
                g_release_lut->setValueAtIndex(lut_index, v);
                v *= f;
            }

            float offset = g_release_lut->valueAtIndex(resolution - 1);
            float scale = 1.0f + offset;
            for (int32_t lut_index = 0; lut_index < resolution; lut_index++) {
                g_release_lut->setValueAtIndex(lut_index, (g_release_lut->valueAtIndex(lut_index) - offset) * scale);
            }

            g_release_lut->setValueAtIndex(0, 1.0f);
            g_release_lut->setValueAtIndex(resolution - 1, 0.0f);
        }


        {
            // Compute `g_release_duration_lut`
            int32_t resolution = 4096;
            g_release_duration_lut = new(std::nothrow) LUT1(resolution);

            auto f = static_cast<float>(Signal::releaseCoef(1, 0, Audio::g_envelope_min_level, 44100));
            float v = 1.0f;
            float threshold = 1;
            int32_t length = 44100;
            int32_t lut_index = resolution - 1;

            do {
                if (v <= threshold) {
                    g_release_duration_lut->setValueAtIndex(lut_index, static_cast<float>(length) / 44100);
                    lut_index--;
                    threshold -= 1.0f / static_cast<float>(resolution - 1);
                }

                v *= f;
                length--;
            } while (v > std::numeric_limits<float>::epsilon() && lut_index >= 0);

            g_release_duration_lut->setValueAtIndex(0, 1.0f);
        }
    }


    void Audio::_exit() noexcept{
    }


    float Audio::durationForNote(float bpm, float length) noexcept {

        //  Returns the duration for a musical note
        //  The maximum duration is 360000 seconds ~ 100 hours
        //  bpm - Beats per minute
        //  length - Note length in beats

        if (length < std::numeric_limits<float>::min()) {
            return 0;
        }

        if (bpm < std::numeric_limits<float>::min()) {
            return 360000; // 100 hours
        }

        return std::clamp(length * 60 / bpm, 0.0f, 360000.0f);
    }


    float Audio::samplesNeededForNote(int32_t sample_rate, float bpm, float length) noexcept {

        return static_cast<float>(sample_rate) * durationForNote(bpm, length);
    }


    float Audio::dezibelFromAmplitude(float amplitude) noexcept {

        if (amplitude < 0.0f) {
            return std::numeric_limits<float>::lowest();
        }
        else {
            float d = 20.0f * std::log10(amplitude);
            return d < std::numeric_limits<float>::lowest() ? std::numeric_limits<float>::lowest() : d;
        }
    }


    float Audio::levelFromAmplitude(float amplitude) noexcept {

        return std::pow(2.0f, dezibelFromAmplitude(amplitude) / 10.0f);
    }


    float Audio::amplitudeFromDezibel(float dezibel) noexcept {

        return std::pow(10.0f, dezibel / 20.0f);
    }


    float Audio::levelFromDezibel(float dezibel) noexcept {

        return std::pow(2.0f, dezibel / 10.0f);
    }


    float Audio::dezibelFromLevel(float level) noexcept {

        if (level < 0.0) {
            return std::numeric_limits<float>::lowest();
        }
        else {
            float d = std::log2f(level) * 10.0f;
            return d < std::numeric_limits<float>::lowest() ? std::numeric_limits<float>::lowest() : d;
        }
    }


    float Audio::amplitudeFromLevel(float level) noexcept {

        if (level < 0.0) {
            return 0.0;
        }
        else {
            float dezibel = std::log2f(level) * 10.0f;
            return std::pow(10, dezibel / 20.0f);
        }
    }


    float Audio::amplitudeFromLevelLUT(float level) noexcept {

        if (level < 0.0f) {
            return 0.0f;
        }
        if (level > 1.0f) {
            return amplitudeFromLevel(level);
        }
        else {
            return g_amplitude_from_level_lut->lookup(level);
        }
    }


    float Audio::freqFromPitch(float pitch, float reference_freq) noexcept {

        return static_cast<float>(reference_freq * std::pow(2, (pitch - 69) / 12));
    }


    float Audio::freqFromPitchAndOctave(float pitch, int32_t octave, float reference_freq) noexcept {

        return static_cast<float>(reference_freq * std::pow(2.0f, (pitch - 69.0f + static_cast<float>(octave) * 12.0f) / 12.0f));
    }


    float Audio::pitchFromFreq(float freq, float reference_freq) noexcept {

        return static_cast<float>(69.0 + 12.0 * std::log(freq / reference_freq) / std::numbers::ln2);
    }


    int32_t Audio::pitchClass(float pitch) noexcept {

        return Audio::pitchClass(static_cast<int32_t>(std::round(pitch)));
    }


    int32_t Audio::pitchClass(int32_t pitch) noexcept {

        int32_t pitch_class = pitch - ((static_cast<int32_t>(pitch - kMusic_A) / 12) * 12);
        if (pitch_class < kMusic_A) {
            pitch_class += 12;
        }
        return pitch_class - kMusic_A;
    }


    int32_t Audio::noteClassFromPitch(float pitch) noexcept {

        return Audio::noteClassFromPitch(static_cast<int32_t>(std::round(pitch)));
    }


    int32_t Audio::noteClassFromPitch(int32_t pitch) noexcept {

        int32_t pitch_class = Audio::pitchClass(pitch) - 3;
        return pitch_class < 0 ? pitch_class + 12 : pitch_class;
    }


    bool Audio::samePitchClass(int32_t pitch, int32_t referencePitch) noexcept {

        return pitchClass(pitch) == pitchClass(referencePitch);
    }


    bool Audio::pitchIsBlackKey(int32_t pitch) noexcept {

        switch (Audio::pitchClass(pitch)) {
            case kMusic_A:
            case kMusic_B:
            case kMusic_C:
            case kMusic_D:
            case kMusic_E:
            case kMusic_F:
            case kMusic_G:
                return false;
        }

        return true;
    }


    bool Audio::pitchIsWhiteKey(int32_t pitch) noexcept {

        return !Audio::pitchIsBlackKey(pitch);
    }


    int32_t Audio::pitchCountBlackKeys(int32_t low_pitch, int32_t high_pitch) noexcept {

        int32_t n = 0;
        for (int32_t pitch = low_pitch; pitch <= high_pitch; pitch++) {
            if (pitchIsBlackKey(pitch)) {
                n++;
            }
        }

        return n;
    }


    void Audio::pitchString(float pitch, bool cent_flag, int32_t str_size, char* out_str) noexcept {

        static const char* classNames[12] = { "A", "Bb", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

        if (out_str) {
            int32_t i_pitch = static_cast<int32_t>(std::roundf(pitch));
            int32_t pitch_class = pitchClass(i_pitch);
            int32_t octave = (i_pitch) / 12 - 1;

            float cent = pitchIntervalAsCent(i_pitch, pitch);
            int32_t i_cent = static_cast<int32_t>(std::roundf(cent));

            if (cent_flag && i_cent != 0) {
                std::snprintf(out_str, str_size, "%s %d  %+d", classNames[pitch_class], octave, i_cent);
            }
            else {
                std::snprintf(out_str, str_size, "%s %d", classNames[pitch_class], octave);
            }
        }
    }


    float Audio::pitchIntervalAsCent(float pitch_a, float pitch_b) noexcept {

        return freqIntervalAsCent(Audio::freqFromPitch(pitch_a), Audio::freqFromPitch(pitch_b));
    }


    float Audio::freqIntervalAsCent(float freq_a, float freq_b) noexcept {

        return (1200.0f * (logf(freq_b / freq_a) / std::numbers::ln2));
    }


    float Audio::shiftetFreqByCent(float freq, float cent) noexcept {

        return freq * std::pow(2, cent * 0.00083333333333f); // 1 / 100 / 12.
    }


    float Audio::shiftetPitchByCent(float pitch, float cent) noexcept {

        return pitchFromFreq(freqFromPitch(pitch) * std::pow(2, cent * 0.00083333333333f)); // 1 / 100 / 12.
    }


    float Audio::loopFreq(int32_t sample_rate, int32_t sample_count) noexcept {

        return static_cast<float>(sample_rate) / static_cast<float>(sample_count);
    }


    float Audio::interauralSampleDelay(int32_t sample_rate, float level) noexcept {

        // Interaural time difference, 660 μs @ 90 degree
        level = std::clamp(level, 0.0f, 1.0f);
        float interaural_f = 660.0f / 1000000.0f * sample_rate;
        return interaural_f * (1.0f - level);
    }


    float Audio::amplitudeAttenuation(float db, float distance) noexcept {

        return Audio::amplitudeFromDezibel(-db * distance / 100.0f);
    }


    float Audio::levelAttenuation(float db, float distance) noexcept {

        return Audio::levelFromDezibel(-db * distance / 100.0f);
    }


    float Audio::dbAttenuation(float db, float distance) noexcept {

        return -db * distance / 100.0f;
    }


    float Audio::soundIntensity(float initial_intensity, float attenuation, float distance) {

        // Convert attenuation from dB/m to a linear scale
        float linear_attenuation = std::pow(10.0f, (attenuation / 10.0f) * distance);

        // Calculate and return final intensity
        return initial_intensity / linear_attenuation;
    }


#if defined(__APPLE__) && defined(__MACH__)

    void Audio::logAudioStreamBasicDescription(Log& log, AudioStreamBasicDescription& asbd, const char* name) noexcept {
        log << "AudioStreamBasicDescription " << name << log.endl;
        log++;

        log << "sample rate: " << asbd.mSampleRate << log.endl;
        log << "format id: " << asbd.mFormatID << log.endl;
        log << "format flags: " << asbd.mFormatFlags << log.endl;

        log++;
        if (asbd.mFormatFlags & kAudioFormatFlagIsFloat) {
            log << "float\n";
        }
        if (asbd.mFormatFlags & kAudioFormatFlagIsBigEndian) {
            log << "big endian\n";
        }
        if (asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger) {
            log << "signed integer\n";
        }
        if (asbd.mFormatFlags & kAudioFormatFlagIsPacked) {
            log << "is packed\n";
        }
        if (asbd.mFormatFlags & kAudioFormatFlagIsAlignedHigh) {
            log << "aligned high\n";
        }
        if (asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
            log << "lognon interleaved\n";
        }
        if (asbd.mFormatFlags & kAudioFormatFlagIsNonMixable) {
            log << "non mixable\n";
        }
        log--;

        log << "bytes per packet: " << asbd.mBytesPerPacket << log.endl;
        log << "frames per packet: " << asbd.mFramesPerPacket << log.endl;
        log << "bytes per frame: " << asbd.mBytesPerFrame << log.endl;
        log << "channels per frame: " << asbd.mChannelsPerFrame << log.endl;
        log << "bits per channel: " << asbd.mBitsPerChannel << log.endl;
        log << "reserved: " << asbd.mReserved << log.endl;
        log--;
    }

    void Audio::logAudioBufferList(Log& log, AudioBufferList& abl, const char* name) noexcept {
        log << "AudioBufferList " << name << log.endl;
        log++;
        log << "number buffers: " << abl.mNumberBuffers << log.endl;
        log++;
        for (uint32_t i = 0; i < abl.mNumberBuffers; i++) {
            log << "buffer index: " << i << log.endl;
            log++;
            log << "number of channels: " << abl.mBuffers[i].mNumberChannels << log.endl;
            log << "data byte size: " << abl.mBuffers[i].mDataByteSize << log.endl;
            log << "has data: " << (abl.mBuffers[i].mData != nullptr) << log.endl;
            log--;
        }
        log--;
        log--;
    }

#endif


} // End of namespace Grain
