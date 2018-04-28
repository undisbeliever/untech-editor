/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "msgraphicsscene.h"
#include "accessors.h"
#include "document.h"
#include "tilesetpixmaps.h"
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/pixmapgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

const QRect MsGraphicsScene::ITEM_RANGE(
    int_ms8_t::MIN, int_ms8_t::MIN, UINT8_MAX, UINT8_MAX);

MsGraphicsScene::MsGraphicsScene(LayerSettings* layerSettings,
                                 TilesetPixmaps* tilesetPixmaps,
                                 QWidget* parent)
    : QGraphicsScene(parent)
    , _layerSettings(layerSettings)
    , _tilesetPixmaps(tilesetPixmaps)
    , _contextMenu(new QMenu())
    , _style(new Style(parent))
    , _tileHitbox(new ResizableAabbGraphicsItem())
    , _horizontalOrigin(new QGraphicsLineItem())
    , _verticalOrigin(new QGraphicsLineItem())
    , _document(nullptr)
    , _frame(nullptr)
    , _inUpdateSelection(false)
    , _inOnSceneSelectionChanged(false)
{
    Q_ASSERT(tilesetPixmaps != nullptr);

    _tileHitbox->setPen(_style->tileHitboxPen());
    _tileHitbox->setBrush(_style->tileHitboxBrush());
    _tileHitbox->setZValue(TILE_HITBOX_ZVALUE);
    _tileHitbox->setVisible(false);
    _tileHitbox->setRange(ITEM_RANGE);
    _tileHitbox->setFlag(QGraphicsItem::ItemIsSelectable);
    _tileHitbox->setFlag(QGraphicsItem::ItemIsMovable);
    addItem(_tileHitbox);

    _horizontalOrigin->setLine(int_ms8_t::MIN, 0, int_ms8_t::MAX, 0);
    _horizontalOrigin->setPen(_style->originPen());
    _horizontalOrigin->setZValue(ORIGIN_ZVALUE);
    _horizontalOrigin->setVisible(false);
    addItem(_horizontalOrigin);

    _verticalOrigin->setLine(0, int_ms8_t::MIN, 0, int_ms8_t::MAX);
    _verticalOrigin->setPen(_style->originPen());
    _verticalOrigin->setZValue(ORIGIN_ZVALUE);
    _verticalOrigin->setVisible(false);
    addItem(_verticalOrigin);

    onLayerSettingsChanged();

    connect(this, &MsGraphicsScene::selectionChanged,
            this, &MsGraphicsScene::onSceneSelectionChanged);

    connect(_layerSettings, &LayerSettings::layerSettingsChanged,
            this, &MsGraphicsScene::onLayerSettingsChanged);

    connect(_tilesetPixmaps, &TilesetPixmaps::pixmapsChanged,
            this, &MsGraphicsScene::onTilesetPixmapsChanged);
}

void MsGraphicsScene::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->frameMap()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->actionPointList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    setFrame(nullptr);

    if (_document) {
        onSelectedFrameChanged();
        updateTileHitboxSelection();
        updateFrameObjectSelection();
        updateActionPointSelection();
        updateEntityHitboxSelection();

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &MsGraphicsScene::onSelectedFrameChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &MsGraphicsScene::updateFrameObjectSelection);
        connect(_document->actionPointList(), &ActionPointList::selectedIndexesChanged,
                this, &MsGraphicsScene::updateActionPointSelection);
        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &MsGraphicsScene::updateEntityHitboxSelection);

        connect(_document->frameMap(), &FrameMap::tileHitboxSelectedChanged,
                this, &MsGraphicsScene::updateTileHitboxSelection);

        connect(_document->frameMap(), &FrameMap::dataChanged,
                this, &MsGraphicsScene::onFrameDataChanged);

        connect(_document->frameObjectList(), &FrameObjectList::dataChanged,
                this, &MsGraphicsScene::onFrameObjectChanged);
        connect(_document->actionPointList(), &ActionPointList::dataChanged,
                this, &MsGraphicsScene::onActionPointChanged);
        connect(_document->entityHitboxList(), &EntityHitboxList::dataChanged,
                this, &MsGraphicsScene::onEntityHitboxChanged);

        // This class uses listChanged signal as it is more efficient when multiple items change

        connect(_document->frameObjectList(), &FrameObjectList::listChanged,
                this, &MsGraphicsScene::onFrameObjectListChanged);
        connect(_document->actionPointList(), &ActionPointList::listChanged,
                this, &MsGraphicsScene::onActionPointListChanged);
        connect(_document->entityHitboxList(), &EntityHitboxList::listChanged,
                this, &MsGraphicsScene::onEntityHitboxListChanged);
    }
}

