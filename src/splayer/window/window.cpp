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

#include "window.h"

#include <GLFW/glfw3.h>
#include <splayer/util/log.h>

#include <cmath>
#include <iostream>

namespace graphics {
static bool cursor_pos_checks(double xpos, double ypos) noexcept {
    if (xpos < 0 || ypos < 0) {
        return false;
    }

    if (std::isnan(xpos) || std::isnan(ypos)) {
        return false;
    }

    return true;
}

void Window::set_with_click_ratio(Window *win, double &xpos, double &ypos) {
    const auto [w, h] = win->get_window_dims();

    if (w == 0 || h == 0) {
        return;
    }

    const double wratio = w / static_cast<double>(win->initial_win_width);
    const double hratio = h / static_cast<double>(win->initial_win_height);

    xpos *= wratio;
    ypos *= hratio;
}

void Window::glfw_error_callback([[maybe_unused]] int error, const char *description) {
    utils::Log(utils::Log::ERROR) << "glfw error: " << description;
}

void Window::glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) noexcept {
    Window *us = static_cast<Window *>(glfwGetWindowUserPointer(window));
    set_with_click_ratio(us, xpos, ypos);

    if (!cursor_pos_checks(xpos, ypos)) {
        return;
    }

    if (us->input_cb) {
        us->input_cb({.type = InputStatType::MOUSE_MOVE,
            .x_pos = static_cast<std::uint_fast32_t>(xpos),
            .y_pos = static_cast<std::uint_fast32_t>(ypos)});
    }
}

void Window::glfw_cursor_button_callback(GLFWwindow *window, int button, int action, int) noexcept {
    Window *us = static_cast<Window *>(glfwGetWindowUserPointer(window));
    double xpos, ypos;

    if (!us->input_cb) {
        return;
    }

    glfwGetCursorPos(window, &xpos, &ypos);
    set_with_click_ratio(us, xpos, ypos);
    if (!cursor_pos_checks(xpos, ypos)) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        us->input_cb({.type = InputStatType::LEFT_MOUSE_PRESS,
            .x_pos = static_cast<std::uint_fast32_t>(xpos),
            .y_pos = static_cast<std::uint_fast32_t>(ypos)});
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        us->input_cb({.type = InputStatType::LEFT_MOUSE_RELEASE,
            .x_pos = static_cast<std::uint_fast32_t>(xpos),
            .y_pos = static_cast<std::uint_fast32_t>(ypos)});
    }
}

void Window::glfw_char_callback(GLFWwindow *window, unsigned int key) noexcept {
    Window *us = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (!std::isalnum(static_cast<unsigned char>(key))) {
        return;
    }

    if (!us->input_cb) {
        return;
    }

    us->input_cb({.type = InputStatType::KEY_INPUT, .key = static_cast<std::uint_fast32_t>(key)});
}

void Window::glfw_key_callback(
    GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/) noexcept {
    Window *us = static_cast<Window *>(glfwGetWindowUserPointer(window));

    InputStats stat;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        stat.type = InputStatType::KEY_PRESS;
    } else {
        stat.type = InputStatType::KEY_RELEASE;
    }

    switch (key) {
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::SHIFT);
            break;
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::CTRL);
            break;
        case GLFW_KEY_DELETE:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::DEL);
            break;
        case GLFW_KEY_ENTER:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::ENTER);
            break;
        case GLFW_KEY_TAB:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::TAB);
            break;
        case GLFW_KEY_BACKSPACE:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::BACKSPACE);
            break;
        case GLFW_KEY_UP:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::UP);
            break;
        case GLFW_KEY_DOWN:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::DOWN);
            break;
        case GLFW_KEY_LEFT:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::LEFT);
            break;
        case GLFW_KEY_RIGHT:
            stat.key = static_cast<std::uint_fast32_t>(InputKeyType::RIGHT);
            break;
        default:
            return;
    }

    us->input_cb(stat);
}

Window::Window() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }
}

std::tuple<int, int> Window::query_true_window_dims() {
    int win_width, win_height;
    glfwGetFramebufferSize(window, &win_width, &win_height);
    window_width = win_width;
    window_height = win_height;
    return get_window_dims();
}

void Window::window_loop(std::function<void()> func) {
    if (!func) {
        throw std::runtime_error("Invalid window loop cb passed");
    }

    while (!glfwWindowShouldClose(window)) {
        int win_width, win_height;
        glfwGetFramebufferSize(window, &win_width, &win_height);
        window_width = win_width;
        window_height = win_height;

        glViewport(0, 0, win_width, win_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, win_width, win_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        // User-provided callback
        func();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Window::create_window(const std::string &title, int w, int h) {
    initial_win_width = w;
    initial_win_height = h;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("Failed to create glfw window");
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
    glfwSetMouseButtonCallback(window, glfw_cursor_button_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    glfwSetKeyCallback(window, glfw_key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

}  // namespace graphics