/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetwidgets.h"
#include "accessors.h"
#include "document.h"
#include "tilesetpixmaps.h"
#include "gui-qt/accessor/listundohelper.h"

using namespace UnTech::GuiQt::Accessor;
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
        _document->paletteList()->disconnect(this);
    }
    _document = document;

    if (_document) {
        updateBackgroundColor();
        onSelectedColorChanged();

        connect(_document->paletteList(), &PaletteList::dataChanged,
                this, &SmallTilesetWidget::onPaletteChanged);

        connect(_document->paletteList(), &PaletteList::selectedIndexChanged,
                this, &SmallTilesetWidget::updateBackgroundColor);

        connect(_document->paletteList(), &PaletteList::selectedColorChanged,
                this, &SmallTilesetWidget::onSelectedColorChanged);
    }
    else {
        setCanDraw(false);
    }
}

void AbstractTilesetWidget::onPaletteChanged(unsigned index)
{
    if (index == _document->paletteList()->selectedIndex()) {
        updateBackgroundColor();
    }
}

void AbstractTilesetWidget::updateBackgroundColor()
{
    if (const auto* pal = _document->paletteList()->selectedPalette()) {
        const auto& rgb = pal->color(0).rgb();
        setBackgroundColor(qRgb(rgb.red, rgb.green, rgb.blue));
    }
}

void AbstractTilesetWidget::onSelectedColorChanged()
{
    setCanDraw(_document->paletteList()->isSelectedColorValid());
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

template <class AccessorT, class TilesetT>
static inline void doDrawPixel(Document* document, AccessorT* accessor, const TilesetT& tileset,
                               int tileId, const QPoint& p, bool first)
{
    if (document->paletteList()->isSelectedColorValid()) {
        unsigned c = document->paletteList()->selectedColor();
        const auto& tile = tileset.tile(tileId);

        if (tile.pixel(p.x(), p.y()) != c) {
            auto newTile = tile;
            newTile.setPixel(p.x(), p.y(), c);

            ListUndoHelper<AccessorT>(accessor)
                .editItemInSelectedListMerge(tileId, newTile, first);
        }
    }
}

void SmallTilesetWidget::drawPixel(int tileId, const QPoint& point, bool first)
{
    doDrawPixel(_document, _document->smallTileTileset(), _document->frameSet()->smallTileset,
                tileId, point, first);
}

void LargeTilesetWidget::drawPixel(int tileId, const QPoint& point, bool first)
{
    doDrawPixel(_document, _document->largeTileTileset(), _document->frameSet()->largeTileset,
                tileId, point, first);
}
