/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "mtgridgraphicsitem.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

const MtGraphicsScene::grid_t MtGraphicsScene::BLANK_GRID;

MtGraphicsScene::MtGraphicsScene(MtTilesetRenderer* renderer, QObject* parent)
    : QGraphicsScene(parent)
    , _renderer(renderer)
    , _gridGraphicsItem(new MtGridGraphicsItem(this))
    , _tilesetItem(nullptr)
{
    Q_ASSERT(renderer);

    this->addItem(_gridGraphicsItem);

    connect(_renderer, &MtTilesetRenderer::tilesetItemChanged,
            this, &MtGraphicsScene::onRendererTilesetItemChanged);
}

void MtGraphicsScene::onRendererTilesetItemChanged()
{
    if (_tilesetItem) {
        _tilesetItem->disconnect(this);
    }
    _tilesetItem = _renderer->tilesetItem();

    emit tilesetItemChanged();
}

MtTilesetGraphicsScene::MtTilesetGraphicsScene(MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(renderer, parent)
    , _grid()
{
    onTilesetItemChanged();

    connect(this, &MtGraphicsScene::tilesetItemChanged,
            this, &MtTilesetGraphicsScene::onTilesetItemChanged);
}

const MtGraphicsScene::grid_t& MtTilesetGraphicsScene::grid() const
{
    return _grid;
}

void MtTilesetGraphicsScene::onTilesetItemChanged()
{
    onTilesetCompiled();

    if (auto* ti = tilesetItem()) {
        connect(ti, &MtTilesetResourceItem::resourceComplied,
                this, &MtTilesetGraphicsScene::onTilesetCompiled);
    }
}

void MtTilesetGraphicsScene::onTilesetCompiled()
{
    usize gSize(0, 0);
    if (auto* ti = tilesetItem()) {
        if (auto* cd = ti->compiledData()) {
            gSize = cd->sourceTileSize();
        }
    }

    if (grid().size() != gSize) {
        _grid = UnTech::grid<uint16_t>(gSize);

        uint16_t i = 0;
        for (uint16_t& cell : _grid) {
            cell = i++;
        }

        emit gridResized();
    }
}

MtScratchpadGraphicsScene::MtScratchpadGraphicsScene(MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(renderer, parent)
{
    onTilesetItemChanged();

    connect(this, &MtGraphicsScene::tilesetItemChanged,
            this, &MtScratchpadGraphicsScene::onTilesetItemChanged);
}

const MtGraphicsScene::grid_t& MtScratchpadGraphicsScene::grid() const
{
    if (auto* ti = tilesetItem()) {
        if (auto* data = ti->data()) {
            return data->scratchpad;
        }
    }

    return BLANK_GRID;
}

void MtScratchpadGraphicsScene::onTilesetItemChanged()
{
    emit gridResized();

    if (auto* ti = tilesetItem()) {
        // ::TODO scratchpad changed signal::

        connect(ti, &MtTilesetResourceItem::resourceLoaded,
                this, &MtScratchpadGraphicsScene::gridResized);
    }
}
