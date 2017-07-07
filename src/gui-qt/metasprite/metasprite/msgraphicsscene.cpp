/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "msgraphicsscene.h"
#include "document.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QGraphicsView>

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

MsGraphicsScene::MsGraphicsScene(LayerSettings* layerSettings, QWidget* parent)
    : QGraphicsScene(parent)
    , _layerSettings(layerSettings)
    , _document(nullptr)
    , _frame(nullptr)
    , _inUpdateSelection(false)
{
    Q_ASSERT(layerSettings != nullptr);

    _style = new Style(parent);

    _tileHitbox = new QGraphicsRectItem();
    _tileHitbox->setPen(_style->tileHitboxPen());
    _tileHitbox->setBrush(_style->tileHitboxBrush());
    _tileHitbox->setZValue(TILE_HITBOX_ZVALUE);
    _tileHitbox->setVisible(false);
    addItem(_tileHitbox);

    onLayerSettingsChanged();

    connect(this, &MsGraphicsScene::selectionChanged,
            this, &MsGraphicsScene::onSceneSelectionChanged);

    connect(_layerSettings, &LayerSettings::layerSettingsChanged,
            this, &MsGraphicsScene::onLayerSettingsChanged);
}

void MsGraphicsScene::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setFrame(nullptr);

    if (_document) {
        onSelectedFrameChanged();
        updateSelection();

        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &MsGraphicsScene::onSelectedFrameChanged);
        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &MsGraphicsScene::updateSelection);

        connect(_document, &Document::frameTileHitboxChanged,
                this, &MsGraphicsScene::onFrameTileHitboxChanged);

        connect(_document, &Document::frameObjectChanged,
                this, &MsGraphicsScene::onFrameObjectChanged);
        connect(_document, &Document::actionPointChanged,
                this, &MsGraphicsScene::onActionPointChanged);
        connect(_document, &Document::entityHitboxChanged,
                this, &MsGraphicsScene::onEntityHitboxChanged);

        connect(_document, &Document::frameObjectAboutToBeRemoved,
                this, &MsGraphicsScene::onFrameObjectAboutToBeRemoved);
        connect(_document, &Document::actionPointAboutToBeRemoved,
                this, &MsGraphicsScene::onActionPointAboutToBeRemoved);
        connect(_document, &Document::entityHitboxAboutToBeRemoved,
                this, &MsGraphicsScene::onEntityHitboxAboutToBeRemoved);

        connect(_document, &Document::frameObjectAdded,
                this, &MsGraphicsScene::onFrameObjectAdded);
        connect(_document, &Document::actionPointAdded,
                this, &MsGraphicsScene::onActionPointAdded);
        connect(_document, &Document::entityHitboxAdded,
                this, &MsGraphicsScene::onEntityHitboxAdded);

        connect(_document, &Document::frameContentsMoved,
                this, &MsGraphicsScene::onFrameContentsMoved);
    }
}

void MsGraphicsScene::setFrame(MS::Frame* frame)
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

        if (_frame != nullptr) {
            setSceneRect(int_ms8_t::MIN, int_ms8_t::MIN, 256, 256);

            updateTileHitbox();

            for (unsigned i = 0; i < _frame->objects.size(); i++) {
                addFrameObject(i);
            }
            for (unsigned i = 0; i < _frame->actionPoints.size(); i++) {
                addActionPoint(i);
            }
            for (unsigned i = 0; i < _frame->entityHitboxes.size(); i++) {
                addEntityHitbox(i);
            }
        }
        else {
            setSceneRect(QRect());
        }

        update();
    }
}

void MsGraphicsScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (_frame != nullptr && _layerSettings->showOrigin()) {
        QRectF r = rect.adjusted(-1, -1, 1, 1);

        painter->save();

        painter->setPen(_style->originPen());

        painter->drawLine(0, 0, 0, r.top());
        painter->drawLine(0, 0, 0, r.bottom());
        painter->drawLine(0, 0, r.left(), 0);
        painter->drawLine(0, 0, r.right(), 0);

        painter->restore();
    }
}

void MsGraphicsScene::updateTileHitbox()
{
    const ms8rect& hitbox = _frame->tileHitbox;

    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & _frame->solid);

    _tileHitbox->setRect(hitbox.x, hitbox.y, hitbox.width, hitbox.height);
}

template <class T>
void MsGraphicsScene::updateItemIndexes(QList<T*>& list, unsigned start,
                                        unsigned baseZValue,
                                        const SelectedItem::Type& type)
{
    for (unsigned i = start; int(i) < list.size(); i++) {
        list.at(i)->setZValue(baseZValue + i);

        SelectedItem si = { type, i };
        list.at(i)->setData(SELECTION_ID, QVariant::fromValue(si));
    }
}

void MsGraphicsScene::addFrameObject(unsigned index)
{
    auto* item = new QGraphicsRectItem();
    _objects.insert(index, item);
    updateItemIndexes(_objects, index,
                      FRAME_OBJECT_ZVALUE, SelectedItem::FRAME_OBJECT);

    item->setFlag(QGraphicsItem::ItemIsSelectable);

    item->setPen(_style->frameObjectPen());
    item->setBrush(QBrush(Qt::black));

    updateFrameObject(index);

    addItem(item);
}

