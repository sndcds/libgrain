//
// Movie.hpp
//
// Created by Roald Christesen on 07.01.26
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainMovie_hpp
#define GrainMovie_hpp

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/channel_layout.h>
}


namespace Grain {
    enum class MovieVideoCodec {
        H264 = 0,
        PRORES
    };

    enum class MovieAudioCodec {
        AAC = 0
    };

    class Movie {
    public:
        Movie() noexcept = default;
        ~Movie() noexcept = default;

        static AVCodecID avVideoCodecId(const MovieVideoCodec video_codec) {
            switch (video_codec) {
                case MovieVideoCodec::H264: return AV_CODEC_ID_H264;
                case MovieVideoCodec::PRORES: return AV_CODEC_ID_PRORES;
                default: return AV_CODEC_ID_NONE;
            }
        }

        static AVCodecID avAudioCodecId(const MovieAudioCodec audio_codec) {
            switch (audio_codec) {
                case MovieAudioCodec::AAC: return AV_CODEC_ID_AAC;
                default: return AV_CODEC_ID_NONE;
            }
        }
    };
} // End of namespace

#endif // GrainMovie_hpp
