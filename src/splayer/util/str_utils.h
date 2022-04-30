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

#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include <string_view>

namespace utils {
constexpr std::string_view filename_split(std::string_view path) noexcept {
    const std::size_t last_path_sep = [&path] {
        const auto ps = path.find_last_of("/");
        return (ps == std::string::npos ? 0 : (ps + 1));
    }();
    return std::string_view(path.data() + last_path_sep, path.length() - last_path_sep);
}

class str {
public:
    enum class ss_error { out_of_range, str_empty, success };
    struct ss_return {
        std::string_view str;
        ss_error err;
    };

    static constexpr auto SPACE_DELIM = ' ';

    template <typename CharType>
    static constexpr auto str_isspace(CharType chr) noexcept {
        switch (chr) {
            case ' ':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
                return true;
            default:
                return false;
        }
    }

    // Returns string section from whitespace `index` to the end of the string
    static constexpr auto split_string_tend_sv(std::string_view str, std::size_t index) noexcept {
        const auto len = str.length();
        std::size_t index_ctr = 0;

        if (len == 0) {
            return ss_return{{}, ss_error::str_empty};
        }

        for (std::size_t i = 0; i < (len - 1); ++i) {
            const auto cur_is_space = str_isspace(str[i]);
            if (cur_is_space && !str_isspace(str[i + 1])) {
                index_ctr += 1;
            }

            if (index_ctr == index) {
                const std::string_view::const_iterator new_iter =
                    (cur_is_space ? (str.begin() + (i + 1)) : (str.begin() + i));
                const long new_iter_sz = str.end() - new_iter;
                return ss_return{{new_iter, static_cast<std::string_view::size_type>(new_iter_sz)},
                    ss_error::success};
            }
        }

        return ss_return{{}, ss_error::out_of_range};
    }

    // Returns string token at whitespace `index`.
    static constexpr auto split_string_sv(std::string_view str, unsigned index) noexcept {
        const auto len = str.length();
        std::size_t index_ctr{};

        if (len == 0) {
            return ss_return{{}, ss_error::str_empty};
        }

        for (std::size_t i = 0; i < (len - 1); ++i) {
            const auto cur_is_space = str_isspace(str[i]);
            if (cur_is_space && !str_isspace(str[i + 1])) {
                index_ctr += 1;
            }

            if (index_ctr == index) {
                const auto new_iter = (cur_is_space ? (str.begin() + (i + 1)) : (str.begin() + i));
                const auto next_space_pos = [&]() {
                    const char *it;
                    for (it = new_iter; it != str.end(); ++it) {
                        if (str_isspace(*it)) {
                            break;
                        }
                    }
                    return it;
                }();
                const long new_iter_sz = next_space_pos - new_iter;
                return ss_return{{new_iter, static_cast<std::string_view::size_type>(new_iter_sz)},
                    ss_error::success};
            }
        }

        return ss_return{{}, ss_error::out_of_range};
    }

    static constexpr bool cmp_equal_sv(std::string_view s1, std::string_view s2) noexcept {
        return (s1 == s2);
    }
};
}  // namespace utils

#endif /* STR_UTILS_H_ */