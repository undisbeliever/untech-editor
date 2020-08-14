/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "models/common/image.h"
#include "vendor/imgui/imgui.h"

// Taken from `imgui/examples/example_sdl_opengl3/main.cpp`
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h> // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE      // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h> // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

namespace UnTech::Gui {

class Texture {
public:
    const static UnTech::Image missingImageSymbol;

private:
    GLuint _textureId;
    usize _size;

public:
    // No copying allowed
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Moving is OK
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;

    Texture(const unsigned width = 0, const unsigned height = 0)
    {
        const bool hasSize = width > 0 && height > 0;
        _size.width = hasSize ? width : 0;
        _size.height = hasSize ? height : 0;

        glGenTextures(1, &_textureId);

        if (hasSize) {
            glBindTexture(GL_TEXTURE_2D, _textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _size.width, _size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
    }

    Texture(const usize& size)
        : Texture(size.width, size.height)
    {
    }

    ~Texture()
    {
        glDeleteTextures(1, &_textureId);
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
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (image.size() == _size) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _size.width, _size.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        }
        else {
            _size = image.size();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _size.width, _size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        }
    }

    void replaceWithMissingImageSymbol()
    {
        replace(missingImageSymbol);
    }
};

}
