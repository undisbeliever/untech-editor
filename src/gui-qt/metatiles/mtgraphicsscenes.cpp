/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgraphicsscenes.h"
#include "erasercursorgraphicsitem.h"
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
    , _selectionAction(new QAction(QIcon(":/icons/mt-selection.svg"), tr("Select Tiles"), this))
    , _actionGroup(new QActionGroup(this))
    , _cursorFactories()
    , _gridSelectionSources()
    , _currentCursorFactory(-1)
    , _cursorItem(nullptr)
    , _cursorRect()
{
    installEventFilter(this);
    addGridSelectionSource(this);

    _actionGroup->setExclusive(true);

    _actionGroup->addAction(_selectionAction);
    _selectionAction->setCheckable(true);
    _selectionAction->setChecked(true);
    _selectionAction->setShortcut(Qt::Key_S);

    addCursorFactory(new TileCursorFactory(this));
    addCursorFactory(new EraserCursorFactory(this));

    connect(this, &MtGraphicsScene::gridResized,
            this, &MtEditableGraphicsScene::onGridResized);

    connect(_actionGroup, &QActionGroup::triggered,
            this, &MtEditableGraphicsScene::onActionGroupTriggered);
}

void MtEditableGraphicsScene::addCursorFactory(AbstractCursorFactory* factory)
{
    Q_ASSERT(factory);

    _cursorFactories.append(factory);

    factory->action()->setCheckable(true);
    _actionGroup->addAction(factory->action());
}

void MtEditableGraphicsScene::populateActions(QWidget* widget)
{
    widget->addAction(_selectionAction);

    for (auto* f : _cursorFactories) {
        widget->addAction(f->action());
    }
}

void MtEditableGraphicsScene::setCursorRect(const QRect& r)
{
    if (_cursorRect != r) {
        _cursorRect = r;
        emit cursorRectChanged();
    }
}

void MtEditableGraphicsScene::setTileCursorGrid(const selection_grid_t& tileCursorGrid)
{
    if (tileCursorGrid.empty()) {
        // Remove TileCursorGraphicsItem if tileCursorGrid is empty
        if (qobject_cast<TileCursorGraphicsItem*>(_cursorItem)) {
            removeCursor();
        }
    }

    _tileCursorGrid = tileCursorGrid;
    emit tileCursorGridChanged();
}

void MtEditableGraphicsScene::onGridResized()
{
    auto& grid = this->grid();
    _cursorRect.setRect(0, 0, grid.width() * 16, grid.height() * 16);

    if (grid.empty()) {
        removeCursor();
    }
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
        Q_ASSERT(_cursorItem == nullptr);
    }

    gridGraphicsItem()->setShowGridSelection(true);
}

void MtEditableGraphicsScene::setFactoryAndCreateCursor(int id)
{
    if (id < 0 || id >= _cursorFactories.size()) {
        id = -1;
    }

    if (_currentCursorFactory != id) {
        removeCursor();

        if (id >= 0) {
            _cursorFactories.at(id)->action()->setChecked(true);
        }
        else {
            _selectionAction->setChecked(true);
        }
    }
    _currentCursorFactory = id;

    if (_cursorItem == nullptr) {
        createCursor();
    }
}

void MtEditableGraphicsScene::createCursor()
{
    if (_cursorItem) {
        removeCursor();
    }

    const int factoryId = _currentCursorFactory;
    if (factoryId < 0 || factoryId >= _cursorFactories.size()) {
        return;
    }

    Q_ASSERT(_cursorItem == nullptr);

    auto* cursor = _cursorFactories.at(factoryId)->createCursor();

    if (cursor) {
        _cursorItem = cursor;

        _cursorFactories.at(factoryId)->action()->setChecked(true);

        cursor->setZValue(99999);
        addItem(cursor);

        bool underMouse = false;
        for (QGraphicsView* view : views()) {
            if (view->underMouse()) {
                underMouse = true;

                QPointF scenePos = view->mapToScene(view->mapFromGlobal(QCursor::pos()));
                _cursorItem->processMouseScenePosition(scenePos);

                break;
            }
        }
        cursor->setVisible(underMouse);

        if (underMouse) {
            cursor->setFocus();
            cursor->grabMouse();
        }

        connect(cursor, &AbstractCursorGraphicsItem::destroyed,
                this, &MtEditableGraphicsScene::onCursorDestroyed);
    }
    else if (cursor == nullptr) {
        _selectionAction->setChecked(true);
    }

    // Do not show selection if cursor is active
    gridGraphicsItem()->setShowGridSelection(_cursorItem == nullptr);
}

void MtEditableGraphicsScene::onCursorDestroyed()
{
    if (sender() == _cursorItem) {
        _cursorItem = nullptr;

        _selectionAction->setChecked(true);
    }
}

void MtEditableGraphicsScene::onActionGroupTriggered(QAction* action)
{
    if (action->isChecked()) {
        for (int i = 0; i < _cursorFactories.size(); i++) {
            if (action == _cursorFactories.at(i)->action()) {
                setFactoryAndCreateCursor(i);
                return;
            }
        }
    }

    // No cursor selected
    setFactoryAndCreateCursor(-1);
}

void MtEditableGraphicsScene::createTileCursor()
{
    for (int i = 0; i < _cursorFactories.size(); i++) {
        if (qobject_cast<TileCursorFactory*>(_cursorFactories.at(i))) {
            setFactoryAndCreateCursor(i);
            return;
        }
    }
    removeCursor();
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
    if (auto* scene = qobject_cast<MtGraphicsScene*>(sender())) {
        setTileCursorGrid(scene->gridSelectionGrid());
        createTileCursor();
    }
}
