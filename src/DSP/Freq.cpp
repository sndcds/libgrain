//
//  Freq.hpp
//
//  Created by Roald Christesen on 30.01.2018
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "DSP/Freq.hpp"
#include "Type/Type.hpp"


namespace Grain {

    /**
     *  @brief Converts a logarithmic frequency value to a position along a
     *         given axis.
     *
     *  This function maps a frequency in Hz to a position (e.g., pixel
     *  position) using a logarithmic scale. It assumes the mapping is done
     *  between two frequencies (`low_freq` to `high_freq`) and their
     *  corresponding positions (`low_pos` to `high_pos`).
     *
     *  @param freq The frequency to convert (Hz).
     *  @param low_freq The lower bound frequency (Hz). Must be > 0 and <
     *                  high_freq.
     *  @param high_freq The upper bound frequency (Hz).
     *  @param low_pos The position corresponding to low_freq (e.g., top of
     *                 screen).
     *  @param high_pos The position corresponding to high_freq (e.g., bottom of
     *                  screen).
     *  @return The position corresponding to the input frequency on a
     *          logarithmic scale. Returns -1.0 if input frequencies are
     *          invalid.
     */
    double Freq::freqToPos(double freq, double low_freq, double high_freq, double low_pos, double high_pos) noexcept {
        if (low_freq >= high_freq || low_freq <= 0.0) {
            return 0.0;
        }
        const double f_max = std::log(high_freq / low_freq) / static_cast<float>(std::numbers::ln2);
        double f = (std::log(freq / low_freq) / static_cast<double>(std::numbers::ln2)) / f_max;
        return low_pos + f * (high_pos - low_pos);
    }


    /**
     *  @brief Converts a position value back to a frequency using a logarithmic
     *         scale.
     *
     *  This function performs the inverse operation of `freqToPos`, converting
     *  a position (e.g., pixel coordinate) back to its corresponding frequency
     *  in Hz based on a logarithmic mapping between `low_freq` and `high_freq`.
     *
     *  @param pos The position to convert (e.g., a pixel y-coordinate).
     *  @param low_freq The lower bound frequency (Hz).
     *  @param high_freq The upper bound frequency (Hz).
     *  @param low_pos The position corresponding to low_freq.
     *  @param high_pos The position corresponding to high_freq.
     *  @return The frequency (Hz) corresponding to the given position.
     */
    double Freq::posToFreq(double pos, double low_freq, double high_freq, double low_pos, double high_pos) noexcept {
        const double f_max = std::log(high_freq / low_freq) / static_cast<double>(std::numbers::ln2);
        return std::exp((((pos - low_pos) / (high_pos - low_pos)) * f_max) * static_cast<double>(std::numbers::ln2)) * low_freq;
    }


    void FreqRange::set(float min_freq, float max_freq) noexcept {

        m_min_freq = min_freq;
        m_max_freq = max_freq;
        m_freq_range = max_freq - min_freq;
        m_max_div_min = max_freq / min_freq;
        m_log_min_freq = log(m_min_freq);
        m_log_max_freq = log(m_max_freq);
        m_freq_range_f = 1.0f / m_freq_range;
    }


    float FreqRange::lerpFreq(float t) const noexcept {

        return t * m_freq_range + m_min_freq;
    }

    /**
     *  @brief Converts a frequency in Hz to a normalized linear value [0.0, 1.0]
     *         based on a logarithmic scale between m_min_freq and m_max_freq.
     *
     *  This function maps a given frequency to its corresponding position on a
     *  logarithmic scale normalized to the range [0.0, 1.0]. The scale is defined
     *  by m_min_freq as the lower bound and m_max_freq as the upper bound.
     *
     *  @param freq Frequency in Hz to be converted. Must be within the range
     *              [m_min_freq, m_max_freq].
     *  @return Normalized logarithmic value in the range [0.0, 1.0].
     *
     *  @note The function assumes that m_max_div_min = m_max_freq / m_min_freq
     *        is precomputed and valid (i.e., > 0).
     *
     *  @see logToLinFreq(), tToFreq()
     */
    float FreqRange::freqToLogNrmScale(float freq) const {

        return (freq - m_min_freq) * m_freq_range_f;
    }


    /**
     *  @brief Converts a frequency in Hz to a normalized linear value [0.0, 1.0]
     *         based on a linear scale between m_min_freq and m_max_freq.
     *
     *  Unlike freqToLin(), this function assumes a **linear frequency mapping**, not
     *  logarithmic. It returns the normalized position of the frequency within the
     *  linear range, using a precomputed scaling factor.
     *
     *  @param freq Frequency in Hz to be converted.
     *  @return Normalized linear value in the range [0.0, 1.0], or println-of-bounds
     *          if freq is outside [m_min_freq, m_max_freq].
     *
     *  @note m_freq_range_f is assumed to be the reciprocal of the frequency range:
     *        1.0 / (m_max_freq - m_min_freq).
     *
     *  @see lerpFreq(), freqToLin()
     */
    float FreqRange::freqToLinNrmScale(float freq) const {

        return std::log(freq / m_min_freq) / std::log(m_max_div_min);
    }

