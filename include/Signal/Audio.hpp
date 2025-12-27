//
//  Audio.hpp
//
//  Created by Roald Christesen on 27.03.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainAudio_hpp
#define GrainAudio_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreAudio/CoreAudioTypes.h>
#endif


namespace Grain {

    class Log;
    class LUT1;

    typedef struct {
        const char* m_name;
        int32_t m_lowest_pitch;
        int32_t m_highest_pitch;
    } InstrumentCharacteristics;


    class AudioFilterBand {
    protected:
        float m_freg = 1000.0f;
        float width_ = 100.0f;
        float m_db = 0.0f;
    };


    class Audio {
    public:
        enum class Scope {
            Input = 0,
            Output,
            Through
        };

        static constexpr float kMinDb = -1500.0f;   ///< Lowest db
        static constexpr float kMinLinear = 1e-12f; ///< Small positive floor

        enum {
            kMusicPitch_C0 = 12,
            kMusicPitch_C1 = 24,
            kMusicPitch_C2 = 36,
            kMusicPitch_C3 = 48,
            kMusicPitch_C4 = 60,
            kMusicPitch_C5 = 72,
            kMusicPitch_C6 = 84,
            kMusicPitch_C7 = 96,
            kMusicPitch_C8 = 108,

            kMusic_C = 0,
            kMusic_C_Sharp = 1,
            kMusic_D_Flat = 1,
            kMusic_D = 2,
            kMusic_D_Sharp = 3,
            kMusic_E_Flat = 3,
            kMusic_E = 4,
            kMusic_F = 5,
            kMusic_F_Sharp = 6,
            kMusic_G_Flat = 6,
            kMusic_G = 7,
            kMusic_G_Sharp = 8,
            kMusic_A_Flat = 8,
            kMusic_A = 9,
            kMusic_A_Sharp = 10,
            kMusic_B_Flat = 10,
            kMusic_B = 11,

            kMusic_PitchesPerOctave = 12,

            kMusic_PianoLowestPitch = 21, // A0
            kMusic_PianoHighestPitch = 108, // C8
            kMusic_PianoKeysCount = 108 - 21 + 1, // A0 - C8

            kMusic_BoesendorderLowestPitch = 12, // C0

            kMidiPitchCount = 128, // 0 to 127

            // MIDI pitches of GM (General MIDI) drumset
            kMidiGSDrum_AcousticBassDrum = 35,
            kMidiGSDrum_BassDrum,
            kMidiGSDrum_SideStick,
            kMidiGSDrum_AcousticSnare,
            kMidiGSDrum_HandClap,
            kMidiGSDrum_ElectricSnare,
            kMidiGSDrum_LowFloorTom,
            kMidiGSDrum_ClosedHiHat,
            kMidiGSDrum_HighFloorTom,
            kMidiGSDrum_PedalHiHat,
            kMidiGSDrum_LowTom,
            kMidiGSDrum_OpenHiHat,
            kMidiGSDrum_LowMidTom,
            kMidiGSDrum_HiMidTom,
            kMidiGSDrum_CrashCymbal1,
            kMidiGSDrum_HighTom,
            kMidiGSDrum_RideCymbal1,
            kMidiGSDrum_ChineseCymbal,
            kMidiGSDrum_RideBell,
            kMidiGSDrum_Tambourine,
            kMidiGSDrum_SplashCymbal,
            kMidiGSDrum_Cowbell,
            kMidiGSDrum_CrashCymbal2,
            kMidiGSDrum_Vibraslap,
            kMidiGSDrum_RideCymbal2,
            kMidiGSDrum_HiBongo,
            kMidiGSDrum_LowBongo,
            kMidiGSDrum_MuteHiConga,
            kMidiGSDrum_OpenHiConga,
            kMidiGSDrum_LowConga,
            kMidiGSDrum_HighTimbale,
            kMidiGSDrum_LowTimbale,
            kMidiGSDrum_HighAgogo,
            kMidiGSDrum_LowAgogo,
            kMidiGSDrum_Cabasa,
            kMidiGSDrum_Maracas,
            kMidiGSDrum_ShortWhistle,
            kMidiGSDrum_LongWhistle,
            kMidiGSDrum_HortGuiro,
            kMidiGSDrum_LongGuiro,
            kMidiGSDrum_Claves,
            kMidiGSDrum_HiWoodBlock,
            kMidiGSDrum_LowWoodBlock,
            kMidiGSDrum_MuteCuica,
            kMidiGSDrum_OpenCuica,
            kMidiGSDrum_MuteTriangle,
            kMidiGSDrum_OpenTriangle
        };

