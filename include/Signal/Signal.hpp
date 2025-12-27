//
//  Signal.hpp
//
//  Created by Roald Christesen on 08.08.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

/*
 *  TODO:
 *  - support variable resolution for SimplifiedSignal
 *  - support INT8, INT16, INT32 bit and REAL32
 *  - mixdown - stereo to mono, multitrack to stereo ...
 *  - Goertzel algorithm
 */

#ifndef GrainSignal_hpp
#define GrainSignal_hpp

#include "Grain.hpp"
#include "Type/Type.hpp"
#include "Type/Object.hpp"
#include "String/String.hpp"
#include "2d/Rect.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "DSP/FFT.hpp"
#include "Type/HiResValue.hpp"
#include "DSP/RingBuffer.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include <AudioToolbox/AudioToolbox.h>
#endif


namespace Grain {

    // Forward declarations
    class Signal;
    class SignalFilter;
    class Partials;
    class LUT1;
    class DSP;

    union SignalSamplePtr {
        void* raw = nullptr;
        int8_t* i8;
        int16_t* i16;
        int32_t* i32;
        float* f32;
        double* f64;
    };

    struct SignalSampleFuncInfo {
        const Signal* m_signal{};           ///< Owning signal reference
        int64_t m_sample_index{};           ///< Current sample index
        SignalSamplePtr m_sample_ptr{};     ///< Pointer to current sample
        int64_t m_processed_n{};            ///< Total n umber of processed samples
        union {
            int8_t i8 = 0;
            int16_t i16;
            int32_t i32;
            float f32;
            double f64;
        } m_value;                          ///< Context or accumulation variable
        void* m_ref{};                      ///< Optional reference to external data
    };

    typedef void (*SignalSampleFunc)(SignalSampleFuncInfo& info_ref);

    typedef int8_t (*SignalSampleReader_Int8)(SignalSamplePtr ptr, int64_t index);
    typedef int16_t (*SignalSampleReader_Int16)(SignalSamplePtr ptr, int64_t index);
    typedef int32_t (*SignalSampleReader_Int32)(SignalSamplePtr ptr, int64_t index);
    typedef float (*SignalSampleReader_Float)(SignalSamplePtr ptr, int64_t index);
    typedef double (*SignalSampleReader_Double)(SignalSamplePtr ptr, int64_t index);

    typedef void (*SignalSampleWriter_Int8)(SignalSamplePtr ptr, int64_t index, int8_t value);
    typedef void (*SignalSampleWriter_Int16)(SignalSamplePtr ptr, int64_t index, int16_t value);
    typedef void (*SignalSampleWriter_Int32)(SignalSamplePtr ptr, int64_t index, int32_t value);
    typedef void (*SignalSampleWriter_Float)(SignalSamplePtr ptr, int64_t index, float value);
    typedef void (*SignalSampleWriter_Double)(SignalSamplePtr ptr, int64_t index, double value);


    /**
     *  @class SignalRegion
     *  @brief Represents a segment or region within a signal.
     *
     *  The SignalRegion class defines a bounded section of a signal, typically used
     *  for operations like selection, editing, visualization, or analysis within a
     *  specified sample range and channel. A region may span the entire signal, a
     *  single channel, or a subsection of it, and can optionally carry metadata
     *  such as a name or locked state.
     *
     *  Regions can be chained via the `next_` pointer, allowing construction of
     *  region lists or sequences. Each region supports locking (to prevent
     *  modification), color indexing for UI representation, and functions for
     *  querying and manipulating signal data within its bounds.
     *
     *  Key features:
     *  - Channel-specific or multi-channel support (`channel_`)
     *  - Left/right sample position boundaries
     *  - Lockable state for edit protection
     *  - Optional naming for identification
     *  - Access to raw and mono signal data
     *  - Utility functions for visualization (e.g., color index)
     *
     *  This class is intended to be lightweight and suitable for integration with
     *  both signal processing pipelines and user interfaces.
     */
    class SignalRegion : public Object {
    public:
        enum {
            kColorIndexNormal = 0,
            kColorIndexSelected,
            kColorIndexLocked,
            kColorIndexLockedSelected,
            kColorCount
        };

    public:
        SignalRegion(Signal* signal, const String& name, int32_t channel, int64_t left, int64_t right) noexcept;
        ~SignalRegion() override;

