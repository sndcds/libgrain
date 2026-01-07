//
//  MovieWriter.hpp
//
//  Created by Roald Christesen on 07.01.2026
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainMovieWriter_hpp
#define GrainMovieWriter_hpp

#include "String/String.hpp"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
    #include <libavutil/channel_layout.h>
}


namespace Grain {

    class Image;
    class Signal;
    class MovieWriter;


    typedef void (*MovieWriterFrameCallbackFunc)(MovieWriter* movie_writer, Image* frame_image, int64_t frame_index);


    enum class MovieVideoCodec {
        H264 = 0,
        PRORES
    };

    enum class MovieAudioCodec {
        AAC = 0
    };

    struct MovieWriterSetting {
        int32_t width = 1920;
        int32_t height = 1080;
        int32_t video_fps = 25;
        MovieVideoCodec video_codec = MovieVideoCodec::H264;
        int32_t video_quality = 23;
        MovieAudioCodec audio_codec = MovieAudioCodec::AAC;
        int32_t audio_bitrate = 128000;
    };

    class MovieWriter {
    public:
        MovieWriter() noexcept = default;
        ~MovieWriter() noexcept = default;

        ErrorCode writeVideoWithAudio(
            const String& file_path,
            int64_t video_frame_count,
            const MovieWriterSetting& settings,
            MovieWriterFrameCallbackFunc frame_callback,
            Image* frame_image,
            Signal* audio_signal) noexcept;


        // Helper
        AVCodecID avVideoCodecId(const MovieWriterSetting& settings) {
            switch (settings.video_codec) {
                case MovieVideoCodec::H264: return AV_CODEC_ID_H264;
                case MovieVideoCodec::PRORES: return AV_CODEC_ID_PRORES;
                default: return AV_CODEC_ID_NONE;
            }
        }

        AVCodecID avAudioCodecId(const MovieWriterSetting& settings) {
            switch (settings.audio_codec) {
                case MovieAudioCodec::AAC: return AV_CODEC_ID_AAC;
                default: return AV_CODEC_ID_NONE;
            }
        }
    protected:
        Image* video_frame_image_{};
    };
    
 } // End of namespace

#endif // GrainMovieWriter_hpp