void MsGraphicsScene::updateFrameObject(unsigned index)
{
    MS::FrameObject& obj = _frame->objects.at(index);
    auto* item = _objects.at(index);

    // ::TODO frameObject tile::
    item->setRect(obj.location.x, obj.location.y,
                  obj.sizePx(), obj.sizePx());
}

void MsGraphicsScene::removeFrameObject(unsigned index)
{
    auto* item = _objects.takeAt(index);
    delete item;
    updateItemIndexes(_objects, index,
                      FRAME_OBJECT_ZVALUE, SelectedItem::FRAME_OBJECT);
}

void MsGraphicsScene::addActionPoint(unsigned index)
{
    auto* item = new QGraphicsRectItem();
    _actionPoints.insert(index, item);
    updateItemIndexes(_actionPoints, index,
                      ACTION_POINT_ZVALUE, SelectedItem::ACTION_POINT);

    item->setFlag(QGraphicsItem::ItemIsSelectable);

    item->setPen(_style->actionPointPen());
    item->setBrush(_style->actionPointBrush());
    item->setRect(0, 0, 1, 1);

    updateActionPoint(index);

    addItem(item);
}

void MsGraphicsScene::updateActionPoint(unsigned index)
{
    MS::ActionPoint& ap = _frame->actionPoints.at(index);
    auto* item = _actionPoints.at(index);

    item->setPos(ap.location.x, ap.location.y);
}

void MsGraphicsScene::removeActionPoint(unsigned index)
{
    auto* item = _actionPoints.takeAt(index);
    delete item;
    updateItemIndexes(_actionPoints, index,
                      ACTION_POINT_ZVALUE, SelectedItem::ACTION_POINT);
}

void MsGraphicsScene::addEntityHitbox(unsigned index)
{
    auto* item = new QGraphicsRectItem();
    _entityHitboxes.insert(index, item);
    updateItemIndexes(_entityHitboxes, index,
                      ENTITY_HITBOX_ZVALUE, SelectedItem::ENTITY_HITBOX);

    item->setFlag(QGraphicsItem::ItemIsSelectable);

    updateEntityHitbox(index);

    addItem(item);
}

void MsGraphicsScene::updateEntityHitbox(unsigned index)
{
    MS::EntityHitbox& eh = _frame->entityHitboxes.at(index);
    auto* item = _entityHitboxes.at(index);

    item->setPen(_style->entityHitboxPen(eh.hitboxType));
    item->setBrush(_style->entityHitboxBrush(eh.hitboxType));

    item->setRect(eh.aabb.x, eh.aabb.y,
                  eh.aabb.width, eh.aabb.height);
}

void MsGraphicsScene::removeEntityHitbox(unsigned index)
{
    auto* item = _entityHitboxes.takeAt(index);
    delete item;
    updateItemIndexes(_entityHitboxes, index,
                      ENTITY_HITBOX_ZVALUE, SelectedItem::ENTITY_HITBOX);
}

void MsGraphicsScene::onLayerSettingsChanged()
{
    bool solid = _frame && _frame->solid;
    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & solid);

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
    setFrame(_document->selection()->selectedFrame());
}

void MsGraphicsScene::updateSelection()
{
    if (_frame == nullptr) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    const auto& sel = _document->selection()->selectedItems();

    auto processList = [&](auto list, SelectedItem::Type type) {
        SelectedItem si{ type, 0 };

        for (QGraphicsItem* item : list) {
            item->setSelected(sel.find(si) != sel.end());
            si.index++;
        }
    };

    processList(_objects, SelectedItem::FRAME_OBJECT);
    processList(_actionPoints, SelectedItem::ACTION_POINT);
    processList(_entityHitboxes, SelectedItem::ENTITY_HITBOX);

    _inUpdateSelection = false;
}

void MsGraphicsScene::onSceneSelectionChanged()
{
    if (_frame == nullptr || _inUpdateSelection) {
        return;
    }

    std::set<SelectedItem> selection;

    for (const QGraphicsItem* item : selectedItems()) {
        QVariant v = item->data(SELECTION_ID);
        if (v.isValid()) {
            selection.insert(v.value<SelectedItem>());
        }
    }

    _document->selection()->setSelectedItems(selection);
}

void MsGraphicsScene::onFrameTileHitboxChanged(const void* framePtr)
{
    if (framePtr == _frame) {
        updateTileHitbox();
    }
}

#define FRAME_CHILDREN_SLOTS(CLS)                    \
    void MsGraphicsScene::on##CLS##Changed(          \
        const void* framePtr, unsigned index)        \
    {                                                \
        if (_frame == framePtr) {                    \
            update##CLS(index);                      \
        }                                            \
    }                                                \
                                                     \
    void MsGraphicsScene::on##CLS##AboutToBeRemoved( \
        const void* framePtr, unsigned index)        \
    {                                                \
        if (_frame == framePtr) {                    \
            remove##CLS(index);                      \
        }                                            \
    }                                                \
    void MsGraphicsScene::on##CLS##Added(            \
        const void* framePtr, unsigned index)        \
    {                                                \
        if (_frame == framePtr) {                    \
            add##CLS(index);                         \
        }                                            \
    }
FRAME_CHILDREN_SLOTS(FrameObject)
FRAME_CHILDREN_SLOTS(ActionPoint)
FRAME_CHILDREN_SLOTS(EntityHitbox)

void MsGraphicsScene::onFrameContentsMoved(
    const void* framePtr, const std::set<SelectedItem>&, int)
{
    if (framePtr == _frame) {
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
}
