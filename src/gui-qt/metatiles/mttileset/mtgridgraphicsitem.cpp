/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgridgraphicsitem.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "models/metatiles/metatile-tileset.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtGridGraphicsItem::MtGridGraphicsItem(MtTilesetRenderer* renderer)
    : QGraphicsObject()
    , _renderer(renderer)
    , _tilesetItem(nullptr)
    , _boundingRect()
{
    connect(renderer, &MtTilesetRenderer::tilesetItemChanged,
            this, &MtGridGraphicsItem::onRendererTilesetItemChanged);

    connect(renderer, &MtTilesetRenderer::pixmapChanged,
            this, &MtGridGraphicsItem::onPixmapChanged);
}

void MtGridGraphicsItem::onRendererTilesetItemChanged()
{
    auto* ti = _renderer->tilesetItem();
    if (_tilesetItem != ti) {
        if (_tilesetItem) {
            _tilesetItem->disconnect(this);
        }
        _tilesetItem = ti;

        tilesetItemChanged();
    }
}

void MtGridGraphicsItem::onPixmapChanged()
{
    update(_boundingRect);
}

QRectF MtGridGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void MtGridGraphicsItem::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem*, QWidget*)
{
    _renderer->drawGridTiles(painter, _grid);
}

void MtGridGraphicsItem::setGrid(UnTech::grid<uint16_t>&& grid)
{
    usize oldSize = _grid.size();

    _grid = std::move(grid);

    if (_grid.size() != oldSize) {
        _boundingRect.setWidth(_grid.width() * 16);
        _boundingRect.setHeight(_grid.height() * 16);

        prepareGeometryChange();
    }
    update();
}

MtTilesetGraphicsItem::MtTilesetGraphicsItem(MtTilesetRenderer* renderer)
    : MtGridGraphicsItem(renderer)
{
}

void MtTilesetGraphicsItem::tilesetItemChanged()
{
    if (auto* ti = tilesetItem()) {
        connect(ti, &MtTilesetResourceItem::resourceComplied,
                this, &MtTilesetGraphicsItem::onTilesetCompiled);
    }

    onTilesetCompiled();
}

void MtTilesetGraphicsItem::onTilesetCompiled()
{
    usize gSize(0, 0);
    if (auto* ti = tilesetItem()) {
        if (auto* cd = ti->compiledData()) {
            gSize = cd->sourceTileSize();
        }
    }

    if (grid().size() != gSize) {
        UnTech::grid<uint16_t> grid(gSize);

        uint16_t i = 0;
        for (uint16_t& cell : grid) {
            cell = i++;
        }

        setGrid(std::move(grid));
    }
}

MtTilesetScratchpadGraphicsItem::MtTilesetScratchpadGraphicsItem(MtTilesetRenderer* renderer)
    : MtGridGraphicsItem(renderer)
{
}

void MtTilesetScratchpadGraphicsItem::tilesetItemChanged()
{
    if (const auto* ti = tilesetItem()) {
        if (const auto* data = ti->data()) {
            setGrid(grid_t(data->scratchpad));
            return;
        }
    }

    setGrid(grid_t());
}
