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

#ifndef WINDOW_H_
#define WINDOW_H_

#include <functional>
#include <string>

struct GLFWwindow;

namespace graphics {
enum class InputKeyType {
    SHIFT,
    CTRL,
    DEL,
    ENTER,
    TAB,
    BACKSPACE,
    COPY,
    CUT,
    PASTE,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class InputStatType {
    MOUSE_MOVE,
    LEFT_MOUSE_PRESS,
    LEFT_MOUSE_RELEASE,
    KEY_PRESS,
    KEY_RELEASE,
    KEY_INPUT
};

struct InputStats {
    InputStatType type;
    std::uint_fast32_t key;
    std::uint_fast32_t x_pos;
    std::uint_fast32_t y_pos;
};

class Window {
public:
    using InputCbSignature = std::function<void(InputStats)>;
    Window();
    Window(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = delete;
    void create_window(const std::string &title, int w, int h);
    void window_loop(std::function<void()> func);
    std::tuple<int, int> get_window_dims() const { return {window_width, window_height}; }

    std::tuple<int, int> query_true_window_dims();

    void set_input_cb(InputCbSignature cb) { input_cb = cb; }

    ~Window();

private:
    static void glfw_cursor_button_callback(
        GLFWwindow *window, int button, int action, int mods) noexcept;
    static void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) noexcept;
    static void glfw_error_callback(int error, const char *description);
    static void glfw_char_callback(GLFWwindow *window, unsigned int codepoint) noexcept;
    static void glfw_key_callback(
        GLFWwindow *window, int key, int scancode, int action, int mods) noexcept;
    static void set_with_click_ratio(Window *win, double &xpos, double &ypos);
    GLFWwindow *window{nullptr};
    int window_width{}, window_height{};
    int initial_win_width{}, initial_win_height{};
    InputCbSignature input_cb;
};
}  // namespace graphics

#endif /* WINDOW_H_ */