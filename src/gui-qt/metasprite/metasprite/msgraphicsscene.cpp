/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "msgraphicsscene.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilesetpixmaps.h"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/pixmapgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/metasprite/common.h"
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
    , _resourceItem(nullptr)
    , _frameIndex(INT_MAX)
    , _inUpdateSelection(false)
    , _inOnSceneSelectionChanged(false)
{
    Q_ASSERT(tilesetPixmaps != nullptr);

    _tileHitbox->setToolTip(tr("Tile Hitbox"));
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

void MsGraphicsScene::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->frameList()->disconnect(this);
        _resourceItem->frameObjectList()->disconnect(this);
        _resourceItem->actionPointList()->disconnect(this);
        _resourceItem->entityHitboxList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    setFrameIndex(INT_MAX);

    if (_resourceItem) {
        onSelectedFrameChanged();
        updateTileHitboxSelection();
        updateFrameObjectSelection();
        updateActionPointSelection();
        updateEntityHitboxSelection();

        connect(_resourceItem->frameList(), &FrameList::selectedIndexChanged,
                this, &MsGraphicsScene::onSelectedFrameChanged);

        connect(_resourceItem->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &MsGraphicsScene::updateFrameObjectSelection);
        connect(_resourceItem->actionPointList(), &ActionPointList::selectedIndexesChanged,
                this, &MsGraphicsScene::updateActionPointSelection);
        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &MsGraphicsScene::updateEntityHitboxSelection);

        connect(_resourceItem->frameList(), &FrameList::tileHitboxSelectedChanged,
                this, &MsGraphicsScene::updateTileHitboxSelection);

        connect(_resourceItem->frameList(), &FrameList::dataChanged,
                this, &MsGraphicsScene::onFrameDataChanged);

        connect(_resourceItem->frameObjectList(), &FrameObjectList::dataChanged,
                this, &MsGraphicsScene::onFrameObjectChanged);
        connect(_resourceItem->actionPointList(), &ActionPointList::dataChanged,
                this, &MsGraphicsScene::onActionPointChanged);
        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::dataChanged,
                this, &MsGraphicsScene::onEntityHitboxChanged);

        // This class uses listChanged signal as it is more efficient when multiple items change

        connect(_resourceItem->frameObjectList(), &FrameObjectList::listChanged,
                this, &MsGraphicsScene::onFrameObjectListChanged);
        connect(_resourceItem->actionPointList(), &ActionPointList::listChanged,
                this, &MsGraphicsScene::onActionPointListChanged);
        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::listChanged,
                this, &MsGraphicsScene::onEntityHitboxListChanged);
    }
}

void MsGraphicsScene::setFrameIndex(size_t frameIndex)
{
    if (_frameIndex != frameIndex) {
        _frameIndex = frameIndex;

        auto* frame = selectedFrame();

        _tileHitbox->setVisible(false);
        qDeleteAll(_objects);
        qDeleteAll(_actionPoints);
        qDeleteAll(_entityHitboxes);

        _objects.clear();
        _actionPoints.clear();
        _entityHitboxes.clear();

        bool showOrigin = frame && _layerSettings->showOrigin();
        _horizontalOrigin->setVisible(showOrigin);
        _verticalOrigin->setVisible(showOrigin);

        if (frame) {
            setSceneRect(int_ms8_t::MIN, int_ms8_t::MIN, 256, 256);

            updateTileHitbox();
            updateTileHitboxSelection();

            // rebuild item lists
            onFrameObjectListChanged(_frameIndex);
            onActionPointListChanged(_frameIndex);
            onEntityHitboxListChanged(_frameIndex);
        }
        else {
            setSceneRect(QRect());
        }

        update();
    }
}

bool MsGraphicsScene::selectedFrameValid() const
{
    return _resourceItem
           && _resourceItem->frameSet()
           && _frameIndex < _resourceItem->frameSet()->frames.size();
}

const MS::Frame* MsGraphicsScene::selectedFrame() const
{
    if (_resourceItem == nullptr) {
        return nullptr;
    }
    auto* fs = _resourceItem->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (_frameIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(_frameIndex);
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
    if (selectedFrame()) {
        _contextMenu->exec(event->screenPos());
    }
}

void MsGraphicsScene::updateTileHitbox()
{
    auto* frame = selectedFrame();
    Q_ASSERT(frame);

    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & frame->solid);
    _tileHitbox->setRect(frame->tileHitbox);
}

void MsGraphicsScene::addFrameObject(const MS::Frame& frame)
{
    unsigned index = _objects.size();
    Q_ASSERT(index < frame.objects.size());

    auto* item = new PixmapGraphicsItem();
    _objects.insert(index, item);

    item->setZValue(FRAME_OBJECT_ZVALUE - index);

    item->setRange(ITEM_RANGE);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    updateFrameObject(index, frame.objects.at(index));

    addItem(item);
}

void MsGraphicsScene::updateFrameObject(unsigned index, const UnTech::MetaSprite::MetaSprite::FrameObject& obj)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

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

void MsGraphicsScene::addActionPoint(const MS::Frame& frame)
{
    unsigned index = _actionPoints.size();
    Q_ASSERT(index < frame.actionPoints.size());

    auto* item = new AabbGraphicsItem();
    _actionPoints.append(item);

    item->setZValue(ACTION_POINT_ZVALUE - index);

    item->setRange(ITEM_RANGE);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    item->setPen(_style->actionPointPen());
    item->setBrush(_style->actionPointBrush());

    updateActionPoint(index, frame.actionPoints.at(index));

    addItem(item);
}

