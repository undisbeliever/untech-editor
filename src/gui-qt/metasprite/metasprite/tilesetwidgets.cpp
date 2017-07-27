/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetwidgets.h"
#include "document.h"
#include "selection.h"
#include "tilesetcommands.h"
#include "tilesetpixmaps.h"

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

AbstractTilesetWidget::AbstractTilesetWidget(QWidget* parent)
    : DrawingPixmapGridWidget(parent)
    , _document(nullptr)
{
}

void AbstractTilesetWidget::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    if (_document) {
        updateBackgroundColor();
        onSelectedColorChanged();

        connect(_document, &Document::paletteChanged,
                this, &SmallTilesetWidget::onPaletteChanged);

        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &SmallTilesetWidget::updateBackgroundColor);

        connect(_document->selection(), &Selection::selectedColorChanged,
                this, &SmallTilesetWidget::onSelectedColorChanged);
    }
    else {
        setCanDraw(false);
    }
}

void AbstractTilesetWidget::onPaletteChanged(unsigned index)
{
    if (index == _document->selection()->selectedPalette()) {
        updateBackgroundColor();
    }
}

void AbstractTilesetWidget::updateBackgroundColor()
{
    const auto& palettes = _document->frameSet()->palettes;
    const unsigned selected = _document->selection()->selectedPalette();

    if (selected < palettes.size()) {
        const auto& rgb = palettes.at(selected).color(0).rgb();
        setBackgroundColor(qRgb(rgb.red, rgb.green, rgb.blue));
    }
}

void AbstractTilesetWidget::onSelectedColorChanged()
{
    setCanDraw(_document->selection()->selectedColor() >= 0);
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

#define DRAW_PIXEL(CLASS, TILESET, COMMAND)                          \
    void CLASS::drawPixel(int tileId, const QPoint& p, bool first)   \
    {                                                                \
        int c = _document->selection()->selectedColor();             \
        if (c >= 0) {                                                \
            const auto& tileset = _document->frameSet()->TILESET;    \
            const auto& tile = tileset.tile(tileId);                 \
                                                                     \
            if (tile.pixel(p.x(), p.y()) != c) {                     \
                auto newTile = tile;                                 \
                newTile.setPixel(p.x(), p.y(), c);                   \
                                                                     \
                _document->undoStack()->push(                        \
                    new COMMAND(_document, tileId, newTile, first)); \
            }                                                        \
        }                                                            \
    }
DRAW_PIXEL(SmallTilesetWidget, smallTileset, ChangeSmallTile)
DRAW_PIXEL(LargeTilesetWidget, largeTileset, ChangeLargeTile)
