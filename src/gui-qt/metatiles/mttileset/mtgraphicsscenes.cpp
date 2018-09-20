/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "mtgridgraphicsitem.h"
#include "mttilesetaccessors.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "stampgraphicsitem.h"

#include <QEvent>
#include <QGraphicsView>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

const MtGraphicsScene::grid_t MtGraphicsScene::BLANK_GRID;
const upoint_vectorset MtGraphicsScene::BLANK_GRID_SELECTION;

MtGraphicsScene::MtGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : QGraphicsScene(parent)
    , _style(style)
    , _renderer(renderer)
    , _gridGraphicsItem(new MtGridGraphicsItem(this))
    , _tilesetItem(nullptr)
{
    Q_ASSERT(style);
    Q_ASSERT(renderer);

    this->addItem(_gridGraphicsItem);

    connect(_renderer, &MtTilesetRenderer::tilesetItemChanged,
            this, &MtGraphicsScene::onRendererTilesetItemChanged);
}

MtGraphicsScene::grid_t MtGraphicsScene::gridSelectionGrid() const
{
    const auto& selection = this->gridSelection();
    if (selection.empty()) {
        return grid_t();
    }

    const auto& tileGrid = this->grid();
    if (tileGrid.empty()) {
        return grid_t();
    }

    unsigned minX = UINT_MAX;
    unsigned maxX = 0;
    unsigned minY = UINT_MAX;
    unsigned maxY = 0;

    for (auto& cell : selection) {
        if (cell.x < minX) {
            minX = cell.x;
        }
        if (cell.x > maxX) {
            maxX = cell.x;
        }
        if (cell.y < minY) {
            minY = cell.y;
        }
        if (cell.y > maxY) {
            maxY = cell.y;
        }
    }

    grid_t selGrid(maxX - minX + 1,
                   maxY - minY + 1,
                   0xffff);

    for (auto& cell : selection) {
        if (cell.x < tileGrid.width() && cell.y < tileGrid.height()) {
            selGrid.set(cell.x - minX, cell.y - minY, tileGrid.at(cell));
        }
    }

    return selGrid;
}

void MtGraphicsScene::editGridSelection(upoint_vectorset&& selectedCells)
{
    setGridSelection(std::move(selectedCells));
    emit gridSelectionEdited();
}

void MtGraphicsScene::onRendererTilesetItemChanged()
{
    MtTilesetResourceItem* oldItem = _tilesetItem;

    if (_tilesetItem) {
        _tilesetItem->disconnect(this);
    }
    _tilesetItem = _renderer->tilesetItem();

    tilesetItemChanged(_tilesetItem, oldItem);
}

MtEditableGraphicsScene::MtEditableGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
    , _gridSelectionSources()
    , _cursorItem(nullptr)
{
    gridGraphicsItem()->setEnableMouseSelection(false);

    installEventFilter(this);

    connect(this, &MtGraphicsScene::gridResized,
            this, &MtEditableGraphicsScene::onGridResized);
}

void MtEditableGraphicsScene::onGridResized()
{
    // Ensure the cursor does not enlarge the scene rect

    const auto& grid = this->grid();

    setSceneRect(0, 0, grid.width() * 16, grid.height() * 16);
}

void MtEditableGraphicsScene::setCursor(AbstractCursorGraphicsItem* cursor)
{
    if (_cursorItem != cursor) {
        removeCursor();
        _cursorItem = cursor;

        cursor->setZValue(99999);

        addItem(cursor);
    }

    // Move cursor to mouse position and grab the mouse
    // if the mouse is over the QGraphicsView.
    if (_cursorItem) {
        bool underMouse = false;
        for (QGraphicsView* view : views()) {
            if (view->underMouse()) {
                underMouse = true;

                QPointF scenePos = view->mapToScene(view->mapFromGlobal(QCursor::pos()));
                _cursorItem->processMouseScenePosition(scenePos);

                break;
            }
        }
        _cursorItem->setVisible(underMouse);

        if (underMouse) {
            _cursorItem->grabMouse();
        }
    }
}

bool MtEditableGraphicsScene::event(QEvent* event)
{
    if (_cursorItem) {
        if (event->type() == QEvent::Leave) {
            _cursorItem->setVisible(false);
        }
        else if (event->type() == QEvent::Enter) {
            _cursorItem->setVisible(true);
            _cursorItem->grabMouse();
        }
    }

    return MtGraphicsScene::event(event);
}

void MtEditableGraphicsScene::removeCursor()
{
    if (_cursorItem) {
        removeItem(_cursorItem);

        delete _cursorItem;
        _cursorItem = nullptr;
    }
}

void MtEditableGraphicsScene::createStampCursor(MtGraphicsScene::grid_t&& grid)
{
    if (grid.empty()) {
        removeCursor();
    }
    else {
        StampGraphicsItem* stamp = qobject_cast<StampGraphicsItem*>(_cursorItem);
        if (stamp == nullptr) {
            stamp = new StampGraphicsItem(this);
        }

        stamp->setGrid(std::move(grid));
        setCursor(stamp);
    }
}

void MtEditableGraphicsScene::createStampCursor(MtGraphicsScene* scene)
{
    Q_ASSERT(scene);
    createStampCursor(scene->gridSelectionGrid());
}

void MtEditableGraphicsScene::createStampCursor()
{
    auto process = [this](MtGraphicsScene* s) {
        grid_t g = s->gridSelectionGrid();
        if (g.empty() == false) {
            createStampCursor(std::move(g));
            return true;
        }
        return false;
    };

    for (MtGraphicsScene* scene : _gridSelectionSources) {
        bool ok = process(scene);
        if (ok) {
            return;
        }
    }

    bool ok = process(this);
    if (!ok) {
        removeCursor();
    }
}