        [[nodiscard]] const char* className() const noexcept override { return "SignalRegion"; }

        [[nodiscard]] int64_t len() const noexcept { return m_right - m_left + 1; }
        [[nodiscard]] const Signal* signal() const noexcept { return m_signal; }
        [[nodiscard]] String name() const noexcept { return m_name; }
        [[nodiscard]] int32_t channel() const noexcept { return m_channel; }
        [[nodiscard]] int64_t left() const noexcept { return m_left; }
        [[nodiscard]] int64_t right() const noexcept { return m_right; }
        [[nodiscard]] int64_t center() const noexcept { return m_left - (m_left - m_right) / 2; }
        [[nodiscard]] bool isLocked() const noexcept { return m_locked; }
        [[nodiscard]] SignalRegion* next() const noexcept { return m_next; }

        [[nodiscard]] int32_t handleColorIndex(bool selected) const noexcept;

        [[nodiscard]] size_t dataSize() const noexcept;
        [[nodiscard]] size_t monoDataSize() const noexcept;
        [[nodiscard]] const void* dataPtr() const noexcept;
        [[nodiscard]] float freq() const noexcept;

        void setName(const String& name) noexcept { m_name = name; }
        void setChannel(int32_t channel) noexcept { m_channel = channel; }
        void setLeftAndRight(int64_t left, int64_t right) noexcept;
        void setLeft(int64_t left) noexcept;
        void setRight(int64_t right) noexcept;
        void setLocked(bool locked) noexcept { m_locked = locked; }
        void toggleLocked() noexcept { m_locked = !m_locked; }
        void lock() noexcept { m_locked = true; }
        void unlock() noexcept { m_locked = false; }

        void setNext(SignalRegion* next) noexcept { m_next = next; }

        void slipLeft() noexcept;
        void slipRight() noexcept;

        [[nodiscard]] Signal* extractSignal() noexcept;
        [[nodiscard]] Signal* extractSignal(bool mono, DataType data_type) noexcept;

    private:
        Signal* m_signal = nullptr;
        String m_name;                  ///< Optional name
        int32_t m_channel = 0;          ///< -1 for all channels or a specific channel index
        int64_t m_left = 0;             ///< Left samplePosition
        int64_t m_right = 0;            ///< Right samplePosition
        bool m_locked = false;
        SignalRegion* m_next = nullptr; ///< Pointer to next region or nullptr
    };


    class SignalRegionRect : public Rectd {
    public:
        SignalRegion* m_region = nullptr;
    };


    /**
     *  @class SimplifiedSignal
     *  @brief A lightweight representation of a Signal with reduced resolution and
     *         bit depth.
     *
     *  This class provides a simplified version of a full-resolution Signal,
     *  optimized for graphical display purposes where high precision is unnecessary.
     *  By reducing resolution and bit depth, it enables more consistent and
     *  efficient rendering in visualizations.
     */
    class SimplifiedSignal : public Object {
    public:
        ~SimplifiedSignal() override {
            free(m_values);
        }

        [[nodiscard]] int16_t* valuesPtr() const noexcept { return m_values; }
        [[nodiscard]] int64_t len() const noexcept { return m_len; }
        [[nodiscard]] int64_t step() const noexcept { return m_step; }

        void update(Signal* signal, int32_t channel) noexcept;

    protected:
        int16_t* m_values = nullptr;
        int64_t m_len = 0;
        int64_t m_step = 0;
    };


    struct SignalInfo {
        int32_t m_channel_count;
        int32_t m_sample_rate;
        int64_t m_sample_count;
        int32_t m_format;
    };


    /**
     *  @class Signal
     *  @brief Represents a time-domain signal and provides basic signal processing
     *         utilities.
     *
     *  The Signal class encapsulates the properties of a discrete-time signal,
     *  including the raw sample data and its associated sampling rate. It supports
     *  multichannel signals. It serves as a foundational component in audio signal
     *  processing and general digital signal processing (DSP) tasks.
     *
     *  This class is designed to simplify the representation and manipulation of
     *  real-valued signals for applications such as:
     *  - Audio signal analysis and transformation (e.g., filtering, windowing)
     *  - Feature extraction (e.g., RMS, peak detection)
     *  - Time-domain visualization and processing
     *  - Signal synthesis and playback preparation
     *  - Preprocessing steps for frequency-domain techniques (e.g., FFT, STFT)
     *
     *  The class can represent both short-time signals (such as frames in streaming
     *  audio) and long-duration signals (e.g., a recorded waveform). It supports
     *  utility functions like automatic time vector computation, allowing easy
     *  synchronization of sample indices with physical time in seconds.
     */
    class Signal : public Object {
        // friend class SignalFile;

