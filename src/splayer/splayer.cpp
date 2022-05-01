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
#include <splayer/codec/decode/hw_decode.h>
#include <splayer/display/gl_texture.h>
#include <splayer/window/window.h>

#include <chrono>
#include <cstring>
#include <thread>

namespace splayer {
constexpr auto WIDTH = 3840;
constexpr auto HEIGHT = 2160;

SplayerApp::SplayerApp() {
    os_window = std::make_unique<graphics::Window>();

    const auto pm_dims = os_window->get_primary_monitor_dims();
    window_w = std::get<0>(pm_dims) * cfg::INITIAL_WINDOW_SCALE_MULTI;
    window_h = std::get<1>(pm_dims) * cfg::INITIAL_WINDOW_SCALE_MULTI;

    os_window->create_window(cfg::PROJECT_NAME, window_w, window_h);

    hw_decoder = std::make_unique<splayer::HwDecoder>();

    hw_decoder->open_input("/home/bennett/Downloads/Sony Surfing 4K Demo.mp4");
}

void SplayerApp::gui_loop() {
    graphics::GlTexture tex{WIDTH, HEIGHT};

    os_window->window_loop([&] {
        const auto f = hw_decoder->decode_frame();
        if (f == nullptr) {
            return;
        }

        const auto sleep_time = 1000.0 / hw_decoder->clip_fps();
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int64_t>(sleep_time)));

        os_window->force_consistent_aspect_r(f->width, f->height);

        const auto window_dims = os_window->get_window_dims();
        window_w = std::get<0>(window_dims);
        window_h = std::get<1>(window_dims);

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
        glVertex2i(0, window_h);
        glTexCoord2f(1, 1);
        glVertex2i(window_w, window_h);
        glTexCoord2f(1, 0);
        glVertex2i(window_w, 0);
        glEnd();

        tex.unbind();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    });
}

SplayerApp::~SplayerApp() { os_window.reset(); }
}  // namespace splayer