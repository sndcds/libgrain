//
//  SignalFile.hpp
//
//  Created by Roald Christesen on 25.02.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalFile.hpp"
// #include <App.hpp> TODO: !!!!
#include "File/File.hpp"
#include "Signal/Signal.hpp"


namespace Grain {

    SignalFileWriteSettings SignalFile::g_std_file_write_settings[] = {
        { SignalFileType::WAVE, 16, false },
        { SignalFileType::WAVE, 24, false },
        { SignalFileType::WAVE, 32, false },
        { SignalFileType::WAVE, 32, true  },
        { SignalFileType::AIFF,  8, false },
        { SignalFileType::AIFF, 16, false },
        { SignalFileType::AIFF, 24, false },
        { SignalFileType::AIFF, 32, false },
        { SignalFileType::AIFF, 32, true  },
        { SignalFileType::AIFC,  8, false },
        { SignalFileType::AIFC, 16, false },
        { SignalFileType::AIFC, 24, false },
        { SignalFileType::AIFC, 32, false },
        { SignalFileType::AIFC, 32, true  },
        { SignalFileType::MP3,  16, false },
        { SignalFileType::AAC,  16, false },
        { SignalFileType::CAF,  16, false },
        { SignalFileType::CAF,  24, false }
    };


    SignalFile::SignalFile(const String& file_path) : File(file_path) {

    }


    void SignalFile::start(int32_t flags) {

        DeferredException deferred_exception;

        CFStringRef file_path = nullptr;
        CFURLRef file_url = nullptr;

        try {
            File::start(flags);

            if (m_read_flag) {
                if (!m_ext_audio_file_ref) {
                    file_path = CFStringCreateWithCString(kCFAllocatorDefault, m_file_path.utf8(), kCFStringEncodingUTF8);
                    if (!file_path) {
                        Exception::throwSpecific(0, "", m_file_path.utf8());
                    }

                    file_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file_path, kCFURLPOSIXPathStyle, false);

                    OSStatus status = ExtAudioFileOpenURL(file_url, &m_ext_audio_file_ref);
                    if (status != noErr || !m_ext_audio_file_ref) {
                        Exception::throwStandard(ErrorCode::FileCantOpen, m_file_path.utf8());
                    }

                    m_read_flag = true;
                }
            }
        }
        catch (const Exception& e) {
            deferred_exception.capture();
        }
        catch (...) {
            deferred_exception.createAndCaptureUnexpceted();
        }


        if (file_path) {
            CFRelease(file_path);
        }

        if (file_url) {
            CFRelease(file_url);
        }