    public:
        enum {
            kErr_InvalidWriteSetting = 0,
            kErr_NothingToWrite,
            kErr_UnsupportedDataType,
            kErr_UnsupportedContainerFormat,
            kErr_ReadAllSamplesFailed
        };

        enum class FileContainerFormat {
            AIFF,
            AIFC,
            WAV,
            APPLE_CAF,      ///< macOS specific format
            APPLE_AAC       ///< macOS specific format
        };

        enum class FileSampleEncoding {
            Original,       ///< use internal sample type
            Int8,
            Int16,
            Int24,
            Int32,
            Float,
            ALAW,
            ULAW,
            IMA_ADPCM,
            MS_ADPCM
        };

        enum class CombineMode {
            Replace = 0,
            Add,
            Subtract,
            Multiply
        };

        enum {
            kMaxChannelCount = 4096     // Maximum number of channels
        };

    public:
        Signal(int32_t sample_rate, int64_t sample_count) noexcept;
        Signal(int32_t channel_count, int32_t sample_rate, int64_t sample_count) noexcept;
        Signal(int32_t channel_count, int32_t sample_rate, int64_t sample_count, DataType data_type, bool weights_mode = false) noexcept;

        ~Signal() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "Signal"; }

        friend std::ostream& operator << (std::ostream& os, const Signal* o) {
            o == nullptr ? os << "Signal nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Signal& o) {
            os << o.m_channel_count << " channels @ ";
            os << o.m_sample_rate << " Hz sample rate with ";
            os << o.m_sample_count << " samples of type ";
            os << TypeInfo::name(o.m_data_type);
            return os;
        }

        // Configuration
        ErrorCode configure(int32_t channel_count, int32_t sample_rate, int64_t sample_count, DataType data_type, bool weights_mode = false) noexcept;
        static ErrorCode checkConfiguration(Signal** signal_ptr, int32_t channel_count, int32_t sample_rate, int64_t sample_count, DataType data_type, bool weights_mode = false) noexcept;

        // Memory
        void freeMem() noexcept;
        ErrorCode growIfNeeded(int64_t sample_count) noexcept;

        // Factory
        [[nodiscard]] Signal* copySignal(int64_t offs, int64_t sample_count, bool weights_mode = false) noexcept;
        [[nodiscard]] Signal* createSignalWithSameSetting() const noexcept;
        [[nodiscard]] Signal* createSignalFromChannel(int32_t channel, int64_t offs, int64_t len) const noexcept;

        // Get
        [[nodiscard]] DataType dataType() const noexcept { return m_data_type; }
        [[nodiscard]] int32_t sampleRate() const noexcept { return m_sample_rate; }
        [[nodiscard]] int32_t channelCount() const noexcept { return m_channel_count; }
        [[nodiscard]] int64_t sampleCount() const noexcept { return m_sample_count; }
        [[nodiscard]] int64_t lastSampleIndex() const noexcept { return m_last_sample_index; }
        [[nodiscard]] int64_t allChannelSampleCount() const noexcept { return m_sample_count * m_channel_count; }
        [[nodiscard]] int32_t bitsPerSample() const noexcept { return m_bits_per_sample; }
        [[nodiscard]] int32_t bytesPerSample() const noexcept { return m_bytes_per_sample; }
        [[nodiscard]] size_t dataSize() const noexcept { return m_data_byte_size; }
        [[nodiscard]] int32_t sampleStep() const noexcept { return m_channel_count; }
        [[nodiscard]] double seconds() const noexcept;

        [[nodiscard]] inline const void* dataPtr() const noexcept { return m_data.raw; }
        [[nodiscard]] const void* dataPtr(int32_t channel, int64_t index = 0) const noexcept;

        [[nodiscard]] inline void* mutDataPtr() const noexcept { return m_data.raw; }
        [[nodiscard]] void* mutDataPtr(int32_t channel, int64_t index = 0) const noexcept;

        [[nodiscard]] inline const void* lastSamplePtr() const noexcept {
            return dataPtr(m_channel_count - 1, m_last_sample_index);
        }

