/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui_impl_opengl3.h"
#include "shaders_opengl3.h"
#include "gui/graphics/tilecollisionimage.h"
#include "gui/style.h"
#include "gui/windows/message-box.h"
#include "models/common/optional.h"
#include <iostream>

namespace UnTech::Gui::Shaders {

static bool g_initialized = false;

static GLuint g_tilesetFramebuffer = 0;

static MtTileset* mtTilesetUpdateRequested = nullptr;

MtTileset::~MtTileset()
{
    if (mtTilesetUpdateRequested == this) {
        mtTilesetUpdateRequested = nullptr;
    }
}

void MtTileset::requestUpdate()
{
    mtTilesetUpdateRequested = this;
}

static void CheckShader(GLuint handle, const char* name)
{
    GLint status = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

    GLint logLength = 0;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    const bool valid = (GLboolean)status != GL_FALSE;

    if (!valid) {
        std::cerr << "ERROR compiling " << name << std::endl;
    }

    if (logLength > 1) {
        std::vector<GLchar> message(logLength);
        glGetShaderInfoLog(handle, logLength, nullptr, message.data());

        if (valid) {
            std::cerr << "INFO LOG for " << name << std::endl;
        }
        std::cerr << message.data()
                  << std::endl;
    }
}

static void CheckProgram(GLuint handle, const char* name)
{
    GLint status = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);

    GLint logLength = 0;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    const bool valid = (GLboolean)status != GL_FALSE;

    if (!valid) {
        std::cerr << "ERROR linking " << name << std::endl;
    }

    if (logLength > 1) {
        std::vector<GLchar> message(logLength);
        glGetProgramInfoLog(handle, logLength, nullptr, message.data());

        if (valid) {
            std::cerr << "INFO LOG for " << name << std::endl;
        }
        std::cerr << message.data()
                  << std::endl;
    }
}

namespace MtTilesetVertexShader {
static const GLchar* vertex_shader = R"glsl(
#version 130

in vec2 Position;
in vec2 UV;

out vec2 Frag_UV;

void main()
{
    Frag_UV = UV;
    gl_Position = vec4(Position, 0.0, 1.0);
}
)glsl";

static constexpr float vertices[] = {
    // Position,  UV
    -1.0f, -1.0f, 0.0f, 0.0f, // top left
    +1.0f, -1.0f, 1.0f, 0.0f, // top right
    +1.0f, +1.0f, 1.0f, 1.0f, // bottom right

    -1.0f, -1.0f, 0.0f, 0.0f, // top left
    -1.0f, +1.0f, 0.0f, 1.0f, // bottom left
    +1.0f, +1.0f, 1.0f, 1.0f, // bottom right
};

static GLint g_vertexHangle = 0;
static GLuint g_vertexBuffer = 0;

