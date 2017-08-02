/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sigraphicsscene.h"
#include "document.h"
#include "framecommands.h"
#include "framecontentcommands.h"
#include "siframegraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiGraphicsScene::SiGraphicsScene(Actions* actions, LayerSettings* layerSettings,
                                 QWidget* parent)
    : QGraphicsScene(parent)
    , _actions(actions)
    , _layerSettings(layerSettings)
    , _document(nullptr)
    , _inUpdateSelection(false)
{
    Q_ASSERT(_actions != nullptr);
    Q_ASSERT(_layerSettings != nullptr);

    _style = new Style(parent);

    _frameSetPixmap = new QGraphicsPixmapItem();
    _frameSetPixmap->setTransformationMode(Qt::FastTransformation);
    addItem(_frameSetPixmap);

    onLayerSettingsChanged();

    connect(this, &SiGraphicsScene::selectionChanged,
            this, &SiGraphicsScene::onSceneSelectionChanged);

    connect(_layerSettings, &LayerSettings::layerSettingsChanged,
            this, &SiGraphicsScene::onLayerSettingsChanged);
}

void SiGraphicsScene::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    buildFrameItems();

    if (_document) {
        updateFrameSetPixmap();
        onSelectedFrameChanged();
        updateSelection();

        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &SiGraphicsScene::onSelectedFrameChanged);
        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &SiGraphicsScene::updateSelection);

        connect(_document, &Document::frameSetImageChanged,
                this, &SiGraphicsScene::updateFrameSetPixmap);
        connect(_document, &Document::frameSetGridChanged,
                this, &SiGraphicsScene::onFrameSetGridChanged);

        connect(_document, &Document::frameLocationChanged,
                this, &SiGraphicsScene::onFrameLocationChanged);
        connect(_document, &Document::frameTileHitboxChanged,
                this, &SiGraphicsScene::onFrameTileHitboxChanged);

        connect(_document, &Document::frameObjectChanged,
                this, &SiGraphicsScene::onFrameObjectChanged);
        connect(_document, &Document::actionPointChanged,
                this, &SiGraphicsScene::onActionPointChanged);
        connect(_document, &Document::entityHitboxChanged,
                this, &SiGraphicsScene::onEntityHitboxChanged);

        connect(_document, &Document::frameObjectAboutToBeRemoved,
                this, &SiGraphicsScene::onFrameObjectAboutToBeRemoved);
        connect(_document, &Document::actionPointAboutToBeRemoved,
                this, &SiGraphicsScene::onActionPointAboutToBeRemoved);
        connect(_document, &Document::entityHitboxAboutToBeRemoved,
                this, &SiGraphicsScene::onEntityHitboxAboutToBeRemoved);

        connect(_document, &Document::frameObjectAdded,
                this, &SiGraphicsScene::onFrameObjectAdded);
        connect(_document, &Document::actionPointAdded,
                this, &SiGraphicsScene::onActionPointAdded);
        connect(_document, &Document::entityHitboxAdded,
                this, &SiGraphicsScene::onEntityHitboxAdded);

        connect(_document, &Document::frameContentsMoved,
                this, &SiGraphicsScene::onFrameContentsMoved);
    }
    else {
        _frameSetPixmap->setPixmap(QPixmap());
    }
}

void SiGraphicsScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (_document == nullptr) {
        return;
    }

    const SI::Frame* frame = _document->selection()->selectedFrame();
    if (frame != nullptr) {
        const urect& fLoc = frame->location.aabb;

        painter->save();

        QPainterPath outer;
        outer.addRect(rect);

        QPainterPath inner;
        inner.addRect(fLoc.x, fLoc.y, fLoc.width, fLoc.height);

        QPainterPath path = outer.subtracted(inner);

        painter->setPen(Qt::NoPen);
        painter->setBrush(_style->antiHighlightBrush());

        painter->drawPath(path);

        painter->restore();
    }
}

void SiGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);

    if (_document && event->button() == Qt::LeftButton) {
        commitMovedItems();

        // Set selected frame to the one under the mouse.
        // This is done manually as making SiFrameGraphicsItem selectable
        // complicates the selection model.

        auto frameItemAt = [this](const QPointF pos) -> SiFrameGraphicsItem* {
            for (QGraphicsItem* item : this->items(pos)) {
                auto* frameItem = dynamic_cast<SiFrameGraphicsItem*>(item);
                if (frameItem) {
                    return frameItem;
                }
            }
            return nullptr;
        };
        auto* press = frameItemAt(event->buttonDownScenePos(Qt::LeftButton));
        auto* release = frameItemAt(event->scenePos());

        if (press == release) {
            if (press) {
                _document->selection()->selectFrame(press->frame());
            }
            else {
                _document->selection()->unselectFrame();
            }
        }
    }
}