void MsGraphicsScene::setFrame(const MS::Frame* frame)
{
    if (_frame != frame) {
        _frame = frame;

        _tileHitbox->setVisible(false);
        qDeleteAll(_objects);
        qDeleteAll(_actionPoints);
        qDeleteAll(_entityHitboxes);

        _objects.clear();
        _actionPoints.clear();
        _entityHitboxes.clear();

        bool showOrigin = _frame && _layerSettings->showOrigin();
        _horizontalOrigin->setVisible(showOrigin);
        _verticalOrigin->setVisible(showOrigin);

        if (_frame != nullptr) {
            setSceneRect(int_ms8_t::MIN, int_ms8_t::MIN, 256, 256);

            updateTileHitbox();
            updateTileHitboxSelection();

            // rebuild item lists
            onFrameObjectListChanged(frame);
            onActionPointListChanged(frame);
            onEntityHitboxListChanged(frame);
        }
        else {
            setSceneRect(QRect());
        }

        update();
    }
}

void MsGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        commitMovedItems();
    }
}

void MsGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    if (_document && _frame) {
        _contextMenu->exec(event->screenPos());
    }
}

void MsGraphicsScene::commitMovedItems()
{
    if (_frame == nullptr) {
        return;
    }

    QList<QUndoCommand*> commands;
    commands.reserve(4);

    if (_document->frameMap()->isTileHitboxSelected()) {
        ms8rect hitbox = _tileHitbox->rectMs8rect();
        auto* c = FrameMapUndoHelper(_document->frameMap())
                      .editSelectedFieldCommand(hitbox, QString(),
                                                [](MS::Frame& f) -> ms8rect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectListUndoHelper(_document->frameObjectList())
            .editSelectedCommand(
                [this](MS::FrameObject& obj, size_t i) {
                    obj.location = _objects.at(i)->posMs8point();
                }));
    commands.append(
        ActionPointListUndoHelper(_document->actionPointList())
            .editSelectedCommand(
                [this](MS::ActionPoint& ap, size_t i) {
                    ap.location = _actionPoints.at(i)->posMs8point();
                }));
    commands.append(
        EntityHitboxListUndoHelper(_document->entityHitboxList())
            .editSelectedCommand(
                [this](MS::EntityHitbox& eh, size_t i) {
                    eh.aabb = _entityHitboxes.at(i)->rectMs8rect();
                }));

    commands.removeAll(nullptr);

    if (!commands.empty()) {
        _document->undoStack()->beginMacro(tr("Move Selected"));
        for (QUndoCommand* c : commands) {
            _document->undoStack()->push(c);
        }
        _document->undoStack()->endMacro();
    }
}

void MsGraphicsScene::updateTileHitbox()
{
    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & _frame->solid);

    _tileHitbox->setRect(_frame->tileHitbox);
}

void MsGraphicsScene::addFrameObject()
{
    Q_ASSERT(_frame);

    unsigned index = _objects.size();
    Q_ASSERT(index < _frame->objects.size());

    auto* item = new PixmapGraphicsItem();
    _objects.insert(index, item);

    item->setZValue(FRAME_OBJECT_ZVALUE - index);

    item->setRange(ITEM_RANGE);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    updateFrameObject(index);

    addItem(item);
}

void MsGraphicsScene::updateFrameObject(unsigned index)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const MS::FrameObject& obj = _frame->objects.at(index);
    auto* item = _objects.at(index);

    if (obj.size == ObjSize::SMALL) {
        item->setPixmap(_tilesetPixmaps->smallTile(obj.tileId));
    }
    else {
        item->setPixmap(_tilesetPixmaps->largeTile(obj.tileId));
    }
    item->setFlip(obj.hFlip, obj.vFlip);
    item->setPos(obj.location);
}

void MsGraphicsScene::addActionPoint()
{
    Q_ASSERT(_frame);

    unsigned index = _actionPoints.size();
    Q_ASSERT(index < _frame->actionPoints.size());

    auto* item = new AabbGraphicsItem();
    _actionPoints.append(item);

    item->setZValue(ACTION_POINT_ZVALUE - index);

    item->setRange(ITEM_RANGE);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    item->setPen(_style->actionPointPen());
    item->setBrush(_style->actionPointBrush());

    updateActionPoint(index);

    addItem(item);
}

void MsGraphicsScene::updateActionPoint(unsigned index)
{
    const MS::ActionPoint& ap = _frame->actionPoints.at(index);
    auto* item = _actionPoints.at(index);

    item->setPos(ap.location);
}

void MsGraphicsScene::addEntityHitbox()
{
    Q_ASSERT(_frame);

    unsigned index = _entityHitboxes.size();
    Q_ASSERT(index < _frame->entityHitboxes.size());

    auto* item = new ResizableAabbGraphicsItem();
    _entityHitboxes.append(item);

    item->setZValue(ENTITY_HITBOX_ZVALUE - index);

    item->setRange(ITEM_RANGE);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    updateEntityHitbox(index);

    addItem(item);
}