static void initialize()
{
    g_vertexHangle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(g_vertexHangle, 1, &vertex_shader, 0);
    glCompileShader(g_vertexHangle);
    CheckShader(g_vertexHangle, "Full Screen vertex shader");

    glGenBuffers(1, &g_vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, g_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

static void cleanup()
{
    if (g_vertexHangle) {
        glDeleteShader(g_vertexHangle);
        g_vertexHangle = 0;
    }

    if (g_vertexBuffer) {
        glDeleteBuffers(1, &g_vertexBuffer);
        g_vertexBuffer = 0;
    }
}

static void draw(const GLint attribPosition, const GLint attribUV)
{
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexBuffer);
    glEnableVertexAttribArray(attribPosition);
    glEnableVertexAttribArray(attribUV);

    glVertexAttribPointer(attribPosition, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(attribUV, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

}

namespace TileCollisions {

const GLchar* tc_fragment_shader = R"glsl(
#version 130

uniform sampler2D TctTexture;
uniform usampler2D TileCollisions;
uniform vec4 Color;

in vec2 Frag_UV;

void main()
{
    int tc = int(texture(TileCollisions, Frag_UV.st).x);

    ivec2 tilePos = ivec2(0, (tc & 0x1f) * 16);

    ivec2 subPos = ivec2(Frag_UV * 256) & 0xf;

    gl_FragColor = Color * texelFetch(TctTexture, tilePos + subPos, 0);
}
)glsl";

static const usize TCT_TEXTURE_SIZE(16, 32 * 16);

static GLuint g_shaderHandle = 0;
static GLint g_uniformTctTexture = 0;
static GLint g_uniformTileCollisions = 0;
static GLint g_uniformColor = 0;
static GLint g_attribPosition = 0;
static GLint g_attribUV = 0;

static void initialize()
{
    if (g_initialized) {
        return;
    }

    // Create the TileCollisionType texture if it doesn't already exist
    tileCollisionTypeTexture();

    GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragHandle, 1, &tc_fragment_shader, 0);
    glCompileShader(fragHandle);
    CheckShader(fragHandle, "Tile Collision fragment shader");

    g_shaderHandle = glCreateProgram();
    glAttachShader(g_shaderHandle, MtTilesetVertexShader::g_vertexHangle);
    glAttachShader(g_shaderHandle, fragHandle);
    glLinkProgram(g_shaderHandle);
    CheckProgram(g_shaderHandle, "Tile Collision shader program");

    g_uniformTctTexture = glGetUniformLocation(g_shaderHandle, "TctTexture");
    g_uniformTileCollisions = glGetUniformLocation(g_shaderHandle, "TileCollisions");
    g_uniformColor = glGetUniformLocation(g_shaderHandle, "Color");

    g_attribPosition = glGetAttribLocation(g_shaderHandle, "Position");
    g_attribUV = glGetAttribLocation(g_shaderHandle, "UV");

    assert(g_attribUV >= 0);

    glDeleteShader(fragHandle);
}

static void cleanup()
{
    if (g_shaderHandle) {
        glDeleteProgram(g_shaderHandle);
        g_shaderHandle = 0;
    }
}

static void draw(const Texture8& tcData)
{
    const auto& tctTexture = tileCollisionTypeTexture();
    assert(tctTexture.size() == TCT_TEXTURE_SIZE);

    glUseProgram(g_shaderHandle);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tcData.openGLTextureId());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tctTexture.openGLTextureId());

    glUniform1i(g_uniformTileCollisions, 1);
    glUniform1i(g_uniformTctTexture, 0);

    glUniform4fv(g_uniformColor, 1, (float*)&Style::tileCollisionTint);

    MtTilesetVertexShader::draw(g_attribPosition, g_attribUV);
}

}

namespace Tilemap {

static const GLchar* tilemap_vertex_shader = R"glsl(
#version 130

uniform mat4 ProjMtx;

in vec2 Position;
in vec2 UV;

out vec2 Frag_UV;

void main()
{
    Frag_UV = UV;

    gl_Position = ProjMtx * vec4(Position.xy, 0.0f, 1.0f);
}
)glsl";

const GLchar* tilemap_fragment_shader = R"glsl(
#version 130

uniform sampler2D Tileset;
uniform usampler2D Map;
uniform vec2 MapSize;

in vec2 Frag_UV;

void main()
{
    int tile = int(texture(Map, Frag_UV.st).x);

    ivec2 tilePos = ivec2((tile & 0xf) * 16, tile & 0xf0);

    ivec2 subPos = ivec2(Frag_UV * MapSize) & 0xf;

    gl_FragColor = texelFetch(Tileset, tilePos + subPos, 0);
}
)glsl";

static GLuint g_shaderHandle = 0;
static GLint g_uniformProjMtx = 0;
static GLint g_uniformTileset = 0;
static GLint g_uniformMap = 0;
static GLint g_uniformMapSize = 0;
static GLint g_attribPosition = 0;
static GLint g_attribUV = 0;
static GLuint g_vertexBuffer = 0;

