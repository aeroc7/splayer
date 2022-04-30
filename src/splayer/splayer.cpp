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

#include "splayer.h"

#include <GL/glew.h>
#include <splayer/cfg.h>
#include <splayer/codec/decode/sw_fallback.h>
#include <splayer/to_file.h>
#include <splayer/window/window.h>

#include <chrono>
#include <iostream>

AVFrame *last_frame{nullptr};

namespace splayer {
SplayerApp::SplayerApp() {
    os_window = std::make_unique<graphics::Window>();
    os_window->create_window(cfg::PROJECT_NAME, 500, 500);
    // static int i = 0;

    sw_decoder = std::make_unique<splayer::SwDecoder>([](AVFrame *frm) {
        // utils::save_frame_to_f<20>(frm, frm->width, frm->height, i);
        last_frame = frm;
        // i += 1;
    });

    auto err = sw_decoder->open_input("/home/bennett/Downloads/Sony Surfing 4K Demo.mp4");

    if (err) {
        std::cout << "Error occured: " << err.error_code() << '\n';
    }
}

void SplayerApp::gui_loop() {
    os_window->window_loop([this] {
        auto t1 = std::chrono::steady_clock::now();

        const auto err = sw_decoder->decode_frame();
        if (err) {
            std::cout << "Error!\n";
            std::abort();
        }

        auto t2 = std::chrono::steady_clock::now();

        std::cout << "Time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << '\n';
    });
}

SplayerApp::~SplayerApp() { os_window.reset(); }
}  // namespace splayer