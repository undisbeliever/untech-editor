/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetwidgets.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilesetpixmaps.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

AbstractTilesetWidget::AbstractTilesetWidget(QWidget* parent)
    : DrawingPixmapGridWidget(parent)
    , _resourceItem(nullptr)
{
}

void AbstractTilesetWidget::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->disconnect(this);
        _resourceItem->paletteList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    if (_resourceItem) {
        updateBackgroundColor();
        onSelectedColorChanged();

        connect(_resourceItem->paletteList(), &PaletteList::dataChanged,
                this, &SmallTilesetWidget::onPaletteChanged);

        connect(_resourceItem->paletteList(), &PaletteList::selectedIndexChanged,
                this, &SmallTilesetWidget::updateBackgroundColor);

        connect(_resourceItem->paletteList(), &PaletteList::selectedColorChanged,
                this, &SmallTilesetWidget::onSelectedColorChanged);
    }
    else {
        setCanDraw(false);
    }
}

void AbstractTilesetWidget::onPaletteChanged(unsigned index)
{
    if (index == _resourceItem->paletteList()->selectedIndex()) {
        updateBackgroundColor();
    }
}

void AbstractTilesetWidget::updateBackgroundColor()
{
    if (const auto* pal = _resourceItem->paletteList()->selectedItem()) {
        const auto& rgb = pal->color(0).rgb();
        setBackgroundColor(qRgb(rgb.red, rgb.green, rgb.blue));
    }
}

void AbstractTilesetWidget::onSelectedColorChanged()
{
    setCanDraw(_resourceItem->paletteList()->isSelectedColorValid());
}

SmallTilesetWidget::SmallTilesetWidget(QWidget* parent)
    : AbstractTilesetWidget(parent)
    , _tilesetPixmaps(nullptr)
{
    setCellSize(QSize(8 * 6, 8 * 6));
}

LargeTilesetWidget::LargeTilesetWidget(QWidget* parent)
    : AbstractTilesetWidget(parent)
    , _tilesetPixmaps(nullptr)
{
    setCellSize(QSize(16 * 6, 16 * 6));
}

void SmallTilesetWidget::setTilesetPixmaps(TilesetPixmaps* tilesetPixmaps)
{
    Q_ASSERT(tilesetPixmaps != nullptr);

    if (_tilesetPixmaps) {
        _tilesetPixmaps->disconnect(this);
    }
    _tilesetPixmaps = tilesetPixmaps;

    connect(_tilesetPixmaps, &TilesetPixmaps::pixmapsRedrawn,
            this, &SmallTilesetWidget::onTilesetPixmapRedrawn);
    connect(_tilesetPixmaps, &TilesetPixmaps::smallTileChanged,
            this, &SmallTilesetWidget::onTilesetPixmapTileChanged);
}

void LargeTilesetWidget::setTilesetPixmaps(TilesetPixmaps* tilesetPixmaps)
{
    Q_ASSERT(tilesetPixmaps != nullptr);

    if (_tilesetPixmaps) {
        _tilesetPixmaps->disconnect(this);
    }
    _tilesetPixmaps = tilesetPixmaps;

    connect(_tilesetPixmaps, &TilesetPixmaps::pixmapsRedrawn,
            this, &LargeTilesetWidget::onTilesetPixmapRedrawn);
    connect(_tilesetPixmaps, &TilesetPixmaps::largeTileChanged,
            this, &LargeTilesetWidget::onTilesetPixmapTileChanged);
}

void SmallTilesetWidget::onTilesetPixmapRedrawn()
{
    setPixmaps(_tilesetPixmaps->smallTileset());
}

void LargeTilesetWidget::onTilesetPixmapRedrawn()
{
    setPixmaps(_tilesetPixmaps->largeTileset());
}

void SmallTilesetWidget::onTilesetPixmapTileChanged(int tileId)
{
    const QPixmap& pixmap = _tilesetPixmaps->smallTileset().at(tileId);
    setPixmap(tileId, pixmap);
}

void LargeTilesetWidget::onTilesetPixmapTileChanged(int tileId)
{
    const QPixmap& pixmap = _tilesetPixmaps->largeTileset().at(tileId);
    setPixmap(tileId, pixmap);
}

void SmallTilesetWidget::drawPixel(int tileId, const QPoint& point, bool first)
{
    _resourceItem->smallTileTileset()->editTile_paintPixel(tileId, point, first);
}

void LargeTilesetWidget::drawPixel(int tileId, const QPoint& point, bool first)
{
    _resourceItem->largeTileTileset()->editTile_paintPixel(tileId, point, first);
}
