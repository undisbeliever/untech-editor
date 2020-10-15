/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "opengl3.h"
#include "texture8_opengl3.h"
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
    Texture8 _texture;
    ImVec2 _mapSize;
    bool _empty;

public:
    MtTilemap()
        : _texture()
        , _mapSize(0, 0)
        , _empty(true)
    {
    }

    ~MtTilemap() = default;

    const Texture8& texture() const { return _texture; }

    const usize& gridSize() const { return _texture.size(); }
    const ImVec2& mapSize() const { return _mapSize; }

    bool empty() const { return _empty; }

    void setMapData(const grid<uint8_t>& data)
    {
        _empty = data.empty();

        _mapSize.x = data.width() * 16;
        _mapSize.y = data.height() * 16;

        if (!data.empty()) {
            _texture.setData(data);
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
        mapTextureId = tilemap.texture().openGLTextureId();
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
