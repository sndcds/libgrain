//
//  Signal.cpp
//
//  Created by Roald Christesen on 08.08.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/Signal.hpp"
#include "Signal/SignalFilter.hpp"
#include "Signal/Audio.hpp"
#include "Type/HiResValue.hpp"
#include "DSP/LUT1.hpp"
#include "DSP/Partials.hpp"
#include "DSP/FFT.hpp"
#include "DSP/DSP.hpp"
#include "Math/Random.hpp"
#include "DSP/RingBuffer.hpp"
#include "Core/Log.hpp"

#include <sndfile.h>


namespace Grain {

    /**
     *  @brief Updates the simplified signal representation from a specified channel
     *         of a source signal.
     *
     *  This method processes a given `Signal` object to generate a simplified
     *  version of the signal's envelope or energy profile, optimized for
     *  visualization. It downsamples the signal by a fixed factor (4096),
     *  calculates a smoothed energy value for each segment, and stores it in an
     *  internal buffer.
     *
     *  @details
     *  The simplified signal is computed by dividing the full signal into blocks
     *  of 4096 samples (fixed), squaring and summing the samples to calculate
     *  the local energy, taking the square root, and scaling the result to fit
     *  within a 16-bit signed integer range. This is especially useful for:
     *
     *  - Audio visualization (e.g., waveform overviews)
     *  - Rough envelope detection
     *  - Signal monitoring or metering
     *
     *  It ensures minimum time resolution based on the signal's sample rate to
     *  maintain consistent visual responsiveness across various input sources.
     *
     *  Memory for the internal buffer is dynamically allocated or reallocated
     *  if the required length changes.
     *
     *  @param signal Pointer to the source Signal object (must not be nullptr).
     *  @param channel Index of the channel to simplify (must be valid for the
     *                 given signal).
     *
     *  @note If the signal pointer is null, the specified channel is invalid, or
     *        the downsampled length is less than 1, the method returns early
     *        without changes.
     *
     *  @warning This method allocates memory dynamically. Ensure `update()` is
     *           called judiciously to avoid memory fragmentation or leaks.
     */
    void SimplifiedSignal::update(Signal* signal, int32_t channel) noexcept {
        int64_t new_len = 0;

        if (!signal) {
            return;
        }

        if (!signal->hasChannel(channel)) {
            return;
        }

        // TODO: Implement custom, context specific divisor instead of using hard coded 4096.
        new_len = signal->sampleCount() / 4096;
        if (new_len < 1) {
            return;
        }

        if (!m_values || new_len != m_len) {
            m_values = (int16_t*)malloc(sizeof(int16_t) * new_len);
            if (!m_values) {
                m_len = 0;
                return;
            }
        }

        m_len = new_len;
        int32_t sample_rate = signal->sampleRate();
        int64_t sample_count = signal->sampleCount();

        auto min_step = static_cast<int64_t>(std::round(static_cast<double>(m_len) / 500 * sample_rate / 44100));
        auto ms = static_cast<int64_t>(std::round(15.0 * sample_rate / 44100));
        if (min_step < ms) {
            min_step = ms;
        }

        int64_t step = m_len;
        if (step < min_step) {
            step = min_step;
        }

        m_step = 4096;

        int16_t* d = m_values;
        for (int64_t i = 0; i < sample_count; ) {
            float v = 0.0f;
            for (int64_t j = 0; j < 4096; j++, i++) {
                float s = signal->readFloat(channel, i);
                if (s < 0.0f) {
                    s = -s;
                }
                v += s * s;
            }
            *d++ = static_cast<int16_t>(std::sqrt(v / static_cast<float>(step)) * std::numeric_limits<int16_t>::max());
        }
    }


    Signal::Signal(int32_t sample_rate, int64_t sample_count) noexcept {
        auto err = configure(1, sample_rate, sample_count, DataType::Float);
        if (err != ErrorCode::None) {
            clearAll();
        }
    }


    Signal::Signal(int32_t channel_count, int32_t sample_rate, int64_t sample_count) noexcept {
    auto err = configure(channel_count, sample_rate, sample_count, DataType::Float);
        if (err != ErrorCode::None) {
            clearAll();
        }
    }


    Signal::Signal(
            int32_t channel_count,
            int32_t sample_rate,
            int64_t sample_count,
            DataType data_type,
            bool weights_mode) noexcept : Object()
    {
        auto err = configure(channel_count, sample_rate, sample_count, data_type, weights_mode);
        if (err != ErrorCode::None) {
            clearAll();
        }
    }


    Signal::~Signal() noexcept {
        // Delete all regions
        SignalRegion* region = m_first_region;
        while (region) {
            SignalRegion* next = region->next();
            delete region;
            region = next;
        }

        std::free(m_data.raw);
        std::free(m_weights);

        if (m_sort_region_ptr_array) {
            std::free(m_sort_region_ptr_array);
        }

        releaseFilterFFTResources();
    }


    /**
     *  @brief Configures the signal with new parameters.
     *
     *  Sets the signal's channel layout, sample rate, sample count, data type, and
     *  whether to allocate an additional buffer for sample weights.
     *
     *  If configuration fails (e.g., due to invalid arguments or memory allocation issues),
     *  the internal state remains unchanged, and an appropriate error code is returned.
     *
     *  @param channel_count Number of signal channels (must be ≥ 1 and ≤ kMaxChannelCount).
     *  @param sample_rate Sampling rate in Hz (must be ≥ 1).
     *  @param sample_count Number of samples per channel (must be ≥ 0).
     *  @param data_type Type of data for each sample (e.g., Float, Int16).
     *  @param weights_mode Whether to enable weight buffer allocation (for per-sample weights).
     *
     *  @return ErrorCode::None if successful, or an appropriate error code:
     *  - ErrorCode::UnsupportedChannelCount if the number of channels is not allowed.
     *  - ErrorCode::UnsupportedSampleRate if the sample rate is invalid.
     *  - ErrorCode::UnsupportedDataType if the data type is unsupported.
     *  - ErrorCode::MemCantAllocate if memory allocation fails.
     */
    ErrorCode Signal::configure(
            int32_t channel_count,
            int32_t sample_rate,
            int64_t sample_count,
            DataType data_type,
            bool weights_mode) noexcept
    {
        if (channel_count <= 0 || channel_count > kMaxChannelCount) {
            return ErrorCode::UnsupportedChannelCount;
        }

        if (sample_rate < 1) {
            return ErrorCode::UnsupportedSampleRate;
        }

        switch (data_type) {
            case DataType::Int8:
                m_bits_per_sample = 8;
                break;
            case DataType::Int16:
                m_bits_per_sample = 16;
                break;
            case DataType::Int32:
            case DataType::Float:
                m_bits_per_sample = 32;
                break;
            default:
                return ErrorCode::UnsupportedDataType;
        }

        m_bytes_per_sample = m_bits_per_sample / 8;

        // Set new parameters
        m_channel_count = channel_count;
        m_sample_rate = sample_rate;
        m_data_type = data_type;
        m_sample_count = sample_count;
        m_last_sample_index = sample_count - 1;
        m_weights_mode = weights_mode;

        // (Re-)allocate memory
        if (sample_count > 0) {
            auto new_data_size = static_cast<size_t>(sample_count) * channel_count * TypeInfo::byteSize(data_type);
            if (new_data_size != m_data_byte_size) {
                void* new_data = std::realloc(m_data.raw, new_data_size);
                if (!new_data) {
                    return ErrorCode::MemCantAllocate;
                }
                m_data.raw = new_data;
                m_data_byte_size = new_data_size;
            }

            if (m_weights_mode) {
                size_t new_weights_size = static_cast<size_t>(sample_count) * sizeof(float);
                if (new_weights_size != m_weights_size) {
                    auto new_weights = reinterpret_cast<float*>(std::realloc(m_weights, new_weights_size));
                    if (!new_weights) {
                        return ErrorCode::MemCantAllocate;
                    }
                    m_weights = new_weights;
                    m_weights_size = new_weights_size;
                }
            }
        }

        _updateAccessors();

        return ErrorCode::None;
    }


    /**
     *  @brief Ensures that the given signal is properly allocated and configured.
     *
     *  This method checks whether the provided pointer to a Signal pointer (`signal_ptr`) is valid.
     *  If `*signal_ptr` is `nullptr`, a new Signal instance is allocated with the given configuration.
     *  If allocation fails, an appropriate error code is returned.
     *  If `*signal_ptr` already points to a Signal instance, it is reconfigured with the specified parameters.
     *
     *  @param signal_ptr Pointer to a pointer to a Signal instance. Must not be `nullptr`.
     *  @param channel_count Number of channels to configure.
     *  @param sample_rate Desired sample rate (in Hz).
     *  @param sample_count Number of samples per channel.
     *  @param data_type Data type of the signal samples.
     *  @param weights_mode If `true`, allocates memory for a weights buffer.
     *
     *  @return An ErrorCode indicating success or failure:
     *  - `ErrorCode::None` if the operation succeeded.
     *  - `ErrorCode::NullData` if the `signal` parameter itself is `nullptr`.
     *  - `ErrorCode::ClassInstantiationFailed` if memory allocation for the signal instance failed.
     *  - Any other error code as returned by `Signal::configure()` if reconfiguration fails.
     *
     *  @see Signal::configure()
     */
    ErrorCode Signal::checkConfiguration(
            Signal** signal_ptr,
            int32_t channel_count,
            int32_t sample_rate,
            int64_t sample_count,
            DataType data_type,
            bool weights_mode) noexcept
    {
        if (!signal_ptr) {
            return ErrorCode::NullData;
        }

        if (!(*signal_ptr)) {
            *signal_ptr = new (std::nothrow) Signal(channel_count, sample_rate, sample_count, data_type, weights_mode);
            if (!(*signal_ptr)) {
                return ErrorCode::ClassInstantiationFailed;
            }
            else {
                return ErrorCode::None;
            }
        }
        else {
            return (*signal_ptr)->configure(channel_count, sample_rate, sample_count, data_type, weights_mode);
        }

        return ErrorCode::Unknown;
    }


    /**
     *  @brief Releases all allocated memory held by this Signal instance,
     *         without destroying the object itself. Safe for reuse.
     *
     *  This method frees the internal data and weights buffers,
     *  setting their pointers to nullptr. The Signal object remains valid
     *  and can be reused or reinitialized later.
     */
    void Signal::freeMem() noexcept {
        std::free(m_data.raw);
        m_data.raw = nullptr;

        std::free(m_weights);
        m_weights = nullptr;
    }


    /**
     *  @brief Ensures the internal buffer is large enough to hold the given number
     *         of samples.
     *
     *  This method checks whether the signal's current sample capacity
     *  (`sample_count_`) is sufficient to accommodate `sample_count` samples.
     *  If the requested count exceeds the current capacity, the signal is resized
     *  by calling `configure()` with the same channel count, sample rate, and data
     *  type, but with the new sample count.
     *
     *  @param sample_count The number of samples the buffer should be able to hold.
     *
     *  @retval ErrorCode::None If the current buffer is already large enough or
     *          after a successful resize.
     *  @retval ErrorCode::<other> If resizing fails, as returned by the `configure()`
     *          method.
     *
     *  @note This function does not shrink the buffer—only grows it when needed.
     */
    ErrorCode Signal::growIfNeeded(int64_t sample_count) noexcept {
        if (sample_count > m_sample_count) {
            return configure(m_channel_count, m_sample_rate, sample_count, m_data_type);
        }
        else {
            return ErrorCode::None;
        }
    }


    /**
     *  @brief Creates a copy of a portion of the signal.
     *
     *  This method creates a new `Signal` instance that contains a copy of the
     *  samples starting at the specified `offs` and spanning `len` samples.
     *  If the `offs` and `len` are invalid (e.g., println of bounds), the function
     *  returns `nullptr`.
     *
     *  The new signal will inherit the original signal's channel count, sample
     *  rate, data type, and optionally, the weight mode if `weights_mode` is
     *  enabled.
     *
     *  @param offs The starting index (in samples) from which to begin copying.
     *  @param len The number of samples to copy from the source signal.
     *  @param weights_mode If true, the copied signal is created in "weights mode",
     *                      used for weighted processing.
     *
     *  @return A pointer to the newly allocated `Signal` containing the copied data,
     *          or `nullptr` if allocation fails or the offs/len is invalid.
     *
     *  @note The caller is responsible for deallocating the returned `Signal` object.
     *
     *  @see Signal::copySamples, Signal::clampOffsAndLen
     */
    Signal* Signal::copySignal(int64_t offs, int64_t len, bool weights_mode) noexcept {
        if (clampOffsAndLen(offs, len) < 1) {
            return nullptr;
        }

        // Create a signal with same sample rate, data type
        auto signal = new (std::nothrow) Signal(m_channel_count, m_sample_rate, len, m_data_type, weights_mode);
        if (signal) {
            signal->copySamples(this, len, offs, 0);
        }

        return signal;
    }


    /**
     *  @brief Constructs a new Signal by copying the setup from another Signal.
     *
     *  This constructor initializes a new Signal instance using the configuration
     *  of the provided signal, including channel count, sample rate, data type,
     *  sample count, and weights mode. If the setup is successful and the
     *  original signal has data, it copies the samples as well. If weights mode
     *  is enabled, the weights are cleared after copying.
     *
     *  @param signal Pointer to the Signal to copy the configuration and data from.
     *                If nullptr, the setup error code is set to ErrorCode::NullData.
     */
    Signal* Signal::createSignalWithSameSetting() const noexcept {
        auto signal = new (std::nothrow) Signal(m_channel_count, m_sample_rate, m_sample_count, m_data_type, m_weights_mode);
        return signal;
    }


    [[nodiscard]] Signal* Signal::createSignalFromChannel(int32_t channel, int64_t offs, int64_t len) const noexcept {
        if (!hasChannel(channel)) {
            return nullptr;
        }
        if (len < 0) {
            len = m_sample_count;
        }

        if (clampOffsAndLen(offs, len) < 1) {
            return nullptr;
        }

        auto signal = new (std::nothrow) Signal(1, m_sample_rate, len, m_data_type);
        if (signal) {
            signal->copySamples(this, len, channel, offs, 0, 0);
        }

        return signal;
    }


    /**
     *  @brief Duration in seconds.
     *
     *  @return The duration of the signal in seconds. If the sample rate is not
     *          positive, -1 is returned.
     */
    double Signal::seconds() const noexcept {
        if (m_sample_rate > 0) {
            return static_cast<double>(m_sample_count) / m_sample_rate;
        }
        else {
            return -1.0;
        }
    }


    const void* Signal::dataPtr(int32_t channel, int64_t index) const noexcept {
        return mutDataPtr(channel, index);
    }


    /**
     *  @brief Retrieves a pointer to the sample data.
     *
     *  This function returns a pointer to the data at the specified channel and
     *  index. It performs validation checks to ensure that the data pointer is
     *  valid, the channel and index are within valid ranges, and the index is not
     *  greater than the last sample index.
     *
     *  @param channel The channel index for which to retrieve the data pointer.
     *  @param index The index of the data sample for which to retrieve the data
     *               pointer.
     *  @return A pointer to the data at the specified channel and index. If any of
     *          the validation checks fail, nullptr is returned.
     */
    void* Signal::mutDataPtr(int32_t channel, int64_t index) const noexcept {
        // Basic bounds checking
        if (!m_data.raw ||
            channel < 0 || channel >= m_channel_count ||
            index < 0 || index > m_last_sample_index) {
            return nullptr;
        }

        int64_t frame_offs = static_cast<int64_t>(index) * m_channel_count;
        int64_t sample_offs = frame_offs + channel;
        int64_t byte_offs = sample_offs * m_bytes_per_sample;

        // Optional: Check for overflow here if needed, for ultra-safety
        // if (byte_offs >= max_allowed_size) { return nullptr; }

        // Byte-wise pointer arithmetic using reinterpret_cast
        auto byte_ptr = static_cast<int8_t*>(m_data.raw);
        return static_cast<void*>(byte_ptr + byte_offs);
    }


    SimplifiedSignal* Signal::simplifiedSignalByChannel(int32_t channel) noexcept {
        return m_simplified_signals.elementAtIndex(channel);
    }


    /**
     *  @brief Checks if the signal uses 32-bit floats and a specified channel
     *         exists.
     *
     *  @param channel The channel index to check.
     *  @return `true` if signal uses 32-bit floats and the specified channel exists;
     *          otherwise, `false`.
     */
    bool Signal::canAccessFloatInChannel(int32_t channel) const noexcept {
        return hasChannelAndData(channel) && m_data_type == DataType::Float;
    }