        deferred_exception.rethrow();
    }


    void SignalFile::setSignalToWrite(const Signal* signal) {

        if (signal) {
            setWriteSettings(signal->fileWriteSettings());
        }
        else {
            SignalFileWriteSettings write_settings;
            write_settings.m_file_type = SignalFileType::AIFF;
            write_settings.m_bits_per_sample = 24;
            write_settings.m_floating_point = false;
            setWriteSettings(write_settings);
        }
    }


    void SignalFile::setWriteSettings(SignalFileWriteSettings write_settings) {

        if (!m_ext_audio_file_ref) {
            m_signal_file_write_settings = write_settings;
            m_ext_audio_file_ref = nullptr;
            m_write_flag = true;
        }
    }


    void SignalFile::close() {

        if (m_ext_audio_file_ref) {
            ExtAudioFileDispose(m_ext_audio_file_ref);
        }

        File::close();
    }


    double SignalFile::signalDuration() const noexcept {

        if (m_signal_sample_rate > 0.0 && m_signal_sample_count > 0) {
            return static_cast<double>(m_signal_sample_count) / m_signal_sample_rate;
        }
        else {
            return -1.0;
        }
    }


    ErrorCode SignalFile::scan() {

        if (scan_done_) {
            return m_scan_err_code;
        }

        checkBeforeReading();
        if (!m_ext_audio_file_ref) {
            throw ErrorCode::FileNoHandle;
        }


        m_signal_file_type = SignalFileType::Unknown;
        /* TODO:!!!!!
         if (hasAIFFSignature())
         mFileType = GrSignalFileType::AIFF;
         else if (hasAIFCSignature())
         mFileType = GrSignalFileType::AIFC;
         else if (hasWAVESignature())
         mFileType = GrSignalFileType::WAVE;
         */

        m_signal_file_format_id = 0;
        m_signal_sample_rate = 0.0;
        m_signal_flags = 0x0;
        m_signal_sample_count = 0;
        m_signal_channel_count = 0;

        m_signal_bytes_per_packet = 0;
        m_signal_frames_per_packet = 0;
        m_signal_bytes_per_frame = 0;
        m_signal_bits_per_channel = 0;

        m_scan_err_code = ErrorCode::None;


        // Local variables.

        OSStatus status = noErr;
        AudioStreamBasicDescription file_asbd;
        UInt32 property_size = 0;
        SInt64 file_length_in_frames = 0;

        try {
            // Get the input format
            property_size = sizeof(file_asbd);
            status = ExtAudioFileGetProperty(m_ext_audio_file_ref, kExtAudioFileProperty_FileDataFormat, &property_size, &file_asbd);
            if (status != noErr) {
                throw Error::specific(kErrUnknownFormat);
            }

            m_signal_file_format_id = file_asbd.mFormatID;

            if (m_signal_file_format_id == '.mp3') {
                m_signal_file_type = SignalFileType::MP3;
            }
            else if (m_signal_file_format_id == 'aac ') {
                m_signal_file_type = SignalFileType::AAC;
            }


            m_signal_sample_rate = file_asbd.mSampleRate;

            if (file_asbd.mFormatFlags & kAudioFormatFlagIsFloat) {
                m_signal_flags |= FLAG_FLOAT;
            }
            if (file_asbd.mFormatFlags & kAudioFormatFlagIsBigEndian) {
                m_signal_flags |= FLAG_BIG_ENDIAN;
            }
            if (file_asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger) {
                m_signal_flags |= FLAG_SIGNED_INT;
            }
            if (file_asbd.mFormatFlags & kAudioFormatFlagIsPacked) {
                m_signal_flags |= FLAG_PACKED;
            }
            if (file_asbd.mFormatFlags & kAudioFormatFlagIsAlignedHigh) {
                m_signal_flags |= FLAG_HIGH_ALIGNED;
            }
            if (file_asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
                m_signal_flags |= FLAG_NON_INTERLEAVED;
            }
            if (file_asbd.mFormatFlags & kAudioFormatFlagIsNonMixable) {
                m_signal_flags |= FLAG_NON_MIXABLE;
            }


            m_signal_bytes_per_packet = static_cast<int32_t>(file_asbd.mBytesPerPacket);
            m_signal_frames_per_packet = static_cast<int32_t>(file_asbd.mFramesPerPacket);
            m_signal_bytes_per_frame = static_cast<int32_t>(file_asbd.mBytesPerFrame);
            m_signal_channel_count = static_cast<int32_t>(file_asbd.mChannelsPerFrame);
            m_signal_bits_per_channel = static_cast<int32_t>(file_asbd.mBitsPerChannel);

            // Get the total frame count = samples per channel
            property_size = sizeof(file_length_in_frames);
            status = ExtAudioFileGetProperty(m_ext_audio_file_ref, kExtAudioFileProperty_FileLengthFrames, &property_size, &file_length_in_frames);
            if (status != noErr) {
                throw Error::specific(kErrUnknownFormat);
            }

            if (file_length_in_frames > std::numeric_limits<int32_t>::max()) {
                throw ErrorCode::UnsupportedFileSize;
            }

            m_signal_sample_count = static_cast<int64_t>(file_length_in_frames);
        }
        catch (const Exception& e) {
            m_scan_err_code = e.code();
        }
        catch (...) {
            m_scan_err_code = ErrorCode::Unknown;
        }


        scan_done_ = true;

        return m_scan_err_code;
    }


    void SignalFile::read(Signal* signal) {

        if (signal) {
            read(signal, 0, m_signal_sample_count);
        }
    }


    void SignalFile::read(Signal* signal, int32_t offset, int32_t length) {
        if (!scan_done_) {
            scan();
        }

        checkBeforeReading();
        if (!m_ext_audio_file_ref) {
            throw ErrorCode::FileNoHandle;
        }

        if (!signal || offset < 0 || length < 1) {
            throw ErrorCode::BadArgs;
        }

        if (m_scan_err_code != ErrorCode::None) {
            throw Error::specific(kErrScanFailed);
        }

        if (m_signal_sample_rate <= 0.0 || m_signal_channel_count <= 0) {
            throw Error::specific(kErrUnknownFormat);
        }

        if (m_signal_sample_count <= 0.0) {
            throw Error::specific(kErrNoSampleData);
        }

        if (m_signal_channel_count > Signal::kMaxChannelCount) {
            throw Error::specific(kErrToManyChannels);

        }

        if (offset >= m_signal_sample_count) {
            throw ErrorCode::BadArgs;
        }

        if (offset + length > m_signal_sample_count) {
            length = m_signal_sample_count - offset;
        }

        if (length < 0) {
            throw ErrorCode::BadArgs;
        }


        DataType data_type = signal->dataType();

        if (signal->configure(m_signal_channel_count, m_signal_sample_rate, length, data_type) != ErrorCode::None) {
            throw Error::specific(kErrBufferSetupFailed);
        }

        auto result = ErrorCode::None;
        OSStatus status = noErr;

        AudioStreamBasicDescription file_asbd;
        AudioStreamBasicDescription client_asbd;

        UInt32 property_size;

        try {

            // Get the input format.

            property_size = sizeof(file_asbd);
            status = ExtAudioFileGetProperty(m_ext_audio_file_ref, kExtAudioFileProperty_FileDataFormat, &property_size, &file_asbd);
            if (status != noErr) {
                throw ErrorCode::FileReadError;
            }


            // Set the client format.

            auto n_channels = static_cast<int32_t>(file_asbd.mChannelsPerFrame);
            int32_t bytes_per_sample = TypeInfo::byteSize(data_type);

            client_asbd = file_asbd;
            client_asbd.mSampleRate = file_asbd.mSampleRate;
            client_asbd.mFormatID = kAudioFormatLinearPCM;
            client_asbd.mFormatFlags = kAudioFormatFlagIsPacked;
            client_asbd.mBitsPerChannel = (UInt32)(bytes_per_sample * 8);
            client_asbd.mChannelsPerFrame = file_asbd.mChannelsPerFrame;
            client_asbd.mFramesPerPacket = 1;
            client_asbd.mBytesPerPacket = client_asbd.mBytesPerFrame = (UInt32)(n_channels * bytes_per_sample);

            /* For interleaved sample data this would be necessary
             clientASBD.mBytesPerPacket = clientASBD.mBytesPerFrame = (UInt32)bytesPerSample;
             clientASBD.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
             */
            if (data_type == DataType::Float) {
                client_asbd.mFormatFlags |= kAudioFormatFlagIsFloat;
            }
            else {
                client_asbd.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
            }


            property_size = sizeof(client_asbd);
            status = ExtAudioFileSetProperty(m_ext_audio_file_ref, kExtAudioFileProperty_ClientDataFormat, property_size, &client_asbd);
            if (status != noErr) {
                throw Error::specific(1);  // TODO: Code!
            }


            // Read all the data into memory.

            AudioBufferList buffer_list;
            buffer_list.mNumberBuffers = 1;
            buffer_list.mBuffers[0].mNumberChannels = client_asbd.mChannelsPerFrame;
            buffer_list.mBuffers[0].mData = signal->mutDataPtr();
            buffer_list.mBuffers[0].mDataByteSize = (UInt32)(length * client_asbd.mBytesPerFrame);


            auto loaded_packets = static_cast<UInt32>(length);

            ExtAudioFileSeek(m_ext_audio_file_ref, offset);
            status = ExtAudioFileRead(m_ext_audio_file_ref, &loaded_packets, &buffer_list);
            if (status != noErr) {
                Exception::throwStandard(ErrorCode::FileReadError);
            }
        }
        catch (const Exception& e) {
            result = e.code();
            if (result != ErrorCode::None) {
                throw;
            }
        }
        catch (...) {
            Exception::throwStandard(ErrorCode::Unknown);
        }
    }


    void SignalFile::write(const Signal* signal) {

        if (signal) {
            write(signal, 0, signal->length());
        }
    }


    void SignalFile::write(const Signal* signal, int32_t offset, int32_t length) {

        // TODO: Implement plattform independent formats ...

        DeferredException deferred_exception;

        CFStringRef file_path = nullptr;
        CFURLRef file_url = nullptr;

        try {
            OSStatus status = noErr;

            if (!m_write_flag) {
                Exception::throwStandard(ErrorCode::FileCantWrite);
            }

            if (!signal || offset < 0 || length < 1) {
                Exception::throwStandard(ErrorCode::BadArgs);
            }

            if (offset >= signal->length()) {
                Exception::throwStandard(ErrorCode::BadArgs);
            }

            if (offset + length > signal->length()) {
                length = signal->length() - offset;
            }

            if (length < 0) {
                Exception::throwStandard(ErrorCode::BadArgs);
            }

            if (m_ext_audio_file_ref) {
                Exception::throwSpecific(kErrExtAudioFileAllreadyOpen);
            }

            m_signal_sample_rate = signal->sampleRate();
            m_signal_channel_count = signal->channelCount();
            m_write_data_type = signal->dataType();

            file_path = CFStringCreateWithCString(kCFAllocatorDefault, m_file_path.utf8(), kCFStringEncodingUTF8);
            if (!file_path) {
                Exception::throwSpecific(0); // TODO: Code!
            }

            file_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file_path, kCFURLPOSIXPathStyle, false);
            if (!file_url) {
                Exception::throwSpecific(1); // TODO: Code!
            }

            AudioStreamBasicDescription client_asbd;
            client_asbd.mSampleRate = m_signal_sample_rate;
            client_asbd.mFormatID = kAudioFormatLinearPCM;
            client_asbd.mFormatFlags = kAudioFormatFlagIsPacked;

            if constexpr (std::endian::native == std::endian::big) {
                client_asbd.mFormatFlags |= kAudioFormatFlagIsBigEndian;
            }

            if (signal->isIntegerType()) {
                client_asbd.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
            }
            else {
                client_asbd.mFormatFlags |= kAudioFormatFlagIsFloat;
            }

            // kAudioFormatFlagIsNonInterleaved	is not set, because audio buffers always are interleaved
            client_asbd.mBytesPerPacket = signal->bytesPerSample() * m_signal_channel_count;
            client_asbd.mFramesPerPacket = 1;
            client_asbd.mBytesPerFrame = signal->bytesPerSample() * m_signal_channel_count;
            client_asbd.mChannelsPerFrame = m_signal_channel_count;
            client_asbd.mBitsPerChannel = signal->bitsPerSample();
            client_asbd.mReserved = 0;

            int32_t file_bytes_per_sample = 0;
            switch (m_signal_file_write_settings.m_bits_per_sample) {
                case 8:	 file_bytes_per_sample = 1; break;
                case 16: file_bytes_per_sample = 2; break;
                case 24: file_bytes_per_sample = 3; break;
                case 32: file_bytes_per_sample = 4; break;
                default:
                    Exception::throwStandard(ErrorCode::BadArgs);
            }

            AudioStreamBasicDescription file_asbd = {0};
            AudioFileTypeID file_type = 0; // Undefined

            if (m_signal_file_write_settings.m_file_type == SignalFileType::AAC) {

                file_type = kAudioFileM4AType;

                // Here we specify the M4A format
                file_asbd.mSampleRate = m_signal_sample_rate;
                file_asbd.mFormatID = kAudioFormatMPEG4AAC;
                file_asbd.mFormatFlags = kMPEG4Object_AAC_Main;
                file_asbd.mChannelsPerFrame = m_signal_channel_count;
                file_asbd.mBytesPerPacket = 0;
                file_asbd.mBytesPerFrame = 0;
                file_asbd.mFramesPerPacket = 1024;
                file_asbd.mBitsPerChannel = 0;
                file_asbd.mReserved = 0;
            }
            else if (m_signal_file_write_settings.m_file_type == SignalFileType::AIFF ||
                     m_signal_file_write_settings.m_file_type == SignalFileType::AIFC) {
                file_type = m_signal_file_write_settings.m_floating_point ? kAudioFileAIFCType : kAudioFileAIFFType;
                FillOutASBDForLPCM(file_asbd, m_signal_sample_rate, m_signal_channel_count,
                                   m_signal_file_write_settings.m_bits_per_sample, m_signal_file_write_settings.m_bits_per_sample,
                                   m_signal_file_write_settings.m_floating_point, true);
            }
            else if (m_signal_file_write_settings.m_file_type == SignalFileType::WAVE) {
                file_type = kAudioFileWAVEType;
                FillOutASBDForLPCM(file_asbd, m_signal_sample_rate, m_signal_channel_count,
                                   m_signal_file_write_settings.m_bits_per_sample, m_signal_file_write_settings.m_bits_per_sample,
                                   m_signal_file_write_settings.m_floating_point, false);
            }
            else if (m_signal_file_write_settings.m_file_type == SignalFileType::CAF) {
                file_type = kAudioFileCAFType;
                // For CAF, we typically use Linear PCM (LPCM) or AAC
                FillOutASBDForLPCM(file_asbd, m_signal_sample_rate, m_signal_channel_count,
                                   m_signal_file_write_settings.m_bits_per_sample,
                                   m_signal_file_write_settings.m_bits_per_sample,
                                   m_signal_file_write_settings.m_floating_point,
                                   false); // interleaved
            }
            else if (m_signal_file_write_settings.m_file_type == SignalFileType::MP3) {
                // As in july 2023, apple frameworks can not save audio in mp3 format
                Exception::throwSpecific(kErrUnsupportedFormatMP3);
            }

            status = ExtAudioFileCreateWithURL(file_url,					// inURL
                                               file_type,					// inFileType
                                               &file_asbd,					// inStreamDesc
                                               nullptr,					    // inChannelLayout
                                               kAudioFileFlags_EraseFile,	// inFlags
                                               &m_ext_audio_file_ref);      // outExtAudioFile
            if (status != noErr) {
                Exception::throwStandard(ErrorCode::FileCantCreate);
            }

            status = ExtAudioFileSetProperty(m_ext_audio_file_ref, kExtAudioFileProperty_ClientDataFormat, sizeof(client_asbd), &client_asbd);
            if (status != noErr) {
                Exception::throwStandard(ErrorCode::FileCantWrite);
            }

            m_write_data_type = signal->dataType();

            // Audio::logAudioStreamBasicDescription(std::cout, client_asbd, "client_asbd");
            // Audio::logAudioStreamBasicDescription(std::cout, file_asbd, "file_asbd");

            if (signal->channelCount() != m_signal_channel_count) {
                Exception::throwSpecific(kErrUnsupportedChannelCount);
            }

            if (signal->dataType() != m_write_data_type) {
                Exception::throwSpecific(kErrUnsupportedDataType);
            }

            AudioBufferList buffer_list;
            buffer_list.mNumberBuffers = 1;
            buffer_list.mBuffers[0].mNumberChannels = (UInt32)m_signal_channel_count;
            buffer_list.mBuffers[0].mDataByteSize = (UInt32)signal->bytesPerSample() * signal->channelCount() * length;
            buffer_list.mBuffers[0].mData = signal->mutDataPtr(0, offset);

            // Audio::logAudioBufferList(std::cout, buffer_list, "buffer_list");

            status = ExtAudioFileWrite(m_ext_audio_file_ref, length, &buffer_list);
            if (status != noErr) {
                Exception::throwStandard(ErrorCode::FileCantWrite);
            }


            status = ExtAudioFileDispose(m_ext_audio_file_ref);
            if (status != noErr) {
                Exception::throwSpecific(kErrExtAudioFileDisposeFailed);
            }

            m_ext_audio_file_ref = nullptr;
        }
        catch (const Exception& e) {
            deferred_exception.capture();
        }
        catch (...) {
            deferred_exception.capture();
        }

        // Cleanup
        if (file_path != nullptr) {
            CFRelease(file_path);
        }

        if (file_url != nullptr) {
            CFRelease(file_url);
        }

        deferred_exception.rethrow();
    }

}  // End of namespace Grain