void MtEditableGraphicsScene::addGridSelectionSource(MtGraphicsScene* scene)
{
    if (_gridSelectionSources.contains(scene)) {
        return;
    }
    _gridSelectionSources.append(scene);

    connect(scene, &MtGraphicsScene::gridSelectionEdited,
            this, &MtEditableGraphicsScene::onGridSelectionEdited);
}

void MtEditableGraphicsScene::onGridSelectionEdited()
{
    MtGraphicsScene* obj = qobject_cast<MtGraphicsScene*>(sender());
    if (obj) {
        createStampCursor(obj);
    }
}

MtTilesetGraphicsScene::MtTilesetGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
    , _grid()
    , _gridSelection()
{
}

const MtGraphicsScene::grid_t& MtTilesetGraphicsScene::grid() const
{
    return _grid;
}

const upoint_vectorset& MtTilesetGraphicsScene::gridSelection() const
{
    return _gridSelection;
}

void MtTilesetGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    auto* tilesetItem = this->tilesetItem();
    if (tilesetItem != nullptr && !_grid.empty()) {
        vectorset<uint16_t> selectedTiles;
        selectedTiles.reserve(selectedCells.size());

        for (auto& p : selectedCells) {
            unsigned tileId = p.y * _grid.width() + p.x;
            if (tileId < renderer()->nMetaTiles()) {
                selectedTiles.insert(tileId);
            }
        }
        tilesetItem->tileParameters()->setSelectedIndexes(std::move(selectedTiles));

        tilesetItem->scratchpadGrid()->clearSelection();
    }
    else {
        if (!_gridSelection.empty()) {
            _gridSelection.clear();
            emit gridSelectionChanged();
        }
    }
}

void MtTilesetGraphicsScene::tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset)
{
    onTilesetCompiled();
    onSelectedTileParametersChanged();

    if (oldTileset) {
        oldTileset->tileParameters()->disconnect();
    }

    if (newTileset) {
        connect(newTileset, &MtTilesetResourceItem::resourceComplied,
                this, &MtTilesetGraphicsScene::onTilesetCompiled);

        connect(newTileset->tileParameters(), &MtTilesetTileParameters::selectedIndexesChanged,
                this, &MtTilesetGraphicsScene::onSelectedTileParametersChanged);
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

void MtTilesetGraphicsScene::onSelectedTileParametersChanged()
{
    _gridSelection.clear();

    if (auto* ti = tilesetItem()) {
        const auto* tileParameters = ti->tileParameters();
        const unsigned gridWidth = _grid.width();

        for (const auto& t : tileParameters->selectedIndexes()) {
            _gridSelection.insert(upoint(t % gridWidth, t / gridWidth));
        }
    }

    emit gridSelectionChanged();
}

MtScratchpadGraphicsScene::MtScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
{
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

const upoint_vectorset& MtScratchpadGraphicsScene::gridSelection() const
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->selectedCells();
    }

    return BLANK_GRID_SELECTION;
}

void MtScratchpadGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->setSelectedCells(std::move(selectedCells));
    }
}

void MtScratchpadGraphicsScene::tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset)
{
    if (oldTileset) {
        oldTileset->scratchpadGrid()->disconnect(this);
    }

    emit gridResized();
    emit gridSelectionChanged();

    if (newTileset) {
        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridChanged,
                this, &MtScratchpadGraphicsScene::gridChanged);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridResized,
                this, &MtScratchpadGraphicsScene::gridResized);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::selectedCellsChanged,
                this, &MtScratchpadGraphicsScene::gridSelectionChanged);
    }
}

MtEditableScratchpadGraphicsScene::MtEditableScratchpadGraphicsScene(Style* style, MtTilesetRenderer* renderer, QObject* parent)
    : MtEditableGraphicsScene(style, renderer, parent)
{
}

const MtGraphicsScene::grid_t& MtEditableScratchpadGraphicsScene::grid() const
{
    if (auto* ti = tilesetItem()) {
        if (auto* data = ti->data()) {
            return data->scratchpad;
        }
    }

    return BLANK_GRID;
}

const upoint_vectorset& MtEditableScratchpadGraphicsScene::gridSelection() const
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->selectedCells();
    }

    return BLANK_GRID_SELECTION;
}

void MtEditableScratchpadGraphicsScene::setGridSelection(upoint_vectorset&& selectedCells)
{
    if (auto* ti = tilesetItem()) {
        return ti->scratchpadGrid()->setSelectedCells(std::move(selectedCells));
    }
}

#include <QDebug>

void MtEditableScratchpadGraphicsScene::placeTiles(const MtGraphicsScene::grid_t& tiles, point location)
{
    qDebug() << "PLACE" << tiles.cellCount() << "TILES AT" << location.x << "," << location.y;
}

void MtEditableScratchpadGraphicsScene::tilesetItemChanged(MtTilesetResourceItem* newTileset, MtTilesetResourceItem* oldTileset)
{
    if (oldTileset) {
        oldTileset->scratchpadGrid()->disconnect(this);
    }

    emit gridResized();
    emit gridSelectionChanged();

    if (newTileset) {
        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridChanged,
                this, &MtEditableScratchpadGraphicsScene::gridChanged);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::gridResized,
                this, &MtEditableScratchpadGraphicsScene::gridResized);

        connect(newTileset->scratchpadGrid(), &MtTilesetScratchpadGrid::selectedCellsChanged,
                this, &MtEditableScratchpadGraphicsScene::gridSelectionChanged);
    }
}