        // Information about the signal
        [[nodiscard]] bool hasData() const noexcept {
            return m_data_byte_size > 0 && m_data.raw;
        }

        [[nodiscard]] bool hasChannel(int32_t channel) const noexcept {
            return channel >= 0 && channel < m_channel_count;
        }

        [[nodiscard]] bool hasChannelAndData(int32_t channel) const noexcept {
            return channel >= 0 && channel < m_channel_count && hasData();
        }

        [[nodiscard]] inline bool hasSampleAtChannel(int32_t channel, int64_t index) const noexcept {
            return
                    m_data.raw &&
                    channel >= 0 && channel < m_channel_count &&
                    index >= 0 && index < m_last_sample_index;
        }

        void _checkProcessTypeChannelIndex(DataType data_type, int32_t channel, int64_t index) const {
            if (!hasData()) {
                Exception::throwMessage(ErrorCode::NoData, "Signal has no data to process.");
            }
            if (!hasChannel(channel)) {
                Exception::throwFormattedMessage(
                        ErrorCode::BadArgs,
                        "Signal has %d channels, but no channel at index %d.",
                        m_channel_count,
                        channel);
            }
            if (m_data_type != data_type) {
                Exception::throwFormattedMessage(
                        ErrorCode::UnsupportedDataType,
                        "Unsupported data type: %s. This function requires type %s.",
                        TypeInfo::name(data_type),
                        TypeInfo::name(m_data_type));
            }
            if (index < 0 || index >= m_last_sample_index) {
                Exception::throwFormattedMessage(
                        ErrorCode::IndexOutOfRange,
                        "Signal has %" PRId64 " samples, but does not contain a sample at index %" PRId64 ".",
                        m_sample_count,
                        index);
            }
        }

        [[nodiscard]] bool isMono() const noexcept { return m_channel_count == 1; }
        [[nodiscard]] bool isStereo() const noexcept { return m_channel_count == 2; }
        [[nodiscard]] bool isQuadra() const noexcept { return m_channel_count == 4; }

        [[nodiscard]] bool canAccessFloatMono() const noexcept { return canAccessFloatInChannel(0); }
        [[nodiscard]] bool canAccessFloatStereo() const noexcept { return canAccessFloatInChannelByMask(0x3); }
        [[nodiscard]] bool canAccessFloatQuadra() const noexcept { return canAccessFloatInChannelByMask(0xF); }
        [[nodiscard]] bool canAccessFloatInChannel(int32_t channel) const noexcept;
        [[nodiscard]] bool canAccessFloatInChannelByMask(uint32_t channel_mask) const noexcept;

        [[nodiscard]] bool isIntegerType() const noexcept { return TypeInfo::isInteger(m_data_type); }
        [[nodiscard]] bool isFloatingPointType() const noexcept { return TypeInfo::isFloatingPoint(m_data_type); }
        [[nodiscard]] bool isInt8Type() const noexcept { return m_data_type == DataType::Int8; }
        [[nodiscard]] bool isInt16Type() const noexcept { return m_data_type == DataType::Int16; }
        [[nodiscard]] bool isInt32Type() const noexcept { return m_data_type == DataType::Int32; }
        [[nodiscard]] bool isFloatType() const noexcept { return m_data_type == DataType::Float; }

        // Set
        void setSampleRate(int32_t sample_rate) noexcept { m_sample_rate = sample_rate; }

        // Compare
        [[nodiscard]] bool hasSameSettingAs(const Signal* signal) const noexcept;
        [[nodiscard]] bool hasSameSampleRateAs(const Signal* signal) const noexcept { return signal && signal->m_sample_rate == m_sample_rate; }

        //
        ErrorCode forEachSample(SignalSampleFunc func, SignalSampleFuncInfo& info, int32_t channel, int64_t offs, int64_t len, int64_t stride) const;
        ErrorCode forEachSampleOfType(DataType data_type, SignalSampleFunc func, SignalSampleFuncInfo& info, int64_t offs, int64_t len, int64_t stride) const;
        ErrorCode forEachSampleOfType(DataType data_type, SignalSampleFunc func, SignalSampleFuncInfo& info, int32_t channel, int64_t offs, int64_t len, int64_t stride) const;

