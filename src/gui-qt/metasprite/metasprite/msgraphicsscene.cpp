/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "msgraphicsscene.h"
#include "actions.h"
#include "document.h"
#include "framecommands.h"
#include "framecontentcommands.h"
#include "tilesetpixmaps.h"
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

MsGraphicsScene::MsGraphicsScene(Actions* actions, LayerSettings* layerSettings,
                                 TilesetPixmaps* tilesetPixmaps,
                                 QWidget* parent)
    : QGraphicsScene(parent)
    , _actions(actions)
    , _layerSettings(layerSettings)
    , _tilesetPixmaps(tilesetPixmaps)
    , _document(nullptr)
    , _frame(nullptr)
    , _inUpdateSelection(false)
{
    Q_ASSERT(actions != nullptr);
    Q_ASSERT(layerSettings != nullptr);
    Q_ASSERT(tilesetPixmaps != nullptr);

    _style = new Style(parent);

    _tileHitbox = new ResizableAabbGraphicsItem();
    _tileHitbox->setPen(_style->tileHitboxPen());
    _tileHitbox->setBrush(_style->tileHitboxBrush());
    _tileHitbox->setZValue(TILE_HITBOX_ZVALUE);
    _tileHitbox->setVisible(false);
    _tileHitbox->setRange(ITEM_RANGE);
    _tileHitbox->setFlag(QGraphicsItem::ItemIsSelectable);
    _tileHitbox->setFlag(QGraphicsItem::ItemIsMovable);
    _tileHitbox->setData(SELECTION_ID, QVariant::fromValue<SelectedItem>(
                                           { SelectedItem::TILE_HITBOX, 0 }));
    addItem(_tileHitbox);

    _horizontalOrigin = new QGraphicsLineItem();
    _horizontalOrigin->setLine(int_ms8_t::MIN, 0, int_ms8_t::MAX, 0);
    _horizontalOrigin->setPen(_style->originPen());
    _horizontalOrigin->setZValue(ORIGIN_ZVALUE);
    _horizontalOrigin->setVisible(false);
    addItem(_horizontalOrigin);

    _verticalOrigin = new QGraphicsLineItem();
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

        bool showOrigin = _frame && _layerSettings->showOrigin();
        _horizontalOrigin->setVisible(showOrigin);
        _verticalOrigin->setVisible(showOrigin);

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
        const auto& selectedItems = _document->selection()->selectedItems();

        QMenu menu;
        bool addSep = false;
        if (_actions->toggleObjSize()->isEnabled()) {
            menu.addAction(_actions->toggleObjSize());
            menu.addAction(_actions->flipObjHorizontally());
            menu.addAction(_actions->flipObjVertically());
            addSep = true;
        }
        if (_actions->entityHitboxTypeMenu()->isEnabled()) {
            menu.addMenu(_actions->entityHitboxTypeMenu());
            addSep = true;
        }
        if (addSep) {
            menu.addSeparator();
        }
        menu.addAction(_actions->addFrameObject());
        menu.addAction(_actions->addActionPoint());
        menu.addAction(_actions->addEntityHitbox());
        menu.addSeparator();
        menu.addAction(_actions->addRemoveTileHitbox());

        if (selectedItems.empty() == false) {
            menu.addSeparator();
            menu.addAction(_actions->raiseSelected());
            menu.addAction(_actions->lowerSelected());
            menu.addAction(_actions->cloneSelected());
            menu.addAction(_actions->removeSelected());
        }

        menu.exec(event->screenPos());
    }
}