static void initialize()
{
    if (g_initialized) {
        return;
    }

    GLuint vertHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertHandle, 1, &tilemap_vertex_shader, 0);
    glCompileShader(vertHandle);
    CheckShader(vertHandle, "MetaTile vertex shader");

    GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragHandle, 1, &tilemap_fragment_shader, 0);
    glCompileShader(fragHandle);
    CheckShader(fragHandle, "MetaTile fragment shader");

    g_shaderHandle = glCreateProgram();
    glAttachShader(g_shaderHandle, vertHandle);
    glAttachShader(g_shaderHandle, fragHandle);
    glLinkProgram(g_shaderHandle);
    CheckProgram(g_shaderHandle, "MetaTile shader program");

    g_uniformProjMtx = glGetUniformLocation(g_shaderHandle, "ProjMtx");
    g_uniformTileset = glGetUniformLocation(g_shaderHandle, "Tileset");
    g_uniformMap = glGetUniformLocation(g_shaderHandle, "Map");
    g_uniformMapSize = glGetUniformLocation(g_shaderHandle, "MapSize");

    g_attribPosition = glGetAttribLocation(g_shaderHandle, "Position");
    g_attribUV = glGetAttribLocation(g_shaderHandle, "UV");

    glGenBuffers(1, &g_vertexBuffer);

    glDeleteShader(vertHandle);
    glDeleteShader(fragHandle);
}

static void cleanup()
{
    if (g_vertexBuffer) {
        glDeleteBuffers(1, &g_vertexBuffer);
        g_vertexBuffer = 0;
    }

    if (g_shaderHandle) {
        glDeleteProgram(g_shaderHandle);
        g_shaderHandle = 0;
    }
}

}

void initialize()
{
    if (g_initialized) {
        return;
    }

    mtTilesetUpdateRequested = nullptr;

    glGenFramebuffers(1, &g_tilesetFramebuffer);

    MtTilesetVertexShader::initialize();
    TileCollisions::initialize();
    Tilemap::initialize();
}

void cleanup()
{
    mtTilesetUpdateRequested = nullptr;

    glDeleteFramebuffers(1, &g_tilesetFramebuffer);

    MtTilesetVertexShader::cleanup();
    TileCollisions::cleanup();
    Tilemap::cleanup();

    g_initialized = false;
}

void drawMtTilemap(const ImDrawList*, const ImDrawCmd* pcmd)
{
    using namespace Gui::Shaders::Tilemap;

    const MtTilemapRenderData* data = static_cast<MtTilemapRenderData*>(pcmd->UserCallbackData);

    glUseProgram(g_shaderHandle);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data->tilesetTextureId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->mapTextureId);

    glUniform1i(g_uniformTileset, 1);
    glUniform1i(g_uniformMap, 0);

    glUniform2f(g_uniformMapSize, data->mapSize.x, data->mapSize.y);

    glUniformMatrix4fv(g_uniformProjMtx, 1, GL_FALSE, ImGui_Projection_Matrix.data());

    const float vertices[] = {
        // Position,        UV
        data->x1, data->y1, 0.0f, 0.0f, // top left
        data->x2, data->y1, 1.0f, 0.0f, // top right
        data->x2, data->y2, 1.0f, 1.0f, // bottom right

        data->x1, data->y1, 0.0f, 0.0f, // top left
        data->x1, data->y2, 0.0f, 1.0f, // bottom left
        data->x2, data->y2, 1.0f, 1.0f, // bottom right
    };

    glBindBuffer(GL_ARRAY_BUFFER, g_vertexBuffer);
    glEnableVertexAttribArray(g_attribPosition);
    glEnableVertexAttribArray(g_attribUV);

    glVertexAttribPointer(g_attribPosition, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(g_attribUV, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void updateMtTileset(const MtTileset& tileset)
{
    const auto& texture = tileset.texture();

    glEnable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);

    glDepthMask(GL_FALSE);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, texture.width(), texture.height());

    glBindFramebuffer(GL_FRAMEBUFFER, g_tilesetFramebuffer);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture.openGLTextureId(), 0);

    if (tileset.showTiles()) {
        // Copy tilesTexture to tileset texture

        const auto& tilesTexture = tileset.tilesTexture();

        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                               tilesTexture.openGLTextureId(), 0);
        glReadBuffer(GL_COLOR_ATTACHMENT1);

        glBlitFramebuffer(0, 0, tilesTexture.width(), tilesTexture.height(),
                          0, 0, texture.width(), texture.height(),
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    else {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // Draw Tile Collisions
    if (tileset.showTileCollisions()) {
        using namespace TileCollisions;

        TileCollisions::draw(tileset.tileCollisionsData());
    }

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void processOffscreenRendering()
{
    if (mtTilesetUpdateRequested) {
        updateMtTileset(*mtTilesetUpdateRequested);

        //mtTilesetUpdateRequested = nullptr;
    }
}

}