        // Information about the signal
        template <typename T>
        static void _absMaxFunc(SignalSampleFuncInfo& info_ref);
        [[nodiscard]] double absMax(int32_t channel, int64_t offs = 0, int64_t len = -1, int64_t stride = 1) const noexcept;
        [[nodiscard]] double absMax() const noexcept;

        template <typename T>
        static void _averageFunc(SignalSampleFuncInfo& info_ref);
        [[nodiscard]] double average(int32_t channel, int64_t offs = 0, int64_t len = -1, int64_t stride = 1) const noexcept;

        template <typename T>
        static void _absAverageFunc(SignalSampleFuncInfo& info_ref);
        [[nodiscard]] double absAverage(int32_t channel, int64_t offs = 0, int64_t len = -1, int64_t stride = 1) const noexcept;

        template <typename T>
        static void _rmsFunc(SignalSampleFuncInfo& info_ref);
        [[nodiscard]] double rms(int32_t channel, int64_t offs, int64_t len, int64_t stride) const noexcept;


        template <typename T>
        static void _scaleFloatFunc(SignalSampleFuncInfo& info_ref);
        template <typename T>
        static void _scaleDoubleFunc(SignalSampleFuncInfo& info_ref);
        void scale(float scale_factor) const noexcept;
        void scale(int32_t channel, int64_t offs, int64_t len, float scale_factor) const noexcept;

        void derivate() noexcept;
        void derivate(int32_t channel, int64_t offs, int64_t len) noexcept;



        [[nodiscard]] SimplifiedSignal* simplifiedSignalByChannel(int32_t channel) noexcept;

        // Utilities
        [[nodiscard]] int64_t samplesNeededForMilliseconds(int64_t milliseconds) const noexcept;
        [[nodiscard]] int64_t samplesNeededForSeconds(float seconds) const noexcept;
        [[nodiscard]] int64_t samplesNeededForNote(float bpm, float len) const noexcept;

        int64_t clampOffsAndLen(int64_t& offs, int64_t& len) const noexcept;
        int64_t clampStartEndIndex(int64_t& start_index, int64_t& end_index) const noexcept;


        // Modify
        [[nodiscard]] int64_t silentTo(float threshold = 0.0001f) noexcept;
        [[nodiscard]] int64_t silentFrom(float threshold = 0.0001f) noexcept;


        // Samples access
        [[nodiscard]] int8_t readInt8(int32_t channel, int64_t index) const noexcept;
        [[nodiscard]] int16_t readInt16(int32_t channel, int64_t index) const noexcept;
        [[nodiscard]] int32_t readInt32(int32_t channel, int64_t index) const noexcept;
        [[nodiscard]] float readFloat(int32_t channel, int64_t index) const noexcept;
        [[nodiscard]] double readDouble(int32_t channel, int64_t index) const noexcept;

        void writeInt8(int32_t channel, int64_t index, int8_t value) noexcept;
        void writeInt16(int32_t channel, int64_t index, int16_t value) noexcept;
        void writeInt32(int32_t channel, int64_t index, int32_t value) noexcept;
        void writeFloat(int32_t channel, int64_t index, float value) noexcept;
        void writeDouble(int32_t channel, int64_t index, double value) noexcept;

        [[nodiscard]] float readFloatLerp(int32_t channel, const HiResValue& sample_pos) const noexcept;

        void addSample(int32_t channel, int64_t index, float value) noexcept;
        void addSampleLerp(int32_t channel, const HiResValue& sample_pos, float value) noexcept;
        void scaleSample(int32_t channel, int64_t index, float scale_factor) noexcept;



        void setRingBufferSample(int32_t channel, int64_t index, float value) noexcept;
        void addRingBufferSample(int32_t channel, int64_t index, float value) noexcept;


        [[nodiscard]] float ringBufferSampleInterpolated(int32_t channel, const HiResValue& sample_pos) const noexcept;

        // Weights
        void clearWeights() noexcept;
        void clearWeights(int64_t n) noexcept;
        [[nodiscard]] int64_t weightedStart() const noexcept { return m_weighted_start; }
        [[nodiscard]] int64_t weightedEnd() const noexcept { return m_weighted_end; }
        bool addWeightedSample(int32_t channel, const HiResValue& sample_pos, float value) noexcept;
        bool finishWeightedSamples(int32_t channel) noexcept;

