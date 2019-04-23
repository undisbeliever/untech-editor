/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "mtgridgraphicsitem.h"
#include "tilecursorgraphicsitem.h"
#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
#include "gui-qt/metatiles/mttileset/resourceitem.h"

#include <QEvent>
#include <QGraphicsView>
#include <QKeyEvent>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

const MtGraphicsScene::grid_t MtGraphicsScene::BLANK_GRID;
const upoint_vectorset MtGraphicsScene::BLANK_GRID_SELECTION;

MtGraphicsScene::MtGraphicsScene(Style* style, MtTileset::MtTilesetRenderer* renderer, QObject* parent)
    : QGraphicsScene(parent)
    , _style(style)
    , _renderer(renderer)
    , _gridGraphicsItem(new MtGridGraphicsItem(this))
    , _tilesetItem(nullptr)
{
    Q_ASSERT(style);
    Q_ASSERT(renderer);

    this->addItem(_gridGraphicsItem);

    connect(_renderer, &MtTileset::MtTilesetRenderer::tilesetItemChanged,
            this, &MtGraphicsScene::onRendererTilesetItemChanged);

    connect(this, &MtGraphicsScene::gridResized,
            this, &MtGraphicsScene::onGridResized);
}

MtGraphicsScene::selection_grid_t MtGraphicsScene::gridSelectionGrid() const
{
    const auto& selection = this->gridSelection();
    if (selection.empty()) {
        return selection_grid_t();
    }

    const auto& tileGrid = this->grid();
    if (tileGrid.empty()) {
        return selection_grid_t();
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

    selection_grid_t selGrid(maxX - minX + 1,
                             maxY - minY + 1,
                             0xffff);

    for (auto& cell : selection) {
        if (cell.x < tileGrid.width() && cell.y < tileGrid.height()) {
            selGrid.set(cell.x - minX, cell.y - minY, tileGrid.at(cell));
        }
    }

    return selGrid;
}

void MtGraphicsScene::onRendererTilesetItemChanged()
{
    MtTileset::ResourceItem* oldItem = _tilesetItem;

    if (_tilesetItem) {
        _tilesetItem->disconnect(this);
    }
    _tilesetItem = _renderer->tilesetItem();

    tilesetItemChanged(_tilesetItem, oldItem);

    if (_tilesetItem) {
        connect(_tilesetItem, &AbstractResourceItem::resourceComplied,
                this, &MtGraphicsScene::gridResized);
    }

    emit gridResized();
}

void MtGraphicsScene::onGridResized()
{
    // resize scene so it wll end up in the centre of the GraphicsView

    const auto& grid = this->grid();
    setSceneRect(0, 0, grid.width() * 16, grid.height() * 16);
}

void MtGraphicsScene::keyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->key() == Qt::Key_Escape) {
        // clear selection when escape presssed
        if (gridSelection().empty() == false) {
            setGridSelection(upoint_vectorset());
            return;
        }
    }

    QGraphicsScene::keyPressEvent(keyEvent);
}

MtEditableGraphicsScene::MtEditableGraphicsScene(Style* style, MtTileset::MtTilesetRenderer* renderer, QObject* parent)
    : MtGraphicsScene(style, renderer, parent)
    , _gridSelectionSources()
    , _cursorItem(nullptr)
    , _cursorRect()
{
    installEventFilter(this);
    addGridSelectionSource(this);

    connect(this, &MtGraphicsScene::gridResized,
            this, &MtEditableGraphicsScene::onGridResized);
}

void MtEditableGraphicsScene::setCursorRect(const QRect& r)
{
    if (_cursorRect != r) {
        _cursorRect = r;
        emit cursorRectChanged();
    }
}

void MtEditableGraphicsScene::onGridResized()
{
    auto& grid = this->grid();
    _cursorRect.setRect(0, 0, grid.width() * 16, grid.height() * 16);
}

void MtEditableGraphicsScene::setCursor(AbstractCursorGraphicsItem* cursor)
{
    if (_cursorItem != cursor) {
        removeCursor();
        _cursorItem = cursor;

        if (cursor) {
            cursor->setZValue(99999);
            addItem(cursor);
        }
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
            _cursorItem->setFocus();
            _cursorItem->grabMouse();
        }
    }

    // Do not show selection if cursor is active
    gridGraphicsItem()->setShowGridSelection(_cursorItem == nullptr);
}

bool MtEditableGraphicsScene::event(QEvent* event)
{
    if (_cursorItem) {
        if (event->type() == QEvent::Leave) {
            _cursorItem->setVisible(false);
        }
        else if (event->type() == QEvent::Enter) {
            _cursorItem->setVisible(true);
            _cursorItem->setFocus();
            _cursorItem->grabMouse();
        }
    }

    return MtGraphicsScene::event(event);
}

void MtEditableGraphicsScene::keyPressEvent(QKeyEvent* keyEvent)
{
    if (_cursorItem
        && keyEvent->key() == Qt::Key_Escape) {

        bool rc = _cursorItem->processEscape();
        if (rc) {
            removeCursor();
        }
    }

    MtGraphicsScene::keyPressEvent(keyEvent);
}

void MtEditableGraphicsScene::removeCursor()
{
    if (_cursorItem) {
        // cannot call `removeItem(removeItem)` here as it causes a `pure virtual method called` error.

        delete _cursorItem;
        _cursorItem = nullptr;
    }

    gridGraphicsItem()->setShowGridSelection(true);
}

void MtEditableGraphicsScene::createTileCursor(MtGraphicsScene::selection_grid_t&& grid)
{
    if (grid.empty()) {
        removeCursor();
    }
    else {
        TileCursorGraphicsItem* cursor = qobject_cast<TileCursorGraphicsItem*>(_cursorItem);
        if (cursor == nullptr) {
            cursor = new TileCursorGraphicsItem(this);
        }

        cursor->setSourceGrid(std::move(grid));
        setCursor(cursor);
    }
}

void MtEditableGraphicsScene::createTileCursor(MtGraphicsScene* scene)
{
    Q_ASSERT(scene);
    createTileCursor(scene->gridSelectionGrid());
}

void MtEditableGraphicsScene::createTileCursor()
{
    auto process = [this](MtGraphicsScene* s) {
        selection_grid_t g = s->gridSelectionGrid();
        if (g.empty() == false) {
            createTileCursor(std::move(g));
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
        createTileCursor(obj);
    }
}
