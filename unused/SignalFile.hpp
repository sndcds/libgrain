//
//  SignalFile.hpp
//
//  Created by Roald Christesen on 25.02.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainSignalFile_hpp
#define GrainSignalFile_hpp

#include "Grain.hpp"
#include "Type/Type.hpp"
#include "File/File.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #import <AudioToolbox/AudioToolbox.h>
#endif


namespace Grain {

    class Signal;

    enum class SignalFileType {
        Unknown = -1,
        AIFF,
        AIFC,
        MP3,
        WAVE,
        AAC,	    ///< Apple Advanced Audio Coding
        CAF,        ///< Apple Core Audio Format
        Count,
        First = Count - 1,
        Last = CAF
    };

    enum class SignalFilePreset {
        Wave16 = 0,
        Wave24,
        Wave32,
        Wave32Float,
        AIFF8,
        AIFF16,
        AIFF24,
        AIFF32,
        AIFF32Float,
        AIFC8,
        AIFC16,
        AIFC24,
        AIFC32,
        AIFC32Float,
        MP3,
        AAC,
        CAF16,
        CAF24,

        Count,
        Last = Count - 1,
        First = 0,

        Wave = Wave16,      ///< Standard Wave Preset
        AIFF = AIFF24,      ///< Standard AIFF Preset
        AIFC = AIFC24       ///< Standard AIFC Preset
    };

    typedef struct SignalFileWriteSettings {
        SignalFileType m_file_type = SignalFileType::AIFF;
        int32_t m_bits_per_sample = 24;
        bool m_floating_point = false;

    public:
        [[nodiscard]] const char* fileTypeName() const noexcept {
            switch (m_file_type) {
                case SignalFileType::AIFF: return "AIFF";
                case SignalFileType::AIFC: return "AIFC";
                case SignalFileType::WAVE: return "WAVE";
                case SignalFileType::MP3: return "MP3";
                case SignalFileType::AAC: return "AAC";
                case SignalFileType::CAF: return "CAF";
                default: return "Unknown";
            }
        }

        [[nodiscard]] const char* fileExt() const noexcept {
            switch (m_file_type) {
                case SignalFileType::AIFF: return "aiff";
                case SignalFileType::AIFC: return "aifc";
                case SignalFileType::WAVE: return "wav";
                case SignalFileType::MP3: return "mp3";
                case SignalFileType::AAC: return "aac";
                case SignalFileType::CAF: return "caf";
                default: return "";
            }
        }

        [[nodiscard]] int32_t bitDepth() const noexcept { return m_bits_per_sample; }

        void filePath(const String& dir_path, const String& file_name, String& out_file_path) {
            out_file_path.setFormatted(1024, "%s/%s-%s-%dbit%s.%s", dir_path.utf8(), file_name.utf8(), fileTypeName(), bitDepth(), m_floating_point ? "-float" : "", fileExt());
        }

    } SignalFileWriteSettings;


    class SignalFile : public File {

    public:

        enum {
            FLAG_FLOAT = 0x1,
            FLAG_BIG_ENDIAN = 0x2,
            FLAG_SIGNED_INT = 0x4,
            FLAG_PACKED = 0x8,
            FLAG_HIGH_ALIGNED = 0x10,
            FLAG_NON_INTERLEAVED = 0x20,
            FLAG_NON_MIXABLE = 0x40
        };

        enum {
            kErrScanFailed = 0,
            kErrUnknownFormat,
            kErrUnsupportedFormatMP3,
            kErrNoSampleData,
            kErrToManyChannels,
            kErrBufferSetupFailed,
            kErrUnsupportedChannelCount,
            kErrUnsupportedDataType,
            kErrExtAudioFileAllreadyOpen,
            kErrExtAudioFileDisposeFailed
        };

    public:
        explicit SignalFile(const String& file_path);
        ~SignalFile() override = default;

        [[nodiscard]] const char* className() const noexcept override { return "SignalFile"; }


        void start(int32_t flags) override;

        void setSignalToWrite(const Signal* signal);
        void setWriteSettings(SignalFileWriteSettings write_settings);

        static const bool writeSettingByPreset(SignalFilePreset preset, SignalFileWriteSettings& out_settings) noexcept {
            if (preset >= SignalFilePreset::First && preset <= SignalFilePreset::Last) {
                out_settings = g_std_file_write_settings[static_cast<int32_t>(preset)];
                return true;
            }
            else {
                out_settings = g_std_file_write_settings[static_cast<int32_t>(SignalFilePreset::AIFF16)];
                return false;
            }
        }


        void close() override;


        [[nodiscard]] double signalDuration() const noexcept;

        fourcc_t signalFormatId() const noexcept { return m_signal_file_format_id; }
        double signalSampleRate() const noexcept { return m_signal_sample_rate; }
        int32_t signalSampleCount() const noexcept { return m_signal_sample_count; }
        int32_t signalChannelCount() const noexcept { return m_signal_channel_count; }
        int32_t signalBitDepth() const noexcept { return m_signal_bits_per_channel; }

        ErrorCode scan();

        void read(Signal* signal);
        void read(Signal* signal, int32_t offset, int32_t length);

        void write(const Signal* signal);
        void write(const Signal* signal, int32_t offset, int32_t length);

    protected:
        SignalFileType m_signal_file_type = SignalFileType::Unknown;

        fourcc_t m_signal_file_format_id = 0;
        double m_signal_sample_rate = 0.0;
        uint32_t m_signal_flags = 0x0;
        int32_t m_signal_sample_count = 0;  ///< Number of samples per channel.
        int32_t m_signal_channel_count = 0; ///< Number of channels.

        int32_t m_signal_bytes_per_packet = 0;
        int32_t m_signal_frames_per_packet = 0;
        int32_t m_signal_bytes_per_frame = 0;
        int32_t m_signal_bits_per_channel = 0;

        bool scan_done_ = false;
        ErrorCode m_scan_err_code = ErrorCode::None;
        DataType m_write_data_type = DataType::Int16;

        SignalFileWriteSettings m_signal_file_write_settings;

        static SignalFileWriteSettings g_std_file_write_settings[];

        #if defined(__APPLE__) && defined(__MACH__)
            ExtAudioFileRef m_ext_audio_file_ref = nullptr;
        #endif
    };


} // End of namespace Grain

#endif // GrainSignalFile_hpp
