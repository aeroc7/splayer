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

#include "decoder.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct SwsContext;

namespace splayer {
class SwDecoder final : public Decoder {
public:
    using CallbackType = void (*)(AVFrame *f);
    SwDecoder(CallbackType cb);
    virtual ~SwDecoder() override;

    DecoderError open_input(const std::string &url) noexcept override;

    DecoderError decode_frame() noexcept;

private:
    DecoderError find_best_stream() noexcept;
    DecoderError find_decoder() noexcept;
    int get_decoder_id() noexcept;
    DecoderError setup_decoder() noexcept;
    DecoderError open_codec() noexcept;

    bool packet_is_from_video_stream(const AVPacket *p) const noexcept;

    void setup_cnvt_process() noexcept;

    AVFormatContext *format_ctx_{nullptr};
    const AVCodec *codec_{nullptr};
    AVCodecContext *codec_ctx_orig_{nullptr}, *codec_ctx_{nullptr};

    AVFramePtr frame, frame_cnvt;

    CallbackType onframe_cb{nullptr};
    SwsContext *sws_ctx{nullptr};

    int best_vid_stream_id_{-1};
    int buf_size{};

    std::uint8_t *cnvt_buf{nullptr};

    static constexpr auto FRAME_BUF_ALIGNMENT = 32;
};
}  // namespace splayer