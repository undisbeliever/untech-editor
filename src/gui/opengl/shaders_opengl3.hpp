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
#include "models/common/imagecache.h"
#include "models/common/optional.h"
#include <iostream>

namespace UnTech::Gui::Shaders {

static bool g_initialized = false;

static std::vector<MtTileset*> mtTilesetInstances;

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

namespace MtTilesetTilesShader {

const GLchar* fragment_shader = R"glsl(
#version 130

uniform usampler2D Texture;
uniform sampler2D Palette;
uniform int PalFrame;

in vec2 Frag_UV;

void main()
{
    int c = int(texture(Texture, Frag_UV.st).x);

    gl_FragColor = texelFetch(Palette, ivec2(c, PalFrame), 0);
}
)glsl";

static const usize TCT_TEXTURE_SIZE(16, 32 * 16);

static GLuint g_shaderHandle = 0;
static GLint g_uniformTexture = 0;
static GLint g_uniformPalette = 0;
static GLint g_uniformPalFrame = 0;
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
    glShaderSource(fragHandle, 1, &fragment_shader, 0);
    glCompileShader(fragHandle);
    CheckShader(fragHandle, "MtTileset Tiles fragment shader");

    g_shaderHandle = glCreateProgram();
    glAttachShader(g_shaderHandle, MtTilesetVertexShader::g_vertexHangle);
    glAttachShader(g_shaderHandle, fragHandle);
    glLinkProgram(g_shaderHandle);
    CheckProgram(g_shaderHandle, "MtTileset Tiles shader program");

    g_uniformTexture = glGetUniformLocation(g_shaderHandle, "Texture");
    g_uniformPalette = glGetUniformLocation(g_shaderHandle, "Palette");
    g_uniformPalFrame = glGetUniformLocation(g_shaderHandle, "PalFrame");

    g_attribPosition = glGetAttribLocation(g_shaderHandle, "Position");
    g_attribUV = glGetAttribLocation(g_shaderHandle, "UV");

    glDeleteShader(fragHandle);
}

static void cleanup()
{
    if (g_shaderHandle) {
        glDeleteProgram(g_shaderHandle);
        g_shaderHandle = 0;
    }
}

static void draw(const Texture8& image, const Texture& palette, unsigned paletteFrame)
{
    const auto& tctTexture = tileCollisionTypeTexture();
    assert(tctTexture.size() == TCT_TEXTURE_SIZE);

    glUseProgram(g_shaderHandle);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, image.openGLTextureId());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, palette.openGLTextureId());

    glUniform1i(g_uniformTexture, 1);
    glUniform1i(g_uniformPalette, 0);

    glUniform1i(g_uniformPalFrame, paletteFrame);

    MtTilesetVertexShader::draw(g_attribPosition, g_attribUV);
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

// Memory safety: This struct MUST exist when draw data is being rendered.
struct RenderData {
private:
    RenderData(const RenderData&) = delete;
    RenderData(RenderData&&) = delete;
    RenderData& operator=(const RenderData&) = delete;
    RenderData& operator=(RenderData&&) = delete;

public:
    GLuint tilesetTextureId;
    GLuint mapTextureId;

    ImVec2 mapSize;
    float x1, y1, x2, y2;

    RenderData() = default;
};
static std::array<RenderData, 8> renderDataBuffer;
static unsigned renderDataCount = 0;

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

    MtTilesetVertexShader::initialize();
    MtTilesetTilesShader::initialize();
    TileCollisions::initialize();
    Tilemap::initialize();

    g_initialized = true;
}

void cleanup()
{
    // all MtTileset instances should be destroyed before calling cleanup().
    assert(mtTilesetInstances.empty());

    MtTilesetVertexShader::cleanup();
    MtTilesetTilesShader::cleanup();
    TileCollisions::cleanup();
    Tilemap::cleanup();

    g_initialized = false;
}

