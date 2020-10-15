/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "opengl3.h"
#include "texture_opengl3.hpp"
#include "models/common/grid.h"

namespace UnTech::Gui::Shaders {

void drawMtTilemap(const ImDrawList*, const ImDrawCmd* pcmd);

struct MtTileset {
private:
    MtTileset(const MtTileset&) = delete;
    MtTileset(MtTileset&&) = delete;
    MtTileset& operator=(const MtTileset&) = delete;
    MtTileset& operator=(MtTileset&&) = delete;

private:
    Texture _tilesTexture;

    // ::TODO combine tileset collision texture with tiles texture::

public:
    MtTileset() = default;

    const Texture& texture() const { return _tilesTexture; }
    const Texture& tilesTexture() const { return _tilesTexture; }

    void reset() { _tilesTexture.replaceWithMissingImageSymbol(); }

    // ::TODO move updateTilesetTexture into this class::
    void setTextureImage(const Image* image)
    {
        if (image && image->size() == usize(256, 256)) {
            _tilesTexture.replace(*image);
        }
        else {
            _tilesTexture.replaceWithMissingImageSymbol();
        }
    }
};

struct MtTilemap {
private:
    MtTilemap(const MtTilemap&) = delete;
    MtTilemap(MtTilemap&&) = delete;
    MtTilemap& operator=(const MtTilemap&) = delete;
    MtTilemap& operator=(MtTilemap&&) = delete;

private:
    GLuint _textureId;
    usize _gridSize;
    ImVec2 _mapSize;
    bool _empty;

public:
    MtTilemap()
        : _textureId(0)
        , _gridSize(0, 0)
        , _mapSize(0, 0)
        , _empty(true)
    {
        GLuint oldTexture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&oldTexture);

        glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, oldTexture);
    }

    ~MtTilemap()
    {
        glDeleteTextures(1, &_textureId);
    }

    GLuint textureId() const { return _textureId; }
    const usize gridSize() const { return _gridSize; }
    const ImVec2& mapSize() const { return _mapSize; }

    bool empty() const { return _empty; }

    void setMapData(const grid<uint8_t>& data)
    {
        const bool sameSize = _gridSize == data.size();

        _empty = data.empty();

        _gridSize = data.size();

        _mapSize.x = data.width() * 16;
        _mapSize.y = data.height() * 16;

        if (!data.empty()) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glBindTexture(GL_TEXTURE_2D, _textureId);

            if (sameSize) {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.width(), data.height(),
                                GL_RED_INTEGER, GL_UNSIGNED_BYTE, data.gridData().data());
            }
            else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, data.width(), data.height(), 0,
                             GL_RED_INTEGER, GL_UNSIGNED_BYTE, data.gridData().data());
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
    }
};

// Memory safety: This struct MUST exist when draw data is being rendered.
struct MtTilemapRenderData {
private:
    MtTilemapRenderData(const MtTilemapRenderData&) = delete;
    MtTilemapRenderData(MtTilemapRenderData&&) = delete;
    MtTilemapRenderData& operator=(const MtTilemapRenderData&) = delete;
    MtTilemapRenderData& operator=(MtTilemapRenderData&&) = delete;

public:
    GLuint tilesetTextureId;
    GLuint mapTextureId;

    ImVec2 mapSize;
    float x1, y1, x2, y2;

    MtTilemapRenderData() = default;

    void addDrawCmd(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size,
                    const MtTileset& tileset, const MtTilemap& tilemap)
    {
        tilesetTextureId = tileset.texture().openGLTextureId();
        mapTextureId = tilemap.textureId();
        mapSize = tilemap.mapSize();

        x1 = pos.x;
        y1 = pos.y;
        x2 = pos.x + size.x;
        y2 = pos.y + size.y;

        drawList->AddCallback(&drawMtTilemap, this);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    }
};

}