void SiGraphicsScene::commitMovedItems()
{
    SI::Frame* frame = _document->selection()->selectedFrame();
    const auto selectedItems = this->selectedItems();

    if (frame == nullptr || selectedItems.isEmpty()) {
        return;
    }

    auto command = std::make_unique<QUndoCommand>(tr("Move"));

    for (QGraphicsItem* item : selectedItems) {
        QVariant v = item->data(SiFrameGraphicsItem::SELECTION_ID);
        if (v.isValid()) {
            SelectedItem id = v.value<SelectedItem>();
            switch (id.type) {
            case SelectedItem::NONE:
                break;

            case SelectedItem::FRAME_OBJECT:
                if (auto* i = dynamic_cast<const AabbGraphicsItem*>(item)) {
                    SI::FrameObject obj = frame->objects.at(id.index);

                    upoint location = i->posUpoint();
                    if (obj.location != location) {
                        obj.location = location;
                        new ChangeFrameObject(_document, frame, id.index, obj,
                                              command.get());
                    }
                }
                break;

            case SelectedItem::ACTION_POINT:
                if (auto* i = dynamic_cast<const AabbGraphicsItem*>(item)) {
                    SI::ActionPoint ap = frame->actionPoints.at(id.index);

                    upoint location = i->posUpoint();
                    if (ap.location != location) {
                        ap.location = location;
                        new ChangeActionPoint(_document, frame, id.index, ap,
                                              command.get());
                    }
                }
                break;

            case SelectedItem::ENTITY_HITBOX:
                if (auto* i = dynamic_cast<const ResizableAabbGraphicsItem*>(item)) {
                    SI::EntityHitbox eh = frame->entityHitboxes.at(id.index);

                    urect aabb = i->rectUrect();
                    if (eh.aabb != aabb) {
                        eh.aabb = aabb;
                        new ChangeEntityHitbox(_document, frame, id.index, eh,
                                               command.get());
                    }
                }
                break;

            case SelectedItem::TILE_HITBOX:
                if (auto* i = dynamic_cast<const ResizableAabbGraphicsItem*>(item)) {
                    urect hitbox = i->rectUrect();
                    if (frame->tileHitbox != hitbox) {
                        new ChangeFrameTileHitbox(_document, frame, hitbox,
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

void SiGraphicsScene::onSelectedFrameChanged()
{
    const SI::Frame* frame = _document->selection()->selectedFrame();

    for (SiFrameGraphicsItem* item : _frameItems) {
        bool s = item->frame() == frame;
        item->setFrameSelected(s);

        // Selected frame is always on top
        item->setZValue(s ? 10 : 1);
    }

    update();
}

void SiGraphicsScene::updateSelection()
{
    if (_document == nullptr) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    const SI::Frame* frame = _document->selection()->selectedFrame();
    const auto& sel = _document->selection()->selectedItems();

    auto it = _frameItems.find(frame);
    if (it != _frameItems.end()) {
        it.value()->updateSelection(sel);
    }

    _inUpdateSelection = false;
}

void SiGraphicsScene::onSceneSelectionChanged()
{
    if (_document == nullptr || _inUpdateSelection) {
        return;
    }

    // Only the selected frame has selectable QGraphicsItems
    // No need to worry about items from different frames being selected

    std::set<SelectedItem> selection;

    for (const QGraphicsItem* item : selectedItems()) {
        QVariant v = item->data(SiFrameGraphicsItem::SELECTION_ID);
        if (v.isValid()) {
            selection.insert(v.value<SelectedItem>());
        }
    }

    _document->selection()->setSelectedItems(selection);
}

void SiGraphicsScene::updateFrameSetPixmap()
{
    if (_document && _document->frameSet()->isImageValid()) {
        const std::string& fn = _document->frameSet()->imageFilename;

        QPixmap p(QString::fromStdString(fn), "PNG");
        _frameSetPixmap->setPixmap(p);
    }
    else {
        _frameSetPixmap->setPixmap(QPixmap());
    }

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::buildFrameItems()
{
    for (SiFrameGraphicsItem* frameItem : _frameItems) {
        removeItem(frameItem);
        delete frameItem;
    }
    _frameItems.clear();

    if (_document) {
        for (auto it : _document->frameSet()->frames) {
            SI::Frame* frame = &it.second;

            auto* frameItem = new SiFrameGraphicsItem(frame, _actions, _style);
            _frameItems.insert(frame, frameItem);
            addItem(frameItem);
        }
    }

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::onLayerSettingsChanged()
{
    for (SiFrameGraphicsItem* item : _frameItems) {
        item->updateLayerSettings(_layerSettings);
    }
}

void SiGraphicsScene::onFrameSetGridChanged()
{
    for (SiFrameGraphicsItem* frameItem : _frameItems) {
        frameItem->updateFrameLocation();
    }

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::onFrameLocationChanged(const void* framePtr)
{
    auto it = _frameItems.find(framePtr);
    if (it != _frameItems.end()) {
        it.value()->updateFrameLocation();
    }

    setSceneRect(itemsBoundingRect());
    update();
}

void SiGraphicsScene::onFrameTileHitboxChanged(const void* framePtr)
{
    auto it = _frameItems.find(framePtr);
    if (it != _frameItems.end()) {
        it.value()->updateTileHitbox();
    }
}

#define FRAME_CHILDREN_SLOTS(CLS)                    \
    void SiGraphicsScene::on##CLS##Changed(          \
        const void* framePtr, unsigned index)        \
    {                                                \
        auto it = _frameItems.find(framePtr);        \
        if (it != _frameItems.end()) {               \
            it.value()->update##CLS(index);          \
        }                                            \
    }                                                \
                                                     \
    void SiGraphicsScene::on##CLS##AboutToBeRemoved( \
        const void* framePtr, unsigned index)        \
    {                                                \
        auto it = _frameItems.find(framePtr);        \
        if (it != _frameItems.end()) {               \
            it.value()->remove##CLS(index);          \
        }                                            \
    }                                                \
    void SiGraphicsScene::on##CLS##Added(            \
        const void* framePtr, unsigned index)        \
    {                                                \
        auto it = _frameItems.find(framePtr);        \
        if (it != _frameItems.end()) {               \
            it.value()->add##CLS(index);             \
        }                                            \
    }
FRAME_CHILDREN_SLOTS(FrameObject)
FRAME_CHILDREN_SLOTS(ActionPoint)
FRAME_CHILDREN_SLOTS(EntityHitbox)

void SiGraphicsScene::onFrameContentsMoved(
    const void* framePtr, const std::set<SelectedItem>&, int)
{
    auto it = _frameItems.find(framePtr);
    if (it != _frameItems.end()) {
        it.value()->updateFrameContents();
    }
}
