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

#ifndef HW_DECODE_H_
#define HW_DECODE_H_

#include "decoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace splayer {
class HwDecoder final : public Decoder {
public:
    HwDecoder();
    virtual ~HwDecoder() override;

    void open_input(const std::string &url) override;

    AVFrame *decode_frame();
    double clip_fps() const noexcept;

private:
    void find_best_stream();
    void find_decoder();
    int get_decoder_id() noexcept;
    void setup_decoder();
    void hw_decode_init();

    bool packet_is_from_video_stream(const AVPacket *p) const noexcept;

    void setup_cnvt_process() noexcept;

    AVHWDeviceType hw_device_type_{AV_HWDEVICE_TYPE_NONE};
    AVFormatContext *format_ctx_{nullptr};
    AVCodec *codec_;
    AVPixelFormat hw_pix_type_{};
    AVCodecContext *codec_ctx_{nullptr};
    AVBufferRef *hw_device_ctx_{nullptr};

    AVFramePtr frame, frame_cnvt, sw_frame;

    SwsContext *sws_ctx{nullptr};

    int best_vid_stream_id_{-1};
    int buf_size{};

    std::uint8_t *cnvt_buf{nullptr};

    static constexpr auto FRAME_BUF_ALIGNMENT = 32;
};
}  // namespace splayer

#endif /* HW_DECODE_H_ */