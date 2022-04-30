// MIT License
//
// Copyright (c) 2022 Bennett Anderson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "sw_fallback.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <splayer/to_file.h>

#include <cassert>
#include <iostream>

namespace splayer {
SwDecoder::SwDecoder() {
    frame.reset(av_frame_alloc());
    frame_cnvt.reset(av_frame_alloc());

    if (!frame || !frame_cnvt) {
        std::cerr << "Failed to allocate frame.\n";
        return;
    }
}

// TODO: In the event that failure happens further down, and we return, we fail to free/close a
// bunch of contexts that were made
DecoderError SwDecoder::open_input(const std::string &url) noexcept {
    int ret{};

    // Note: avformat_open_input will allocate our context for us.
    ret = avformat_open_input(&format_ctx_, url.c_str(), nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to open input stream and/or read the header of: " << url << '\n';
        return DecoderError{DecoderErrorDesc::FAILURE, ret};
    }

    ret = avformat_find_stream_info(format_ctx_, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to read stream information.\n";
        return DecoderError{DecoderErrorDesc::FAILURE, ret};
    }

    // Initialization
    if (const auto err = find_best_stream(); err) {
        return err;
    }

    return DecoderError{DecoderErrorDesc::SUCCESS};
}

DecoderError SwDecoder::find_best_stream() noexcept {
    int ret{};

    ret = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        std::cerr << "Failed to find valid stream/decoder\n";
        return DecoderError{DecoderErrorDesc::FAILURE, ret};
    }
    best_vid_stream_id_ = ret;

    return find_decoder();
}

DecoderError SwDecoder::find_decoder() noexcept {
    const auto decoder_id = get_decoder_id();

    codec_ = avcodec_find_decoder(static_cast<AVCodecID>(decoder_id));
    if (!codec_) {
        std::cerr << "Unsupported codec\n";
        return DecoderError{DecoderErrorDesc::FAILURE};
    }

    return setup_decoder();
}

int SwDecoder::get_decoder_id() noexcept {
    assert(best_vid_stream_id_ >= 0);

    const AVCodecID id = format_ctx_->streams[best_vid_stream_id_]->codecpar->codec_id;

    return id;
}

DecoderError SwDecoder::setup_decoder() noexcept {
    assert(codec_);
    int ret{};

    codec_ctx_orig_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_orig_) {
        std::cerr << "Failed to create codec context.\n";
        return DecoderError{DecoderErrorDesc::FAILURE};
    }

    ret = avcodec_parameters_to_context(
        codec_ctx_orig_, format_ctx_->streams[best_vid_stream_id_]->codecpar);
    if (ret < 0) {
        std::cerr << "Failed to fill context with paramters.\n";
        return DecoderError{DecoderErrorDesc::FAILURE, ret};
    }

    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_) {
        std::cerr << "Failed to create codec context copy.\n";
        return DecoderError{DecoderErrorDesc::FAILURE};
    }

    ret = avcodec_parameters_to_context(
        codec_ctx_, format_ctx_->streams[best_vid_stream_id_]->codecpar);
    if (ret < 0) {
        std::cerr << "Failed to fill context copy with paramters.\n";
        return DecoderError{DecoderErrorDesc::FAILURE, ret};
    }

    return open_codec();
}

DecoderError SwDecoder::open_codec() noexcept {
    int ret{};

    // set codec to automatically determine how many threads suits best for the decoding job
    codec_ctx_->thread_count = 0;

    if (codec_->capabilities | AV_CODEC_CAP_FRAME_THREADS) {
        codec_ctx_->thread_type = FF_THREAD_FRAME;
    } else if (codec_->capabilities | AV_CODEC_CAP_SLICE_THREADS) {
        codec_ctx_->thread_type = FF_THREAD_SLICE;
    } else {
        codec_ctx_->thread_count = 1;  // don't use multithreading
    }

    ret = avcodec_open2(codec_ctx_, codec_, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to open codec.\n";
        return DecoderError{DecoderErrorDesc::FAILURE, ret};
    }

    return DecoderError{DecoderErrorDesc::SUCCESS};
}

bool SwDecoder::packet_is_from_video_stream(const AVPacket *p) const noexcept {
    return (p->stream_index == best_vid_stream_id_);
}

void SwDecoder::setup_cnvt_process() noexcept {
    if (buf_size) {
        return;
    }

    buf_size = av_image_get_buffer_size(
        AV_PIX_FMT_RGB24, codec_ctx_->width, codec_ctx_->height, FRAME_BUF_ALIGNMENT);
    cnvt_buf = static_cast<std::uint8_t *>(av_malloc(buf_size * sizeof(std::uint8_t)));

    av_image_fill_arrays(frame_cnvt->data, frame_cnvt->linesize, cnvt_buf, AV_PIX_FMT_RGB24,
        codec_ctx_->width, codec_ctx_->height, FRAME_BUF_ALIGNMENT);

    sws_ctx = sws_getContext(codec_ctx_->width, codec_ctx_->height, codec_ctx_->pix_fmt,
        codec_ctx_->width, codec_ctx_->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr,
        nullptr);
}

AVFrame *SwDecoder::decode_frame() noexcept {
    AVPacket pkt{};
    int ret{};

    if (buf_size == 0) {
        setup_cnvt_process();
    }

    while (av_read_frame(format_ctx_, &pkt) >= 0) {
        if (pkt.stream_index == best_vid_stream_id_) {
            ret = avcodec_send_packet(codec_ctx_, &pkt);
            if (ret < 0) {
                std::cerr << "Error sending packet for decoding.\n";
                // return DecoderError{DecoderErrorDesc::FAILURE, ret};
                return nullptr;
            }

            bool frm_success = (ret >= 0);

            while (frm_success) {
                ret = avcodec_receive_frame(codec_ctx_, frame.get());

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error while decoding.\n";
                    // return DecoderError{DecoderErrorDesc::FAILURE, ret};
                    return nullptr;
                }

                sws_scale(sws_ctx, static_cast<uint8_t const *const *>(frame->data),
                    frame->linesize, 0, codec_ctx_->height, frame_cnvt->data, frame_cnvt->linesize);

                frame_cnvt->width = codec_ctx_->width;
                frame_cnvt->height = codec_ctx_->height;

                return frame_cnvt.get();
            }

            if (frm_success) {
                break;
            }
        }
    }

    // return DecoderError{DecoderErrorDesc::SUCCESS};
    return nullptr;
}

SwDecoder::~SwDecoder() {
    av_free(cnvt_buf);
    avcodec_free_context(&codec_ctx_);

    cnvt_buf = nullptr;
}
}  // namespace splayer