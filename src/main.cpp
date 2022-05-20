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

#include <splayer/codec/decode/hw_decode.h>
#include <splayer/splayer.h>
#include <splayer/window/window.h>

#include <iostream>

namespace {
std::unique_ptr<splayer::SplayerApp> splayer_app;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage is ./splayer [filename]\n";
        return -1;
    }

    const std::string vid_file(argv[1]);

    try {
        splayer_app = std::make_unique<splayer::SplayerApp>(vid_file);
        splayer_app->gui_loop();
    } catch (const splayer::DecoderError &e) {
        std::cout << "Error: " << e.error_string() << '\n';
    }
}