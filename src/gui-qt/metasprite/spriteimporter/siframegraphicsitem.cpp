/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "siframegraphicsitem.h"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/metasprite/abstractselection.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QPen>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiFrameGraphicsItem::SiFrameGraphicsItem(SI::Frame* frame, Style* style,
                                         QGraphicsItem* parent)
    : AabbGraphicsItem(parent)
    , _frame(frame)
    , _style(style)
    , _showTileHitbox(true)
    , _frameSelected(false)
{
    Q_ASSERT(frame != nullptr);
    Q_ASSERT(style != nullptr);

    setPen(style->frameOutlinePen());

    _tileHitbox = new ResizableAabbGraphicsItem(this);
    _tileHitbox->setZValue(TILE_HITBOX_ZVALUE);
    _tileHitbox->setPen(style->tileHitboxPen());
    _tileHitbox->setBrush(style->tileHitboxBrush());
    _tileHitbox->setFlag(QGraphicsItem::ItemIsSelectable);
    _tileHitbox->setFlag(QGraphicsItem::ItemIsMovable);
    _tileHitbox->setData(SELECTION_ID, QVariant::fromValue<SelectedItem>(
                                           { SelectedItem::TILE_HITBOX, 0 }));

    _horizontalOrigin = new QGraphicsLineItem(this);
    _horizontalOrigin->setZValue(ORIGIN_ZVALUE);
    _horizontalOrigin->setPen(style->originPen());

    _verticalOrigin = new QGraphicsLineItem(this);
    _verticalOrigin->setZValue(ORIGIN_ZVALUE);
    _verticalOrigin->setPen(style->originPen());

    updateFrameLocation();
    updateTileHitbox();

    for (unsigned i = 0; i < frame->objects.size(); i++) {
        addFrameObject(i);
    }
    for (unsigned i = 0; i < frame->actionPoints.size(); i++) {
        addActionPoint(i);
    }
    for (unsigned i = 0; i < frame->entityHitboxes.size(); i++) {
        addEntityHitbox(i);
    }
}

template <class T>
void SiFrameGraphicsItem::updateItemIndexes(QList<T*>& list, unsigned start,
                                            unsigned baseZValue,
                                            const SelectedItem::Type& type)
{
    for (unsigned i = start; int(i) < list.size(); i++) {
        list.at(i)->setZValue(baseZValue + i);

        SelectedItem si = { type, i };
        list.at(i)->setData(SELECTION_ID, QVariant::fromValue(si));
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

void SiFrameGraphicsItem::updateSelection(const std::set<SelectedItem>& selection)
{
    auto processList = [&](auto list, SelectedItem::Type type) {
        SelectedItem si{ type, 0 };

        for (QGraphicsItem* item : list) {
            item->setSelected(selection.find(si) != selection.end());
            si.index++;
        }
    };

    processList(_objects, SelectedItem::FRAME_OBJECT);
    processList(_actionPoints, SelectedItem::ACTION_POINT);
    processList(_entityHitboxes, SelectedItem::ENTITY_HITBOX);
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

void SiFrameGraphicsItem::updateTileHitbox()
{
    _tileHitbox->setVisible(_showTileHitbox & _frame->solid);
    _tileHitbox->setRect(_frame->tileHitbox);
}

void SiFrameGraphicsItem::addFrameObject(unsigned index)
{
    auto* item = new AabbGraphicsItem(this);
    _objects.insert(index, item);
    updateItemIndexes(_objects, index,
                      FRAME_OBJECT_ZVALUE, SelectedItem::FRAME_OBJECT);

    item->setRange(0, 0, _frame->location.aabb.size());
    item->setFlag(QGraphicsItem::ItemIsSelectable, _frameSelected);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    item->setPen(_style->frameObjectPen());

    updateFrameObject(index);
}

void SiFrameGraphicsItem::updateFrameObject(unsigned index)
{
    SI::FrameObject& obj = _frame->objects.at(index);
    auto* item = _objects.at(index);

    item->setRect(obj.location, obj.sizePx());
}

void SiFrameGraphicsItem::removeFrameObject(unsigned index)
{
    auto* item = _objects.takeAt(index);
    delete item;
    updateItemIndexes(_objects, index,
                      FRAME_OBJECT_ZVALUE, SelectedItem::FRAME_OBJECT);
}

void SiFrameGraphicsItem::addActionPoint(unsigned index)
{
    auto* item = new AabbGraphicsItem(this);
    _actionPoints.insert(index, item);
    updateItemIndexes(_actionPoints, index,
                      ACTION_POINT_ZVALUE, SelectedItem::ACTION_POINT);

    item->setRange(0, 0, _frame->location.aabb.size());
    item->setFlag(QGraphicsItem::ItemIsSelectable, _frameSelected);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    item->setPen(_style->actionPointPen());
    item->setBrush(_style->actionPointBrush());

    updateActionPoint(index);
}

void SiFrameGraphicsItem::updateActionPoint(unsigned index)
{
    SI::ActionPoint& ap = _frame->actionPoints.at(index);
    auto* item = _actionPoints.at(index);

    item->setPos(ap.location);
}

void SiFrameGraphicsItem::removeActionPoint(unsigned index)
{
    auto* item = _actionPoints.takeAt(index);
    delete item;
    updateItemIndexes(_actionPoints, index,
                      ACTION_POINT_ZVALUE, SelectedItem::ACTION_POINT);
}

void SiFrameGraphicsItem::addEntityHitbox(unsigned index)
{
    auto* item = new ResizableAabbGraphicsItem(this);
    _entityHitboxes.insert(index, item);
    updateItemIndexes(_entityHitboxes, index,
                      ENTITY_HITBOX_ZVALUE, SelectedItem::ENTITY_HITBOX);

    item->setRange(0, 0, _frame->location.aabb.size());
    item->setFlag(QGraphicsItem::ItemIsSelectable, _frameSelected);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    updateEntityHitbox(index);
}

void SiFrameGraphicsItem::updateEntityHitbox(unsigned index)
{
    SI::EntityHitbox& eh = _frame->entityHitboxes.at(index);
    auto* item = _entityHitboxes.at(index);

    item->setPen(_style->entityHitboxPen(eh.hitboxType));
    item->setBrush(_style->entityHitboxBrush(eh.hitboxType));

    item->setRect(eh.aabb);
}

void SiFrameGraphicsItem::removeEntityHitbox(unsigned index)
{
    auto* item = _entityHitboxes.takeAt(index);
    delete item;
    updateItemIndexes(_entityHitboxes, index,
                      ENTITY_HITBOX_ZVALUE, SelectedItem::ENTITY_HITBOX);
}

void SiFrameGraphicsItem::updateFrameContents()
{
    for (int i = 0; i < _objects.size(); i++) {
        updateFrameObject(i);
    }
    for (int i = 0; i < _actionPoints.size(); i++) {
        updateActionPoint(i);
    }
    for (int i = 0; i < _entityHitboxes.size(); i++) {
        updateEntityHitbox(i);
    }
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