void drawMtTilemap(const ImDrawList*, const ImDrawCmd* pcmd)
{
    using namespace Gui::Shaders::Tilemap;

    const RenderData* data = static_cast<RenderData*>(pcmd->UserCallbackData);

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

MtTileset::MtTileset()
    : _texture(TEXTURE_SIZE, TEXTURE_SIZE)
    , _tilesTexture(TEXTURE_SIZE, TEXTURE_SIZE)
    , _tileCollisionsData(TC_TEXTURE_SIZE, TC_TEXTURE_SIZE)
    , _palette()
    , _paletteFrame(0)
    , _tilesetFrames()
    , _nTilesetFrames(0)
    , _tilesetFrame(0)
    , _textureFrameBuffer(0)
    , _tilesTextureFrameBuffer(0)
    , _textureValid(true)
    , _tilesTextureValid(true)
    , _showTiles(true)
    , _showTileCollisions(true)
{
    // Add to list of MtTileset instances
    mtTilesetInstances.push_back(this);

    // Setup _texture FBO
    {
        glGenFramebuffers(1, &_textureFrameBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, _textureFrameBuffer);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               _texture.openGLTextureId(), 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                               _tilesTexture.openGLTextureId(), 0);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
    }

    // Setup _tilesTexture FBO
    {
        glGenFramebuffers(1, &_tilesTextureFrameBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, _tilesTextureFrameBuffer);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               _tilesTexture.openGLTextureId(), 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

MtTileset::~MtTileset()
{
    // Remove from list of MtTileset instances
    auto it = std::find(mtTilesetInstances.begin(), mtTilesetInstances.end(), this);
    assert(it != mtTilesetInstances.end());
    mtTilesetInstances.erase(it);

    glDeleteFramebuffers(1, &_textureFrameBuffer);
    glDeleteFramebuffers(1, &_tilesTextureFrameBuffer);
}

inline void MtTileset::drawTextures_openGL()
{
    if (_tilesTextureValid && _textureValid) {
        return;
    }

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);

    glDepthMask(GL_FALSE);

    glViewport(0, 0, _texture.width(), _texture.height());

    if (!_tilesTextureValid) {
        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, _tilesTextureFrameBuffer);

        bool valid = _tilesetData && _paletteData
                     && _tilesetFrame < _tilesetFrames.size()
                     && _paletteFrame < _palette.height();

        if (valid) {
            MtTilesetTilesShader::draw(_tilesetFrames.at(_tilesetFrame), _palette, _paletteFrame);
        }
        else {
            std::shared_ptr<const Image> image;
            if (_tilesetFrame < _tilesetImageFilenames.size()) {
                image = ImageCache::loadPngImage(_tilesetImageFilenames.at(_tilesetFrame));
            }

            if (image && image->size() == usize(TEXTURE_SIZE, TEXTURE_SIZE)) {
                _tilesTexture.replace(*image);
            }
            else {
                glClearColor(0.75f, 0.75f, 0.75f, 0.5f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
        }

        _tilesTextureValid = true;
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindFramebuffer(GL_FRAMEBUFFER, _textureFrameBuffer);

    if (_showTiles) {
        // Copy tilesTexture to tileset texture

        glBlitFramebuffer(0, 0, _tilesTexture.width(), _tilesTexture.height(),
                          0, 0, _texture.width(), _texture.height(),
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    else {
        // Clear buffer
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // Draw Tile Collisions
    if (_showTileCollisions) {
        TileCollisions::draw(_tileCollisionsData);
    }

    _textureValid = true;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MtTilemap::addToDrawList(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size,
                              const MtTileset& tileset) const
{
    using namespace Tilemap;

    if (renderDataCount >= renderDataBuffer.size()) {
        std::cerr << "Too many MtTilemap draw calls per frame" << std::endl;
        return;
    }

    RenderData& rd = renderDataBuffer.at(renderDataCount);
    renderDataCount++;

    rd.tilesetTextureId = tileset.texture().openGLTextureId();
    rd.mapTextureId = _texture.openGLTextureId();
    rd.mapSize = _mapSize;

    rd.x1 = pos.x;
    rd.y1 = pos.y;
    rd.x2 = pos.x + size.x;
    rd.y2 = pos.y + size.y;

    drawList->AddCallback(&drawMtTilemap, &rd);
    drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void newFrame()
{
    Tilemap::renderDataCount = 0;
}

void processOffscreenRendering()
{
    for (auto* mt : mtTilesetInstances) {
        mt->drawTextures_openGL();
    }
}

}
