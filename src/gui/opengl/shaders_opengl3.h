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

namespace UnTech::MetaTiles {
enum class TileCollisionType : uint8_t;
}

namespace UnTech::Gui::Shaders {

void drawMtTilemap(const ImDrawList*, const ImDrawCmd* pcmd);

struct MtTileset {
private:
    MtTileset(const MtTileset&) = delete;
    MtTileset(MtTileset&&) = delete;
    MtTileset& operator=(const MtTileset&) = delete;
    MtTileset& operator=(MtTileset&&) = delete;

private:
    constexpr static unsigned TEXTURE_SIZE = 256;
    constexpr static unsigned TC_TEXTURE_SIZE = 16;
    constexpr static unsigned N_METATILES = 256;
    using TileCollisionData = std::array<MetaTiles::TileCollisionType, N_METATILES>;

private:
    Texture _texture;
    Texture _tilesTexture;
    Texture8 _tileCollisionsData;

    Texture _palette;
    unsigned _paletteFrame;

    std::array<Texture8, 32> _tilesetFrames;
    unsigned _nTilesetFrames;
    unsigned _tilesetFrame;

    GLuint _textureFrameBuffer = 0;
    GLuint _tilesTextureFrameBuffer = 0;

    bool _tilesTextureValid;

    bool _showTiles;
    bool _showTileCollisions;

public:
    MtTileset();
    ~MtTileset();

    // Must only be called by `processOffscreenRendering()`
    void drawTextures_openGL();

    bool showTiles() const { return _showTiles; }
    bool showTileCollisions() const { return _showTileCollisions; }

    const Texture& texture() const { return _texture; }
    const Texture& tilesTexture() const { return _tilesTexture; }

    const Texture8& tileCollisionsData() const { return _tileCollisionsData; }

    void reset()
    {
        _tilesTexture.replaceWithMissingImageSymbol();

        requestUpdate();
    }

    void setTileCollisions(const TileCollisionData& tileCollisions)
    {
        static_assert(sizeof(MetaTiles::TileCollisionType) == 1);
        static_assert(sizeof(TileCollisionData) == TC_TEXTURE_SIZE * TC_TEXTURE_SIZE);

        _tileCollisionsData.setData(usize(TC_TEXTURE_SIZE, TC_TEXTURE_SIZE),
                                    reinterpret_cast<const uint8_t*>(tileCollisions.data()));

        requestUpdate();
    }

    void requestUpdate();

    inline unsigned nPaletteFrames() const { return _palette.height(); }
    inline unsigned nTilesetFrames() const { return _nTilesetFrames; }

    inline unsigned paletteFrame() const { return _paletteFrame; }
    inline unsigned tilesetFrame() const { return _tilesetFrame; }

    void setShowTiles(bool s)
    {
        _showTiles = s;
        requestUpdate();
    }

    void setShowTileCollisions(bool s)
    {
        _showTileCollisions = s;
        requestUpdate();
    }

    void setPaletteFrame(unsigned n)
    {
        n = (nPaletteFrames() > 1) ? n % nPaletteFrames() : 0;

        if (n != _paletteFrame) {
            _paletteFrame = n;

            _tilesTextureValid = false;
            requestUpdate();
        }
    }

    void nextPaletteFrame()
    {
        setPaletteFrame(_paletteFrame + 1);
    }

    void setPaletteImage(const Image& pal)
    {
        const bool sizeChanged = _palette.size() != pal.size();

        _palette.replace(pal);

        assert(pal.size().height == 0 || pal.size().width == UINT8_MAX);

        if (sizeChanged) {
            setPaletteFrame(0);
        }

        _tilesTextureValid = false;
        requestUpdate();
    }

    void setTilesetFrame(unsigned n)
    {
        n = (_nTilesetFrames > 1) ? n % _nTilesetFrames : 0;

        if (n != _tilesetFrame) {
            _tilesetFrame = n;

            _tilesTextureValid = false;
            requestUpdate();
        }
    }

    void nextTilesetFrame()
    {
        setTilesetFrame(_tilesetFrame + 1);
    }

    void setNTilesetFrames(unsigned n)
    {
        n = std::min<unsigned>(n, _tilesetFrames.size());

        if (_nTilesetFrames != n) {
            _nTilesetFrames = n;
            if (_tilesetFrame >= _nTilesetFrames) {
                _tilesetFrame = 0;
            }
            _tilesTextureValid = false;
        }
    }

    void setTilesetFrame(unsigned n, grid<uint8_t>& tiles)
    {
        assert(tiles.size() == usize(TEXTURE_SIZE, TEXTURE_SIZE));

        if (n < _tilesetFrames.size()) {
            _tilesetFrames.at(n).setData(tiles);

            _tilesTextureValid = false;
            requestUpdate();
        }
    }

    void setTextureImage(const Image* image)
    {
        if (image && image->size() == usize(256, 256)) {
            _tilesTexture.replace(*image);
        }
        else {
            _tilesTexture.replaceWithMissingImageSymbol();
        }

        _tilesTextureValid = true;
        requestUpdate();
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

    void addToDrawList(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size, const MtTileset& tileset) const;
};

}