void MsGraphicsScene::commitMovedItems()
{
    const auto selectedItems = this->selectedItems();

    if (_frame == nullptr || selectedItems.isEmpty()) {
        return;
    }

    auto command = std::make_unique<QUndoCommand>(tr("Move"));

    for (QGraphicsItem* item : selectedItems) {
        QVariant v = item->data(SELECTION_ID);
        if (v.isValid()) {
            SelectedItem id = v.value<SelectedItem>();
            switch (id.type) {
            case SelectedItem::NONE:
                break;

            case SelectedItem::FRAME_OBJECT:
                if (auto* i = dynamic_cast<const PixmapGraphicsItem*>(item)) {
                    MS::FrameObject obj = _frame->objects.at(id.index);

                    ms8point location = i->posMs8point();
                    if (obj.location != location) {
                        obj.location = location;
                        new ChangeFrameObject(_document, _frame, id.index, obj,
                                              command.get());
                    }
                }
                break;

            case SelectedItem::ACTION_POINT:
                if (auto* i = dynamic_cast<const AabbGraphicsItem*>(item)) {
                    MS::ActionPoint ap = _frame->actionPoints.at(id.index);

                    ms8point location = i->posMs8point();
                    if (ap.location != location) {
                        ap.location = location;
                        new ChangeActionPoint(_document, _frame, id.index, ap,
                                              command.get());
                    }
                }
                break;

            case SelectedItem::ENTITY_HITBOX:
                if (auto* i = dynamic_cast<const ResizableAabbGraphicsItem*>(item)) {
                    MS::EntityHitbox eh = _frame->entityHitboxes.at(id.index);

                    ms8rect aabb = i->rectMs8rect();
                    if (eh.aabb != aabb) {
                        eh.aabb = aabb;
                        new ChangeEntityHitbox(_document, _frame, id.index, eh,
                                               command.get());
                    }
                }
                break;

            case SelectedItem::TILE_HITBOX:
                if (auto* i = dynamic_cast<const ResizableAabbGraphicsItem*>(item)) {
                    ms8rect hitbox = i->rectMs8rect();
                    if (_frame->tileHitbox != hitbox) {
                        new ChangeFrameTileHitbox(_document, _frame, hitbox,
                                                  command.get());
                    }
                }
                break;
            }
        }
    }

    if (command->childCount() > 0) {
        _document->undoStack()->push(command.release());
    }
}

void MsGraphicsScene::updateTileHitbox()
{
    _tileHitbox->setVisible(_layerSettings->showTileHitbox() & _frame->solid);

    _tileHitbox->setRect(_frame->tileHitbox);
}

template <class T>
void MsGraphicsScene::updateItemIndexes(QList<T*>& list, unsigned start,
                                        unsigned baseZValue,
                                        const SelectedItem::Type& type)
{
    for (unsigned i = start; int(i) < list.size(); i++) {
        list.at(i)->setZValue(baseZValue - i);

        SelectedItem si = { type, i };
        list.at(i)->setData(SELECTION_ID, QVariant::fromValue(si));
    }
}

void MsGraphicsScene::addFrameObject(unsigned index)
{
    auto* item = new PixmapGraphicsItem();
    _objects.insert(index, item);
    updateItemIndexes(_objects, index,
                      FRAME_OBJECT_ZVALUE, SelectedItem::FRAME_OBJECT);

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

void MsGraphicsScene::removeFrameObject(unsigned index)
{
    auto* item = _objects.takeAt(index);
    delete item;
    updateItemIndexes(_objects, index,
                      FRAME_OBJECT_ZVALUE, SelectedItem::FRAME_OBJECT);
}

void MsGraphicsScene::addActionPoint(unsigned index)
{
    auto* item = new AabbGraphicsItem();
    _actionPoints.insert(index, item);
    updateItemIndexes(_actionPoints, index,
                      ACTION_POINT_ZVALUE, SelectedItem::ACTION_POINT);

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

void MsGraphicsScene::removeActionPoint(unsigned index)
{
    auto* item = _actionPoints.takeAt(index);
    delete item;
    updateItemIndexes(_actionPoints, index,
                      ACTION_POINT_ZVALUE, SelectedItem::ACTION_POINT);
}

void MsGraphicsScene::addEntityHitbox(unsigned index)
{
    auto* item = new ResizableAabbGraphicsItem();
    _entityHitboxes.insert(index, item);
    updateItemIndexes(_entityHitboxes, index,
                      ENTITY_HITBOX_ZVALUE, SelectedItem::ENTITY_HITBOX);

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

    _tileHitbox->setSelected(
        std::any_of(sel.begin(), sel.end(),
                    [](auto& s) { return s.type == SelectedItem::TILE_HITBOX; }));

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

void MsGraphicsScene::onTilesetPixmapsChanged()
{
    for (int i = 0; i < _objects.size(); i++) {
        updateFrameObject(i);
    }
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
