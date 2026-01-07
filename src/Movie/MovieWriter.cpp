//
//  MovieWriter.cpp
//
//  Created by Roald Christesen on 07.01.2026
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "Movie/MovieWriter.hpp"
#include "Image/Image.hpp"
#include "Signal/Signal.hpp"

#include <iostream>


namespace Grain {

    ErrorCode MovieWriter::writeVideoWithAudio(
            const String& file_path,
            int64_t video_frame_count,
            const MovieWriterSetting& settings,
            MovieWriterFrameCallbackFunc frame_callback,
            Image* frame_image,
            Signal* audio_signal) noexcept
    {
        // TODO: Validate Settings

        const int32_t width = settings.width;
        const int32_t height = settings.height;

        avformat_network_init();

        // Format
        AVFormatContext* fmt_ctx = nullptr;
        avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, file_path.utf8());
        if (!fmt_ctx) {
            return ErrorCode::MemCantAllocate;
        }

        // Video
        auto av_video_codec_id = avVideoCodecId(settings);
        if (av_video_codec_id == AV_CODEC_ID_NONE) {
            return ErrorCode::UnsupportedSettings;
        }

        const AVCodec* video_codec = avcodec_find_encoder(av_video_codec_id);
        if (!video_codec) {
            return ErrorCode::MemCantAllocate;
        }

        AVStream* video_stream = avformat_new_stream(fmt_ctx, nullptr);
        AVCodecContext* video_ctx = avcodec_alloc_context3(video_codec);

        video_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        video_ctx->width = width;
        video_ctx->height = height;
        video_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        video_ctx->time_base = AVRational{ 1, settings.video_fps };
        video_ctx->framerate = AVRational{ settings.video_fps, 1 };
        video_ctx->gop_size = settings.video_fps;
        video_ctx->max_b_frames = 2;

        // global header flag
        if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            video_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        // Set Video Options
        AVDictionary* opts = nullptr;
        if (av_video_codec_id == AV_CODEC_ID_H264) {
            av_dict_set(&opts, "preset", "fast", 0); // encoding speed
            av_dict_set(&opts, "crf", std::to_string(settings.video_quality).c_str(), 0); // quality
            video_ctx->pix_fmt = AV_PIX_FMT_YUV420P; // default for H.264
        }
        else if (av_video_codec_id == AV_CODEC_ID_PRORES) {
            av_dict_set(&opts, "profile", std::to_string(settings.video_quality).c_str(), 0);
            switch (settings.video_quality) {
                case 0: // Proxy
                case 1: // LT
                case 2: // 422
                case 3: // HQ
                    video_ctx->pix_fmt = AV_PIX_FMT_YUV422P10LE;
                    break;
                case 4: // 444
                    video_ctx->pix_fmt = AV_PIX_FMT_YUV444P10LE;
                    break;
                case 5: // 4444 (alpha)
                    video_ctx->pix_fmt = AV_PIX_FMT_YUVA444P10LE;
                    break;
                default:
                    video_ctx->pix_fmt = AV_PIX_FMT_YUV422P10LE;
                    break;
            }
        }

        // Decide if we need an intermediate format (RGBF32 -> YUV422P10 is unsupported)
        AVPixelFormat intermediate_fmt = AV_PIX_FMT_NONE;
        if (video_ctx->pix_fmt == AV_PIX_FMT_YUV422P10LE) {
            intermediate_fmt = AV_PIX_FMT_YUV444P10LE;
        }

        // Open codec with options
        if (avcodec_open2(video_ctx, video_codec, &opts) < 0) {
            std::cerr << "Failed to open video codec\n";
            return ErrorCode::FormatMismatch;
        }
        av_dict_free(&opts);

        // copy context parameters to stream
        avcodec_parameters_from_context(video_stream->codecpar, video_ctx);
        video_stream->time_base = video_ctx->time_base;


        // Audio
        auto av_audio_codec_id = avAudioCodecId(settings);
        if (av_audio_codec_id == AV_CODEC_ID_NONE) {
            return ErrorCode::UnsupportedSettings;
        }
        const AVCodec* audio_codec = avcodec_find_encoder(av_audio_codec_id);
        if (!audio_codec) {
            return ErrorCode::UnsupportedSettings;
        }