        // Copy, read, write, combine
        int64_t copyAll(const Signal* src, int32_t src_channel, int64_t dst_offs, uint32_t dst_channelMask = 0xFFFF) noexcept;
        int64_t copySamples(const Signal* src) noexcept;
        int64_t copySamples(const Signal* src, int64_t len, int64_t src_offs = 0, int64_t dst_offs = 0) noexcept;
        int64_t copySamples(const Signal* src, int64_t len, int32_t src_channel, int64_t src_offs, int32_t dst_channel, int64_t dst_offs) noexcept;

        int64_t copyChannel(int32_t src_channel, int32_t dst_channel) noexcept;
        int64_t copyChannel(int32_t src_channel, int32_t dst_channel, int64_t offs, int64_t len) noexcept;

        int64_t readSamplesAsFloatWithZeroPadding(int32_t channel, int64_t offs, int64_t len, float* out_samples) const noexcept;
        int64_t readSamples(int32_t channel, int64_t offs, int64_t len, DataType data_type, void* out_samples) noexcept;

        int64_t writeSamples(int32_t channel, int64_t offs, int64_t len, const float* samples, CombineMode combine_mode = CombineMode::Replace) noexcept;

        int64_t combineSamples(const Signal* src, int64_t len, int32_t src_channel, int64_t src_offs, int32_t dst_channel, int64_t dst_offs, CombineMode combine_mode, float amount) noexcept;
        int64_t combineSamples(const Signal* src, int64_t len, int64_t src_offs, int64_t dst_offs, CombineMode combine_mode, float amount) noexcept;

        // Mix
        int64_t mixByAudioPos(const Signal* src, int64_t len, int64_t src_offs, int64_t dst_offs, const Vec3d& audio_pos) noexcept;

        // Clear
        void clear() noexcept;
        void clear(int64_t len) noexcept;
        void clear(int64_t offs, int64_t len) noexcept;
        void clearChannel(int32_t channel) noexcept;
        void clearChannel(int32_t channel, int64_t offs, int64_t len) noexcept;
        void clearMaskedChannels(uint32_t channel_mask, int64_t offs, int64_t len) noexcept;
        void clearAll() noexcept;

        // Modify
        void normalize(float target_level = 1.0) noexcept;
        void normalize(int32_t channel, int64_t offs, int64_t len, float target_level = 1.0) noexcept;

        void centerPowerOfChannel(int32_t channel, int64_t offs, int64_t len) noexcept;

        void fadeIn(int64_t offs, int64_t len) noexcept;
        void fadeInChannel(int32_t channel, int64_t offs, int64_t len) noexcept;
        void fadeOut(int64_t offs, int64_t len) noexcept;
        void fadeOutChannel(int32_t channel, int64_t offs, int64_t len) noexcept;
        void fadeChannel(int32_t channel, int64_t offs, int64_t len, bool fade_out_mode) noexcept;

        double envelope(int64_t offs, int64_t len, float start_amplitude, float end_amplitude, float shape = 0.0f) noexcept;
        double envelope(int32_t channel, int64_t offs, int64_t len, float start_amplitude, float end_amplitude, float shape = 0.0f) noexcept;

        template<typename T>
        double _envelopeTyped(int32_t channel, int64_t offs, int64_t len, T start_amplitude, T end_amplitude, float shape) noexcept;

        void applyEnvelopeLUT(int32_t channel, int64_t offs, const LUT1* lut) noexcept;


            //
        template<typename T>
        void reverseTyped(T* left, T* right, int64_t count, int64_t step);

        void reverse() noexcept;
        void reverse(int64_t offs, int64_t len) noexcept;
        void reverseChannel(int32_t channel, int64_t offs, int64_t len) noexcept;



        ErrorCode resample(int32_t channel, int32_t sample_rate, int64_t offs, int64_t len, float* out_ptr, int64_t step = 1) noexcept;

        ErrorCode changeSampleRate(int32_t sample_rate) noexcept;

        // Ring buffer
        [[nodiscard]] int64_t ringBufferIndex(int64_t index) const noexcept;
        [[nodiscard]] float ringBufferSample(int32_t channel, int64_t index) const noexcept;

        // Filter and Effects
        void distortChannel(int32_t channel, int64_t offs, int64_t len, float coef) noexcept;

