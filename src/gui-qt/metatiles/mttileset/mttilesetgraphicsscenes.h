/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metatiles/mtgraphicsscenes.h"

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace MtTileset {

class MtTilesetGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

private:
    static const grid_t _grid;

public:
    MtTilesetGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtTilesetGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    void tilesetItemChanged(ResourceItem* newTileset, ResourceItem* oldTileset) final;

private slots:
    void onSelectedTileParametersChanged();

private:
    upoint_vectorset _gridSelection;
};

class MtScratchpadGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtScratchpadGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    void tilesetItemChanged(ResourceItem* newTileset, ResourceItem* oldTileset) final;
};

class MtEditableScratchpadGraphicsScene : public MtEditableGraphicsScene {
    Q_OBJECT

public:
    MtEditableScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtEditableScratchpadGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

    virtual void placeTiles(const selection_grid_t& tiles, point location, const QString& text) final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    void tilesetItemChanged(ResourceItem* newTileset, ResourceItem* oldTileset) final;
};
}
}
}
}
