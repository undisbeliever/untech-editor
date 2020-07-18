/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mtgridgraphicsitem.h"
#include "mtgraphicsscenes.h"
#include "style.h"
#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
#include "gui-qt/metatiles/mttileset/resourceitem.h"
#include "models/metatiles/metatile-tileset.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtGridGraphicsItem::MtGridGraphicsItem(MtGraphicsScene* scene)
    : QGraphicsObject()
    , _scene(scene)
    , _boundingRect()
    , _tileGridPainter()
    , _previousMouseCell(0, 0)
    , _firstCellOfRectangularSelection(0, 0)
    , _gridSelectionBeforeRectangularSelection()
    , _showBackgroundColor(true)
    , _showGridSelection(true)
    , _enableMouseSelection(true)
    , _inRectangularSelection(false)
    , _uncommittedSelection(false)
{
    Q_ASSERT(scene);

    _scene->installEventFilter(this);

    connect(scene, &MtGraphicsScene::gridResized,
            this, &MtGridGraphicsItem::onGridResized);

    connect(scene, &MtGraphicsScene::gridChanged,
            this, &MtGridGraphicsItem::updateTileGridFragments);

    connect(scene, &MtGraphicsScene::gridSelectionChanged,
            this, &MtGridGraphicsItem::onGridSelectionChanged);

    connect(scene->renderer(), &MtTileset::MtTilesetRenderer::pixmapChanged,
            this, &MtGridGraphicsItem::updateAll);

    connect(scene->renderer(), &MtTileset::MtTilesetRenderer::interactiveTilesChanged,
            this, &MtGridGraphicsItem::updateAll);

    connect(scene->style(), &Style::showLayersChanged,
            this, &MtGridGraphicsItem::updateAll);
}

void MtGridGraphicsItem::setShowBackgroundColor(bool showBackgroundColor)
{
    if (_showBackgroundColor != showBackgroundColor) {
        _showBackgroundColor = showBackgroundColor;
        update();
    }
}

void MtGridGraphicsItem::setShowGridSelection(bool showGridSelection)
{
    if (_showGridSelection != showGridSelection) {
        _showGridSelection = showGridSelection;
        update();
    }
}

QRectF MtGridGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void MtGridGraphicsItem::updateAll()
{
    update();
}

void MtGridGraphicsItem::onGridResized()
{
    const auto& grid = _scene->grid();

    _boundingRect.setWidth(grid.width() * 16);
    _boundingRect.setHeight(grid.height() * 16);

    if (_firstCellOfRectangularSelection.x >= grid.width()
        || _firstCellOfRectangularSelection.y >= grid.height()) {

        _firstCellOfRectangularSelection = upoint(0, 0);
        _gridSelectionBeforeRectangularSelection = _scene->gridSelection();
    }

    _uncommittedSelection = false;

    prepareGeometryChange();
    updateTileGridFragments();
}

void MtGridGraphicsItem::onGridSelectionChanged()
{
    if (_inRectangularSelection == false) {
        // Multiple shift clicks will resize the rectangle.
        //
        // Therefore we need to remember the state of the selection before
        // the initial rectangle is selected.

        _gridSelectionBeforeRectangularSelection = _scene->gridSelection();
    }

    update();
}

void MtGridGraphicsItem::updateTileGridFragments()
{
    _tileGridPainter.updateFragments(_scene->grid());
    _tileGridPainter.updateInteractiveTileFragments(_scene->grid(), _scene);
    updateAll();
}

void MtGridGraphicsItem::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem*, QWidget*)
{
    auto* renderer = _scene->renderer();

    if (_showBackgroundColor) {
        painter->fillRect(_boundingRect, renderer->backgroundColor());
    }

    auto* style = _scene->style();

    if (style->showTiles()) {
        _tileGridPainter.paintTiles(painter, renderer);
    }

    if (style->showTileCollisions()) {
        painter->setPen(QPen(style->tileCollisionsColor()));
        painter->setBackgroundMode(Qt::TransparentMode);
        _tileGridPainter.paintCollisions(painter, renderer);
    }

    if (style->showInteractiveTiles()) {
        _tileGridPainter.paintInteractiveTiles(painter, renderer, style);
    }

    if (style->showGrid()) {
        painter->setPen(style->gridPen());
        painter->setBrush(QBrush());

        int width = _boundingRect.width();
        int height = _boundingRect.height();

        for (int x = 0; x <= width; x += 16) {
            painter->drawLine(x, 0, x, height);
        }
        for (int y = 0; y <= height; y += 16) {
            painter->drawLine(0, y, width, y);
        }
    }

    const auto& sel = _scene->gridSelection();
    if (_showGridSelection && !sel.empty()) {
        painter->setPen(style->gridSelectionPen());
        painter->setBrush(style->gridSelectionBrush());

        for (const upoint& p : sel) {
            painter->drawRect(p.x * 16, p.y * 16, 16, 16);
        }
    }
}

