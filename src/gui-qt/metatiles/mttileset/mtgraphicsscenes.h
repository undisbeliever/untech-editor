/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/vectorset-upoint.h"
#include <QGraphicsScene>
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class Style;
class MtTilesetRenderer;
class MtTilesetResourceItem;
class MtGridGraphicsItem;

class MtGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint16_t>;
    const static grid_t BLANK_GRID;

    const static upoint_vectorset BLANK_GRID_SELECTION;

public:
    MtGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtGraphicsScene() = default;

    Style* style() const { return _style; }
    MtTilesetRenderer* renderer() const { return _renderer; }
    MtTilesetResourceItem* tilesetItem() const { return _tilesetItem; }

    virtual const grid_t& grid() const = 0;
    virtual const upoint_vectorset& gridSelection() const = 0;

    virtual void setGridSelection(upoint_vectorset&& selectedCells) = 0;

protected:
    virtual void tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset) = 0;

signals:
    // MUST be emitted by the subclass when the grid changed
    void gridChanged();
    // MUST be emitted by the subclass when the grid is resized
    void gridResized();

    // MUST be emitted by the subclass when the grid selection changes
    void gridSelectionChanged();

private slots:
    void onRendererTilesetItemChanged();

private:
    Style* const _style;
    MtTilesetRenderer* const _renderer;
    MtGridGraphicsItem* const _gridGraphicsItem;

    MtTilesetResourceItem* _tilesetItem;
};

class MtTilesetGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtTilesetGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtTilesetGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

protected:
    void tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset) final;

private slots:
    void onTilesetCompiled();
    void onSelectedTileParametersChanged();

private:
    grid_t _grid;
    upoint_vectorset _gridSelection;
};

class MtScratchpadGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtScratchpadGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

protected:
    void tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset) final;
};
}
}
}
