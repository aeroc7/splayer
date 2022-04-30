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
#include <splayer/display/gl_texture.h>
#include <splayer/to_file.h>
#include <splayer/window/window.h>

#include <chrono>
#include <cstring>
#include <iostream>

AVFrame *last_frame{nullptr};

namespace splayer {
constexpr auto WIDTH = 3840;
constexpr auto HEIGHT = 2160;
constexpr auto MONITOR_W = 1920;
constexpr auto MONITOR_H = 1080;

SplayerApp::SplayerApp() {
    os_window = std::make_unique<graphics::Window>();
    os_window->create_window(cfg::PROJECT_NAME, MONITOR_W, MONITOR_H);

    sw_decoder = std::make_unique<splayer::SwDecoder>();

    auto err = sw_decoder->open_input("/home/bennett/Downloads/Sony Bravia OLED 4K Demo.mp4");

    if (err) {
        std::cout << "Error occured: " << err.error_code() << '\n';
        std::exit(err.error_code());
    }
}

void SplayerApp::gui_loop() {
    graphics::GlTexture tex{WIDTH, HEIGHT};

    os_window->window_loop([&] {
        const auto f = sw_decoder->decode_frame();
        if (f == nullptr) {
            return;
        }

        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        tex.bind();
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, f->data[0]);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2i(0, 0);
        glTexCoord2f(0, 1);
        glVertex2i(0, MONITOR_H);
        glTexCoord2f(1, 1);
        glVertex2i(MONITOR_W, MONITOR_H);
        glTexCoord2f(1, 0);
        glVertex2i(MONITOR_W, 0);
        glEnd();

        tex.unbind();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    });
}

SplayerApp::~SplayerApp() { os_window.reset(); }
}  // namespace splayer