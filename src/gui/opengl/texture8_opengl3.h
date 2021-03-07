/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "opengl3.h"
#include "models/common/grid.h"
#include "vendor/imgui/imgui.h"

namespace UnTech::Gui {

struct Texture8 {
private:
    Texture8(const Texture8&) = delete;
    Texture8(Texture8&&) = delete;
    Texture8& operator=(const Texture8&) = delete;
    Texture8& operator=(Texture8&&) = delete;

private:
    GLuint _textureId;
    usize _size;

public:
    Texture8(const unsigned width = 0, const unsigned height = 0)
    {
        const bool hasSize = width > 0 && height > 0;
        _size.width = hasSize ? width : 0;
        _size.height = hasSize ? height : 0;

        GLuint oldTexture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&oldTexture);

        glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (hasSize) {
            glBindTexture(GL_TEXTURE_2D, _textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, _size.width, _size.height, 0,
                         GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
        }

        glBindTexture(GL_TEXTURE_2D, oldTexture);
    }

    Texture8(const usize& size)
        : Texture8(size.width, size.height)
    {
    }

    ~Texture8()
    {
        glDeleteTextures(1, &_textureId);
    }

    GLuint openGLTextureId() const { return _textureId; };
    ImTextureID imguiTextureId() const { return (ImTextureID)(intptr_t)_textureId; };

    const usize& size() const { return _size; }
    unsigned width() const { return _size.width; }
    unsigned height() const { return _size.height; }

    void setData(const usize& size, const uint8_t* data)
    {
        const bool sameSize = _size == size;
        _size = size;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glBindTexture(GL_TEXTURE_2D, _textureId);

        if (sameSize) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width, size.height,
                            GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
        }
        else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, size.width, size.height, 0,
                         GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    inline void setData(const grid<uint8_t>& data)
    {
        setData(data.size(), data.gridData().data());
    }
};

}