    bool Signal::canAccessFloatInChannelByMask(uint32_t channel_mask) const noexcept {
        if (!hasData() || m_data_type != DataType::Float) {
            return false;
        }

        for (int32_t channel = 0; channel < kMaxChannelCount; channel++) {
            if (channel_mask & (0x1 << channel)) {
                if (!hasChannelAndData(channel)) {
                    return false;
                }
            }
        }

        return true;
    }


    /**
     *  @brief Compares the settings of two Signal objects.
     *
     *  Checks whether the provided Signal object has the same channel count,
     *  sample rate, and data type as this Signal.
     *
     *  @param signal A pointer to the Signal to compare with.
     *  @return `true` if the settings match and `signal` is not `nullptr`, otherwise `false`.
     */
    bool Signal::hasSameSettingAs(const Signal* signal) const noexcept {
        return
                signal &&
                signal->m_channel_count == m_channel_count &&
                signal->m_sample_rate == m_sample_rate &&
                signal->m_data_type == m_data_type;
    }


    /**
     *  @brief Iterates over samples in a specified channel and range, invoking a callback function for each sample.
     *
     *  This method allows external code to process or inspect individual samples by supplying a custom
     *  callback function (`SignalSampleFunc`). The iteration can be customized by specifying an offs,
     *  len, and stride. The callback is provided with the current sample pointer, the index, and a
     *  user-defined reference object (`ref`) for context.
     *
     *  @param func A function pointer (callback) to be called for each sample. Must not be `nullptr`.
     *  @param ref A user-defined pointer passed to the callback (can be `nullptr` if unused).
     *  @param channel The channel index to operate on.
     *  @param offs The starting sample index within the channel.
     *  @param len Number of samples to process. If negative, the entire remaining range is used.
     *  @param stride Step size (in samples) between each callback invocation. Must be >= 1.
     *
     *  @return `ErrorCode::None` on success. Otherwise, returns:
     *  - `ErrorCode::NullPointer` if `func` is `nullptr`.
     *  - `ErrorCode::BadArgs` if `stride < 1`.
     *  - `ErrorCode::NoData` if the signal has no data.
     *  - `ErrorCode::InvalidChannel` if the specified channel does not exist.
     *  - `ErrorCode::RegionOutOfRange` if the specified offs and len are invalid.
     *  - `ErrorCode::UnexpectedRuntimeError` if the data pointer could not be obtained.
     *
     *  @note The sample pointer passed to the callback points to the current sample at the given index.
     *  Use the `sampleStep()` and `bytesPerSample()` methods to understand sample layout in memory.
     */
    ErrorCode Signal::forEachSample(
            const SignalSampleFunc func,
            SignalSampleFuncInfo& info_ref,
            int32_t channel, int64_t offs,
            int64_t len,
            int64_t stride) const
    {
        if (!func) {
            return ErrorCode::NullPointer;
        }

        if (stride < 1) {
            return ErrorCode::BadArgs;
        }

        if (!hasData()) {
            return ErrorCode::NoData;
        }

        if (!hasChannel(channel)) {
            return ErrorCode::InvalidChannel;
        }

        if (len < 0) {
            len = m_sample_count;
        }

        if (clampOffsAndLen(offs, len) < 1) {
            return ErrorCode::RegionOutOfRange;
        }

        info_ref.m_signal = this;
        info_ref.m_sample_ptr.raw = mutDataPtr(channel, offs);
        if (!info_ref.m_sample_ptr.raw) {
            return ErrorCode::UnexpectedRuntimeError;
        }

        auto sample_byte_step = sampleStep() * bytesPerSample();
        for (int64_t i = 0; i < len; i += stride) {
            info_ref.m_sample_index = i;
            func(info_ref);
            info_ref.m_sample_ptr.i8 += sample_byte_step;
            info_ref.m_processed_n++;
        }

        return ErrorCode::None;
    }


    ErrorCode Signal::forEachSampleOfType(
            DataType data_type,
            SignalSampleFunc func,
            SignalSampleFuncInfo& info_ref,
            int64_t offs,
            int64_t len,
            int64_t stride) const
    {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            auto err = forEachSampleOfType(data_type, func, info_ref, channel, offs, len, stride);
            if (err != ErrorCode::None) {
                return err;
            }
        }

