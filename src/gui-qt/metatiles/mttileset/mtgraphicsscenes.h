/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include <QGraphicsScene>
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtTilesetRenderer;
class MtTilesetResourceItem;
class MtGridGraphicsItem;

class MtGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint16_t>;
    const static grid_t BLANK_GRID;

public:
    MtGraphicsScene(MtTilesetRenderer* renderer, QObject* parent);
    ~MtGraphicsScene() = default;

    MtTilesetRenderer* renderer() const { return _renderer; }
    MtTilesetResourceItem* tilesetItem() const { return _tilesetItem; }

    virtual const grid_t& grid() const = 0;

signals:
    // MUST be emitted by the subclass when the grid changed
    void gridChanged();
    // MUST be emitted by the subclass when the grid is resized
    void gridResized();

    // emitted by this class when the tileset item changes
    void tilesetItemChanged();

private slots:
    void onRendererTilesetItemChanged();

private:
    MtTilesetRenderer* const _renderer;
    MtGridGraphicsItem* const _gridGraphicsItem;

    MtTilesetResourceItem* _tilesetItem;
};

class MtTilesetGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtTilesetGraphicsScene(MtTilesetRenderer* renderer, QObject* parent);
    ~MtTilesetGraphicsScene() = default;

    virtual const grid_t& grid() const final;

private slots:
    void onTilesetItemChanged();
    void onTilesetCompiled();

private:
    grid_t _grid;
};

class MtScratchpadGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtScratchpadGraphicsScene(MtTilesetRenderer* renderer, QObject* parent);
    ~MtScratchpadGraphicsScene() = default;

    virtual const grid_t& grid() const final;

private slots:
    void onTilesetItemChanged();
};
}
}
}
