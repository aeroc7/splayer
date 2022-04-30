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

#ifndef GL_TEXTURE_H_
#define GL_TEXTURE_H_

#include <GL/glew.h>

namespace graphics {
class GlTexture final {
public:
    using size_type = int;
    using tex_type = GLuint;

    GlTexture(size_type width, size_type height);
    GlTexture(const GlTexture &) = delete;
    GlTexture &operator=(const GlTexture &) = delete;
    GlTexture(GlTexture &&) noexcept;
    GlTexture &operator=(GlTexture &&) noexcept;
    ~GlTexture();

    void bind() const noexcept;
    void unbind() const noexcept;
    void regen_texture(size_type width, size_type height) noexcept;

private:
    static constexpr auto NULL_TEXTURE = 0;
    void try_delete_texture() noexcept;
    void create_new_texture(size_type width, size_type height) noexcept;

    tex_type tex_id{NULL_TEXTURE};
};
}  // namespace graphics

#endif /* GL_TEXTURE_H_ */