void MsGraphicsScene::updateEntityHitbox(unsigned index)
{
    const MS::EntityHitbox& eh = _frame->entityHitboxes.at(index);
    auto* item = _entityHitboxes.at(index);

    item->setPen(_style->entityHitboxPen(eh.hitboxType));
    item->setBrush(_style->entityHitboxBrush(eh.hitboxType));

    item->setRect(eh.aabb);
}

void MsGraphicsScene::onLayerSettingsChanged()
{
    bool solid = _frame && _frame->solid;
    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & solid);

    bool showOrigin = _frame && _layerSettings->showOrigin();
    _horizontalOrigin->setVisible(showOrigin);
    _verticalOrigin->setVisible(showOrigin);

    for (auto* item : _objects) {
        item->setVisible(_layerSettings->showFrameObjects());
    }
    for (auto* item : _actionPoints) {
        item->setVisible(_layerSettings->showActionPoints());
    }
    for (auto* item : _entityHitboxes) {
        item->setVisible(_layerSettings->showEntityHitboxes());
    }

    update();
}

void MsGraphicsScene::onSelectedFrameChanged()
{
    setFrame(_document->frameMap()->selectedFrame());
}

template <class T>
void MsGraphicsScene::updateSelection(QList<T>& items,
                                      const vectorset<size_t>& selectedIndexes)
{
    if (_frame == nullptr || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    for (int i = 0; i < items.size(); i++) {
        QGraphicsItem* item = items.at(i);
        item->setSelected(selectedIndexes.contains(i));
    }

    _inUpdateSelection = false;
}

void MsGraphicsScene::updateFrameObjectSelection()
{
    updateSelection(_objects, _document->frameObjectList()->selectedIndexes());
}

void MsGraphicsScene::updateActionPointSelection()
{
    updateSelection(_actionPoints, _document->actionPointList()->selectedIndexes());
}

void MsGraphicsScene::updateEntityHitboxSelection()
{
    updateSelection(_entityHitboxes, _document->entityHitboxList()->selectedIndexes());
}

void MsGraphicsScene::updateTileHitboxSelection()
{
    if (_frame == nullptr || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    bool hbSelected = _document->frameMap()->isTileHitboxSelected();

    _tileHitbox->setSelected(hbSelected);

    _inUpdateSelection = false;
}

void MsGraphicsScene::onSceneSelectionChanged()
{
    if (_frame == nullptr || _inUpdateSelection) {
        return;
    }

    Q_ASSERT(_inOnSceneSelectionChanged == false);
    _inOnSceneSelectionChanged = true;

    auto getSelected = [](const auto& items) {
        std::vector<size_t> sel;

        for (int i = 0; i < items.size(); i++) {
            const QGraphicsItem* item = items.at(i);

            if (item->isSelected()) {
                sel.push_back(i);
            }
        }

        return std::move(sel);
    };

    _document->frameObjectList()->setSelectedIndexes(getSelected(_objects));
    _document->actionPointList()->setSelectedIndexes(getSelected(_actionPoints));
    _document->entityHitboxList()->setSelectedIndexes(getSelected(_entityHitboxes));

    _document->frameMap()->setTileHitboxSelected(_tileHitbox->isSelected());

    _inOnSceneSelectionChanged = false;
}

void MsGraphicsScene::onTilesetPixmapsChanged()
{
    for (int i = 0; i < _objects.size(); i++) {
        updateFrameObject(i);
    }
}

void MsGraphicsScene::onFrameDataChanged(const void* framePtr)
{
    if (framePtr == _frame) {
        updateTileHitbox();
    }
}

#define FRAME_CHILDREN_SLOTS(CLS, ITEM_LIST, FRAME_LIST)             \
    void MsGraphicsScene::on##CLS##Changed(                          \
        const void* framePtr, size_t index)                          \
    {                                                                \
        if (_frame == framePtr) {                                    \
            update##CLS(index);                                      \
        }                                                            \
    }                                                                \
                                                                     \
    void MsGraphicsScene::on##CLS##ListChanged(const void* framePtr) \
    {                                                                \
        if (_frame != framePtr) {                                    \
            return;                                                  \
        }                                                            \
                                                                     \
        int flSize = _frame->FRAME_LIST.size();                      \
                                                                     \
        while (ITEM_LIST.size() > flSize) {                          \
            auto* item = ITEM_LIST.takeLast();                       \
            delete item;                                             \
        }                                                            \
                                                                     \
        for (int i = 0; i < ITEM_LIST.size(); i++) {                 \
            update##CLS(i);                                          \
        }                                                            \
                                                                     \
        while (ITEM_LIST.size() < flSize) {                          \
            add##CLS();                                              \
        }                                                            \
    }
FRAME_CHILDREN_SLOTS(FrameObject, _objects, objects)
FRAME_CHILDREN_SLOTS(ActionPoint, _actionPoints, actionPoints)
FRAME_CHILDREN_SLOTS(EntityHitbox, _entityHitboxes, entityHitboxes)