        enum class MidiGSInstrument {
            Piano = 0,
            Guitar,
            BassGuitar,
            Violin,
            Viola,
            Cello,
            DoubleBass,
            Flute,
            Clarinet,
            Saxophone,
            Trumpet,
            Trombone,
            FrenchHorn,
            Tuba,
            VoiceMale,
            VoiceFemale,
            VoiceChild,
            DrumsPercussion,

            First = 0,
            Last = DrumsPercussion
        };

        enum {
            kMinSampleRate = 1,
            kMaxSampleRate = 384000,

            kPitchToFreqResolution = 20000,
            kPitchToFreqDenominator = 100,

            kPitchStringLength = 20
        };


    public:
        static const double g_envelope_min_level;
        static LUT1* g_release_lut;
        static LUT1* g_release_duration_lut;

        static const InstrumentCharacteristics g_instrument_characteristics[];

    public:
        static void _init() noexcept;
        static void _exit() noexcept;

        // Musical note
        [[nodiscard]] static float durationForNote(float bpm, float length) noexcept;
        [[nodiscard]] static float samplesNeededForNote(int32_t sample_rate, float bpm, float length) noexcept;

        // Amplitude, Dezibel
        [[nodiscard]] static float linearToDb(float level) noexcept;
        [[nodiscard]] static float dbToLinear(float db) noexcept;

        // Frequency, Pitch
        [[nodiscard]] static float freqFromPitch(float pitch, float reference_freq = 440) noexcept;
        [[nodiscard]] static float freqFromPitchAndOctave(float pitch, int32_t octave, float reference_freq = 440) noexcept;

        [[nodiscard]] static float pitchFromFreq(float freq, float reference_freq = 440) noexcept;
        [[nodiscard]] static int32_t pitchClass(float pitch) noexcept;
        [[nodiscard]] static int32_t pitchClass(int32_t pitch) noexcept;
        [[nodiscard]] static int32_t noteClassFromPitch(int32_t pitch) noexcept;
        [[nodiscard]] static int32_t noteClassFromPitch(float pitch) noexcept;

        [[nodiscard]] static bool samePitchClass(int32_t pitch, int32_t reference_pitch) noexcept;
        [[nodiscard]] static bool pitchIsBlackKey(int32_t pitch) noexcept;
        [[nodiscard]] static bool pitchIsWhiteKey(int32_t pitch) noexcept;
        [[nodiscard]] static int32_t pitchCountBlackKeys(int32_t low_pitch, int32_t high_pitch) noexcept;
        static void pitchString(float pitch, bool cent_flag, int32_t str_size, char* out_str) noexcept;

        [[nodiscard]] static float pitchIntervalAsCent(float pitch_a, float pitch_b) noexcept;
        [[nodiscard]] static float freqIntervalAsCent(float freq_a, float freq_b) noexcept;
        [[nodiscard]] static float shiftetFreqByCent(float freq, float cent) noexcept;
        [[nodiscard]] static float shiftetPitchByCent(float pitch, float cent) noexcept;

        [[nodiscard]] static float loopFreq(int32_t sample_rate, int32_t sample_count) noexcept;

        // Utilities
        [[nodiscard]] static float interauralSampleDelay(int32_t sample_rate, float level) noexcept;

        [[nodiscard]] static float amplitudeAttenuation(float db, float distance) noexcept;
        [[nodiscard]] static float dbAttenuation(float db, float distance) noexcept;
        [[nodiscard]] static float soundIntensity(float initial_intensity, float attenuation, float distance);

        #if defined(__APPLE__) && defined(__MACH__)
            static void logAudioStreamBasicDescription(Log& l, AudioStreamBasicDescription& asbd, const char* name) noexcept;
            static void logAudioBufferList(Log& l, AudioBufferList& abl, const char* name) noexcept;
        #endif
    };


} // End of namespace Grain

#endif // GrainAudio_hpp
