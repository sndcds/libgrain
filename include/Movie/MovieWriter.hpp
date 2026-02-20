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
#include "Movie/Movie.hpp"


namespace Grain {

    class Image;
    class Signal;
    class MovieWriter;


    typedef void (*MovieWriterFrameCallbackFunc)(MovieWriter* movie_writer, int64_t frame_index);


    struct MovieWriterConfig {
        int32_t width = 1920;
        int32_t height = 1080;
        int32_t video_fps = 25;
        MovieVideoCodec video_codec = MovieVideoCodec::H264;
        int32_t video_quality = 23;
        MovieAudioCodec audio_codec = MovieAudioCodec::AAC;
        int32_t audio_bitrate = 128000;

        AVCodecID avVideoCodecId() const {
            switch (video_codec) {
                case MovieVideoCodec::H264: return AV_CODEC_ID_H264;
                case MovieVideoCodec::PRORES: return AV_CODEC_ID_PRORES;
                default: return AV_CODEC_ID_NONE;
            }
        }

        AVCodecID avAudioCodecId() const {
            switch (audio_codec) {
                case MovieAudioCodec::AAC: return AV_CODEC_ID_AAC;
                default: return AV_CODEC_ID_NONE;
            }
        }
    };

    class MovieWriter {
    public:
        MovieWriter() noexcept = default;
        ~MovieWriter() noexcept;

        ErrorCode writeVideoWithAudio(
            const String& file_path,
            int64_t video_frame_count,
            const MovieWriterConfig& config,
            MovieWriterFrameCallbackFunc frame_callback,
            Signal* audio_signal,
            void* ref) noexcept;

        Image* videoFrameBufferPtr() { return video_frame_buffer_; }
        void* refPtr() { return ref_ptr_; }

    protected:
        Image* video_frame_buffer_{};
        void* ref_ptr_ = nullptr;
    };
    
 } // End of namespace

#endif // GrainMovieWriter_hpp