    float FreqRange::freqToMel(float freq) const noexcept {

        float mel = 2595.0f * std::log10(1.0f + freq / 700.0f);
        float mel_min = 2595.0f * std::log10(1.0f + m_min_freq / 700.0f);
        float mel_max = 2595.0f * std::log10(1.0f + m_max_freq / 700.0f);
        return (mel - mel_min) / (mel_max - mel_min);
    }


    float FreqRange::melToFreq(float mel_nrm) const noexcept {

        float mel_min = 2595.0f * std::log10(1.0f + m_min_freq / 700.0f);
        float mel_max = 2595.0f * std::log10(1.0f + m_max_freq / 700.0f);
        float mel = mel_nrm * (mel_max - mel_min) + mel_min;
        return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
    }


    float FreqRange::freqToBark(float freq) const noexcept {

        float bark = 7.0f * std::asinh(freq / 650.0f);
        float bark_min = 7.0f * std::asinh(m_min_freq / 650.0f);
        float bark_max = 7.0f * std::asinh(m_max_freq / 650.0f);
        return (bark - bark_min) / (bark_max - bark_min);
    }


    float FreqRange::barkToFreq(float bark_nrm) const noexcept {

        float bark_min = 7.0f * std::asinh(m_min_freq / 650.0f);
        float bark_max = 7.0f * std::asinh(m_max_freq / 650.0f);
        float bark = bark_nrm * (bark_max - bark_min) + bark_min;
        return 650.0f * std::sinh(bark / 7.0f);
    }


    float FreqRange::freqToERB(float freq) const noexcept {

        float erb = 21.4f * std::log10(0.00437f * freq + 1.0f);
        float erb_min = 21.4f * std::log10(0.00437f * m_min_freq + 1.0f);
        float erb_max = 21.4f * std::log10(0.00437f * m_max_freq + 1.0f);
        return (erb - erb_min) / (erb_max - erb_min);
    }


    float FreqRange::erbToFreq(float erb_nrm) const noexcept {

        float erb_min = 21.4f * std::log10(0.00437f * m_min_freq + 1.0f);
        float erb_max = 21.4f * std::log10(0.00437f * m_max_freq + 1.0f);
        float erb = erb_nrm * (erb_max - erb_min) + erb_min;
        return (std::pow(10.0f, erb / 21.4f) - 1.0f) / 0.00437f;
    }


    float FreqRange::freqToScale(float freq, Freq::Scale scale) const noexcept {

        switch (scale) {
            case Freq::Scale::Logarithmic: return freqToLogNrmScale(freq);
            case Freq::Scale::Mel: return freqToMel(freq);
            case Freq::Scale::Bark: return freqToBark(freq);
            case Freq::Scale::ERB: return freqToERB(freq);
            default: return freqToLinNrmScale(freq);
        }
    }


    float FreqRange::logToLinFreq(float log_nrm) const noexcept {

        return m_min_freq * std::pow(m_max_div_min, log_nrm);
    }


    /**
     *  @brief Convert linear scale to logarithmic scale.
     */
    float FreqRange::linToLog(float lin_nrm) const noexcept {

        float freq = m_min_freq * std::pow(m_max_div_min, lin_nrm);
        return (freq - m_min_freq) * m_freq_range_f;
    }


    /**
     *  @brief Convert linear scale to Mel scale.
     *
     *  The mel scale (after the word melody) is a perceptual scale of pitches
     *  judged by listeners to be equal in distance from one another. The
     *  reference point between this scale and normal frequency measurement is
     *  defined by assigning a perceptual pitch of 1000 mels to a 1000 Hz tone,
     *  40 dB above the listener's threshold. Above about 500 Hz, increasingly
     *  large intervals are judged by listeners to produce equal pitch increments.
     */
    float FreqRange::linToMel(float lin_nrm) const noexcept {

        float freq = m_min_freq * std::pow(m_max_freq / m_min_freq, lin_nrm);
        float mel = 2595.0f * std::log10(1.0f + freq / 700.0f);
        float mel_min = 2595.0f * std::log10(1.0f + m_min_freq / 700.0f);
        float mel_max = 2595.0f * std::log10(1.0f + m_max_freq / 700.0f);
        return (mel - mel_min) / (mel_max - mel_min);
    }


    /**
     *  @brief Convert linear scale to Bark scale.
     *
     *  The human auditory system consists of a series of bandpass filters
     *  (filters that have a certain center frequency and that increasingly
     *  attenuate signals as they deviate from that cf). The bandwidth of these
     *  auditory filters increases with higher frequencies and the precision of
     *  the frequency perception decreases. Taking into account this property
     *  of the human auditory system, the Bark scale is a perceptually realistic
     *  scale of frequency.
     */
    float FreqRange::linToBark(float lin_nrm) const noexcept {

        float freq = m_min_freq * std::pow(m_max_freq / m_min_freq, lin_nrm);
        float bark = 7.0f * std::asinh(freq / 650.0f);
        float bark_min = 7.0f * std::asinh(m_min_freq / 650.0f);
        float bark_max = 7.0f * std::asinh(m_max_freq / 650.0f);
        return (bark - bark_min) / (bark_max - bark_min);
    }


