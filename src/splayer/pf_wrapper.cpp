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

#include "pf_wrapper.h"

#include <ctime>

#include "utils.h"

namespace utils {
long gettime_highres() {
#ifdef LIN
    struct timespec tp;
    VERIF0(clock_gettime(CLOCK_MONOTONIC, &tp));
    return (tp.tv_sec * 1000000000L) + tp.tv_nsec;
#else
    return {};
#endif
}

long gettime_seconds() {
#ifdef LIN
    struct timespec tp;
    VERIF0(clock_gettime(CLOCK_MONOTONIC, &tp));
    return tp.tv_sec;
#else
    return {};
#endif
}

std::string formatted_logtime() {
#ifdef LIN
    std::tm tbuf, *now;
    time_t t;
    char strbuf[16];

    t = std::time(nullptr);
    now = localtime_r(&t, &tbuf);
    std::strftime(strbuf, sizeof(strbuf), "%H:%M:%S", now);

    return strbuf;
#else
    return {};
#endif
}
}  // namespace utils