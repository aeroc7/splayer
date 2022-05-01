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

#include "hw_decode.h"

#include <splayer/util/utils.h>

using namespace utils;

namespace splayer {
HwDecoder::HwDecoder() {
    frame.reset(av_frame_alloc());
    frame_cnvt.reset(av_frame_alloc());
    sw_frame.reset(av_frame_alloc());

    if (!frame || !frame_cnvt || !sw_frame) {
        throw std::runtime_error("Failed to allocate av_frame.");
    }

    AVHWDeviceType type{AV_HWDEVICE_TYPE_NONE};
    while (true) {
        type = av_hwdevice_iterate_types(type);

        if (type == AV_HWDEVICE_TYPE_NONE) {
            break;
        }

        if (type == AV_HWDEVICE_TYPE_CUDA || type == AV_HWDEVICE_TYPE_OPENCL) {
            hw_device_type_ = type;
            break;
        }
    }

    // Fallback to software decode at this point
    if (hw_device_type_ == AV_HWDEVICE_TYPE_NONE) {
        throw std::runtime_error("Failed to find suitable hardware device for decoding.");
    }

    Log(Log::INFO) << "Using hardware device " << av_hwdevice_get_type_name(hw_device_type_)
                   << " for decoding.";
}

// TODO: In the event that failure happens further down, and we return, we fail to free/close a
// bunch of contexts that were made
void HwDecoder::open_input(const std::string &url) {
    int ret{};

    // Note: avformat_open_input will allocate our context for us.
    ret = avformat_open_input(&format_ctx_, url.c_str(), nullptr, nullptr);
    if (ret < 0) {
        Log(Log::ERROR) << "Failed to open input stream and/or read the header of: " << url;
        throw DecoderError(DecoderErrorDesc::FAILURE, ret);
    }

    ret = avformat_find_stream_info(format_ctx_, nullptr);
    if (ret < 0) {
        Log(Log::ERROR) << "Failed to read stream information.";
        throw DecoderError(DecoderErrorDesc::FAILURE, ret);
    }

    find_best_stream();
}

void HwDecoder::find_best_stream() {
    int ret{};

    ret = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, &codec_, 0);
    if (ret < 0) {
        Log(Log::ERROR) << "Failed to find valid stream/decoder.";
        throw DecoderError(DecoderErrorDesc::FAILURE, ret);
    }

    best_vid_stream_id_ = ret;

    find_decoder();
}

void HwDecoder::find_decoder() {
    for (int i = 0;; i += 1) {
        const AVCodecHWConfig *conf = avcodec_get_hw_config(codec_, i);
        if (!conf) {
            Log(Log::ERROR) << "Decoder " << codec_->name << " does not support "
                            << av_hwdevice_get_type_name(hw_device_type_);
            throw DecoderError(DecoderErrorDesc::FAILURE);
        }

        if (conf->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            conf->device_type == hw_device_type_) {
            hw_pix_type_ = conf->pix_fmt;
            break;
        }
    }

    setup_decoder();
}

void HwDecoder::setup_decoder() {
    ASSERT(codec_);
    int ret{};

    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_) {
        Log(Log::ERROR) << "Failed to create codec context.";
        throw DecoderError(DecoderErrorDesc::FAILURE);
    }

    ret = avcodec_parameters_to_context(
        codec_ctx_, format_ctx_->streams[best_vid_stream_id_]->codecpar);
    if (ret < 0) {
        Log(Log::ERROR) << "Failed to fill context with paramters.";
        throw DecoderError(DecoderErrorDesc::FAILURE, ret);
    }

    hw_decode_init();
}

void HwDecoder::hw_decode_init() {
    int err{};

    err = av_hwdevice_ctx_create(&hw_device_ctx_, hw_device_type_, nullptr, nullptr, 0);
    if (err < 0) {
        Log(Log::ERROR) << "Failed to create HW device context.";
        throw DecoderError(DecoderErrorDesc::FAILURE, err);
    }

    codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);

    err = avcodec_open2(codec_ctx_, codec_, nullptr);
    if (err < 0) {
        Log(Log::ERROR) << "Failed to open codec for stream " << best_vid_stream_id_;
        throw DecoderError(DecoderErrorDesc::FAILURE, err);
    }

    setup_cnvt_process();
}

bool HwDecoder::packet_is_from_video_stream(const AVPacket *p) const noexcept {
    return (p->stream_index == best_vid_stream_id_);
}

void HwDecoder::setup_cnvt_process() noexcept {
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

AVFrame *HwDecoder::decode_frame() {
    AVPacket pkt{};
    int ret{};

    while (av_read_frame(format_ctx_, &pkt) >= 0) {
        if (packet_is_from_video_stream(&pkt)) {
            ret = avcodec_send_packet(codec_ctx_, &pkt);
            if (ret < 0) {
                Log(Log::ERROR) << "Error sending packet for decoding.";
                throw DecoderError{DecoderErrorDesc::FAILURE, ret};
            }

            bool frm_success = (ret >= 0);

            while (frm_success) {
                ret = avcodec_receive_frame(codec_ctx_, frame.get());

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    Log(Log::ERROR) << "Error while decoding.";
                    throw DecoderError{DecoderErrorDesc::FAILURE, ret};
                }

                if (frame->format == hw_pix_type_) {
                    const auto err = av_hwframe_transfer_data(sw_frame.get(), frame.get(), 0);
                    if (err < 0) {
                        Log(Log::ERROR) << "Error while tranferring data to system memory.";
                        throw DecoderError{DecoderErrorDesc::FAILURE, err};
                    }
                }

                /*AVPixelFormat *frmts;
                int a = av_hwframe_transfer_get_formats(
                    hw_device_ctx_, AV_HWFRAME_TRANSFER_DIRECTION_TO, &frmts, 0);
                std::cout << a << '\n';

                AVPixelFormat *orig = frmts;

                while (true) {
                    if (*frmts != AV_PIX_FMT_NONE) {
                        std::cout << av_get_pix_fmt_name(*frmts) << '\n';
                        frmts++;
                        continue;
                    } else {
                        av_free(orig);
                        break;
                    }
                }

                const char *gpu_pixfmt = av_get_pix_fmt_name((AVPixelFormat)frame->format);
                const char *cpu_pixfmt = av_get_pix_fmt_name((AVPixelFormat)sw_frame->format);

                std::cout << gpu_pixfmt << ' ' << cpu_pixfmt << '\n';
                // sw_frame->linesize[0] = frame->linesize[0];
                // } else {
                //     sw_frame = std::move(frame);
                // }

                // std::cout << sw_frame->linesize[0] << '\n';

                sws_scale(sws_ctx, sw_frame->data, sw_frame->linesize, 0, codec_ctx_->height,
                    frame_cnvt->data, frame_cnvt->linesize);
                // frame_cnvt = std::move(sw_frame);

                frame_cnvt->width = codec_ctx_->width;
                frame_cnvt->height = codec_ctx_->height;
                // sw_frame->width = codec_ctx_->width;
                // sw_frame->height = codec_ctx_->height;*/

                return frame_cnvt.get();
            }

            if (frm_success) {
                break;
            }
        }
    }

    return nullptr;
}

double HwDecoder::clip_fps() const noexcept {
    return av_q2d(format_ctx_->streams[best_vid_stream_id_]->r_frame_rate);
}

HwDecoder::~HwDecoder() {
    av_free(cnvt_buf);
    avcodec_free_context(&codec_ctx_);

    cnvt_buf = nullptr;
}
}  // namespace splayer