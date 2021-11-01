/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "opengl3.h"
#include "models/common/image.h"
#include "vendor/imgui/imgui.h"

namespace UnTech::Gui {

class Texture {
public:
    static const Image& missingImageSymbol();

private:
    GLuint _textureId;
    usize _size;

public:
    // No copying allowed
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = delete;

    // Returning from a function is OK
    Texture(Texture&& source)
        : _textureId(source._textureId)
        , _size(source._size)
    {
        source._textureId = 0;
    }

    ~Texture()
    {
        if (_textureId != 0) {
            glDeleteTextures(1, &_textureId);
        }
    }

    Texture(const unsigned width = 0, const unsigned height = 0)
    {
        const bool hasSize = width > 0 && height > 0;
        _size.width = hasSize ? width : 0;
        _size.height = hasSize ? height : 0;

        glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (hasSize) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _size.width, _size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
    }

    Texture(const usize& size)
        : Texture(size.width, size.height)
    {
    }

    static Texture createFromImage(const Image& image)
    {
        Texture t(image.size());
        t.replace(image);
        return t;
    }

    GLuint openGLTextureId() const { return _textureId; };
    ImTextureID imguiTextureId() const { return (void*)(intptr_t)_textureId; };

    const usize& size() const { return _size; }
    unsigned width() const { return _size.width; }
    unsigned height() const { return _size.height; }

    void replace(const UnTech::Image& image)
    {
        assert(_textureId != 0);

        glBindTexture(GL_TEXTURE_2D, _textureId);

        if (image.size() == _size) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _size.width, _size.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data().data());
        }
        else {
            _size = image.size();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _size.width, _size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data().data());
        }
    }

    void loadPngImage(const std::filesystem::path& filename);
    void replaceWithMissingImageSymbol();
};

}
