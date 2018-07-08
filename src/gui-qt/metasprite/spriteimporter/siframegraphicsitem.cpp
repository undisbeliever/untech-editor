/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "siframegraphicsitem.h"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QPen>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiFrameGraphicsItem::SiFrameGraphicsItem(SI::Frame* frame,
                                         QMenu* contextMenu, Style* style,
                                         QGraphicsItem* parent)
    : AabbGraphicsItem(parent)
    , _frame(frame)
    , _contextMenu(contextMenu)
    , _style(style)
    , _showTileHitbox(true)
    , _frameSelected(false)
{
    Q_ASSERT(frame != nullptr);
    Q_ASSERT(contextMenu != nullptr);
    Q_ASSERT(style != nullptr);

    setPen(style->frameOutlinePen());

    _tileHitbox = new ResizableAabbGraphicsItem(this);
    _tileHitbox->setZValue(TILE_HITBOX_ZVALUE);
    _tileHitbox->setPen(style->tileHitboxPen());
    _tileHitbox->setBrush(style->tileHitboxBrush());
    _tileHitbox->setFlag(QGraphicsItem::ItemIsSelectable);
    _tileHitbox->setFlag(QGraphicsItem::ItemIsMovable);

    _horizontalOrigin = new QGraphicsLineItem(this);
    _horizontalOrigin->setZValue(ORIGIN_ZVALUE);
    _horizontalOrigin->setPen(style->originPen());

    _verticalOrigin = new QGraphicsLineItem(this);
    _verticalOrigin->setZValue(ORIGIN_ZVALUE);
    _verticalOrigin->setPen(style->originPen());

    updateFrameLocation();
    onFrameDataChanged();

    for (unsigned i = 0; i < frame->objects.size(); i++) {
        addFrameObject();
    }
    for (unsigned i = 0; i < frame->actionPoints.size(); i++) {
        addActionPoint();
    }
    for (unsigned i = 0; i < frame->entityHitboxes.size(); i++) {
        addEntityHitbox();
    }
}

void SiFrameGraphicsItem::setFrameSelected(bool sel)
{
    if (_frameSelected != sel) {
        _frameSelected = sel;

        for (auto* item : _objects) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, sel);
        }
        for (auto* item : _actionPoints) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, sel);
        }
        for (auto* item : _entityHitboxes) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, sel);
        }
    }
}

template <typename T>
static void updateSelection(QList<T>& items, const UnTech::vectorset<size_t>& selectedIndexes)
{
    for (int i = 0; i < items.size(); i++) {
        QGraphicsItem* item = items.at(i);
        item->setSelected(selectedIndexes.contains(i));
    }
}

void SiFrameGraphicsItem::updateFrameObjectSelection(const vectorset<size_t>& selectedIndexes)
{
    updateSelection(_objects, selectedIndexes);
}

void SiFrameGraphicsItem::updateActionPointSelection(const vectorset<size_t>& selectedIndexes)
{
    updateSelection(_actionPoints, selectedIndexes);
}

void SiFrameGraphicsItem::updateEntityHitboxSelection(const vectorset<size_t>& selectedIndexes)
{
    updateSelection(_entityHitboxes, selectedIndexes);
}

void SiFrameGraphicsItem::updateTileHitboxSelected(bool s)
{
    _tileHitbox->setSelected(s);
}

void SiFrameGraphicsItem::updateFrameLocation()
{
    const urect& aabb = _frame->location.aabb;
    const upoint& origin = _frame->location.origin;
    const QRect itemRange(0, 0, aabb.width, aabb.height);

    setRect(aabb);

    _tileHitbox->setRange(itemRange);

    _horizontalOrigin->setLine(0, origin.y, aabb.width, origin.y);
    _verticalOrigin->setLine(origin.x, 0, origin.x, aabb.height);

    for (auto* item : _objects) {
        item->setRange(itemRange);
    }
    for (auto* item : _actionPoints) {
        item->setRange(itemRange);
    }
    for (auto* item : _entityHitboxes) {
        item->setRange(itemRange);
    }
}

void SiFrameGraphicsItem::onFrameDataChanged()
{
    // Do not update frame location, there is a seperate signal for that

    _tileHitbox->setVisible(_showTileHitbox & _frame->solid);
    _tileHitbox->setRect(_frame->tileHitbox);
}

