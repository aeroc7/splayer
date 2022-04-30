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

#include "gl_texture.h"

#include <algorithm>

namespace graphics {

GlTexture::GlTexture(size_type width, size_type height) { regen_texture(width, height); }

GlTexture::GlTexture(GlTexture &&o) noexcept : tex_id{std::exchange(o.tex_id, NULL_TEXTURE)} {}

GlTexture &GlTexture::operator=(GlTexture &&o) noexcept {
    if (this != &o) {
        tex_id = std::exchange(o.tex_id, NULL_TEXTURE);
    }

    return *this;
}

void GlTexture::bind() const noexcept { glBindTexture(GL_TEXTURE_2D, tex_id); }

void GlTexture::unbind() const noexcept { glBindTexture(GL_TEXTURE_2D, 0); }

void GlTexture::regen_texture(int width, int height) noexcept {
    // Calls delete on texture
    create_new_texture(width, height);
}

void GlTexture::try_delete_texture() noexcept {
    if (tex_id > 0) {
        glDeleteTextures(1, &tex_id);
        tex_id = 0;
    }
}

void GlTexture::create_new_texture(int width, int height) noexcept {
    try_delete_texture();

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GlTexture::~GlTexture() { try_delete_texture(); }

}  // namespace graphics