        ErrorCode applyFilter(SignalFilter* filter) noexcept;
        ErrorCode applyFilter(SignalFilter* filter, int64_t offs, int64_t len) noexcept;
        ErrorCode applyFilterToChannel(SignalFilter* filter, int32_t channel) noexcept;
        ErrorCode applyFilterToChannel(SignalFilter* filter, int32_t channel, int64_t offs, int64_t len) noexcept;

        ErrorCode applyFilterFFT(const Partials* partials) noexcept;
        ErrorCode applyFilterFFT(const Partials* partials, int64_t len) noexcept;
        ErrorCode applyFilterFFTToChannel(const Partials* partials, int32_t channel) noexcept;
        ErrorCode applyFilterFFTToChannel(const Partials* partials, int32_t channel, int64_t len) noexcept;

        void releaseFilterFFTResources();

        ErrorCode convolve(int64_t a_len, const Signal* b_signal, Signal* result_signal, int64_t partition_len) const noexcept {
            return convolveChannel(0, 0, a_len, b_signal, 0, 0, -1, result_signal, 0, partition_len);
        }

        ErrorCode convolveChannel(
                int32_t channel, int64_t offs, int64_t len,
                const Signal* ir, int32_t ir_channel, int64_t ir_offs, int64_t ir_len,
                Signal* result_signal, int32_t result_channel,
                int64_t partition_len
        ) const noexcept;

        // Generate
        void addWhiteNoise(int64_t offs, int64_t len, float amount = 1.0f, float threshold = 1.0f) noexcept;
        void addWhiteNoiseToChannel(int32_t channel, int64_t offs, int64_t len, float amount = 1.0f, float threshold = 1.0f) noexcept;

        void generateSine(int32_t channel, int64_t offs, int64_t len, float freq) noexcept;
        void generateSineSweep(int32_t channel, float start, float duration, float freq_start, float freq_end, float db_start, float db_end, float fade_in_duration, float fade_out_duration) noexcept;


        // File
        [[nodiscard]] ErrorCode writeToFile(
                const String& file_path,
                FileContainerFormat container_format,
                FileSampleEncoding sample_encoding,
                int64_t offs,
                int64_t len) const noexcept;

        [[nodiscard]] ErrorCode writeToFile(
                const String& file_path,
                FileContainerFormat container_format,
                FileSampleEncoding sample_encoding) const noexcept;

        [[nodiscard]] ErrorCode writeRegionToFile(
                const String& file_path,
                FileContainerFormat container_format,
                FileSampleEncoding sample_encoding,
                int32_t region_index) const noexcept;

        [[nodiscard]] static int32_t fileInfo(const String& file_path, SignalInfo& out_info) noexcept;

        [[nodiscard]] static Signal* createFromFile(
                const String& file_path,
                DataType data_type,
                ErrorCode& out_err) noexcept;

        // Regions
        [[nodiscard]] int32_t regionCount() const noexcept { return m_region_count; }
        [[nodiscard]] SignalRegion* firstRegionPtr() const noexcept { return m_first_region; }
        [[nodiscard]] SignalRegion* regionPtrAtIndex(int32_t index) const noexcept;
        [[nodiscard]] SignalRegion* addRegion(const String& name, int32_t channel, int64_t left, int64_t right) noexcept;
        bool removeRegion(SignalRegion* region) noexcept;
        [[nodiscard]] static int _compareRegion(const void* region_ptr0, const void* region_ptr1);
        [[nodiscard]] ErrorCode sortRegions() noexcept;
        void setMustSortRegions() noexcept { m_regions_must_sort = true; }

        // Utilities
        [[nodiscard]] int64_t sampleIndexAtSecond(double sec) const noexcept {
            return static_cast<int64_t>(std::round(sec * m_sample_rate));
        }

        [[nodiscard]] static double findNearestFrequency(int32_t sample_rate, int64_t buffer_len, double freq) noexcept;

        [[nodiscard]] static double releaseCoef(double start_level, double end_level, double min_level, int32_t sample_rate, double duration_seconds) noexcept;
        [[nodiscard]] static double releaseCoef(double start_level, double end_level, double min_level, int64_t sample_count) noexcept;
        [[nodiscard]] static double releaseLen(double start_level, double end_level, double min_level, double coef) noexcept;
        [[nodiscard]] static double releaseValue(double start_level, double coef, int64_t t) noexcept;


        [[nodiscard]] inline static float drive1(float value, float f, float amount) {
            float d = std::tanh(value * (amount * f + 0.01f));
            return value + (d - value) * amount;
        }

