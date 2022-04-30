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

#ifndef LOG_H_
#define LOG_H_

#include <cstdint>
#include <iostream>

#include "pf_wrapper.h"
#include "str_utils.h"

namespace utils {
struct SourceLocation {
    constexpr SourceLocation(const char *f, const char *func, std::uint_least32_t l)
        : filename(f), function(func), line(l) {}
    constexpr const char *get_filename() const noexcept { return filename; }
    constexpr std::uint_least32_t get_line() const noexcept { return line; }
    constexpr const char *get_function() const noexcept { return function; }

    static constexpr SourceLocation current(
#if defined(__clang__) || defined(__GNUC__)
        const char *fileName = __builtin_FILE(), const char *functionName = __builtin_FUNCTION(),
        std::uint_least32_t line = __builtin_LINE()) noexcept {
#else
#error "unsupported";
#endif
        return SourceLocation(fix_filename_path(fileName), functionName, line);
    }

private:
    static constexpr const char *fix_filename_path(const char *fn) noexcept {
        const auto fn_split = filename_split(fn);
        const auto sz = fn_split.size();
        auto fn_data = fn_split.data();

        if (fn_data[sz] != '\0') {
            fn_data = "invalid_path";
        }

        return fn_data;
    }

    const char *filename;
    const char *function;
    std::uint_least32_t line;
};

class Log {
public:
    enum class LogType : std::uint8_t { Info = 0, Error, Verbose };
    static constexpr auto INFO = LogType::Info;
    static constexpr auto ERROR = LogType::Error;
    static constexpr auto VERBOSE = LogType::Verbose;

    Log(LogType logtype = INFO, SourceLocation slc = SourceLocation::current())
        : out_log(log_stream(logtype)) {
        out_log << LOGGER_HEADER << ' ' << formatted_logtime() << " [" << slc.get_filename() << ':'
                << slc.get_line() << "] " << get_logtype_str(logtype) << ": ";
    }

    ~Log() { out_log << '\n'; }
    template <typename T>
    std::ostream &operator<<(const T &msg) noexcept {
        out_log << msg;
        return out_log;
    }

private:
    std::ostream &log_stream(LogType lt) const noexcept {
        switch (lt) {
            case LogType::Info:
            case LogType::Verbose:
                return std::cout;
            case LogType::Error:
                return std::cerr;
        }

        return std::cout;
    }

    constexpr std::string_view get_logtype_str(LogType lt) const noexcept {
        switch (lt) {
            case LogType::Info:
                return "Info";
            case LogType::Error:
                return "Error";
            case LogType::Verbose:
                return "Verbose";
        }

        return "Info";
    }

    std::ostream &out_log;
};

}  // namespace utils

#endif /* LOG_H_ */