void SiFrameGraphicsItem::addFrameObject()
{
    size_t index = _objects.size();
    Q_ASSERT(index < _frame->objects.size());

    auto* item = new AabbGraphicsItem(this);
    _objects.append(item);

    item->setZValue(FRAME_OBJECT_ZVALUE - index);

    item->setRange(0, 0, _frame->location.aabb.size());
    item->setFlag(QGraphicsItem::ItemIsSelectable, _frameSelected);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    item->setPen(_style->frameObjectPen());

    updateFrameObject(index);
}

void SiFrameGraphicsItem::updateFrameObject(size_t index)
{
    const SI::FrameObject& obj = _frame->objects.at(index);
    auto* item = _objects.at(index);

    item->setRect(obj.location, obj.sizePx());
}

void SiFrameGraphicsItem::addActionPoint()
{
    Q_ASSERT(_frame);

    unsigned index = _actionPoints.size();
    Q_ASSERT(index < _frame->actionPoints.size());

    auto* item = new AabbGraphicsItem(this);
    _actionPoints.append(item);

    item->setZValue(ACTION_POINT_ZVALUE - index);

    item->setRange(0, 0, _frame->location.aabb.size());
    item->setFlag(QGraphicsItem::ItemIsSelectable, _frameSelected);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    item->setPen(_style->actionPointPen());
    item->setBrush(_style->actionPointBrush());

    updateActionPoint(index);
}

void SiFrameGraphicsItem::updateActionPoint(size_t index)
{
    const SI::ActionPoint& ap = _frame->actionPoints.at(index);
    auto* item = _actionPoints.at(index);

    item->setPos(ap.location);
}

void SiFrameGraphicsItem::addEntityHitbox()
{
    Q_ASSERT(_frame);

    size_t index = _entityHitboxes.size();
    Q_ASSERT(index < _frame->entityHitboxes.size());

    auto* item = new ResizableAabbGraphicsItem(this);
    _entityHitboxes.append(item);

    item->setZValue(ENTITY_HITBOX_ZVALUE - index);

    item->setRange(0, 0, _frame->location.aabb.size());
    item->setFlag(QGraphicsItem::ItemIsSelectable, _frameSelected);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    updateEntityHitbox(index);
}

void SiFrameGraphicsItem::updateEntityHitbox(size_t index)
{
    const SI::EntityHitbox& eh = _frame->entityHitboxes.at(index);
    auto* item = _entityHitboxes.at(index);

    item->setPen(_style->entityHitboxPen(eh.hitboxType));
    item->setBrush(_style->entityHitboxBrush(eh.hitboxType));
    item->setToolTip(EH_LONG_STRING_VALUES.at(eh.hitboxType.romValue()));

    item->setRect(eh.aabb);
}

void SiFrameGraphicsItem::updateLayerSettings(const LayerSettings* settings)
{
    _showTileHitbox = settings->showTileHitbox();
    _tileHitbox->setVisible(_showTileHitbox & _frame->solid);

    _horizontalOrigin->setVisible(settings->showOrigin());
    _verticalOrigin->setVisible(settings->showOrigin());

    for (auto* item : _objects) {
        item->setVisible(settings->showFrameObjects());
    }
    for (auto* item : _actionPoints) {
        item->setVisible(settings->showActionPoints());
    }
    for (auto* item : _entityHitboxes) {
        item->setVisible(settings->showEntityHitboxes());
    }

    update();
}

void SiFrameGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    if (_frameSelected) {
        _contextMenu->exec(event->screenPos());
    }
    else {
        event->ignore();
    }
}

#define ON_LIST_CHANGED(CLS, ITEM_LIST, FRAME_LIST)  \
                                                     \
    void SiFrameGraphicsItem::on##CLS##ListChanged() \
    {                                                \
        int flSize = _frame->FRAME_LIST.size();      \
                                                     \
        while (ITEM_LIST.size() > flSize) {          \
            auto* item = ITEM_LIST.takeLast();       \
            delete item;                             \
        }                                            \
                                                     \
        for (int i = 0; i < ITEM_LIST.size(); i++) { \
            update##CLS(i);                          \
        }                                            \
                                                     \
        while (ITEM_LIST.size() < flSize) {          \
            add##CLS();                              \
        }                                            \
    }
ON_LIST_CHANGED(FrameObject, _objects, objects)
ON_LIST_CHANGED(ActionPoint, _actionPoints, actionPoints)
ON_LIST_CHANGED(EntityHitbox, _entityHitboxes, entityHitboxes)