        return ErrorCode::None;
    }


    ErrorCode Signal::forEachSampleOfType(DataType data_type, SignalSampleFunc func, SignalSampleFuncInfo& info_ref, int32_t channel, int64_t offs, int64_t len, int64_t stride) const {
        if (data_type != m_data_type) {
            return ErrorCode::UnsupportedDataType;
        }

        return forEachSample(func, info_ref, channel, offs, len, stride);
    }


    template <typename T>
    void Signal::_absMaxFunc(SignalSampleFuncInfo& info) {
        auto v = static_cast<double>(*reinterpret_cast<T*>(info.m_sample_ptr.raw));
        if (v < 0.0) { v = -v; }
        if (v > info.m_value.f64) { info.m_value.f64 = v; }
    }


    double Signal::absMax(int32_t channel, int64_t offs, int64_t len, int64_t stride) const noexcept {
        SignalSampleFuncInfo info{};
        info.m_value.f64 = 0.0;

        switch (m_data_type) {
            case DataType::Int8:
                forEachSampleOfType(m_data_type, _absMaxFunc<int8_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int16:
                forEachSampleOfType(m_data_type, _absMaxFunc<int16_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int32:
                forEachSampleOfType(m_data_type, _absMaxFunc<int32_t>, info, channel, offs, len, stride);
                break;
            case DataType::Float:
                forEachSampleOfType(m_data_type, _absMaxFunc<float>, info, channel, offs, len, stride);
                break;
            case DataType::Double:
                forEachSampleOfType(m_data_type, _absMaxFunc<double>, info, channel, offs, len, stride);
                break;
            default:
                break;
        }

        return info.m_value.f64;
    }

    double Signal::absMax() const noexcept {
        double max = 0.0;

        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            double channel_max = absMax(channel, 0, -1, 1);
            if (channel_max > max) {
                max = channel_max;
            }
        }

        return max;
    }


    template <typename T>
    void Signal::_averageFunc(SignalSampleFuncInfo& info) {
        auto v = static_cast<double>(*reinterpret_cast<T*>(info.m_sample_ptr.raw));
        info.m_value.f64 += v;
    }


    double Signal::average(int32_t channel, int64_t offs, int64_t len, int64_t stride) const noexcept {
        SignalSampleFuncInfo info{};
        info.m_value.f64 = 0.0;

        switch (m_data_type) {
            case DataType::Int8:
                forEachSampleOfType(m_data_type, _averageFunc<int8_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int16:
                forEachSampleOfType(m_data_type, _averageFunc<int16_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int32:
                forEachSampleOfType(m_data_type, _averageFunc<int32_t>, info, channel, offs, len, stride);
                break;
            case DataType::Float:
                forEachSampleOfType(m_data_type, _averageFunc<float>, info, channel, offs, len, stride);
                break;
            case DataType::Double:
                forEachSampleOfType(m_data_type, _averageFunc<double>, info, channel, offs, len, stride);
                break;
            default:
                break;
        }

        return info.m_value.f64 / static_cast<double>(info.m_processed_n);
    }


    template <typename T>
    void Signal::_absAverageFunc(SignalSampleFuncInfo& info) {
        auto v = static_cast<double>(*reinterpret_cast<T*>(info.m_sample_ptr.raw));
        if (v < 0.0) { v = -v; }
        info.m_value.f64 += v;
    }


    double Signal::absAverage(int32_t channel, int64_t offs, int64_t len, int64_t stride) const noexcept {
        SignalSampleFuncInfo info{};
        info.m_value.f64 = 0.0;

        switch (m_data_type) {
            case DataType::Int8:
                forEachSampleOfType(m_data_type, _absAverageFunc<int8_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int16:
                forEachSampleOfType(m_data_type, _absAverageFunc<int16_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int32:
                forEachSampleOfType(m_data_type, _absAverageFunc<int32_t>, info, channel, offs, len, stride);
                break;
            case DataType::Float:
                forEachSampleOfType(m_data_type, _absAverageFunc<float>, info, channel, offs, len, stride);
                break;
            case DataType::Double:
                forEachSampleOfType(m_data_type, _absAverageFunc<double>, info, channel, offs, len, stride);
                break;
            default:
                break;
        }

        return info.m_value.f64 / static_cast<double>(info.m_processed_n);
    }


    template <typename T>
    void Signal::_rmsFunc(SignalSampleFuncInfo& info) {
        auto v = static_cast<double>(*reinterpret_cast<T*>(info.m_sample_ptr.raw));
        info.m_value.f64 += v * v; // Accumulate square
    }


    double Signal::rms(int32_t channel, int64_t offs, int64_t len, int64_t stride) const noexcept {
        SignalSampleFuncInfo info{};
        info.m_value.f64 = 0.0;

        switch (m_data_type) {
            case DataType::Int8:
                forEachSampleOfType(m_data_type, _rmsFunc<int8_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int16:
                forEachSampleOfType(m_data_type, _rmsFunc<int16_t>, info, channel, offs, len, stride);
                break;
            case DataType::Int32:
                forEachSampleOfType(m_data_type, _rmsFunc<int32_t>, info, channel, offs, len, stride);
                break;
            case DataType::Float:
                forEachSampleOfType(m_data_type, _rmsFunc<float>, info, channel, offs, len, stride);
                break;
            case DataType::Double:
                forEachSampleOfType(m_data_type, _rmsFunc<double>, info, channel, offs, len, stride);
                break;
            default:
                return 0.0;
        }

        if (info.m_processed_n < 1) return 0.0;
        return std::sqrt(info.m_value.f64 / static_cast<double>(info.m_processed_n));
    }


    template <typename T>
    void Signal::_scaleFloatFunc(SignalSampleFuncInfo& info) {
        *info.m_sample_ptr.f32 *= info.m_value.f32;
    }


    template <typename T>
    void Signal::_scaleDoubleFunc(SignalSampleFuncInfo& info) {
        *info.m_sample_ptr.f64 *= info.m_value.f64;
    }


    void Signal::scale(float scale_factor) const noexcept {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            scale(channel, 0, -1, scale_factor);
        }
    }


    void Signal::scale(int32_t channel, int64_t offs, int64_t len, float scale_factor) const noexcept {
        SignalSampleFuncInfo info{};

        switch (m_data_type) {
            case DataType::Float:
                info.m_value.f32 = static_cast<float>(scale_factor);
                forEachSampleOfType(m_data_type, _scaleFloatFunc<float>, info, channel, offs, len, 1);
                break;
            case DataType::Double:
                info.m_value.f64 = static_cast<double>(scale_factor);
                forEachSampleOfType(m_data_type, _scaleDoubleFunc<double>, info, channel, offs, len, 1);
                break;
            default:
            break;
        }
    }


    void Signal::normalize(float target_level) noexcept {
        auto max = static_cast<float>(absMax());
        if (Safe::canSafelyDivideBy(max)) {
            scale(1.0f / max * target_level);
        }
    }


    void Signal::normalize(int32_t channel, int64_t offs, int64_t len, float target_level) noexcept {
        if (hasChannelAndData(channel) && clampOffsAndLen(offs, len) > 0) {
            if (m_data_type == DataType::Float) {
                double max = absMax(channel, offs, len, 1);
                if (Safe::canSafelyDivideBy(max)) {
                    scale(channel, offs, len, 1.0 / max * target_level);
                }
            }
        }
    }


    void Signal::derivate() noexcept {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            derivate(channel, 0, -1);
        }
    }


    void Signal::derivate(int32_t channel, int64_t offs, int64_t len) noexcept {
        if (len < 0) {
            len = m_sample_count;
        }

        // TODO: Test!

        clampOffsAndLen(offs, len);

        int64_t start = offs;
        int64_t end = offs + len - 1;

        if (m_data_type == DataType::Float) {
            auto d = reinterpret_cast<float*>(mutDataPtr(channel, start + 1));
            float temp = d[-1];
            for (int64_t i = start + 1; i < end - 1; i++) {
                auto temp2 = *d;
                *d = d[1] - temp;
                temp = temp2;
                d += m_channel_count;
            }
            writeFloat(channel, offs, 0.0f);
            writeFloat(channel, offs + len - 1, 0.0f);
        }
        else if (m_data_type == DataType::Double) {
            auto d = reinterpret_cast<double*>(mutDataPtr(channel, start + 1));
            float temp = d[-1];
            for (int64_t i = start + 1; i < end - 1; i++) {
                auto temp2 = *d;
                *d = d[1] - temp;
                temp = temp2;
                d += m_channel_count;
            }
            writeDouble(channel, offs, 0.0f);
            writeDouble(channel, offs + len - 1, 0.0f);
        }
    }


    /**
     *  @brief Calculates the number of samples needed for a specific duration.
     *
     *  This function calculates the number of samples required to achieve a given duration in milliseconds.
     *
     *  @param millisecond The duration in milliseconds.
     *  @return The number of samples needed to achieve the specified duration.
     */
    int64_t Signal::samplesNeededForMilliseconds(int64_t milliseconds) const noexcept {
        return milliseconds < 0 ? 0 : static_cast<int64_t>(m_sample_rate) * milliseconds / 1000;
    }


    /**
     *  @brief Calculates the number of samples needed for a specific duration.
     *
     *  This function calculates the number of samples required to achieve a given duration in seconds.
     *
     *  @param seconds The duration in seconds.
     *  @return The number of samples needed to achieve the specified duration.
     */
    int64_t Signal::samplesNeededForSeconds(float seconds) const noexcept {
        return seconds <= 0.0f ? 0 : static_cast<int64_t>(std::round(static_cast<double>(m_sample_rate) * seconds));
    }


    /**
     *  @brief Calculates the number of samples needed for a music note.
     *
     *  This function calculates the number of samples required to represent a music note based on the beats per minute (bpm) and note length.
     *
     *  @param bpm The beats per minute.
     *  @param len The length of the note. A value of 1 represents a whole note, 0.5 represents a half note, and so on.
     *  @return The number of samples needed to represent the given music note.
     */
    int64_t Signal::samplesNeededForNote(float bpm, float len) const noexcept {
        if (bpm < std::numeric_limits<float>::epsilon()) {
            return 0;
        }
        else {
            return samplesNeededForSeconds(60.0f / bpm * len);
        }
    }


    /**
     *  @brief Clamps the offs and length to define a valid range of samples.
     *
     *  This function ensures that the offs and length parameters define a valid range of samples. If necessary, it adjusts the values to fit within the valid range.
     *
     *  @param[in,out] offs The index of the first sample in the range. It may be modified to fit within the valid range.
     *  @param[in,out] len The number of samples in the range. It may be adjusted to fit within the valid range.
     *  @return The number of samples in the clamped range after any necessary adjustments.
     */
    int64_t Signal::clampOffsAndLen(int64_t& offs, int64_t& len) const noexcept {
        int64_t end_index = offs + len - 1;
        int64_t n = clampStartEndIndex(offs, end_index);
        if (n < 1) {
            n = 0;
        }
        len = n;
        return n;
    }


    /**
     *  @brief Clamps the start and end indices to define a valid range of samples.
     *
     *  This function ensures that the start and end indices define a valid range of samples. If necessary, it adjusts the values to fit within the valid range.
     *
     *  @param[in,out] start_index The start index of the range. It may be modified to fit within the valid range.
     *  @param[in,out] end_index The end index of the range. It may be adjusted to fit within the valid range.
     *  @return The number of samples in the clamped range after any necessary adjustments.
     */
    int64_t Signal::clampStartEndIndex(int64_t& start_index, int64_t& end_index) const noexcept {
        int64_t n = 0;
        if (start_index <= end_index && start_index <= m_last_sample_index && end_index >= 0) {
            start_index = std::clamp<int64_t>(start_index, 0, m_last_sample_index);
            end_index = std::clamp<int64_t>(end_index, start_index, m_last_sample_index);
            n = end_index - start_index + 1;
        }

        return n;
    }


    /**
     *  @brief Converts an index to a valid ring buffer index.
     *
     *  This function takes an index and converts it to ensure that it always points to valid data within the audio buffer.
     *
     *  @param index The index to be converted.
     *  @return The converted index, which is always greater than or equal to 0 and less than the number of samples in the audio buffer.
     */
    int64_t Signal::ringBufferIndex(int64_t index) const noexcept {
        return index >= 0 ? index % m_sample_count : (m_sample_count - 1) + ((index + 1) % m_sample_count);
    }


    int64_t Signal::silentFrom(float threshold) noexcept {
        int64_t index = lastSampleIndex();

        if (m_data_type == DataType::Float) {
            int64_t sample_step = sampleStep();

            for (int32_t channel_index = 0; channel_index < m_channel_count; channel_index++) {
                auto s = reinterpret_cast<const float*>(dataPtr(channel_index, lastSampleIndex()));

                for (int64_t i = lastSampleIndex(); i > 0; i--) {
                    if (*s > threshold) {
                        if (i < index) {
                            index = i;
                        }
                        break;
                    }

                    s -= sample_step;
                }
            }
        }

        // TODO: Implement other data types!

        return index;
    }


    int64_t Signal::silentTo(float threshold) noexcept {
        int64_t index = 0;

        if (m_data_type == DataType::Float) {
            int64_t sample_step = sampleStep();

            for (int32_t channel_index = 0; channel_index < m_channel_count; channel_index++) {
                auto s = reinterpret_cast<const float*>(dataPtr(channel_index));

                for (int64_t i = lastSampleIndex(); i > 0; i--) {
                    if (*s > threshold) {
                        if (i > index) {
                            index = i;
                        }
                        break;
                    }

                    s += sample_step;
                }
            }
        }

        // TODO: Implement other data types!

        return index;
    }



    int8_t Signal::readInt8(int32_t channel, int64_t index) const noexcept {
        if (hasSampleAtChannel(channel, index)) {
            return _m_reader_i8(m_data, index * m_channel_count + channel);
        }
        return 0;
    }

    int16_t Signal::readInt16(int32_t channel, int64_t index) const noexcept {
        if (hasSampleAtChannel(channel, index)) {
            return _m_reader_i16(m_data, index * m_channel_count + channel);
        }
        return 0;
    }

    int32_t Signal::readInt32(int32_t channel, int64_t index) const noexcept {
        if (hasSampleAtChannel(channel, index)) {
            return _m_reader_i32(m_data, index * m_channel_count + channel);
        }
        return 0;
    }

    float Signal::readFloat(int32_t channel, int64_t index) const noexcept {
        if (hasSampleAtChannel(channel, index)) {
            return _m_reader_f32(m_data, index * m_channel_count + channel);
        }
        return 0.0f;
    }

    double Signal::readDouble(int32_t channel, int64_t index) const noexcept {
        if (hasSampleAtChannel(channel, index)) {
            return _m_reader_f64(m_data, index * m_channel_count + channel);
        }
        return 0.0f;
    }

    void Signal::writeInt8(int32_t channel, int64_t index, int8_t value) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            _m_writer_i8(m_data, index * m_channel_count + channel, value);
        }
    }

    void Signal::writeInt16(int32_t channel, int64_t index, int16_t value) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            _m_writer_i16(m_data, index * m_channel_count + channel, value);
        }
    }

    void Signal::writeInt32(int32_t channel, int64_t index, int32_t value) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            _m_writer_i32(m_data, index * m_channel_count + channel, value);
        }
    }

    void Signal::writeFloat(int32_t channel, int64_t index, float value) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            _m_writer_f32(m_data, index * m_channel_count + channel, value);
        }
    }

    void Signal::writeDouble(int32_t channel, int64_t index, double value) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            _m_writer_f64(m_data, index * m_channel_count + channel, value);
        }
    }


    /**
     *  @brief Returns a linearly interpolated sample value at a sub-sample position.
     *
     *  This method performs linear interpolation between two adjacent samples
     *  (`si0` and `si1`) at the specified channel using the fractional position
     *  from the given `HiResValue`. It allows reading smooth values between discrete
     *  samples, which is useful for resampling, pitch shifting, or other DSP tasks.
     *
     *  If a sample is not available at either `si0` or `si1`, that sample is treated as zero.
     *
     *  @param channel The channel index to read from.
     *  @param sample_pos A high-resolution position structure containing:
     *    - `i_`: the integer sample index
     *    - `f_`: the fractional part between [0.0, 1.0)
     *
     *  @return The interpolated floating-point sample value.
     *
     *  @note This method is `noexcept` and safe against println-of-bounds access.
     *        Missing samples are treated as 0.0.
     */
    float Signal::readFloatLerp(int32_t channel, const HiResValue& sample_pos) const noexcept {
        if (!m_data.raw || channel < 0 || channel >= m_channel_count) {
            return 0.0f;
        }

        const int64_t si0 = sample_pos.m_i;
        const int64_t si1 = si0 + 1;

        bool has_v0 = si0 >= 0 && si0 < m_last_sample_index;
        bool has_v1 = si1 >= 0 && si1 < m_last_sample_index;
        if (!has_v0 && !has_v1) {
            return 0.0f;
        }

        int64_t data_index = si0 * m_channel_count + channel;
        float v0 = has_v0 ? _m_reader_f32(m_data, data_index) : 0.0f;
        if (!has_v1) {
            return v0;
        }
        float v1 = has_v1 ? _m_reader_f32(m_data, data_index + channel) : 0.0f;
        if (!has_v0) {
            return v1;
        }

        const auto f1 = static_cast<float>(sample_pos.m_f);
        const auto f0 = 1.0f - f1;

        return v0 * f0 + v1 * f1;
    }


    float Signal::ringBufferSample(int32_t channel, int64_t index) const noexcept {
        return readFloat(channel, ringBufferIndex(index));
    }


    void Signal::addSample(int32_t channel, int64_t index, float value) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            int64_t data_index = index * m_channel_count + channel;
            float v = _m_reader_f32(m_data, data_index);
            _m_writer_f32(m_data, data_index, v + value);
        }
    }


    void Signal::addSampleLerp(int32_t channel, const HiResValue& sample_pos, float value) noexcept {
        if (!m_data.raw || channel < 0 || channel >= m_channel_count) {
            return;
        }

        const int64_t si0 = sample_pos.m_i;
        const int64_t si1 = si0 + 1;

        bool has_v0 = si0 >= 0 && si0 < m_last_sample_index;
        bool has_v1 = si1 >= 0 && si1 < m_last_sample_index;
        if (!has_v0 && !has_v1) {
            return;
        }

        int64_t di0 = si0 * m_channel_count + channel;
        float v0 = has_v0 ? _m_reader_f32(m_data, di0) : 0.0f;
        if (!has_v1) {
            _m_writer_f32(m_data, di0, v0 + value);
            return;
        }
        int64_t di1 = di0 + m_channel_count;
        float v1 = has_v1 ? _m_reader_f32(m_data, di1) : 0.0f;
        if (!has_v0) {
            _m_writer_f32(m_data, di1, v1 + value);
            return;
        }

        const auto f1 = static_cast<float>(sample_pos.m_f);
        const auto f0 = 1.0f - f1;

        _m_writer_f32(m_data, di0, v0 + + value * f0);
        _m_writer_f32(m_data, di1, v1 + + value * f1);
    }


    void Signal::scaleSample(int32_t channel, int64_t index, float scale_factor) noexcept {
        if (hasSampleAtChannel(channel, index)) {
            int64_t data_index = index * m_channel_count + channel;
            float v = _m_reader_f32(m_data, data_index);
            _m_writer_f32(m_data, data_index, v * scale_factor);
        }
    }


    void Signal::setRingBufferSample(int32_t channel, int64_t index, float value) noexcept {
        writeFloat(channel, ringBufferIndex(index), value);
    }


    void Signal::addRingBufferSample(int32_t channel, int64_t index, float value) noexcept {
        addSample(channel, ringBufferIndex(index), value);
    }


    float Signal::ringBufferSampleInterpolated(int32_t channel, const HiResValue& sample_pos) const noexcept {
        int64_t si0 = sample_pos.m_i % m_sample_count;
        if (si0 < 0) {
            si0 += m_sample_count;
        }

        int64_t si1 = si0 + 1;
        if (si1 >= m_sample_count) {
            si1 -= m_sample_count;
        }

        const auto f1 = static_cast<float>(sample_pos.m_f);
        const auto f0 = 1.0f - f1;
        const auto v0 = readFloat(channel, si0);
        const auto v1 = readFloat(channel, si1);

        return v0 * f0 + v1 * f1;
    }


    void Signal::clearWeights() noexcept {
        clearWeights(m_sample_count);
    }


    void Signal::clearWeights(int64_t n) noexcept {
        if (m_weights_mode && m_weights) {
            Type::clearArray<float>(m_weights, std::min(n, m_sample_count));
        }
        m_weighted_start = std::numeric_limits<int64_t>::max();
        m_weighted_end = std::numeric_limits<int64_t>::min();
    }


    bool Signal::addWeightedSample(
            int32_t channel,
            const HiResValue& sample_pos,
            float value
    ) noexcept
    {
        if (m_data_type != DataType::Float || !m_weights || !m_weights_mode) {
            return false;
        }

        if (channel < 0 || channel >= m_channel_count) {
            return false;
        }

        const auto index0 = sample_pos.m_i;
        const auto index1 = index0 + 1;

        const float f1 = sample_pos.posf();
        const float f0 = 1.0f - f1;

        bool status = false;

        auto updateSample = [&](int64_t idx, float weight) {
            if (idx >= 0 && idx < m_sample_count) {
                if (idx < m_weighted_start) m_weighted_start = idx;
                if (idx > m_weighted_end) m_weighted_end = idx;

                auto d = static_cast<float*>(mutDataPtr(channel, idx));
                *d += weight * value;
                m_weights[idx] += weight;

                return true;
            }
            return false;
        };

        status |= updateSample(index0, f0);
        status |= updateSample(index1, f1);

        return status;
    }


    bool Signal::finishWeightedSamples(int32_t channel) noexcept {
        if (m_weighted_start >= m_sample_count || m_weighted_start > m_weighted_end) {
            return false;
        }

        if (!canAccessFloatInChannel(channel)) {
            return false;
        }

        int64_t d_step = sampleStep();
        // Inclusive end index: add 1 to include m_weighted_end
        int64_t end = std::min(m_weighted_end, m_last_sample_index) + 1;

        // Normalize weighted samples
        auto d = static_cast<float*>(mutDataPtr(channel, m_weighted_start));
        for (int64_t i = m_weighted_start; i < end; i++) {
            if (m_weights[i] > std::numeric_limits<float>::epsilon()) {
                *d /= m_weights[i];
            } else {
                *d = std::numeric_limits<float>::max();  // Mark missing values
            }
            d += d_step;
        }

        // Fill missing values by linear interpolation
        int64_t missing = 0;
        float prev_value = 0.0f;
        float start_value = 0.0f;
        int64_t missing_index = 0;

        d = static_cast<float*>(mutDataPtr(channel, m_weighted_start));
        for (int64_t i = m_weighted_start; i < end; i++) {
            float v = *d;

            if (v == std::numeric_limits<float>::max()) {
                // Start of missing value sequence
                if (missing == 0) {
                    start_value = prev_value;
                    missing_index = i;
                }
                missing++;
            }
            else if (missing > 0) {
                // End of missing sequence: interpolate
                float value_delta = v - start_value;
                float value_step = value_delta / (missing + 1);
                float missing_value = start_value;

                auto d2 = static_cast<float*>(mutDataPtr(channel, missing_index));
                for (int64_t j = 0; j < missing; j++) {
                    missing_value += value_step;
                    *d2 = missing_value;
                    d2 += d_step;
                }
                missing = 0;
            }

            prev_value = v;
            d += d_step;
        }

        // Optional: handle trailing missing samples here if needed

        return true;
    }


    int64_t Signal::copyAll(
            const Signal* src,
            int32_t src_channel,
            int64_t dst_offs,
            uint32_t dst_channel_mask
    ) noexcept
    {
        if (!src) {
            return 0;
        }

        if (!src->hasChannelAndData(src_channel)) {
            return 0;
        }

        int64_t n = 0;  // Number of copied samples
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            if (dst_channel_mask & (0x1 << channel)) {
                n += copySamples(src, src->m_sample_count, src_channel, 0, channel, dst_offs);
            }
        }

        return n;
    }


    int64_t Signal::copySamples(const Signal* src) noexcept {
        return src != nullptr ? copySamples(src, src->sampleCount(), 0, 0) : 0;
    }


    int64_t Signal::copySamples(
            const Signal* src,
            int64_t len,
            int64_t src_offs,
            int64_t dst_offs
    ) noexcept
    {
        int64_t sn = 0; // Number of copied samples
        int64_t cn = 0; // Number of copied channels

        Signal* dst_signal = this;
        if (src->m_channel_count == dst_signal->m_channel_count) {
            for (int32_t channel = 0; channel < src->m_channel_count; cn++, channel++) {
                sn += copySamples(src, len, channel, src_offs, channel, dst_offs);
            }
        }
        else if (src->m_channel_count > 1 && dst_signal->isMono() && dst_signal->isFloatType())    {
            dst_signal->clear(dst_offs, dst_offs + len - 1);
            float channel_amount = 1.0f / static_cast<float>(src->m_channel_count);
            for (int32_t channel = 0; channel < src->m_channel_count; cn++, channel++) {
                sn += dst_signal->combineSamples(src, len, channel, src_offs, 0, dst_offs, CombineMode::Add, channel_amount);
            }
        }
        else if (src->isMono() && dst_signal->m_channel_count > 1) {
            for (int32_t channel = 0; channel < dst_signal->m_channel_count; cn++, channel++) {
                sn += copySamples(src, len, 0, src_offs, channel, dst_offs);
            }
        }
        else {
            return 0;
        }

        return cn > 0 ? sn / cn : 0;
    }


    int64_t Signal::copySamples(
            const Signal* src,
            int64_t len,
            int32_t src_channel,
            int64_t src_offs,
            int32_t dst_channel,
            int64_t dst_offs
    ) noexcept
    {
        if (!src || len < 1) {
            return 0;
        }

        if (!src->hasChannelAndData(src_channel) || !hasChannelAndData(dst_channel)) {
            return 0;
        }

        if (src == this && src_channel == dst_channel) {
            return 0;  // Source and destination are the same, this is not possible!
        }

        // Check boundaries and get indices
        int64_t copy_src_offs, copy_dst_offs;
        auto n = Type::validCopyRegion<int64_t>(len, src->m_sample_count, src_offs, m_sample_count, dst_offs, copy_src_offs, copy_dst_offs);
        if (n < 1) {
            return 0;
        }

        int64_t sn = n;  // Number of copied samples
        int64_t s_step = src->sampleStep();
        int64_t d_step = sampleStep();

        if (src->isInt8Type()) {
            auto s = reinterpret_cast<const int8_t*>(src->dataPtr(src_channel, copy_src_offs));

            if (isInt8Type()) {
                auto d = reinterpret_cast<int8_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = *s;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt16Type()) {
                auto d = reinterpret_cast<int16_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    auto v = static_cast<int16_t>(*s);
                    *d = static_cast<int16_t>(v << 8);
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt32Type()) {
                auto d = reinterpret_cast<int32_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int32_t>(*s) << 24;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isFloatType()) {
                auto d = reinterpret_cast<float*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<float>(*s) / std::numeric_limits<int8_t>::max();
                    d += d_step;
                    s += s_step;
                }
            }
        }
        else if (src->isInt16Type()) {
            auto s = reinterpret_cast<const int16_t*>(src->dataPtr(src_channel, copy_src_offs));

            if (isInt8Type())  {
                auto d = reinterpret_cast<int8_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int8_t>(*s >> 8);
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt16Type()) {
                auto d = reinterpret_cast<int16_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = *s;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt32Type()) {
                auto d = reinterpret_cast<int32_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int8_t>(*s) << 16;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isFloatType()) {
                auto d = reinterpret_cast<float*>(mutDataPtr(dst_channel, copy_dst_offs));
                float scale = 1.0f / std::numeric_limits<int16_t>::max();
                while (n--) {
                    *d = static_cast<float>(*s) * scale;
                    d += d_step;
                    s += s_step;
                }
            }
        }
        else if (src->isInt32Type()) {
            auto s = reinterpret_cast<const int32_t*>(src->dataPtr(src_channel, copy_src_offs));

            if (isInt8Type()) {
                auto d = reinterpret_cast<int8_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int8_t>(*s >> 24);
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt16Type()) {
                auto d = reinterpret_cast<int16_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int16_t>(*s >> 16);
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt32Type()) {
                auto d = reinterpret_cast<int32_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = *s;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isFloatType()) {
                auto d = reinterpret_cast<float*>(mutDataPtr(dst_channel, copy_dst_offs));
                float scale = 1.0f / static_cast<float>(std::numeric_limits<int32_t>::max());
                while (n--) {
                    *d = static_cast<float>(*s) * scale;
                    d += d_step;
                    s += s_step;
                }
            }
        }
        else if (src->isFloatType()) {
            auto s = reinterpret_cast<const float*>(src->dataPtr(src_channel, copy_src_offs));

            if (isInt8Type()) {
                auto d = reinterpret_cast<int8_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int8_t>(*s * std::numeric_limits<int8_t>::max());
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt16Type()) {
                auto d = reinterpret_cast<int16_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = static_cast<int16_t>(*s * std::numeric_limits<int16_t>::max());
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isInt32Type()) {
                auto d = reinterpret_cast<int32_t*>(mutDataPtr(dst_channel, copy_dst_offs));
                auto scale = static_cast<float>(std::numeric_limits<int32_t>::max());
                while (n--) {
                    *d = static_cast<int32_t>(*s * scale);
                    d += d_step;
                    s += s_step;
                }
            }
            else if (isFloatType()) {
                auto d = reinterpret_cast<float*>(mutDataPtr(dst_channel, copy_dst_offs));
                while (n--) {
                    *d = *s;
                    d += d_step;
                    s += s_step;
                }
            }
        }

        return sn;
    }


    int64_t Signal::copyChannel(int32_t src_channel, int32_t dst_channel) noexcept {
        if (src_channel != dst_channel) {
            return copySamples(this, m_sample_count, src_channel, 0, dst_channel, 0);
        }
        else {
            return 0;
        }
    }


    int64_t Signal::copyChannel(
            int32_t src_channel,
            int32_t dst_channel,
            int64_t offs,
            int64_t len
    ) noexcept
    {
        if (src_channel != dst_channel) {
            return copySamples(this, len, src_channel, offs, dst_channel, offs);
        }
        else {
            return 0;
        }
    }


    /**
     *  @brief Read `len` samples starting from `offs` into `out_samples`.
     *         Pads with zeros if the requested range lies partially or completely
     *         outside the signal.
     *
     *  @param channel Channel index.
     *  @param offs Starting sample index (can be < 0 or > signal length).
     *  @param len Number of samples to read.
     *  @param out_samples Output buffer (must have space for `len` floats).
     *  @return Number of *valid* samples actually read from the signal.
     */
    int64_t Signal::readSamplesAsFloatWithZeroPadding(
            int32_t channel,
            int64_t offs,
            int64_t len,
            float* out_samples
    ) const noexcept
    {
        if (!out_samples || !hasChannelAndData(channel)) {
            return 0;
        }

        int64_t s_index, d_index;
        auto n = Type::validCopyRegion<int64_t>(len, m_sample_count, offs, len, 0, s_index, d_index);

        if (n > 0) {
            int64_t s_end_index = s_index + n;
            if (d_index > 0) {
                Type::clearArray<float>(out_samples, d_index);
            }

            float* d = &out_samples[d_index];
            int64_t s_step = sampleStep();
            int64_t i = n;

            if (m_data_type == DataType::Float) {
                auto s = reinterpret_cast<const float*>(dataPtr(channel, s_index));
                while (i--) {
                    *d++ = *s;
                    s += s_step;
                }
            }
            else if (m_data_type == DataType::Int8) {
                auto s = reinterpret_cast<const int8_t*>(dataPtr(channel, s_index));
                while (i--) {
                    *d++ = static_cast<float>(*s) / std::numeric_limits<int8_t>::max();
                    s += s_step;
                }
            }
            else if (m_data_type == DataType::Int16) {
                auto s = reinterpret_cast<const int16_t*>(dataPtr(channel, s_index));
                while (i--) {
                    *d++ = static_cast<float>(*s) / std::numeric_limits<int16_t>::max();
                    s += s_step;
                }
            }
            else if (m_data_type == DataType::Int32) {
                auto s = reinterpret_cast<const int32_t*>(dataPtr(channel, s_index));
                float scale = 1.0f / static_cast<float>(std::numeric_limits<int32_t>::max());
                while (i--) {
                    *d++ = static_cast<float>(*s) * scale;
                    s += s_step;
                }
            }

            // Tail zero fill
            int64_t d_end_index = d_index + n;
            int64_t tail_n = std::max<int64_t>(0, len - d_end_index);
            if (d_index + n > 0) {
                int64_t tail_offs = s_index - offs;
                Type::clearArray<float>(out_samples + tail_offs, tail_n);
            }
        }

        return n;
    }


    int64_t Signal::readSamples(
            int32_t channel,
            int64_t offs,
            int64_t len,
            DataType data_type,
            void* out_samples
    ) noexcept
    {
        if (!out_samples || !hasChannelAndData(channel)) {
            return -1;
        }

        int64_t s_offs, d_offs;
        auto n = Type::validCopyRegion<int64_t>(len, m_sample_count, offs, len, 0, s_offs, d_offs);


        int64_t valid_start_index = offs;
        int64_t valid_n = len;
        if (clampOffsAndLen(valid_start_index, valid_n) < 1) {
            return 0;
        }

        int64_t valid_end_index = valid_start_index + valid_n;

        int64_t s_step = sampleStep();

        if (m_data_type == DataType::Float) {
            auto s =  reinterpret_cast<const float*>(dataPtr(channel, valid_start_index));
            int64_t i = valid_n;

            if (data_type == DataType::Int8) {
                auto d = reinterpret_cast<int8_t*>(out_samples);
                while (i--) {
                    *d++ = static_cast<int8_t>(*s * 0x7F);
                    s += s_step;
                }
            }
            else if (data_type == DataType::Int16) {
                auto d = reinterpret_cast<int16_t*>(out_samples);
                while (i--) {
                    *d++ = static_cast<int16_t>(*s * 0x7FFF);
                    s += s_step;
                }
            }
            else if (data_type == DataType::Int32) {
                auto d = reinterpret_cast<int32_t*>(out_samples);
                auto scale = static_cast<float>(std::numeric_limits<int32_t>::max());
                while (i--) {
                    *d++ = static_cast<int32_t>(*s * scale);
                    s += s_step;
                }
            }
            else if (data_type == DataType::Float) {
                auto d = reinterpret_cast<float*>(out_samples);
                while (i--) {
                    *d++ = *s;
                    s += s_step;
                }
            }
            else {
                return -2;
            }
        }

        return valid_n;
    }


    int64_t Signal::writeSamples(
            int32_t channel,
            int64_t offs,
            int64_t len,
            const float* samples,
            CombineMode combine_mode
    ) noexcept
    {
        if (!samples || !hasChannelAndData(channel) || !isFloatType()) {
            return 0;
        }

        int64_t valid_start_index = offs;
        int64_t valid_n = len;
        if (clampOffsAndLen(valid_start_index, valid_n) < 1) {
            return 0;
        }
        int64_t valid_end_index = valid_start_index + valid_n;

        int64_t samples_offs = valid_start_index - offs;
        auto s = &samples[samples_offs];
        int64_t d_step = sampleStep();
        int64_t i = valid_n;

        if (m_data_type == DataType::Float) {
            auto d = reinterpret_cast<float*>(mutDataPtr(channel, valid_start_index));
            if (combine_mode == CombineMode::Replace) {
                while (i--) {
                    *d = *s++;
                    d += d_step;
                }
            }
            else if (combine_mode == CombineMode::Add) {
                while (i--) {
                    *d += *s++;
                    d += d_step;
                }
            }
            else if (combine_mode == CombineMode::Subtract) {
                while (i--) {
                    *d -= *s++;
                    d += d_step;
                }
            }
            else if (combine_mode == CombineMode::Multiply) {
                while (i--) {
                    *d *= *s++;
                    d += d_step;
                }
            }
        }

        return valid_n;
    }


    int64_t Signal::combineSamples(
            const Signal* src,
            int64_t len,
            int32_t src_channel,
            int64_t src_offs,
            int32_t dst_channel,
            int64_t dst_offs,
            CombineMode combine_mode,
            float amount
    ) noexcept
    {
        if (!src || len < 1) {
            return 0;
        }

        if (!isFloatType()) {
            return 0;
        }

        if (!src->hasChannelAndData(src_channel) || !hasChannelAndData(dst_channel)) {
            return 0;
        }

        // Check boundaries and get offs and number of samples to copy
        int64_t copy_src_offs, copy_dst_offs;
        auto n = Type::validCopyRegion<int64_t>(len, src->m_sample_count, src_offs, m_sample_count, dst_offs, copy_src_offs, copy_dst_offs);

        if (n < 1) {
            return 0;
        }

        if ((copy_src_offs + n) > src->m_sample_count || (copy_dst_offs + n) > m_sample_count) {
            return 0;
        }

        auto d = reinterpret_cast<float*>(mutDataPtr(dst_channel, copy_dst_offs));
        int64_t d_step = sampleStep();
        int64_t i = n;  // Counter for while loop

        if (src->isInt8Type()) {
            auto s = reinterpret_cast<int8_t*>(src->mutDataPtr(src_channel, copy_src_offs));
            int64_t s_step = src->sampleStep();
            amount = amount / std::numeric_limits<int8_t>::max();

            if (combine_mode == CombineMode::Replace) {
                while (i--) {
                    *d = static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Add) {
                while (i--) {
                    *d += static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Subtract) {
                while (i--) {
                    *d -= static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Multiply) {
                while (i--) {
                    *d *= static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
        }
        else if (src->isInt16Type()) {
            auto s = reinterpret_cast<int16_t*>(src->mutDataPtr(src_channel, copy_src_offs));
            int64_t s_step = src->sampleStep();
            amount = amount / std::numeric_limits<int16_t>::max();

            if (combine_mode == CombineMode::Replace) {
                while (i--) {
                    *d = static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Add) {
                while (i--) {
                    *d += static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Subtract) {
                while (i--) {
                    *d -= static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Multiply) {
                while (i--) {
                    *d *= static_cast<float>(*s) * amount;
                    d += d_step;
                    s += s_step;
                }
            }
        }
        else if (src->isInt32Type()) {
            auto s = reinterpret_cast<int32_t*>(src->mutDataPtr(src_channel, copy_src_offs));
            int64_t s_step = src->sampleStep();
            auto scale = amount / static_cast<float>(std::numeric_limits<int32_t>::max());

            if (combine_mode == CombineMode::Replace) {
                while (i--) {
                    *d = static_cast<float>(*s) * scale;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Add) {
                while (i--) {
                    *d += static_cast<float>(*s) * scale;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Subtract) {
                while (i--) {
                    *d -= static_cast<float>(*s) * scale;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Multiply) {
                while (i--) {
                    *d *= static_cast<float>(*s) * scale;
                    d += d_step;
                    s += s_step;
                }
            }
        }
        else if (src->isFloatType()) {
            auto s = reinterpret_cast<float*>(src->mutDataPtr(src_channel, copy_src_offs));
            int64_t s_step = src->sampleStep();

            if (combine_mode == CombineMode::Replace) {
                while (i--) {
                    *d = *s * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Add) {
                while (i--) {
                    *d += *s * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Subtract) {
                while (i--) {
                    *d -= *s * amount;
                    d += d_step;
                    s += s_step;
                }
            }
            else if (combine_mode == CombineMode::Multiply) {
                while (i--) {
                    *d *= *s * amount;
                    d += d_step;
                    s += s_step;
                }
            }
        }

        return n;
    }


    int64_t Signal::combineSamples(
            const Signal* src,
            int64_t len,
            int64_t src_offs,
            int64_t dst_offs,
            CombineMode combine_mode,
            float amount
    ) noexcept
    {
        int64_t n = 0;

        if (!src) {
            return 0;
        }

        if (src->m_channel_count == 1) {
            // Source signal has one channel
            // Combine it with all channels in destination
            for (int32_t channel = 0; channel < m_channel_count; channel++) {
                n += combineSamples(src, len, 0, src_offs, channel, dst_offs, combine_mode, amount);
                if (channel == 0 && n < 1) {
                    return 0;
                }
            }
        }
        else {
            int32_t channel_n = std::min(src->m_channel_count, m_channel_count);
            for (int32_t channel = 0; channel < channel_n; channel++) {
                n += combineSamples(src, len, channel, src_offs, channel, dst_offs, combine_mode, amount);
                if (channel == 0 && n < 1) {
                    return 0;
                }
            }
        }

        return n;
    }


// TODO: location instead of audioPos!
    int64_t Signal::mixByAudioPos(const Signal* src, int64_t len, int64_t src_offs, int64_t dst_offs, const Vec3d& audio_pos) noexcept {
        if (!src) {
            return 0;
        }

        // Check boundaries
        if (!Type::isValidCopyRegion<int64_t>(len, src->m_sample_count, src_offs, m_sample_count, dst_offs)) {
            return 0;
        }

        // Destination (this) must be 1, 2 or 4 channel
        #pragma message("Signal::mixByAudioPos() must be implemented")
        /*

        GrAudioLocationSystem aps;
        switch (mChannelCount) {
            case 1: aps.setMono(); break;
            case 2: aps.setStereo(); break;
            case 4: aps.setQuadra(); break;
            default:
                return 0;
        }

         aps.setAudioPos(audioPos);


        if (src->isMono()) {

            // Destination (this) must be 1, 2 or 4 channel

            int64_t n = 0;
            for (int32_t channel = 0; channel < mChannelCount; channel++) {

                float level = mChannelCount == 1 ? 1 : aps.getLevel(channel);
                int32_t srcChannel = 0;  // Use allways channel 0 from source
                printf("mixByAudioPos level: %f\n", level);
                level = 1.0f;

                if (Safe::canSafelyDivideBy(level)) {
                    // TODO: ???
                    // int32_t effectmeDesOffset = desOffset + (int32_t)GrAudio::interauralSampleDelay(mSampleRate, level);
                    n = combineSamples(src, len, srcChannel, srcOffset, channel, desOffset, CombineMode::Add, level);
                }
            }

            return n;
        }

         */

        return 0;
    }


    void Signal::clear() noexcept {
        if (hasData()) {
            memset(m_data.raw, 0, dataSize());
        }
    }


    void Signal::clear(int64_t len) noexcept {
        if (hasData()) {
            if (len > m_sample_count) {
                len = m_sample_count;
            }
            memset(m_data.raw, 0, len * bytesPerSample() * channelCount());
        }
    }


    void Signal::clear(int64_t offs, int64_t len) noexcept {
        if (hasData()) {
            if (offs < 0) {
                len += offs;
                offs = 0;
            }
            if (len > 0) {
                if (offs + len > m_sample_count) {
                    len = m_sample_count - offs;
                }
                size_t size = len * bytesPerSample() * channelCount();
                memset(m_data.raw, 0, size);
            }
        }
    }


    void Signal::clearChannel(int32_t channel) noexcept {
        clearChannel(channel, 0, m_sample_count);
    }


    void Signal::clearChannel(int32_t channel, int64_t offs, int64_t len) noexcept {
        if (!hasChannelAndData(channel)) {
            return;
        }

        if (clampOffsAndLen(offs, len) < 1) {
            return;
        }

        int64_t d_step = sampleStep();

        if (isInt8Type()) {
            Type::fillStridedArray<int8_t>(
                reinterpret_cast<int8_t*>(mutDataPtr(channel, offs)),
                0, d_step, len, m_sample_count, 0);
        }
        else if (isInt16Type()) {
            Type::fillStridedArray<int16_t>(
                    reinterpret_cast<int16_t*>(mutDataPtr(channel, offs)),
                    0, d_step, len, m_sample_count, 0);
        }
        else if (isInt32Type()) {
            Type::fillStridedArray<int32_t>(
                    reinterpret_cast<int32_t*>(mutDataPtr(channel, offs)),
                    0, d_step, len, m_sample_count, 0);
        }
        else if (isFloatType()) {
            Type::fillStridedArray<float>(
                    reinterpret_cast<float*>(mutDataPtr(channel, offs)),
                    0, d_step, len, m_sample_count, 0);
        }
    }


    void Signal::clearMaskedChannels(uint32_t channel_mask, int64_t offs, int64_t len) noexcept {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            if (channel_mask & (0x1U << channel)) {
                clearChannel(channel, offs, len);
            }
        }
    }


    void Signal::clearAll() noexcept {
        clear();
        clearWeights();
    }


    void Signal::centerPowerOfChannel(int32_t channel, int64_t offs, int64_t len) noexcept {
        if (hasChannelAndData(channel) && clampOffsAndLen(offs, len) > 0) {
            float average = this->average(channel, offs, len, 1);
            scale(channel, offs, len, 1.0f / average);
            int64_t s_step = sampleStep();
            if (m_data_type == DataType::Float) {
                auto s = reinterpret_cast<float*>(mutDataPtr(channel, offs));
                int64_t n = len;
                while (n--) {
                    *s -= average;
                    s += s_step;
                }
            }
        }
    }


    void Signal::fadeIn(int64_t offs, int64_t len) noexcept {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            fadeInChannel(channel, offs, len);
        }
    }


    void Signal::fadeInChannel(int32_t channel, int64_t offs, int64_t len) noexcept {
        fadeChannel(channel, offs, len, false);
    }


    void Signal::fadeOut(int64_t offs, int64_t len) noexcept {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            fadeOutChannel(channel, offs, len);
        }
    }


    void Signal::fadeOutChannel(int32_t channel, int64_t offs, int64_t len) noexcept {
        fadeChannel(channel, offs, len, true);
    }


    void Signal::fadeChannel(int32_t channel, int64_t offs, int64_t len, bool fade_out_mode) noexcept {
        if (hasChannelAndData(channel)) {
            int64_t requested_offs = offs;

            if (clampOffsAndLen(offs, len) > 0) {
                if (m_data_type == DataType::Float) {
                    auto d = reinterpret_cast<float*>(mutDataPtr(channel, offs));
                    int64_t d_step = sampleStep();

                    double phase_step = 1.0 / static_cast<double>(len) * std::numbers::pi;
                    double phase = phase_step * static_cast<double>(offs - requested_offs);

                    if (fade_out_mode) {
                        for (int64_t i = 0; i < len; i++, phase += phase_step) {
                            // [MEM_WRITE]
                            *d *= 0.5 * (std::cos(phase) + 1.0);
                            d += d_step;
                        }
                    }
                    else {
                        for (int64_t i = 0; i < len; i++, phase += phase_step) {
                            // [MEM_WRITE]
                            *d *= 1.0 - 0.5 * (std::cos(phase) + 1.0);
                            d += d_step;
                        }
                    }
                }

                // TODO: Support other data types
            }
        }
    }


    double Signal::envelope(
            int64_t offs,
            int64_t len,
            float start_amplitude,
            float end_amplitude,
            float shape)
    noexcept {
        double result = 0.0;

        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            result = envelope(channel, offs, len, start_amplitude, end_amplitude, shape);
        }

        return result;
    }


    double Signal::envelope(
            int32_t channel,
            int64_t offs,
            int64_t len,
            float start_amplitude,
            float end_amplitude,
            float shape)
    noexcept {
        switch (m_data_type) {
            case DataType::Float: {
                return _envelopeTyped<float>(channel, offs, len, start_amplitude, end_amplitude, shape);
            }
            case DataType::Double: {
                return _envelopeTyped<double>(channel, offs, len, start_amplitude, end_amplitude, shape);
            }
            default: {
                return 0.0;
            }
        }
    }


    template<typename T>
    double Signal::_envelopeTyped(
            int32_t channel,
            int64_t offs,
            int64_t len,
            T start_amplitude,
            T end_amplitude,
            float shape)
    noexcept {
        if (!hasChannelAndData(channel)) {
            return static_cast<T>(0);
        }

        const int64_t start_index = offs;
        const int64_t end_index = offs + len - 1;
        const int64_t n = end_index - start_index + 1;
        if (n < 1 || end_index < 0 || start_index > m_last_sample_index) {
            return static_cast<T>(0);
        }

        start_amplitude = std::clamp(start_amplitude, static_cast<T>(0), static_cast<T>(1));
        end_amplitude = std::clamp(end_amplitude, static_cast<T>(0), static_cast<T>(1));
        shape = std::clamp<float>(shape, 0.0f, 1.0f);
        const double lin_f = shape;
        const double exp_f = 1.0 - shape;

        const bool decay_mode = start_amplitude >= end_amplitude;

        double f_lin = 1.0;
        double f_lin_step = -1.0 / static_cast<double>(n);
        double f_exp, f_exp_coef;
        double f_mix = 0.0;
        if (decay_mode) {
            f_exp_coef = Signal::releaseCoef(
                    start_amplitude, end_amplitude,
                    static_cast<T>((start_amplitude - end_amplitude) / 100000.0),
                    n
            );
            f_exp = static_cast<double>(start_amplitude);
        }
        else {
            f_exp_coef = Signal::releaseCoef(
                    end_amplitude, start_amplitude,
                    static_cast<T>((end_amplitude - start_amplitude) / 100000.0),
                    n
            );
            f_exp = static_cast<double>(end_amplitude);
        }

        const int64_t i_start = std::max<int64_t>(start_index, 0);
        const int64_t i_end = std::min(end_index, m_last_sample_index);
        if ((i_end - i_start + 1) < 1) {
            return static_cast<T>(end_amplitude);
        }

        const int64_t d_step = sampleStep();

        T* d = reinterpret_cast<T*>(mutDataPtr(channel, decay_mode ? i_start : i_end));

        if (decay_mode) {
            if (start_index < 0) {
                f_exp *= std::pow(f_exp_coef, -start_index);
                f_lin -= static_cast<double>(start_index) * f_lin_step;
            }
            for (int64_t i = i_start; i <= i_end; ++i) {
                f_mix = lin_f * f_lin + exp_f * f_exp;
                // [MEM_WRITE]
                *d *= static_cast<T>(f_mix);
                d += d_step;
                f_lin += f_lin_step;
                f_exp *= f_exp_coef;
            }
        }
        else {
            if (end_index > m_last_sample_index) {
                f_exp *= std::pow(f_exp_coef, end_index - m_last_sample_index);
                f_lin -= static_cast<double>(end_index - m_last_sample_index) * f_lin_step;
            }
            for (int64_t i = i_end; i >= i_start; --i) {
                f_mix = lin_f * f_lin + exp_f * f_exp;
                // [MEM_WRITE]
                *d *= static_cast<T>(f_mix);
                d -= d_step;
                f_lin += f_lin_step;
                f_exp *= f_exp_coef;
            }
        }

        return f_mix;
    }


    void Signal::applyEnvelopeLUT(int32_t channel, int64_t offs, const LUT1* lut) noexcept {
        if (!hasChannelAndData(channel)  || !lut) {
            return;
        }

        int64_t len = lut->resolution();
        if (offs > m_last_sample_index || offs + len < 0) {
            return;
        }

        int64_t s_index = 0;
        int64_t d_index = offs;

        if (offs < 0) {
            s_index = -offs;
            len += offs;
            d_index = 0;
        }

        if (len > (m_sample_count - offs)) {
            len = m_sample_count - offs;
        }

        auto s = lut->mutSamplePtrAtIndex(s_index);
        auto d = reinterpret_cast<float*>(mutDataPtr(channel, d_index));
        int64_t d_step = sampleStep();
        for (int64_t i = 0; i < len; i++) {
            *d *= *s++;
            d += d_step;
        }
    }


    template<typename T>
    void Signal::reverseTyped(T* left, T* right, int64_t count, int64_t step) {
        for (int64_t i = 0; i < count; ++i) {
            T temp = *left;
            *left = *right;
            *right = temp;
            left += step;
            right -= step;
        }
    }


    void Signal::reverse() noexcept {
        reverse(0, m_last_sample_index);
    }


    void Signal::reverse(int64_t offs, int64_t len) noexcept {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            reverseChannel(channel, offs, len);
        }
    }


    void Signal::reverseChannel(int32_t channel, int64_t offs, int64_t len) noexcept {
        if (!hasChannelAndData(channel)) {
            return;
        }

        if (len < 0) {
            len = m_sample_count;
        }

        if (clampOffsAndLen(offs, len) < 1) {
            return;
        }

        int64_t step = sampleStep();
        int64_t end_index = offs + len - 1;
        int64_t half_len = len / 2;

        if (isFloatType()) {
            auto* l = reinterpret_cast<float*>(mutDataPtr(channel, offs));
            auto* r = reinterpret_cast<float*>(mutDataPtr(channel, end_index));
            reverseTyped<float>(l, r, half_len, step);
        }
        else if (isInt8Type()) {
            auto* l = reinterpret_cast<int8_t*>(mutDataPtr(channel, offs));
            auto* r = reinterpret_cast<int8_t*>(mutDataPtr(channel, end_index));
            reverseTyped<int8_t>(l, r, half_len, step);
        }
        else if (isInt16Type()) {
            auto* l = reinterpret_cast<int16_t*>(mutDataPtr(channel, offs));
            auto* r = reinterpret_cast<int16_t*>(mutDataPtr(channel, end_index));
            reverseTyped<int16_t>(l, r, half_len, step);
        }
        else if (isInt32Type()) {
            auto* l = reinterpret_cast<int32_t*>(mutDataPtr(channel, offs));
            auto* r = reinterpret_cast<int32_t*>(mutDataPtr(channel, end_index));
            reverseTyped<int32_t>(l, r, half_len, step);
        }
    }


    ErrorCode Signal::resample(int32_t channel, int32_t sample_rate, int64_t offs, int64_t len, float* out_ptr, int64_t step) noexcept {
        if (!hasChannel(channel)) {
            return ErrorCode::InvalidChannel;
        }

        if (sample_rate < 1) {
            return ErrorCode::BadArgs;
        }

        if (sample_rate == m_sample_rate) {
            return ErrorCode::SampleRateMustBeDifferent;
        }

        if (!isFloatType()) {
            return ErrorCode::UnsupportedDataType;
        }

        if (offs < 0 || offs > lastSampleIndex()) {
            return ErrorCode::OffsOutOfRange;
        }

        if (len < 0 || offs + len - 1 > lastSampleIndex()) {
            return ErrorCode::LenOutOfRange;
        }

        if (!out_ptr) {
            return ErrorCode::NullData;
        }

        if (step < 1) {
            return ErrorCode::UnsupportedStepSize;
        }

        double pos_step = static_cast<double>(sampleRate()) / sample_rate;
        HiResValue pos(static_cast<int64_t>(step * offs));
        pos.setStep(pos_step);

        while(pos.pos() < lastSampleIndex()) {
            *out_ptr = readFloatLerp(channel, pos);
            out_ptr += step;
            pos.stepForward();
        }

        return ErrorCode::None;
    }


    /**
     *  @brief Changes the sample rate of the audio processing.
     *
     *  This function is used to modify the sample rate of the signal. It involves resampling of the sample data.
     *
     *  @return Returns ErrorCode::None if the sample rate change is successful, or an appropriate error code if any error occurs during the process.
     */
    ErrorCode Signal::changeSampleRate(int32_t sample_rate) noexcept {
        auto result = ErrorCode::None;

        try {

            if (!hasData()) {
                throw ErrorCode::NoData;
            }

            if (m_data_type != DataType::Float) {
                throw ErrorCode::UnsupportedDataType;
            }

            if (sample_rate < 1) {
                throw ErrorCode::BadArgs;
            }

            if (m_sample_rate != sample_rate) {
                int64_t sample_count = static_cast<double>(m_sample_count) / m_sample_rate * sample_rate;
                if (sample_count > 1) {
                    auto signal = new (std::nothrow) Signal(m_channel_count, sample_rate, sample_count, DataType::Float, sample_rate < m_sample_rate);
                    if (!signal) {
                        throw ErrorCode::MemCantAllocate;
                    }

                    for (int32_t channel = 0; channel < m_channel_count; channel++) {
                        HiResValue pos;

                        if (sample_rate > m_sample_rate) {

                            pos.setStep(static_cast<double>(m_sample_rate) / sample_rate);

                            for (int64_t i = 0; i < sample_count; i++) {
                                float value = readFloatLerp(channel, pos);
                                signal->writeFloat(channel, i, value);
                                pos.stepForward();
                            }
                        }
                        else {

                            signal->clearWeights();
                            pos.setStep(static_cast<double>(sample_rate) / m_sample_rate);

                            for (int64_t i = 0; i < m_sample_count; i++) {
                                float value = readFloat(channel, i);
                                signal->addWeightedSample(channel, pos, value);
                                pos.stepForward();
                            }

                            signal->finishWeightedSamples(channel);
                        }
                    }

                    std::free(m_data.raw);
                    m_data.raw = signal->m_data.raw;
                    signal->m_data.raw = nullptr;

                    if (m_weights_mode) {
                        std::free(m_weights);
                        m_weights = signal->m_weights;
                        signal->m_weights = nullptr;
                    }

                    m_sample_rate = sample_rate;
                    m_sample_count = signal->m_sample_count;
                    m_last_sample_index = signal->m_last_sample_index;
                    m_data_byte_size = signal->m_data_byte_size;

                    delete signal;
                }
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    void Signal::distortChannel(int32_t channel, int64_t offs, int64_t len, float coef) noexcept {
        if (hasChannelAndData(channel) && clampOffsAndLen(offs, len) > 0) {
            if (m_data_type == DataType::Float) {
                auto s = reinterpret_cast<float*>(mutDataPtr(channel, offs));
                int64_t s_step = sampleStep();
                int64_t n = len;
                while (n--) {
                    *s = std::tanh(*s * coef) / coef;
                    s += s_step;
                }
            }
        }
    }


    ErrorCode Signal::applyFilter(SignalFilter* filter) noexcept {
        auto result = ErrorCode::None;

        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            result = applyFilterToChannel(filter, channel);
            if (result != ErrorCode::None) {
                break;
            }
        }

        return result;
    }


    ErrorCode Signal::applyFilter(SignalFilter* filter, int64_t offs, int64_t len) noexcept {
        auto result = ErrorCode::None;

        if (!filter) {
            return ErrorCode::NullData;
        }

        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            result = applyFilterToChannel(filter, channel, offs, len);
            if (result != ErrorCode::None) {
                break;
            }
        }

        return result;
    }


    ErrorCode Signal::applyFilterToChannel(SignalFilter* filter, int32_t channel) noexcept {
        return applyFilterToChannel(filter, channel, 0, m_sample_count);
    }


    ErrorCode Signal::applyFilterToChannel(SignalFilter* filter, int32_t channel, int64_t offs, int64_t len) noexcept {
        if (!filter) {
            return ErrorCode::NullData;
        }

        if (!hasChannelAndData(channel)) {
            return ErrorCode::InvalidChannel;
        }

        if (!isFloatType()) {
            return ErrorCode::UnsupportedDataType;
        }

        if (!filter->isValid()) {
            return ErrorCode::Unknown;
        }

        if (filter->sampleRate() != m_sample_rate) {
            return ErrorCode::SampleRateMustBeEqual;
        }

        if (clampOffsAndLen(offs, len) > 0) {
            filter->reset();
            auto s = reinterpret_cast<float*>(mutDataPtr(channel, offs));
            int64_t s_step = sampleStep();

            for (int64_t i = 0; i < len; i++) {
                *s = filter->process(*s);
                s += s_step;
            }
        }

        return ErrorCode::None;
    }


    ErrorCode Signal::applyFilterFFT(const Partials* partials) noexcept {
        return applyFilterFFT(partials, m_sample_count);
    }


    ErrorCode Signal::applyFilterFFT(const Partials* partials, int64_t len) noexcept {
        auto result = ErrorCode::None;

        for (int32_t channel = 0; channel < channelCount(); channel++) {
            result = applyFilterFFTToChannel(partials, channel, len);
            if (result != ErrorCode::None) {
                break;
            }
        }

        return result;
    }


    ErrorCode Signal::applyFilterFFTToChannel(const Partials* partials, int32_t channel) noexcept {
        return applyFilterFFTToChannel(partials, channel, m_sample_count);
    }


    void Signal::_prepareFilterFFT(int32_t fft_len, int32_t window_len) {
        if (m_fft) {
            if (m_fft->len() != fft_len) {
                delete m_fft;
                m_fft = nullptr;
            }
            else {
                return;
            }
        }

        int32_t fft_log_n = Math::log2IfPowerOfTwo(fft_len);
        if (fft_log_n < 1) {
            Exception::throwMessage(ErrorCode::BadArgs, "FFT length must be greater than 1");
        }


        std::cout << "fft_log_n: " << fft_log_n << std::endl;
        if (!m_fft) {
            m_fft = new (std::nothrow) FFT(fft_log_n);
            if (!m_fft) {
                Exception::throwMessage(ErrorCode::ClassInstantiationFailed, "Failed to allocate FFT instance");
            }
        }

        m_fft_buffer = static_cast<float*>(std::malloc(sizeof(float) * fft_len));
        if (!m_fft_buffer) {
            Exception::throwMessage(ErrorCode::MemCantAllocate, "Failed to allocate FFT buffer");
        }

        m_fft_window_len = window_len;
        m_fft_window = static_cast<float*>(std::malloc(sizeof(float) * m_fft_window_len));
        if (!m_fft_window) {
            Exception::throwMessage(ErrorCode::MemCantAllocate, "Failed to allocate FFT window");
        }

        DSP::hanningWindowSymmetric(m_fft_window_len, m_fft_window);
    }


    void Signal::releaseFilterFFTResources() {
        std::free(m_fft_buffer);
        m_fft_buffer = nullptr;

        std::free(m_fft_window);
        m_fft_window = nullptr;

        delete m_fft;
        m_fft = nullptr;
    }


    /**
     * @brief Apply an FFT filter to a single channel of the signal.
     *
     * @param partials Amplitudes and phases, provided as a Partials object.
     * @param channel Index of the channel to be filtered.
     * @param len Number of samples to filter.
     * @return An error code, or ErrorCode::None if the operation succeeds.
     */
    ErrorCode Signal::applyFilterFFTToChannel(const Partials* partials, int32_t channel, int64_t len) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!partials) {
                Exception::throwMessage(ErrorCode::BadArgs, "Partials is null");
            }

            _checkProcessTypeChannelIndex(DataType::Float, channel, 0);

            len = len < 0 ? m_sample_count : std::clamp<int64_t>(len, 0, m_sample_count);
            if (len < 1) {
                return ErrorCode::None;
            }

            int32_t partial_n = partials->resolution();
            int32_t step_n = partial_n / 4;
            int32_t block_n = 8 * step_n;
            int32_t window_n = 2 * step_n;
            int32_t ramp_n = 3 * step_n;
            int32_t ringbuffer_n = step_n * 8;

            int32_t fft_log_n = Math::log2IfPowerOfTwo(block_n);
            if (fft_log_n < 1) {
                Exception::throwMessage(ErrorCode::BadArgs, "Partials resolution must be a power of two.");
            }

            _prepareFilterFFT(block_n, window_n);

            std::cout << m_fft << std::endl;

            std::cout << "partial_n: " << partial_n << std::endl;
            std::cout << "step_n: " << step_n << std::endl;
            std::cout << "block_n: " << block_n << std::endl;
            std::cout << "window_n: " << window_n << std::endl;
            std::cout << "ramp_n: " << ramp_n << std::endl;
            std::cout << "ringbuffer_n: " << ringbuffer_n << std::endl;
            std::cout << "fft_log_n: " << fft_log_n << std::endl;

            auto window = m_fft_window;

            int64_t sample_read_index = 0;
            int64_t signal_write_index = 0;

            // Init ringbuffer
            RingBuffer<float> ringbuffer(ringbuffer_n);
            if (!ringbuffer.isUsable()) {
                Exception::throwMessage(ErrorCode::ClassInstantiationFailed, "RingBuffer instantiation failed.");
            }


            // Build initial ringbuffer
            ringbuffer.writeZeros(step_n * 4);
            for (int32_t i = 0; i < step_n * 4; i++) {
                ringbuffer.write(readFloat(channel, sample_read_index));
                sample_read_index++;
            }

            clearChannel(channel, 0, step_n * 4);

            int64_t ri = 4;
            int64_t ri_end = len / step_n;

            auto forward_window = static_cast<float*>(std::malloc(sizeof(float) * block_n));
            float t = 0.0f;
            float t_step = 1.0f / static_cast<float>(step_n * 3);
            for (int32_t i = 0, j = block_n - 1; i < step_n * 3; i++, j--) {
                forward_window[i] = forward_window[j] = DSP::hanningLeft(t);
                t += t_step;
            }
            for (int32_t i = step_n * 3; i < step_n * 5; i++) {
                forward_window[i] = 1.0f;
            }


            for (;;) {
                auto d = m_fft_buffer;
                for (int64_t i = 0; i < block_n; i++) {
                    *d++ = ringbuffer.read() * forward_window[i];
                }
                ringbuffer.shiftReadPos(-step_n * 7);

                // auto sig = new Signal(2, sampleRate(), block_n);
                // sig->writeSamples(0, 0, block_n, m_fft_buffer);

                m_fft->fft(m_fft_buffer);
                m_fft->filter(partials);
                m_fft->ifft(m_fft_buffer);

                for (int32_t i = step_n * 3, j = 0; i < step_n * 5; i++, j++) {
                    m_fft_buffer[i] *= m_fft_window[j];
                }

                // sig->writeSamples(1, 0, block_n, m_fft_buffer);
                // String file_path;
                // file_path.setFormatted(1024, "../output/x_%d.aiff", ri);
                // sig->writeToFile(file_path, Signal::FileContainerFormat::AIFF, Signal::FileSampleEncoding::Original);
                // delete sig;

                // Add filtered buffer back to signal
                auto s = &m_fft_buffer[step_n * 3];
                for (int64_t i = 0; i < window_n; i++) {
                    addSample(channel, signal_write_index + i, *s++);
                }
                signal_write_index += step_n;

                if (ri > ri_end + 1) {
                    break;
                }

                // read ri into ringbuffer
                for (int64_t i = 0; i < step_n; i++) {
                    ringbuffer.write(readFloat(channel, sample_read_index));
                    sample_read_index++;
                }

                // clear signal ri
                clearChannel(channel, ri * step_n, step_n);

                ri += 1;
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Perform partitioned convolution of one channel of the signal with
     *         one channel of an impulse response (IR) signal, writing the result
     *         into another signal.
     *
     *  This implements a frequency-domain partitioned convolution algorithm:
     *  - The input signal is processed in blocks of size L = 2^(partition_log_n - 1).
     *  - Each block is FFT-transformed, multiplied with precomputed FFTs of the IR
     *    partitions, accumulated, and transformed back using an inverse FFT.
     *  - Overlap-save/add is used to handle the block convolution tails.
     *
     *  @param channel Channel index of the input signal to convolve.
     *  @param offs Sample offs into the input signal where convolution starts.
     *  @param len Number of input samples to process (if < 0, uses the remainder of the signal).
     *  @param ir Pointer to the impulse response signal.
     *  @param ir_channel Channel index of the impulse response to use.
     *  @param ir_offs Sample offs into the impulse response.
     *  @param ir_len Number of samples from the impulse response to use (if < 0, uses the remainder).
     *  @param result_signal Pointer to the signal object where the output will be written.
     *  @param result_channel Channel index of the result signal to write into.
     *  @param partition_len Desired length of each partition. The actual FFT size will be rounded up to the next power of two.
     *
     *  @return ErrorCode::None on success, or an appropriate error code on failure:
     *  - ErrorCode::BadArgs if arguments are invalid.
     *  - ErrorCode::NullPointer if required pointers are null.
     *  - ErrorCode::BuffersMustBeDifferent if the IR and input signal are the same object.
     *  - ErrorCode::InvalidChannel if channels are invalid.
     *  - ErrorCode::MemCantGrow if result signal cannot be resized.
     *  - ErrorCode::MemCantAllocate if temporary buffers or structures cannot be allocated.
     *  - ErrorCode::ClassInstantiationFailed if FFT instance cannot be created.
     *
     *  @note This method assumes both input and IR signals are floating-point.
     *        The result signal will be resized to fit the full convolution length:
     *        result_len = len + ir_len - 1.
     */
#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode Signal::convolveChannel(
            int32_t channel,
            int64_t offs,
            int64_t len,
            const Signal* ir,
            int32_t ir_channel,
            int64_t ir_offs,
            int64_t ir_len,
            Signal* conv_signal,
            int32_t conv_channel,
            int64_t partition_len
    ) const noexcept
    {
        auto result = ErrorCode::None;

        // Check the signals ...
        if (!ir || !conv_signal) {
            return ErrorCode::NullPointer;
        }

        if (!hasChannelAndData(channel) ||
            !ir->hasChannelAndData(ir_channel) ||
            !conv_signal->hasChannel(conv_channel)) {
            return ErrorCode::InvalidChannel;
        }

        // Clamp offsets and lengths ...
        if (len < 0) {
            len = m_sample_count;
        }
        clampOffsAndLen(offs, len);

        if (ir_len < 0) {
            ir_len = ir->sampleCount();
        }

        ir->clampOffsAndLen(ir_offs, ir_len);

        // Collect variables ...
        const int32_t partition_log_n = std::clamp<int32_t>(Math::nextLog2(partition_len), FFT::kMinLogN, FFT::kMaxLogN);
        partition_len = 1L << partition_log_n;
        const int32_t partition_n = std::max(static_cast<int32_t>((ir_len + partition_len - 1) / partition_len), 1);

        const int32_t fft_len = static_cast<int32_t>(Math::nextPowerOfTwo(2 * partition_len));
        const int32_t fft_half_len = fft_len / 2;
        const int32_t fft_log = Math::log2IfPowerOfTwo(fft_len);
        const int32_t overlap_len = static_cast<int32_t>(fft_len - partition_len);

        // Prepare the resulting signal ...
        int64_t result_len = len + ir_len - 1;
        if (conv_signal->growIfNeeded(result_len) != ErrorCode::None) {
            return ErrorCode::MemCantGrow;
        }
        conv_signal->clearChannel(conv_channel);

        // Memory and objects ...
        FFTSetup fft_setup = nullptr;

        auto time_buffer = static_cast<float*>(std::calloc(fft_len, sizeof(float)));
        auto interleaved_buffer = static_cast<float*>(std::calloc(fft_len, sizeof(float)));
        auto write_buffer = static_cast<float*>(std::calloc(partition_len, sizeof(float)));
        auto overlap_buffer = static_cast<float*>(std::calloc(overlap_len > 0 ? overlap_len : 0, sizeof(float)));
        auto t_out = static_cast<float*>(std::calloc(fft_len, sizeof(float)));
        auto ir_partials = new(std::nothrow) FFTComplexSplitArray(partition_n, fft_half_len);
        auto x_ring = new(std::nothrow) FFTComplexSplitArray(partition_n, fft_half_len);
        auto y_freq = new(std::nothrow) FFTComplexSplit(fft_half_len);

        try {
            if (!time_buffer ||
                !interleaved_buffer ||
                !write_buffer ||
                !overlap_buffer ||
                !t_out ||
                !ir_partials ||
                !x_ring ||
                !y_freq) {
                Exception::throwMessage(ErrorCode::MemCantAllocate, "Failed to allocate buffers");
            }

            // Prepare the FFT ...
            fft_setup = vDSP_create_fftsetup(fft_log, kFFTRadix2);
            if (!fft_setup) {
                Exception::throwMessage(ErrorCode::ClassInstantiationFailed, "Failed to allocate FFT setup");
            }

            float ir_rms;
            vDSP_rmsqv(static_cast<float *>(ir->mutDataPtr(channel)), ir->channelCount(), &ir_rms, ir_len);
            std::cout << "ir_rms: " << ir_rms << std::endl;

            // Prepare the impulse response ...
            for (int p = 0; p < partition_n; p++) {
                ir->readSamplesAsFloatWithZeroPadding(
                        ir_channel,
                        ir_offs + static_cast<int64_t>(p) * partition_len,
                        partition_len,
                        time_buffer);

                // pack into interleaved {re, im=0}
                for (int i = 0; i < fft_half_len; ++i) {
                    int idx = 2 * i;
                    interleaved_buffer[idx] = (idx < fft_len) ? time_buffer[idx] : 0.0f;
                    interleaved_buffer[idx + 1] = (idx + 1 < fft_len) ? time_buffer[idx + 1] : 0.0f;
                }

                auto split = ir_partials->splitAtIndex(p);

                DSPSplitComplex tmp;
                tmp.realp = split->m_real;
                tmp.imagp = split->m_imag;
                vDSP_ctoz((const DSPComplex*) interleaved_buffer, 2, &tmp, 1, fft_half_len);
                vDSP_fft_zrip(fft_setup, &tmp, 1, fft_log, FFT_FORWARD);
            }

            int32_t ring_head = 0;
            int64_t write_pos = 0;
            int64_t processed = 0;

            // Helper function
            auto processBlock = [&](const float *input) {
                // Build interleaved
                for (int i = 0; i < fft_half_len; ++i) {
                    int idx = 2 * i;
                    interleaved_buffer[2 * i] = (idx < fft_len) ? input[idx] : 0.0f;
                    interleaved_buffer[2 * i + 1] = (idx + 1 < fft_len) ? input[idx + 1] : 0.0f;
                }

                DSPSplitComplex X;
                X.realp = x_ring->splitAtIndex(ring_head)->m_real;
                X.imagp = x_ring->splitAtIndex(ring_head)->m_imag;

                vDSP_ctoz((const DSPComplex*) interleaved_buffer, 2, &X, 1, fft_half_len);
                vDSP_fft_zrip(fft_setup, &X, 1, fft_log, FFT_FORWARD);


                y_freq->clear(); // Zero accumulator
                DSPSplitComplex Y;
                Y.realp = y_freq->m_real;
                Y.imagp = y_freq->m_imag;

                // Accumulate Y += Xj * Hj
                for (int j = 0; j < partition_n; ++j) {
                    int idx = ring_head - j;
                    if (idx < 0) idx += partition_n;

                    DSPSplitComplex Xj;
                    Xj.realp = x_ring->splitAtIndex(idx)->m_real;
                    Xj.imagp = x_ring->splitAtIndex(idx)->m_imag;

                    DSPSplitComplex Hj;
                    Hj.realp = ir_partials->splitAtIndex(j)->m_real;
                    Hj.imagp = ir_partials->splitAtIndex(j)->m_imag;

                    vDSP_zvma(&Xj, 1, &Hj, 1, &Y, 1, &Y, 1, fft_half_len);
                }

                // Inverse FFT
                vDSP_fft_zrip(fft_setup, &Y, 1, fft_log, FFT_INVERSE);
                float scale = 1.0f / static_cast<float>(fft_len);
                vDSP_vsmul(Y.realp, 1, &scale, Y.realp, 1, fft_half_len);
                vDSP_vsmul(Y.imagp, 1, &scale, Y.imagp, 1, fft_half_len);

                // Back to interleaved
                vDSP_ztoc(&Y, 1, (DSPComplex*)interleaved_buffer, 2, fft_half_len);

                // Reconstruct time-domain
                for (int i = 0; i < fft_half_len; ++i) {
                    int idx = 2 * i;
                    if (idx < fft_len) t_out[idx] = interleaved_buffer[2 * i];
                    if (idx + 1 < fft_len) t_out[idx + 1] = interleaved_buffer[2 * i + 1];
                }

                const int64_t remaining = result_len - write_pos;
                const int64_t out_len = std::min<int64_t>(partition_len, remaining);

                for (int i = 0; i < out_len; ++i) {
                    float s = t_out[i];
                    if (overlap_len > 0 && i < overlap_len) s += overlap_buffer[i];
                    write_buffer[i] = s;
                }
                conv_signal->writeSamples(conv_channel, write_pos, out_len, write_buffer, CombineMode::Add);

                if (overlap_len > 0) {
                    std::copy(&t_out[partition_len], &t_out[partition_len + overlap_len], overlap_buffer);
                }

                ring_head = (ring_head + 1) % partition_n;
                write_pos += out_len;
            };

            // Processing
            while (processed < len) {
                int64_t n = std::min<int64_t>(partition_len, len - processed);
                // TODO: remove? std::fill(timeBuf.begin(), timeBuf.end(), 0.0f);
                readSamplesAsFloatWithZeroPadding(channel, offs + processed, n, time_buffer);
                processBlock(time_buffer);
                processed += n;
            }

            // Flush tail (zero input)
            while (write_pos < result_len) {
                // TODO: remove? std::fill(timeBuf.begin(), timeBuf.end(), 0.0f);
                processBlock(time_buffer);
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }

        // Cleanup
        std::free(time_buffer);
        std::free(interleaved_buffer);
        std::free(write_buffer);
        std::free(overlap_buffer);
        std::free(t_out);

        delete x_ring;
        delete y_freq;
        delete ir_partials;

        if (fft_setup) {
            vDSP_destroy_fftsetup(fft_setup);
        }

        return result;
    }
#else
ErrorCode Signal::convolveChannel(
            int32_t channel,
            int64_t offs,
            int64_t len,
            const Signal* ir,
            int32_t ir_channel,
            int64_t ir_offs,
            int64_t ir_len,
            Signal* result_signal,
            int32_t result_channel,
            int64_t partition_len
    ) const noexcept
    {
        #pragma message("Signal::convolveChannel() must be implemented for Linux!")
    }
#endif


    void Signal::addWhiteNoise(
            int64_t offs,
            int64_t len,
            float amount,
            float threshold
    ) noexcept
    {
        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            addWhiteNoiseToChannel(channel, offs, len, amount, threshold);
        }
    }


    void Signal::addWhiteNoiseToChannel(
            int32_t channel,
            int64_t offs,
            int64_t len,
            float amount,
            float threshold
    ) noexcept
    {
        if (!hasChannelAndData(channel)) {
            return;
        }

        if (len < 0) {
            len = m_sample_count - offs;
        }

        if (clampOffsAndLen(offs, len) < 1) {
            return;
        }


        int64_t n = len;
        if (isFloatType()) {
            auto s = reinterpret_cast<float*>(mutDataPtr(channel, offs));
            int64_t s_step = sampleStep();
            if (threshold < 1.0f) {
                while (n--) {
                    if (Random::next() > threshold) {
                        *s += Random::nextBipolar() * amount;
                    }
                    s += s_step;
                }
            }
            else {
                while (n--) {
                    *s += Random::nextBipolar() * amount;
                    s += s_step;
                }
            }
        }
        else {
            int64_t index = offs;
            if (threshold < 1.0f) {
                while (n--) {
                    if (Random::next() > threshold) {
                        writeFloat(channel, index++, Random::nextBipolar() * amount);
                    }
                }
            }
            else {
                while (n--) {
                    writeFloat(channel, index++, Random::nextBipolar() * amount);
                }
            }
        }

    }


    void Signal::generateSine(int32_t channel, int64_t offs, int64_t len, float freq) noexcept {
        if (clampOffsAndLen(offs, len) > 0 && freq > std::numeric_limits<float>::epsilon()) {
            double t = 0.0;
            double t_step = std::numbers::pi * 2 / m_sample_rate * freq;

            int64_t last_index = offs + len - 1;
            for (int64_t i = offs; i <= last_index; i++) {
                writeFloat(channel, i, std::sin(t));
                t += t_step;
            }
        }
    }


    void Signal::generateSineSweep(
            int32_t channel,
            float start,
            float duration,
            float freq_start,
            float freq_end,
            float db_start,
            float db_end,
            float fade_in_duration,
            float fade_out_duration) noexcept
    {
        int64_t offs = sampleIndexAtSecond(start);
        int64_t len = sampleIndexAtSecond(duration);
        if (len < 1) {
            return;
        }

        int64_t requested_offs = offs;
        if (clampOffsAndLen(offs, len) < 1) {
            return;
        }

        freq_start = std::clamp<float>(freq_start, 0.01f, 0.5f * m_sample_rate);
        freq_end = std::clamp<float>(freq_end, freq_start, 0.5f * m_sample_rate);

        double start_pitch = Audio::pitchFromFreq(freq_start);
        double end_pitch = Audio::pitchFromFreq(freq_end);
        double pitch = start_pitch;
        double pitch_step = static_cast<double>(end_pitch - start_pitch) / len;

        double db = db_start;
        double db_step = (db_end - db_start) / len;

        double a = std::numbers::pi * 2 / m_sample_rate;
        double t = 0.0;

        if (requested_offs < 0) {
            pitch += pitch_step * -requested_offs;
            db += db_step * -requested_offs;
        }

        int64_t lastIndex = offs + len - 1;
        for (int64_t i = offs; i <= lastIndex; i++) {

            float f = Audio::dbToLinear(db);
            writeFloat(channel, i, std::sin(t) * f);

            float freq = Audio::freqFromPitch(pitch);
            pitch += pitch_step;
            db += db_step;
            t += a * freq;
        }

        if (fade_in_duration > 0.0f) {
            int64_t n = sampleIndexAtSecond(fade_in_duration);
            fadeIn(offs, n);
        }

        if (fade_out_duration > 0.0f) {
            int64_t n = sampleIndexAtSecond(fade_out_duration);
            fadeOut(offs + len - n, n);
        }
    }


    ErrorCode Signal::writeToFile(
            const String &file_path,
            FileContainerFormat container_format,
            FileSampleEncoding sample_encoding,
            int64_t offs,
            int64_t len)
            const noexcept
    {
        if (clampOffsAndLen(offs, len) < 1) {
            return Error::specific(kErr_NothingToWrite);
        }

        SF_INFO sfinfo;
        sfinfo.samplerate = m_sample_rate;
        sfinfo.channels = m_channel_count;

        int sf_encoding = 0;
        switch (m_data_type) {
            case DataType::Int16: sf_encoding = SF_FORMAT_PCM_16; break;
            case DataType::Int32: sf_encoding = SF_FORMAT_PCM_32; break;
            case DataType::Float: sf_encoding = SF_FORMAT_FLOAT;  break;
            default:
                return Error::specific(kErr_UnsupportedDataType);
        }

        if (sample_encoding != FileSampleEncoding::Original) {
            switch (sample_encoding) {
                case FileSampleEncoding::Int8: sf_encoding = SF_FORMAT_PCM_S8; break;
                case FileSampleEncoding::Int16: sf_encoding = SF_FORMAT_PCM_16; break;
                case FileSampleEncoding::Int24: sf_encoding = SF_FORMAT_PCM_24; break;
                case FileSampleEncoding::Int32: sf_encoding = SF_FORMAT_PCM_32; break;
                case FileSampleEncoding::Float: sf_encoding = SF_FORMAT_FLOAT;  break;
                case FileSampleEncoding::ALAW: sf_encoding = SF_FORMAT_ALAW;  break;
                case FileSampleEncoding::ULAW: sf_encoding = SF_FORMAT_ULAW;  break;
                case FileSampleEncoding::IMA_ADPCM: sf_encoding = SF_FORMAT_IMA_ADPCM;  break;
                case FileSampleEncoding::MS_ADPCM: sf_encoding = SF_FORMAT_MS_ADPCM;  break;
                case FileSampleEncoding::Original: /* handled above */ break;
            }
        }

        int sf_container_format = 0;
        switch (container_format) {
            case FileContainerFormat::AIFF: sf_container_format = SF_FORMAT_AIFF; break;
            case FileContainerFormat::AIFC: sf_container_format = SF_FORMAT_AIFF; break;
            case FileContainerFormat::WAV: sf_container_format = SF_FORMAT_WAV; break;
            default:
                return Error::specific(kErr_UnsupportedContainerFormat);
        }

        sfinfo.format = sf_container_format | sf_encoding;
        if (!sf_format_check(&sfinfo)) {
            return Error::specific(kErr_InvalidWriteSetting);
        }

        SNDFILE* out_file = sf_open(file_path.utf8(), SFM_WRITE, &sfinfo);
        if (!out_file) {
            return ErrorCode::FileCantOpen;
        }

        auto sample_ptr = mutDataPtr(0, offs);
        sf_count_t frame_n = len;
        sf_count_t written_frame_n = 0;

        // Dispatch based on internal type
        switch (m_data_type) {
            case DataType::Int16:
                written_frame_n = sf_writef_short(out_file, static_cast<short*>(sample_ptr), frame_n);
                break;
            case DataType::Int32:
                written_frame_n = sf_writef_int(out_file, static_cast<int*>(sample_ptr), frame_n);
                break;
            case DataType::Float:
                written_frame_n = sf_writef_float(out_file, static_cast<float*>(sample_ptr), frame_n);
                break;
            default:
                sf_close(out_file);
                return Error::specific(kErr_UnsupportedDataType);
        }

        sf_close(out_file);

        if (written_frame_n != frame_n) {
            return ErrorCode::FileCantWrite;
        }

        return ErrorCode::None;
    }


    ErrorCode Signal::writeToFile(
            const String &file_path,
            FileContainerFormat container_format,
            FileSampleEncoding sample_encoding) const noexcept
    {
        return writeToFile(file_path, container_format, sample_encoding, 0, m_sample_count);
    }


    ErrorCode Signal::writeRegionToFile(
            const String &file_path,
            FileContainerFormat container_format,
            FileSampleEncoding sample_encoding,
            int32_t region_index) const noexcept
    {
        auto result = ErrorCode::None;

        if (auto region = regionPtrAtIndex(region_index)) {
            result = writeToFile(file_path, container_format, sample_encoding, region->left(), region->len());
        }
        else {
            result = ErrorCode::RegionOutOfRange;
        }

        return result;
    }


    [[nodiscard]] int32_t Signal::fileInfo(const String& file_path, SignalInfo& out_info) noexcept {
        SF_INFO sf_info;
        SNDFILE *snd_file = sf_open(file_path.utf8(), SFM_READ, &sf_info);
        if (!snd_file) {
            return 1;
        }

        out_info.m_channel_count = sf_info.channels;
        out_info.m_sample_rate = sf_info.samplerate;
        out_info.m_sample_count = sf_info.frames;
        out_info.m_format = sf_info.format; // TODO: sf_info.format to SignalInfo format
        sf_close(snd_file);

        return 0;
    }


    Signal* Signal::createFromFile(const String& file_path, DataType data_type, ErrorCode& out_err) noexcept {
        out_err = ErrorCode::None;
        Signal* signal = nullptr;
        SNDFILE *snd_file = nullptr;

        try {
            switch (data_type) {
                case DataType::Int8:
                case DataType::Int16:
                case DataType::Int32:
                case DataType::Float:
                    break;
                default:
                    Exception::throwStandard(ErrorCode::UnsupportedDataType);
            }

            SF_INFO sf_info;
            sf_info.format = 0; // Required
            snd_file = sf_open(file_path.utf8(), SFM_READ, &sf_info);
            if (!snd_file) {
                Exception::throwStandard(ErrorCode::FileCantOpen);
            }

            signal = new(std::nothrow) Signal(
                    sf_info.channels,
                    sf_info.samplerate,
                    sf_info.frames,
                    data_type);
            if (!signal) {
                Exception::throwStandard(ErrorCode::ClassInstantiationFailed);
            }

            sf_count_t n = 0;
            void *sample_ptr = signal->mutDataPtr();

            switch (data_type) {
                case DataType::Int16:
                    n = sf_readf_short(snd_file, static_cast<short *>(sample_ptr), sf_info.frames);
                    break;
                case DataType::Int32:
                    n = sf_readf_int(snd_file, static_cast<int *>(sample_ptr), sf_info.frames);
                    break;
                case DataType::Float:
                    n = sf_readf_float(snd_file, static_cast<float *>(sample_ptr), sf_info.frames);
                    break;
                case DataType::Int8: {
                    auto buffer = static_cast<int16_t *>(malloc(sizeof(int8_t) * sf_info.frames * sf_info.channels));
                    if (!buffer) {
                        Exception::throwStandard(ErrorCode::MemCantAllocate);
                    }
                    n = sf_readf_short(snd_file, buffer, sf_info.frames);
                    auto *out = static_cast<int8_t *>(sample_ptr);
                    for (size_t i = 0; i < static_cast<size_t>(n * sf_info.channels); i++) {
                        out[i] = static_cast<int8_t>(buffer[i] >> 8);
                    }
                    free(buffer);
                    break;
                }
                default:
                    Exception::throwSpecific(kErr_UnsupportedDataType);
            }

            if (n != sf_info.frames) {
                Exception::throwSpecific(kErr_ReadAllSamplesFailed);
            }
        }
        catch (const Exception& e) {
            out_err = e.code();
            if (signal) {
                delete signal;
                signal = nullptr;
            }
        }
        catch (...) {
            out_err = ErrorCode::Unknown;
        }

        if (snd_file) {
            sf_close(snd_file);
        }

        return signal;
    }


    SignalRegion *Signal::regionPtrAtIndex(int32_t index) const noexcept {
        if (index < 0 || index >= m_region_count) {
            return nullptr;
        }

        int64_t i = 0;
        SignalRegion *region = m_first_region;
        while (region) {
            if (i == index) {
                return region;
            }
            i++;
            region = region->next();
        }

        return nullptr;
    }


    SignalRegion *Signal::addRegion(const String &name, int32_t channel, int64_t left, int64_t right) noexcept {
        SignalRegion *region = new (std::nothrow) SignalRegion(this, name, channel, left, right);
        if (!region) {
            return nullptr;
        }

        // Put the new region into the chain
        region->setNext(m_first_region);
        m_first_region = region;
        m_region_count++;
        m_regions_must_sort = true;

        return region;
    }


    bool Signal::removeRegion(SignalRegion *region) noexcept {
        if (!region) {
            return false;
        }

        // Scan through the chain
        if (region == m_first_region) {
            m_first_region = region->next();
            m_region_count--;
            delete region;
            return true;
        }
        else {
            SignalRegion *r = m_first_region;
            while (r) {
                if (region == r->next()) {
                    r->setNext(region->next());
                    m_region_count--;
                    delete region;
                    return true;
                }
            }
        }

        return false;
    }


    int Signal::_compareRegion(const void *region_ptr0, const void *region_ptr1) {
        SignalRegion *region0 = *(SignalRegion**)region_ptr0;
        SignalRegion *region1 = *(SignalRegion**)region_ptr1;

        int64_t pos0 = region0->left();
        int64_t pos1 = region1->left();

        if (pos0 > pos1) {
            return 1;
        }

        if (pos0 < pos1) {
            return -1;
        }

        return 0;
    }


    ErrorCode Signal::sortRegions() noexcept {
        int32_t n = regionCount();

        if (!m_regions_must_sort || n < 2) {
            return ErrorCode::None; // Nothing to sort
        }

        if (n > m_sort_region_ptr_array_capacity) {
            auto size = sizeof(SignalRegion*) * (n + 16);
            m_sort_region_ptr_array = static_cast<SignalRegion**>(std::realloc(m_sort_region_ptr_array, size));
            if (!m_sort_region_ptr_array) {
                m_sort_region_ptr_array_capacity = 0;
                return ErrorCode::MemCantAllocate;
            }
        }

        auto region_ptr_array = m_sort_region_ptr_array;

        int64_t index = 0;
        SignalRegion* region = firstRegionPtr();
        while (region) {
            region_ptr_array[index] = region;
            index++;
            region = region->next();
        }

        qsort((void*)region_ptr_array, n, sizeof(SignalRegion*), Signal::_compareRegion);

        m_first_region = region_ptr_array[0];
        for (int64_t i = 1; i < n; i++) {
            region_ptr_array[i - 1]->setNext(region_ptr_array[i]);
        }
        region_ptr_array[n - 1]->setNext(nullptr);

        m_regions_must_sort = false;
        return ErrorCode::None;
    }


    SignalRegion::SignalRegion(Signal* signal, const String &name, int32_t channel, int64_t left, int64_t right) noexcept {
        m_signal = signal;
        m_name = name;
        m_channel = channel;
        setLeftAndRight(left, right);
        m_locked = false;
        m_next = nullptr;
    }


    SignalRegion::~SignalRegion() {
    }


    int32_t SignalRegion::handleColorIndex(bool selected) const noexcept {
        return kColorIndexNormal + selected + static_cast<int32_t>(m_locked) * 2;
    }


    size_t SignalRegion::dataSize() const noexcept {
        if (m_signal != nullptr) {
            return len() * m_signal->bytesPerSample() * m_signal->channelCount();
        }

        return 0;
    }


    size_t SignalRegion::monoDataSize() const noexcept {
        if (m_signal != nullptr) {
            return len() * m_signal->bytesPerSample();
        }

        return 0;
    }


    const void* SignalRegion::dataPtr() const noexcept {
        if (m_signal != nullptr) {
            if (m_channel < 0) {
                return m_signal->mutDataPtr(0, m_left);
            }
            else if (m_signal->hasChannelAndData(m_channel)) {
                return m_signal->mutDataPtr(m_channel, m_left);
            }
        }

        return nullptr;
    }


    float SignalRegion::freq() const noexcept {
        if (!m_signal) {
            return 0;
        }

        int64_t sampleCount = len();
        if (sampleCount > 0) {
            return static_cast<float>(m_signal->sampleRate()) / sampleCount;
        }

        return 0;
    }


    void SignalRegion::setLeftAndRight(int64_t left, int64_t right) noexcept {
        if (m_signal != nullptr) {
            left = std::clamp<int64_t>(left, 0, m_signal->lastSampleIndex());
            right = std::clamp<int64_t>(right, 0, m_signal->lastSampleIndex());
        }

        m_left = std::min(left, right);
        m_right = std::max(left, right);

        m_signal->setMustSortRegions();
    }


    void SignalRegion::setLeft(int64_t left) noexcept {
        if (m_signal != nullptr) {
            left = std::clamp<int64_t>(left, 0, m_signal->lastSampleIndex());
        }

        m_left = left;
        if (m_left > m_right) {
            std::swap(m_left, m_right);
        }

        if (m_signal) {
            m_signal->setMustSortRegions();
        }
    }


    void SignalRegion::setRight(int64_t right) noexcept {
        if (m_signal != nullptr) {
            right = std::clamp<int64_t>(right, 0, m_signal->lastSampleIndex());
        }

        m_right = right;
        if (m_right < m_left) {
            std::swap(m_left, m_right);
        }

        m_signal->setMustSortRegions();
    }


    void SignalRegion::slipLeft() noexcept {
        int64_t n = len();
        if (m_left - n < 0) {
            m_left = 0;
            m_right = m_left + n - 1;
        }
        else {
            m_left -= n;
            m_right -= n;
        }

        m_signal->setMustSortRegions();
    }


    void SignalRegion::slipRight() noexcept {
        int64_t n = len();
        if (m_right + n > m_signal->lastSampleIndex()) {
            m_right = m_signal->lastSampleIndex();
            m_left = m_right - n + 1;
        }
        else {
            m_left += n;
            m_right += n;
        }

        m_signal->setMustSortRegions();
    }


    Signal* SignalRegion::extractSignal() noexcept {
        return extractSignal(false, m_signal->dataType());
    }


    Signal* SignalRegion::extractSignal(bool mono, DataType data_type) noexcept {
        if (!m_signal) {
            return nullptr;
        }

        int32_t channel_count = mono ? 1 : m_signal->channelCount();

        auto extracted_signal = new (std::nothrow) Signal(channel_count, m_signal->sampleRate(), len(), data_type, false);
        if (!extracted_signal) {
            return nullptr;
        }

        extracted_signal->copySamples(m_signal, len(), left(), 0);

        return extracted_signal;
    }


    double Signal::findNearestFrequency(
            int32_t sample_rate,
            int64_t buffer_len,
            double freq) noexcept
    {
        double cycles = (freq * buffer_len) / sample_rate;
        double rounded_cycles = round(cycles);
        double nearest_freq = rounded_cycles * sample_rate / buffer_len;

        return nearest_freq;
    }


    double Signal::releaseCoef(
            double start_level,
            double end_level,
            double min_level,
            int32_t sample_rate,
            double duration_seconds) noexcept
    {
        return releaseCoef(
                start_level,
                end_level,
                min_level,
                static_cast<int64_t>(duration_seconds * sample_rate));
    }


    double Signal::releaseCoef(
            double start_level,
            double end_level,
            double min_level,
            int64_t sample_count) noexcept
    {
        if (sample_count < 1) {
            return 0.0;
        }

        if (start_level < min_level) {
            start_level = min_level;
        }

        if (end_level < min_level) {
            end_level = min_level;
        }

        return std::clamp<double>(1.0 + (std::log(end_level) - std::log(start_level)) / sample_count, 0, 1);
    }


    double Signal::releaseLen(
            double start_level,
            double end_level,
            double min_level,
            double coef) noexcept
    {
        if (start_level < min_level) {
            start_level = min_level;
        }

        if (end_level < min_level) {
            end_level = min_level;
        }

        double len = std::log10(end_level / start_level) / std::log10(coef);
        return len < 0.0 ? 0.0 : len;
    }


    double Signal::releaseValue(double start_level, double coef, int64_t t) noexcept {
        return start_level * std::pow(coef, t);
    }


    int64_t Signal::_updateSimplified() noexcept {
        int64_t len = 0;

        // Be sure there is a SimplifiedSignal for each channel in signal.
        while (m_simplified_signals.size() < m_channel_count) {
            m_simplified_signals.push(new SimplifiedSignal());
        }

        for (int32_t channel = 0; channel < m_channel_count; channel++) {
            auto simplified_signal = m_simplified_signals.elementAtIndex(channel);
            if (simplified_signal != nullptr) {
                simplified_signal->update(this, channel);
                len = simplified_signal->len();
            }
        }

        return len;
    }


    int8_t _read_i8_as_i8(SignalSamplePtr data, int64_t index) { return data.i8[index]; }
    int16_t _read_i8_as_i16(SignalSamplePtr data, int64_t index) { return static_cast<int16_t>(data.i8[index]) * 256; }
    int32_t _read_i8_as_i32(SignalSamplePtr data, int64_t index) { return static_cast<int32_t>(data.i8[index]) * 16777216; }
    float _read_i8_as_f32(SignalSamplePtr data, int64_t index) { return static_cast<float>(data.i8[index]) / 128.0f; }
    double _read_i8_as_f64(SignalSamplePtr data, int64_t index) { return static_cast<double>(data.i8[index]) / 128.0; }

    int8_t _read_i16_as_i8(SignalSamplePtr data, int64_t index) { return static_cast<int8_t>(data.i16[index] / 256); }
    int16_t _read_i16_as_i16(SignalSamplePtr data, int64_t index) { return data.i16[index]; }
    int32_t _read_i16_as_i32(SignalSamplePtr data, int64_t index) { return static_cast<int32_t>(data.i16[index]) * 65536; }
    float _read_i16_as_f32(SignalSamplePtr data, int64_t index) { return static_cast<float>(data.i16[index]) / 32768.0f; }
    double _read_i16_as_f64(SignalSamplePtr data, int64_t index) { return static_cast<double>(data.i16[index]) / 32768.0; }

    int8_t _read_i32_as_i8(SignalSamplePtr data, int64_t index) { return static_cast<int8_t>(data.i32[index] / 16777216); }
    int16_t _read_i32_as_i16(SignalSamplePtr data, int64_t index) { return static_cast<int16_t>(data.i32[index] / 65536); }
    int32_t _read_i32_as_i32(SignalSamplePtr data, int64_t index) { return data.i32[index]; }
    float _read_i32_as_f32(SignalSamplePtr data, int64_t index) { return static_cast<float>(data.i32[index]) / 2147483648.0f; }
    double _read_i32_as_f64(SignalSamplePtr data, int64_t index) { return static_cast<double>(data.i32[index]) / 2147483648.0; }

    int8_t _read_f32_as_i8(SignalSamplePtr data, int64_t index) { return static_cast<int8_t>(data.f32[index] * 127.0f); }
    int16_t _read_f32_as_i16(SignalSamplePtr data, int64_t index) { return static_cast<int16_t>(data.f32[index] * 32767.0f); }
    int32_t _read_f32_as_i32(SignalSamplePtr data, int64_t index) { return static_cast<int32_t>(data.f32[index] * 2147483647.0f); }
    float _read_f32_as_f32(SignalSamplePtr data, int64_t index) { return data.f32[index]; }
    double _read_f32_as_f64(SignalSamplePtr data, int64_t index) { return static_cast<double>(data.f32[index]); }

    int8_t _read_f64_as_i8(SignalSamplePtr data, int64_t index) { return static_cast<int8_t>(data.f64[index] * 127.0); }
    int16_t _read_f64_as_i16(SignalSamplePtr data, int64_t index) { return static_cast<int16_t>(data.f64[index] * 32767.0); }
    int32_t _read_f64_as_i32(SignalSamplePtr data, int64_t index) { return static_cast<int32_t>(data.f64[index] * 2147483647.0); }
    float _read_f64_as_f32(SignalSamplePtr data, int64_t index) { return static_cast<float>(data.f64[index]); }
    double _read_f64_as_f64(SignalSamplePtr data, int64_t index) { return data.f64[index]; }

    void _writer_i8_as_i8(SignalSamplePtr data, int64_t index, int8_t value) { data.i8[index] = value; }
    void _writer_i8_as_i16(SignalSamplePtr data, int64_t index, int8_t value) { data.i16[index] = static_cast<int16_t>(value) * 256; }
    void _writer_i8_as_i32(SignalSamplePtr data, int64_t index, int8_t value) { data.i32[index] = static_cast<int32_t>(value) * 16777216; }
    void _writer_i8_as_f32(SignalSamplePtr data, int64_t index, int8_t value) { data.f32[index] = static_cast<float>(value) / 128.0f; }
    void _writer_i8_as_f64(SignalSamplePtr data, int64_t index, int8_t value) { data.f64[index] = static_cast<double>(value) / 128.0f; }

    void _writer_i16_as_i8(SignalSamplePtr data, int64_t index, int16_t value) { data.i8[index] = static_cast<int8_t>(value / 256); }
    void _writer_i16_as_i16(SignalSamplePtr data, int64_t index, int16_t value) { data.i16[index] = value; }
    void _writer_i16_as_i32(SignalSamplePtr data, int64_t index, int16_t value) { data.i32[index] = static_cast<int32_t>(value) * 65536; }
    void _writer_i16_as_f32(SignalSamplePtr data, int64_t index, int16_t value) { data.f32[index] = static_cast<float>(value) / 32768.0f; }
    void _writer_i16_as_f64(SignalSamplePtr data, int64_t index, int16_t value) { data.f64[index] = static_cast<double>(value) / 32768.0f; }

    void _writer_i32_as_i8(SignalSamplePtr data, int64_t index, int32_t value) { data.i8[index] = static_cast<int8_t>(value / 16777216); }
    void _writer_i32_as_i16(SignalSamplePtr data, int64_t index, int32_t value) { data.i16[index] = static_cast<int32_t>(value / 65536); }
    void _writer_i32_as_i32(SignalSamplePtr data, int64_t index, int32_t value) { data.i32[index] = value; }
    void _writer_i32_as_f32(SignalSamplePtr data, int64_t index, int32_t value) { data.f32[index] = static_cast<float>(value) / 2147483648.0f; }
    void _writer_i32_as_f64(SignalSamplePtr data, int64_t index, int32_t value) { data.f64[index] = static_cast<double>(value) / 2147483648.0f; }

    void _writer_f32_as_i8(SignalSamplePtr data, int64_t index, float value) { data.i8[index] = static_cast<int8_t>(value * 127.0f); }
    void _writer_f32_as_i16(SignalSamplePtr data, int64_t index, float value) { data.i16[index] = static_cast<int32_t>(value * 32767.0f); }
    void _writer_f32_as_i32(SignalSamplePtr data, int64_t index, float value) { data.i32[index] = static_cast<int32_t>(value * 2147483647.0f); }
    void _writer_f32_as_f32(SignalSamplePtr data, int64_t index, float value) { data.f32[index] = value; }
    void _writer_f32_as_f64(SignalSamplePtr data, int64_t index, float value) { data.f64[index] = static_cast<double>(value); }

    void _writer_f64_as_i8(SignalSamplePtr data, int64_t index, double value) { data.i8[index] = static_cast<int8_t>(value * 127.0); }
    void _writer_f64_as_i16(SignalSamplePtr data, int64_t index, double value) { data.i16[index] = static_cast<int32_t>(value * 32767.0); }
    void _writer_f64_as_i32(SignalSamplePtr data, int64_t index, double value) { data.i32[index] = static_cast<int32_t>(value * 2147483647.0); }
    void _writer_f64_as_f32(SignalSamplePtr data, int64_t index, double value) { data.f32[index] = static_cast<float>(value); }
    void _writer_f64_as_f64(SignalSamplePtr data, int64_t index, double value) { data.f64[index] = value; }

    void Signal::_updateAccessors() {
        switch (m_data_type) {
            case DataType::Int8: {
                _m_reader_i8 =  _read_i8_as_i8;
                _m_reader_i16 = _read_i8_as_i16;
                _m_reader_i32 = _read_i8_as_i32;
                _m_reader_f32 = _read_i8_as_f32;
                _m_reader_f64 = _read_i8_as_f64;
                _m_writer_i8 = _writer_i8_as_i8;
                _m_writer_i16 = _writer_i16_as_i8;
                _m_writer_i32 = _writer_i32_as_i8;
                _m_writer_f32 = _writer_f32_as_i8;
                _m_writer_f64 = _writer_f64_as_i8;
                break;
            }
            case DataType::Int16: {
                _m_reader_i8 =  _read_i16_as_i8;
                _m_reader_i16 = _read_i16_as_i16;
                _m_reader_i32 = _read_i16_as_i32;
                _m_reader_f32 = _read_i16_as_f32;
                _m_reader_f64 = _read_i16_as_f64;
                _m_writer_i8 = _writer_i8_as_i16;
                _m_writer_i16 = _writer_i16_as_i16;
                _m_writer_i32 = _writer_i32_as_i16;
                _m_writer_f32 = _writer_f32_as_i16;
                _m_writer_f64 = _writer_f64_as_i16;
                break;
            }
            case DataType::Int32: {
                _m_reader_i8 =  _read_i32_as_i8;
                _m_reader_i16 = _read_i32_as_i16;
                _m_reader_i32 = _read_i32_as_i32;
                _m_reader_f32 = _read_i32_as_f32;
                _m_reader_f64 = _read_i32_as_f64;
                _m_writer_i8 = _writer_i8_as_i32;
                _m_writer_i16 = _writer_i16_as_i32;
                _m_writer_i32 = _writer_i32_as_i32;
                _m_writer_f32 = _writer_f32_as_i32;
                _m_writer_f64 = _writer_f64_as_i32;
                break;
            }
            case DataType::Float: {
                _m_reader_i8 =  _read_f32_as_i8;
                _m_reader_i16 = _read_f32_as_i16;
                _m_reader_i32 = _read_f32_as_i32;
                _m_reader_f32 = _read_f32_as_f32;
                _m_reader_f64 = _read_f32_as_f64;
                _m_writer_i8 = _writer_i8_as_f32;
                _m_writer_i16 = _writer_i16_as_f32;
                _m_writer_i32 = _writer_i32_as_f32;
                _m_writer_f32 = _writer_f32_as_f32;
                _m_writer_f64 = _writer_f64_as_f32;
                break;
            }
            case DataType::Double: {
                _m_reader_i8 =  _read_f64_as_i8;
                _m_reader_i16 = _read_f64_as_i16;
                _m_reader_i32 = _read_f64_as_i32;
                _m_reader_f32 = _read_f64_as_f32;
                _m_reader_f64 = _read_f64_as_f64;
                _m_writer_i8 = _writer_i8_as_f64;
                _m_writer_i16 = _writer_i16_as_f64;
                _m_writer_i32 = _writer_i32_as_f64;
                _m_writer_f32 = _writer_f32_as_f64;
                _m_writer_f64 = _writer_f64_as_f64;
                break;
            }
            default:
                break;
        }
    }


    SignalConvolveSetup::SignalConvolveSetup(int64_t ir_len, int32_t partition_len) {
        checkSettings(ir_len, partition_len);
    }


    ErrorCode SignalConvolveSetup::checkSettings(int64_t ir_len, int32_t partition_len) noexcept {
        if (ir_len == m_ir_len && partition_len == m_partition_len) {
            return ErrorCode::None;
        }

        int32_t new_partition_log_n = std::clamp<int32_t>(Math::nextLog2(partition_len), FFT::kMinLogN, FFT::kMaxLogN);
        int64_t new_partition_len = 1L << m_partition_log_n;
        int32_t new_partition_count = std::max(static_cast<int32_t>((ir_len + new_partition_len - 1) / new_partition_len), 1);

        if (new_partition_log_n == m_partition_log_n &&
            new_partition_len == m_partition_len &&
            new_partition_count == m_partition_count) {
            return ErrorCode::None;
        }

        m_partition_log_n = new_partition_log_n;
        m_partition_len = new_partition_len;
        m_partition_count = new_partition_count;
        m_fft_len = static_cast<int32_t>(Math::nextPowerOfTwo(2 * m_partition_len));
        m_fft_half_len = m_fft_len / 2;
        m_fft_log = Math::log2IfPowerOfTwo(m_fft_len);
        m_overlap_len = m_fft_len - partition_len;

        m_time_buffer = static_cast<float*>(std::calloc(m_fft_len, sizeof(float)));
        m_interleaved_buffer = static_cast<float*>(std::calloc(m_fft_len, sizeof(float)));
        m_write_buffer = static_cast<float*>(std::calloc(m_partition_len, sizeof(float)));
        m_overlap_buffer = static_cast<float*>(std::calloc(m_overlap_len > 0 ? m_overlap_len : 0, sizeof(float)));
        m_t_out = static_cast<float*>(std::calloc(m_fft_len, sizeof(float)));
        m_ir_partials = new(std::nothrow) FFTComplexSplitArray(m_partition_count, m_fft_half_len);
        m_x_ring = new(std::nothrow) FFTComplexSplitArray(m_partition_count, m_fft_half_len);
        m_y_freq = new(std::nothrow) FFTComplexSplit(m_fft_half_len);

        return ErrorCode::None;
    }


    void SignalConvolveSetup::freeMemory() noexcept {
    }

} // End of namespace Grain