        AVStream* audio_stream = avformat_new_stream(fmt_ctx, nullptr);
        AVCodecContext* audio_ctx = avcodec_alloc_context3(audio_codec);

        audio_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        audio_ctx->sample_rate = audio_signal->sampleRate();
        audio_ctx->bit_rate = settings.audio_bitrate;
        audio_ctx->time_base = AVRational{1, audio_ctx->sample_rate};

        av_channel_layout_default(
            &audio_ctx->ch_layout,
            audio_signal->channelCount()
        );

        if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            audio_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        avcodec_open2(audio_ctx, audio_codec, nullptr);
        avcodec_parameters_from_context(audio_stream->codecpar, audio_ctx);
        audio_stream->time_base = audio_ctx->time_base;

        // Output
        if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&fmt_ctx->pb, file_path.utf8(), AVIO_FLAG_WRITE) < 0) {
                return ErrorCode::FileCantOpen;
            }
        }

        auto av_err = avformat_write_header(fmt_ctx, nullptr);
        if (av_err < 0) {
            char errbuf[256];
            av_strerror(av_err, errbuf, sizeof(errbuf));
            std::cerr << "Error writing header: " << errbuf << "\n";
            return ErrorCode::Fatal;
        }

        // Video Frames
        AVFrame* frame_yuv = av_frame_alloc();
        frame_yuv->format = video_ctx->pix_fmt;
        frame_yuv->width  = width;
        frame_yuv->height = height;
        av_frame_get_buffer(frame_yuv, 32);

        // Intermediate frame (YUV444P10) if required
        AVFrame* frame_yuv_intermediate = nullptr;
        if (intermediate_fmt != AV_PIX_FMT_NONE) {
            frame_yuv_intermediate = av_frame_alloc();
            frame_yuv_intermediate->format = intermediate_fmt;
            frame_yuv_intermediate->width  = width;
            frame_yuv_intermediate->height = height;
            av_frame_get_buffer(frame_yuv_intermediate, 32);
        }

        AVFrame* frame_rgb = av_frame_alloc();
        frame_rgb->format = AV_PIX_FMT_RGBF32LE;
        frame_rgb->width  = width;
        frame_rgb->height = height;
        av_frame_get_buffer(frame_rgb, 32);


        // RGBF32 -> YUV (either final or intermediate)
        SwsContext* sws_rgb_to_yuv = sws_getContext(
            width, height,
            AV_PIX_FMT_RGBF32LE,
            width, height,
            (intermediate_fmt != AV_PIX_FMT_NONE) ? intermediate_fmt : video_ctx->pix_fmt,
            SWS_LANCZOS | SWS_ACCURATE_RND,
            nullptr, nullptr, nullptr
        );

        if (!sws_rgb_to_yuv) {
            av_log(nullptr, AV_LOG_ERROR, "Failed RGB -> YUV sws context\n");
            return ErrorCode::FormatMismatch;
        }

        // Optional YUV444P10 -> YUV422P10
        SwsContext* sws_444_to_422 = nullptr;
        if (intermediate_fmt != AV_PIX_FMT_NONE) {
            sws_444_to_422 = sws_getContext(
                width, height,
                intermediate_fmt,
                width, height,
                video_ctx->pix_fmt,
                SWS_LANCZOS | SWS_ACCURATE_RND,
                nullptr, nullptr, nullptr
            );

            if (!sws_444_to_422) {
                av_log(nullptr, AV_LOG_ERROR, "Failed YUV444 -> YUV422 sws context\n");
                return ErrorCode::FormatMismatch;
            }
        }


        AVPacket* pkt = av_packet_alloc();

        // Write Video
        float pixel[4]{};
        ImageAccess fia(frame_image, pixel);

        for (int64_t frame_index = 0; frame_index < video_frame_count; frame_index++) {
            frame_callback(this, frame_image, frame_index);

            while (fia.stepY()) {
                float* row = reinterpret_cast<float*>(frame_rgb->data[0] + fia.y() * frame_rgb->linesize[0]);

                int32_t x = 0;
                while (fia.stepX()) {
                    fia.read();
                    row[x + 0] = pixel[0];
                    row[x + 1] = pixel[1];
                    row[x + 2] = pixel[2];
                    x += 3;
                }
            }

            // RGBF32 -> YUV444P10 or final YUV
            sws_scale(
                sws_rgb_to_yuv,
                frame_rgb->data, frame_rgb->linesize,
                0, height,
                (intermediate_fmt != AV_PIX_FMT_NONE)
                    ? frame_yuv_intermediate->data
                    : frame_yuv->data,
                (intermediate_fmt != AV_PIX_FMT_NONE)
                    ? frame_yuv_intermediate->linesize
                    : frame_yuv->linesize
            );

            // If needed: YUV444P10 -> YUV422P10
            if (intermediate_fmt != AV_PIX_FMT_NONE) {
                sws_scale(
                    sws_444_to_422,
                    frame_yuv_intermediate->data, frame_yuv_intermediate->linesize,
                    0, height,
                    frame_yuv->data, frame_yuv->linesize
                );
            }

            frame_yuv->pts = frame_index;

            avcodec_send_frame(video_ctx, frame_yuv);
            while (avcodec_receive_packet(video_ctx, pkt) == 0) {
                av_packet_rescale_ts(pkt, video_ctx->time_base, video_stream->time_base);
                pkt->stream_index = video_stream->index;
                av_interleaved_write_frame(fmt_ctx, pkt);
                av_packet_unref(pkt);
            }
        }

        // Write Audio (no resampling)
        AVFrame* audio_frame = av_frame_alloc();
        audio_frame->format = audio_ctx->sample_fmt;
        audio_frame->sample_rate = audio_ctx->sample_rate;
        av_channel_layout_copy(&audio_frame->ch_layout, &audio_ctx->ch_layout);

        audio_frame->nb_samples = audio_ctx->frame_size;
        av_frame_get_buffer(audio_frame, 0);

        int64_t sample_pos = 0;
        const int channels = audio_ctx->ch_layout.nb_channels;
        const int64_t total_samples = audio_signal->sampleCount();

        while (sample_pos < total_samples) {
            int nb = std::min<int64_t>(audio_ctx->frame_size,
                                      total_samples - sample_pos);

            audio_frame->nb_samples = nb;

            for (int ch = 0; ch < channels; ch++) {
                float* dst = reinterpret_cast<float*>(audio_frame->data[ch]);
                for (int i = 0; i < nb; i++) {
                    dst[i] = audio_signal->readFloat(ch, sample_pos + i);
                }
            }

            audio_frame->pts = sample_pos;

            avcodec_send_frame(audio_ctx, audio_frame);
            while (avcodec_receive_packet(audio_ctx, pkt) == 0) {
                av_packet_rescale_ts(pkt, audio_ctx->time_base, audio_stream->time_base);
                pkt->stream_index = audio_stream->index;
                av_interleaved_write_frame(fmt_ctx, pkt);
                av_packet_unref(pkt);
            }

            sample_pos += nb;
        }


        // Flush
        avcodec_send_frame(video_ctx, nullptr);
        avcodec_send_frame(audio_ctx, nullptr);

        while (avcodec_receive_packet(video_ctx, pkt) == 0 ||
               avcodec_receive_packet(audio_ctx, pkt) == 0) {
            av_packet_unref(pkt);
        }

        av_write_trailer(fmt_ctx);


        // Cleanup
        sws_freeContext(sws_rgb_to_yuv);
        sws_freeContext(sws_444_to_422);

        av_frame_free(&frame_rgb);
        av_frame_free(&frame_yuv);
        av_frame_free(&frame_yuv_intermediate);

        av_frame_free(&audio_frame);
        av_packet_free(&pkt);

        avcodec_free_context(&video_ctx);
        avcodec_free_context(&audio_ctx);
        avio_closep(&fmt_ctx->pb);
        avformat_free_context(fmt_ctx);

        return ErrorCode::None;
    }

} // End of namespace