upoint MtGridGraphicsItem::positionToGridCell(const QPointF& pos)
{
    return upoint(pos.x() / 16, pos.y() / 16);
}

void MtGridGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (_enableMouseSelection == false) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        upoint cell = positionToGridCell(event->pos());
        _previousMouseCell = cell;

        if (event->modifiers() & Qt::ShiftModifier) {
            // When shift clicked, select a rectangle.
            // Multiple shift-clicks will resize the rectangle.

            processRenctangularSelection(cell);
        }
        else if (event->modifiers() == Qt::ControlModifier) {
            // When control clicked, toggle cell selection

            upoint_vectorset sel = _scene->gridSelection();
            auto it = sel.find(cell);
            if (it == sel.end()) {
                sel.insert(cell);
            }
            else {
                sel.erase(it);
            }
            _scene->setGridSelection(std::move(sel));
        }
        else {
            // No Modifiers, replace selection with clicked cell

            upoint_vectorset sel = { cell };
            _scene->setGridSelection(std::move(sel));
        }

        if ((event->modifiers() & Qt::ShiftModifier) == false) {
            _firstCellOfRectangularSelection = cell;
        }

        _uncommittedSelection = true;
    }
}

void MtGridGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (_enableMouseSelection == false) {
        return;
    }

    if (event->buttons() == Qt::LeftButton) {
        QPointF pos = event->pos();

        if (_boundingRect.contains(pos)) {
            upoint cell = positionToGridCell(pos);

            if (cell != _previousMouseCell) {
                processRenctangularSelection(cell);

                _previousMouseCell = cell;
            }
        }
    }
}

void MtGridGraphicsItem::processRenctangularSelection(const upoint& cell)
{
    if (_enableMouseSelection == false) {
        return;
    }

    _inRectangularSelection = true;

    unsigned minX = qMin(_firstCellOfRectangularSelection.x, cell.x);
    unsigned maxX = qMax(_firstCellOfRectangularSelection.x, cell.x);
    unsigned minY = qMin(_firstCellOfRectangularSelection.y, cell.y);
    unsigned maxY = qMax(_firstCellOfRectangularSelection.y, cell.y);

    unsigned nCells = (maxX - minX + 1) * (maxY - minY + 1);

    upoint_vectorset sel = _gridSelectionBeforeRectangularSelection;
    sel.reserve(sel.size() + nCells);

    for (unsigned y = minY; y <= maxY; y++) {
        for (unsigned x = minX; x <= maxX; x++) {
            sel.insert(upoint(x, y));
        }
    }

    _scene->setGridSelection(std::move(sel));

    _uncommittedSelection = true;

    _inRectangularSelection = false;
}

void MtGridGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (_enableMouseSelection == false) {
        return;
    }

    if (event->button() == Qt::LeftButton
        && event->modifiers() != Qt::ControlModifier) {

        _scene->gridSelectionEdited();
        _uncommittedSelection = false;
    }
}

bool MtGridGraphicsItem::eventFilter(QObject* watched, QEvent* event)
{
    // This code uses QObject::eventFilter as QGraphicsItem::sceneEvent does not catch Leave/FocusOut events.

    if (watched == _scene && _uncommittedSelection) {
        bool commitSelection = false;

        if (event->type() == QEvent::Leave) {
            // Mouse has left the scene
            commitSelection = true;
        }
        if (event->type() == QEvent::FocusOut) {
            // Focus has left the scene
            commitSelection = true;
        }
        else if (event->type() == QEvent::KeyRelease) {
            if (auto* keyEvent = dynamic_cast<QKeyEvent*>(event)) {
                if (keyEvent->key() == Qt::Key_Control) {
                    // Somewhere in the scene the user released control
                    commitSelection = true;
                }
            }
        }

        if (commitSelection) {
            emit _scene->gridSelectionEdited();
            _uncommittedSelection = false;
        }
    }

    return false;
}
