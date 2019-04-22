/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
class MtGridGraphicsItem;

namespace MtTileset {
class MtTilesetRenderer;
class ResourceItem;
}

class MtGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint8_t>;
    using selection_grid_t = UnTech::grid<uint16_t>; // Values > N_METATILES are transparent.
    const static grid_t BLANK_GRID;

    const static upoint_vectorset BLANK_GRID_SELECTION;

public:
    MtGraphicsScene(Style* style, MtTileset::MtTilesetRenderer* renderer, QObject* parent);
    ~MtGraphicsScene() = default;

    Style* style() const { return _style; }
    MtTileset::MtTilesetRenderer* renderer() const { return _renderer; }
    MtTileset::ResourceItem* tilesetItem() const { return _tilesetItem; }

    // converts the gridSelection vectorset into a grid of MetaTiles.
    // Empty tiles contain values > N_METATILES.
    selection_grid_t gridSelectionGrid() const;

    virtual const grid_t& grid() const = 0;
    virtual const upoint_vectorset& gridSelection() const = 0;

    // Sets the grid selection then emits gridSelectionEdited.
    // Should only be called by MtGridGraphicsItem
    void editGridSelection(upoint_vectorset&& selectedCells);

protected:
    // returns true if the selected cells changed.
    virtual void setGridSelection(upoint_vectorset&& selectedCells) = 0;

    virtual void tilesetItemChanged(MtTileset::ResourceItem* newTileset, MtTileset::ResourceItem* oldTileset) = 0;

    MtGridGraphicsItem* gridGraphicsItem() const { return _gridGraphicsItem; }

    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

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
    void onGridResized();

private:
    Style* const _style;
    MtTileset::MtTilesetRenderer* const _renderer;
    MtGridGraphicsItem* const _gridGraphicsItem;

    MtTileset::ResourceItem* _tilesetItem;
};

class MtEditableGraphicsScene : public MtGraphicsScene {
    Q_OBJECT

public:
    MtEditableGraphicsScene(Style* style, MtTileset::MtTilesetRenderer* renderer, QObject* parent);
    ~MtEditableGraphicsScene() = default;

    // The boundary in which a curosr is valid.
    // NOTE: This class does not preform any validity checks, that is the responsibility
    // of the cursor and subclass.
    //
    // NOTE: This value is set to the grid's size when the grid size is changed.
    const QRect& cursorRect() const { return _cursorRect; }
    void setCursorRect(const QRect& r);

    AbstractCursorGraphicsItem* cursorItem() const { return _cursorItem; }
    void removeCursor();

    void createTileCursor(selection_grid_t&& grid);
    // Create a tile cursor using the `gridSelectionGrid` of a MetaTile graphics scene
    // Returns true of the scene has a valid selection.
    void createTileCursor(MtGraphicsScene* scene);
    // Creates a tile cursor using the `gridSelectionGrid` of the first valid gridSelection
    // in the GridSelectionSources list.
    //
    // This SHOULD BE called when the Resource Item changes and AFTER the
    // gridSelectionSources and their renderer's have had their resource items set.
    void createTileCursor();

    // Connects the scene's gridSelection to the tile cursor
    void addGridSelectionSource(MtGraphicsScene* scene);

    // Be aware location may be outside the grid.
    virtual void placeTiles(const selection_grid_t& tiles, point location) = 0;

protected:
    void setCursor(AbstractCursorGraphicsItem* cursor);

    virtual bool event(QEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

signals:
    void cursorRectChanged();

private slots:
    void onGridResized();
    void onGridSelectionEdited();

private:
    QList<MtGraphicsScene*> _gridSelectionSources;
    AbstractCursorGraphicsItem* _cursorItem;
    QRect _cursorRect;
};
}
}
}
