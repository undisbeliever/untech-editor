/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetpixmaps.h"
#include "accessors.h"
#include "gui-qt/snes/tile.hpp"

#include <QImage>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using namespace UnTech::GuiQt::Snes;

static inline QPixmap blankTile(unsigned size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::black);
    return pixmap;
}

TilesetPixmaps::TilesetPixmaps(QObject* parent)
    : QObject(parent)
    , _resourceItem(nullptr)
    , _smallTileset()
    , _largeTileset()
    , _blankSmallTile(blankTile(8))
    , _blankLargeTile(blankTile(16))
{
}

void TilesetPixmaps::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem != nullptr) {
        _resourceItem->disconnect(this);
        _resourceItem->smallTileTileset()->disconnect(this);
        _resourceItem->largeTileTileset()->disconnect(this);
        _resourceItem->paletteList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    redrawTilesets();

    if (_resourceItem) {
        connect(_resourceItem->paletteList(), &PaletteList::dataChanged,
                this, &TilesetPixmaps::onPaletteChanged);

        connect(_resourceItem->smallTileTileset(), &SmallTileTileset::listChanged,
                this, &TilesetPixmaps::redrawTilesets);

        connect(_resourceItem->largeTileTileset(), &LargeTileTileset::listChanged,
                this, &TilesetPixmaps::redrawTilesets);

        connect(_resourceItem->smallTileTileset(), &SmallTileTileset::dataChanged,
                this, &TilesetPixmaps::onSmallTileChanged);

        connect(_resourceItem->largeTileTileset(), &LargeTileTileset::dataChanged,
                this, &TilesetPixmaps::onLargeTileChanged);

        connect(_resourceItem->paletteList(), &PaletteList::selectedIndexChanged,
                this, &TilesetPixmaps::redrawTilesets);
    }
}

const QPixmap& TilesetPixmaps::smallTile(unsigned index) const
{
    if (index < (unsigned)_smallTileset.size()) {
        return _smallTileset.at(index);
    }
    else {
        return _blankSmallTile;
    }
}

const QPixmap& TilesetPixmaps::largeTile(unsigned index) const
{
    if (index < (unsigned)_largeTileset.size()) {
        return _largeTileset.at(index);
    }
    else {
        return _blankLargeTile;
    }
}

void TilesetPixmaps::redrawTilesets()
{
    if (_resourceItem != nullptr) {
        const auto* frameSet = _resourceItem->frameSet();
        const auto& palette = this->palette();

        auto update = [&palette](auto& pixmaps, const auto& tileset) -> void {
            const unsigned TS = tileset.TILE_SIZE;
            QImage img(TS, TS, QImage::Format_ARGB32_Premultiplied);

            pixmaps.resize(tileset.size());
            for (unsigned i = 0; i < tileset.size(); i++) {
                img.fill(0);
                drawTransparentTile(img, tileset.tile(i), palette, 0, 0);

                pixmaps.replace(i, QPixmap::fromImage(img));
            }
        };

        update(_smallTileset, frameSet->smallTileset);
        update(_largeTileset, frameSet->largeTileset);
    }
    else {
        _smallTileset.clear();
        _largeTileset.clear();
    }

    emit pixmapsChanged();
    emit pixmapsRedrawn();
}

void TilesetPixmaps::onPaletteChanged(unsigned index)
{
    if (index == _resourceItem->paletteList()->selectedIndex()) {
        redrawTilesets();
    }
}

void TilesetPixmaps::onSmallTileChanged(unsigned tileId)
{
    Q_ASSERT(tileId < unsigned(_smallTileset.size()));

    const auto& tile = _resourceItem->frameSet()->smallTileset.tile(tileId);
    const auto& palette = this->palette();

    QImage img(8, 8, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    drawTransparentTile(img, tile, palette, 0, 0);

    _smallTileset.replace(tileId, QPixmap::fromImage(img));

    emit smallTileChanged(tileId);
    emit pixmapsChanged();
}

void TilesetPixmaps::onLargeTileChanged(unsigned tileId)
{
    Q_ASSERT(tileId < unsigned(_largeTileset.size()));

    const auto& tile = _resourceItem->frameSet()->largeTileset.tile(tileId);
    const auto& palette = this->palette();

    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    drawTransparentTile(img, tile, palette, 0, 0);

    _largeTileset.replace(tileId, QPixmap::fromImage(img));

    emit largeTileChanged(tileId);
    emit pixmapsChanged();
}

const UnTech::Snes::Palette4bpp& TilesetPixmaps::palette() const
{
    static const UnTech::Snes::Palette4bpp BLANK_PALETTE;

    if (_resourceItem == nullptr) {
        return BLANK_PALETTE;
    }

    if (const auto* p = _resourceItem->paletteList()->selectedItem()) {
        return *p;
    }
    else {
        return BLANK_PALETTE;
    }
}
