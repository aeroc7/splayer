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

#include <memory>
#include <string>

extern "C" {
#include <libavutil/frame.h>
}

namespace splayer {
struct AvFramePtrDeleter {
    void operator()(AVFrame *f) noexcept { av_frame_free(&f); }
};

using AVFramePtr = std::unique_ptr<AVFrame, AvFramePtrDeleter>;

enum class DecoderErrorDesc { FAILURE = -1, SUCCESS = 0 };
class DecoderError {
public:
    constexpr DecoderError() = default;
    constexpr DecoderError(DecoderErrorDesc d) : desc(d) {}
    constexpr explicit DecoderError(DecoderErrorDesc d, int code) : desc(d), err_code(code) {}
    constexpr explicit operator bool() const noexcept { return status(); }
    constexpr bool status() const noexcept { return (desc != DecoderErrorDesc::SUCCESS); }
    constexpr int error_code() const noexcept { return err_code; }
    std::string error_string() const {
        std::array<char, 128> err_buf;

        av_strerror(err_code, err_buf.data(), err_buf.size());
        return std::string{err_buf.data()};
    }

private:
    DecoderErrorDesc desc{DecoderErrorDesc::SUCCESS};
    int err_code{};
};

class Decoder {
public:
    virtual DecoderError open_input(const std::string &url) noexcept = 0;
    virtual ~Decoder() = default;

protected:
private:
    friend DecoderError;
};
}  // namespace splayer