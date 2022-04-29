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

#ifndef TO_FILE_H_
#define TO_FILE_H_

#include <cstdio>
#include <iostream>
#include <string>

namespace utils {
template <int max>
void save_frame_to_f(AVFrame *avFrame, int width, int height, int frameIndex) {
    std::string filename;
    int y{};

    if (frameIndex == max) {
        std::cout << "Max reached\n";
        std::abort();
    }

    filename = "frame" + std::to_string(frameIndex) + ".ppm";

    std::FILE *pFile = std::fopen(filename.c_str(), "wb");
    if (pFile == nullptr) {
        return;
    }

    // Write header
    std::fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++) {
        std::fwrite(avFrame->data[0] + (y * avFrame->linesize[0]), 1, width * 3, pFile);
    }

    // Close file
    std::fclose(pFile);
}
}  // namespace utils

#endif /* TO_FILE_H_ */