void MsGraphicsScene::updateActionPoint(unsigned index, const UnTech::MetaSprite::MetaSprite::ActionPoint& ap)
{
    auto* item = _actionPoints.at(index);

    item->setPos(ap.location);
    item->setToolTip(QString::fromStdString(ap.type));
}

void MsGraphicsScene::addEntityHitbox(const MS::Frame& frame)
{
    unsigned index = _entityHitboxes.size();
    Q_ASSERT(index < frame.entityHitboxes.size());

    auto* item = new ResizableAabbGraphicsItem();
    _entityHitboxes.append(item);

    item->setZValue(ENTITY_HITBOX_ZVALUE - index);

    item->setRange(ITEM_RANGE);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);

    updateEntityHitbox(index, frame.entityHitboxes.at(index));

    addItem(item);
}

void MsGraphicsScene::updateEntityHitbox(unsigned index, const UnTech::MetaSprite::MetaSprite::EntityHitbox& eh)
{
    auto* item = _entityHitboxes.at(index);

    item->setPen(_style->entityHitboxPen(eh.hitboxType));
    item->setBrush(_style->entityHitboxBrush(eh.hitboxType));
    item->setToolTip(EH_LONG_STRING_VALUES.at(eh.hitboxType.romValue()));

    item->setRect(eh.aabb);
}

void MsGraphicsScene::onLayerSettingsChanged()
{
    auto* frame = selectedFrame();

    bool solid = frame && frame->solid;
    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & solid);

    bool showOrigin = frame && _layerSettings->showOrigin();
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
    setFrameIndex(_resourceItem->frameList()->selectedIndex());
}

template <class T>
void MsGraphicsScene::updateSelection(QList<T>& items,
                                      const vectorset<size_t>& selectedIndexes)
{
    if (!selectedFrameValid() || _inOnSceneSelectionChanged) {
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
    updateSelection(_objects, _resourceItem->frameObjectList()->selectedIndexes());
}

void MsGraphicsScene::updateActionPointSelection()
{
    updateSelection(_actionPoints, _resourceItem->actionPointList()->selectedIndexes());
}

void MsGraphicsScene::updateEntityHitboxSelection()
{
    updateSelection(_entityHitboxes, _resourceItem->entityHitboxList()->selectedIndexes());
}

void MsGraphicsScene::updateTileHitboxSelection()
{
    if (!selectedFrameValid() || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    bool hbSelected = _resourceItem->frameList()->isTileHitboxSelected();

    _tileHitbox->setSelected(hbSelected);

    _inUpdateSelection = false;
}

void MsGraphicsScene::onSceneSelectionChanged()
{
    if (!selectedFrameValid() || _inUpdateSelection) {
        return;
    }

    Q_ASSERT(_inOnSceneSelectionChanged == false);
    _inOnSceneSelectionChanged = true;

    bool contentSelected = false;

    auto getSelected = [&](const auto& items) {
        std::vector<size_t> sel;

        for (int i = 0; i < items.size(); i++) {
            const QGraphicsItem* item = items.at(i);

            if (item->isSelected()) {
                sel.push_back(i);
                contentSelected = true;
            }
        }

        return std::move(sel);
    };

    _resourceItem->frameObjectList()->setSelectedIndexes(getSelected(_objects));
    _resourceItem->actionPointList()->setSelectedIndexes(getSelected(_actionPoints));
    _resourceItem->entityHitboxList()->setSelectedIndexes(getSelected(_entityHitboxes));

    _resourceItem->frameList()->setTileHitboxSelected(_tileHitbox->isSelected());

    if (contentSelected) {
        emit frameContentSelected();
    }

    _inOnSceneSelectionChanged = false;
}

void MsGraphicsScene::onTilesetPixmapsChanged()
{
    if (auto* frame = selectedFrame()) {
        for (int i = 0; i < _objects.size(); i++) {
            updateFrameObject(i, frame->objects.at(i));
        }
    }
}

void MsGraphicsScene::onFrameDataChanged(size_t frameIndex)
{
    if (_frameIndex == frameIndex) {
        updateTileHitbox();
    }
}

#define FRAME_CHILDREN_SLOTS(CLS, ITEM_LIST, FRAME_LIST)          \
    void MsGraphicsScene::on##CLS##Changed(                       \
        size_t frameIndex, size_t index)                          \
    {                                                             \
        if (_frameIndex == frameIndex) {                          \
            auto* frame = selectedFrame();                        \
            Q_ASSERT(frame);                                      \
            update##CLS(index, frame->FRAME_LIST.at(index));      \
        }                                                         \
    }                                                             \
                                                                  \
    void MsGraphicsScene::on##CLS##ListChanged(size_t frameIndex) \
    {                                                             \
        if (_frameIndex != frameIndex) {                          \
            return;                                               \
        }                                                         \
                                                                  \
        auto* frame = selectedFrame();                            \
        Q_ASSERT(frame);                                          \
        int flSize = frame->FRAME_LIST.size();                    \
                                                                  \
        while (ITEM_LIST.size() > flSize) {                       \
            auto* item = ITEM_LIST.takeLast();                    \
            delete item;                                          \
        }                                                         \
                                                                  \
        for (int i = 0; i < ITEM_LIST.size(); i++) {              \
            update##CLS(i, frame->FRAME_LIST.at(i));              \
        }                                                         \
                                                                  \
        while (ITEM_LIST.size() < flSize) {                       \
            add##CLS(*frame);                                     \
        }                                                         \
    }
FRAME_CHILDREN_SLOTS(FrameObject, _objects, objects)
FRAME_CHILDREN_SLOTS(ActionPoint, _actionPoints, actionPoints)
FRAME_CHILDREN_SLOTS(EntityHitbox, _entityHitboxes, entityHitboxes)