        [[nodiscard]] inline static float drive2(float value, float amount) {
            float a = std::sin(((amount + 0.01f) / 1.02f) * static_cast<float>(std::numbers::pi / 2));
            float k = (a + a) / (1.0f - a);
            return (k + 1) * value / (1.0f + k * std::fabs(value));
        }

        int64_t _updateSimplified() noexcept;

    private:
        void _prepareFilterFFT(int32_t fft_len, int32_t window_len);

    protected:
        DataType m_data_type = DataType::Undefined; ///< Sample data type
        int32_t m_sample_rate = 0;                  ///< Sample rate as samples per second
        int32_t m_channel_count = 0;                ///< The number of channels
        int64_t m_sample_count = 0;                 ///< Number of samples (per channel)
        SignalSamplePtr m_data;                     ///< Pointer to sample data

        ObjectList<SimplifiedSignal*> m_simplified_signals;

        int64_t m_last_sample_index = 0;            ///< The index of the last sample, useful for loops etc.
        int32_t m_bits_per_sample = 0;              ///< Number of bits in a single sample
        int32_t m_bytes_per_sample = 0;             ///< Number of bytes in a single sample
        size_t m_data_byte_size = 0;                ///< Size of all samples in bytes

        bool m_weights_mode = false;                ///< If set to true samples can be weighted, useful for for interpolations and other computations
        int64_t m_weighted_start = 0;               ///< Index of the first weighted sample
        int64_t m_weighted_end = 0;                 ///< Index of the last weighted sample
        size_t m_weights_size = 0;                  ///< Size of the weights buffer
        float* m_weights = nullptr;                 ///< Pointer to the weights

        int32_t m_region_count = 0;                 ///< Number of regions
        SignalRegion* m_first_region = nullptr;     ///< Pointer to the first region or nullptr, if no regions exist
        bool m_regions_must_sort = false;           ///< If set to true, regions have to be sorted before usage
        SignalRegion** m_sort_region_ptr_array = nullptr;   ///< Used for sorting
        int32_t m_sort_region_ptr_array_capacity = 0;       ///< Capacity of the arry for sorting

        // Memory and Resources used for computations
        FFT* m_fft = nullptr;
        float* m_fft_buffer = nullptr;
        int32_t m_fft_window_len = 0;
        float* m_fft_window = nullptr;

    private:
        //
        SignalSampleReader_Int8 _m_reader_i8{};
        SignalSampleReader_Int16 _m_reader_i16{};
        SignalSampleReader_Int32 _m_reader_i32{};
        SignalSampleReader_Float _m_reader_f32{};
        SignalSampleReader_Double _m_reader_f64{};

        SignalSampleWriter_Int8 _m_writer_i8{};
        SignalSampleWriter_Int16 _m_writer_i16{};
        SignalSampleWriter_Int32 _m_writer_i32{};
        SignalSampleWriter_Float _m_writer_f32{};
        SignalSampleWriter_Double _m_writer_f64{};

        void _updateAccessors();

    };


    class SignalConvolveSetup {
    public:
        int64_t m_ir_len = -1;

        int64_t m_partition_len = -1;
        int32_t m_partition_log_n = -1;
        int32_t m_partition_count = -1;

        int32_t m_fft_len = -1;
        int32_t m_fft_half_len = -1;
        int32_t m_fft_log = -1;
        int32_t m_overlap_len = -1;

        float* m_time_buffer = nullptr;
        float* m_interleaved_buffer = nullptr;
        float* m_write_buffer = nullptr;
        float* m_overlap_buffer = nullptr;
        float* m_t_out = nullptr;
        FFTComplexSplitArray* m_ir_partials = nullptr;
        FFTComplexSplitArray* m_x_ring = nullptr;
        FFTComplexSplit* m_y_freq = nullptr;

#if defined(__APPLE__) && defined(__MACH__)
        FFTSetup fft_setup = nullptr;
#else
        #pragma message("SignalConvolveSetup fft_setup must be implemented for Linux")
#endif

    public:
        SignalConvolveSetup(int64_t ir_len, int32_t partition_len);
        ErrorCode checkSettings(int64_t ir_len, int32_t partition_len) noexcept;
        void freeMemory() noexcept;
    };


} // End of namespace Grain

#endif // GrainSignal_hpp