    /**
     *  @brief Convert linear scale to ERB scale.
     *
     *  The equivalent rectangular bandwidth or ERB is a measure used in
     *  psychoacoustics, which gives an approximation to the bandwidths of the
     *  filters in human hearing, using the unrealistic but convenient
     *  simplification of modeling the filters as rectangular band-pass filters,
     *  or band-stop filters, like in tailor-made notched music training (TMNMT).
     */
    float FreqRange::linToERB(float lin_nrm) const noexcept {

        float freq = m_min_freq * std::pow(m_max_freq / m_min_freq, lin_nrm);
        float erb = 21.4f * std::log10(0.00437f * freq + 1.0f);
        float erb_min = 21.4f * std::log10(0.00437f * m_min_freq + 1.0f);
        float erb_max = 21.4f * std::log10(0.00437f * m_max_freq + 1.0f);
        return (erb - erb_min) / (erb_max - erb_min);
    }


    /**
     *  @brief Converts a normalized value `t` in the range [0.0, 1.0] to a
     *         frequency in Hz according to the specified perceptual or linear
     *         frequency scale.
     *
     *  This function maps the normalized position `t` (typically derived from
     *  screen or image coordinates) to a frequency value using the chosen
     *  scale. This is useful for generating frequency axis mappings for
     *  spectrograms or visualizations where perceptual spacing (e.g., Mel,
     *  Bark, ERB) is desired instead of linear frequency.
     *
     *  @param t Normalized position in the range [0.0, 1.0], where 0.0
     *           corresponds to the bottom of the range and 1.0 to the top.
     *  @param scale The frequency scale to use for mapping. Options include:
     *               - Freq::Scale::Linear — linear frequency
     *               - Freq::Scale::Logarithmic — logarithmic frequency
     *               - Freq::Scale::Mel — Mel scale
     *               - Freq::Scale::Bark — Bark scale
     *               - Freq::Scale::ERB — Equivalent Rectangular Bandwidth (ERB) scale
     *
     *  @return Frequency in Hz corresponding to the normalized `t` in the
     *          selected scale.
     *
     *  @note If the scale is not recognized, defaults to linear interpolation
     *        (`lerpFreq`).
     *
     *  @see lerpFreq(), logToLinFreq(), melToFreq(), barkToFreq(), erbToFreq()
     */
    float FreqRange::tToFreq(const float t, Freq::Scale scale) const {

        switch (scale) {
            case Freq::Scale::Logarithmic:
                return logToLinFreq(t);
            case Freq::Scale::Mel:
                return melToFreq(t);
            case Freq::Scale::Bark:
                return barkToFreq(t);
            case Freq::Scale::ERB:
                return erbToFreq(t);
            case Freq::Scale::Linear:
            default:
                return lerpFreq(t);
        }
    }


    void FreqBands::set(float left_freq, float center_freq, float right_freq) noexcept {
        left_freq_ = left_freq;
        center_freq_ = center_freq;
        right_freq_ = right_freq;
    }


    void FreqBands::setupBands(float bands_per_octave, float start_freq, float end_freq) noexcept {
        bands_per_octave_ = std::clamp(bands_per_octave, 0.0f, 1000.0f);
        band_start_freq_ = std::clamp(start_freq, 10.0f, 20000.0f);
        band_end_freq_ = std::clamp(end_freq, 10.0f, 20000.0f);

        if (band_start_freq_ > band_end_freq_) {
            std::swap(band_start_freq_, band_end_freq_);
        }

        int32_t band_index = 0;
        while (band_index < 1000) {
            float a = std::pow(2.0f, 1.0f / bands_per_octave);
            float c_freq = start_freq * std::pow(a, static_cast<float>(band_index));
            if (c_freq > end_freq) {
                band_count_ = band_index + 1;
                break;
            }
            band_index++;
        }
    }


    bool FreqBands::setBand(int32_t index) noexcept {

        if (index >= 0 && index < band_count_) {
            float a = std::pow(2.0f, 1.0f / bands_per_octave_);

            center_freq_ = band_start_freq_ * std::pow(a, static_cast<float>(index));
            left_freq_ = center_freq_ / a;
            right_freq_ = center_freq_ * a;

            return true;
        }
        else {
            return false;
        }
    }


    void FreqBands::setCenterFreq(float center_freq, float octave_range) noexcept {

        octave_range = std::clamp(octave_range, 0.01f, 10.0f);
        center_freq = std::clamp(center_freq, 0.01f, 20000.0f);

        float a = std::pow(2.0f, octave_range);

        center_freq_ = center_freq;
        left_freq_ = a != 0.0f ? center_freq / a : 0.0f;
        right_freq_ = center_freq * a;
    }


} // End of namespace Grain
