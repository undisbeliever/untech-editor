/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/vectorset-upoint.h"
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class Style;
class AbstractCursorGraphicsItem;
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

    // converts the gridSelection vectorset into a grid of MetaTiles.
    // Empty tiles contain are 0xffff and easily identifyable.
    grid_t gridSelectionGrid() const;

    virtual const grid_t& grid() const = 0;
    virtual const upoint_vectorset& gridSelection() const = 0;

    // Sets the grid selection then emits gridSelectionEdited.
    // Should only be called by MtGridGraphicsItem
    void editGridSelection(upoint_vectorset&& selectedCells);

protected:
    // returns true if the selected cells changed.
    virtual void setGridSelection(upoint_vectorset&& selectedCells) = 0;

    virtual void tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset) = 0;

    MtGridGraphicsItem* gridGraphicsItem() const { return _gridGraphicsItem; }

signals:
    // MUST be emitted by the subclass when the grid changed
    void gridChanged();
    // MUST be emitted by the subclass when the grid is resized
    void gridResized();

    // MUST be emitted by the subclass when the grid selection changes
    void gridSelectionChanged();

    // emitted when the user edits the grid selection.
    void gridSelectionEdited();

private slots:
    void onRendererTilesetItemChanged();

private:
    Style* const _style;
    MtTilesetRenderer* const _renderer;
    MtGridGraphicsItem* const _gridGraphicsItem;

    MtTilesetResourceItem* _tilesetItem;
};

class MtEditableGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtEditableGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtEditableGraphicsScene() = default;

    AbstractCursorGraphicsItem* cursorItem() const { return _cursorItem; }
    void removeCursor();

    void createStampCursor(grid_t&& grid);
    // Create a stamp cursor using the selection of a MetaTile graphics scene
    // Returns true of the scene has a valid selection.
    void createStampCursor(MtGraphicsScene* scene);
    // Creates a stamp cursor using the selection of the first valid gridSelection
    // in the GridSelectionSources list.
    //
    // This SHOULD BE called when the Resource Item changes and AFTER the
    // gridSelectionSources and their renderer's have had their resource items set.
    void createStampCursor();

    // Connects the scene's gridSelection to the stamp cursor
    void addGridSelectionSource(MtGraphicsScene* scene);

    // Be aware location may be outside the grid.
    virtual void placeTiles(const grid_t& tiles, point location) = 0;

protected:
    void setCursor(AbstractCursorGraphicsItem* cursor);

    virtual bool event(QEvent* event) override;

private slots:
    void onGridResized();
    void onGridSelectionEdited();

private:
    QList<MtGraphicsScene*> _gridSelectionSources;
    AbstractCursorGraphicsItem* _cursorItem;
};

class MtTilesetGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtTilesetGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtTilesetGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

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

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    void tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset) final;
};

class MtEditableScratchpadGraphicsScene : public MtEditableGraphicsScene {
    Q_OBJECT

public:
    MtEditableScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent);
    ~MtEditableScratchpadGraphicsScene() = default;

    virtual const grid_t& grid() const final;
    virtual const upoint_vectorset& gridSelection() const final;

    virtual void placeTiles(const grid_t& tiles, point location) final;

protected:
    virtual void setGridSelection(upoint_vectorset&& selectedCells) final;

    void tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset) final;
};
}
}
}
