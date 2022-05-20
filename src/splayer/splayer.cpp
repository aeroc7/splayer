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
#include <splayer/util/log.h>
#include <splayer/window/window.h>

#include <chrono>
#include <cstring>
#include <thread>

using namespace utils;

namespace splayer {
constexpr auto WIDTH = 3840;
constexpr auto HEIGHT = 2160;

SplayerApp::SplayerApp(const std::string &f) {
    os_window = std::make_unique<graphics::Window>();

    const auto pm_dims = os_window->get_primary_monitor_dims();
    window_w = std::get<0>(pm_dims) * cfg::INITIAL_WINDOW_SCALE_MULTI;
    window_h = std::get<1>(pm_dims) * cfg::INITIAL_WINDOW_SCALE_MULTI;

    os_window->create_window(cfg::PROJECT_NAME, window_w, window_h);

    sw_decoder = std::make_unique<splayer::SwDecoder>();

    sw_decoder->open_input(f);
}

void SplayerApp::gui_loop() {
    graphics::GlTexture tex{WIDTH, HEIGHT};

    os_window->window_loop([&] {
        auto decode_t_beg = std::chrono::steady_clock::now();

        const auto f = sw_decoder->decode_frame();
        if (f == nullptr) {
            return;
        }

        auto decode_t_end = std::chrono::steady_clock::now();
        const auto decode_time_taken_us = std::chrono::duration_cast<std::chrono::microseconds>(decode_t_end - decode_t_beg).count();

        double clip_fps_us = (1000.0 / sw_decoder->clip_fps()) * 1000.0;

        if (decode_time_taken_us < clip_fps_us) {
            clip_fps_us -= decode_time_taken_us;
            std::cout << clip_fps_us << '\n';
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int64_t>(clip_fps_us)));
        }

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