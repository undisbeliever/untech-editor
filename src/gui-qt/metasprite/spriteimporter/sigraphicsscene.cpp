/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sigraphicsscene.h"
#include "accessors.h"
#include "document.h"
#include "siframegraphicsitem.h"
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <functional>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiGraphicsScene::SiGraphicsScene(LayerSettings* layerSettings, QWidget* parent)
    : QGraphicsScene(parent)
    , _layerSettings(layerSettings)
    , _frameContextMenu(new QMenu())
    , _style(new Style(parent))
    , _frameSetPixmap(new QGraphicsPixmapItem())
    , _paletteOutline(new QGraphicsPathItem())
    , _document(nullptr)
    , _inUpdateSelection(false)
    , _inOnSceneSelectionChanged(false)
{
    Q_ASSERT(_frameContextMenu != nullptr);
    Q_ASSERT(_layerSettings != nullptr);

    _frameSetPixmap->setTransformationMode(Qt::FastTransformation);
    _frameSetPixmap->setZValue(PIXMAP_ZVALUE);
    addItem(_frameSetPixmap);

    _paletteOutline->setPen(_style->paletteOutlinePen());
    _paletteOutline->setVisible(false);
    _paletteOutline->setZValue(PALETTE_ZVALUE);
    addItem(_paletteOutline);

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
        _document->frameMap()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->actionPointList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    // reset scene
    removeAllFrameItems();
    _frameSetPixmap->setPixmap(QPixmap());
    _paletteOutline->setPath(QPainterPath());
    _paletteOutline->setVisible(false);
    setSceneRect(QRect(0, 0, 0, 0));

    if (_document) {
        updateFrameSetPixmap();
        updatePaletteOutline();
        buildFrameItems();
        onSelectedFrameChanged();

        updateFrameObjectSelection();
        updateActionPointSelection();
        updateEntityHitboxSelection();
        updateTileHitboxSelection();

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &SiGraphicsScene::onSelectedFrameChanged);

        connect(_document, &Document::externalFilesModified,
                this, &SiGraphicsScene::updateFrameSetPixmap);
        connect(_document, &Document::externalFilesModified,
                this, &SiGraphicsScene::updatePaletteOutline);
        connect(_document, &Document::frameSetPaletteChanged,
                this, &SiGraphicsScene::updatePaletteOutline);
        connect(_document, &Document::frameSetGridChanged,
                this, &SiGraphicsScene::onFrameSetGridChanged);

        connect(_document->frameMap(), &FrameMap::itemAdded,
                this, &SiGraphicsScene::onFrameAdded);
        connect(_document->frameMap(), &FrameMap::itemAboutToBeRemoved,
                this, &SiGraphicsScene::onFrameAboutToBeRemoved);

        connect(_document->frameMap(), &FrameMap::frameLocationChanged,
                this, &SiGraphicsScene::onFrameLocationChanged);
        connect(_document->frameMap(), &FrameMap::dataChanged,
                this, &SiGraphicsScene::onFrameDataChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &SiGraphicsScene::updateFrameObjectSelection);
        connect(_document->actionPointList(), &ActionPointList::selectedIndexesChanged,
                this, &SiGraphicsScene::updateActionPointSelection);
        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &SiGraphicsScene::updateEntityHitboxSelection);

        connect(_document->frameMap(), &FrameMap::tileHitboxSelectedChanged,
                this, &SiGraphicsScene::updateTileHitboxSelection);

        connect(_document->frameObjectList(), &FrameObjectList::dataChanged,
                this, &SiGraphicsScene::onFrameObjectChanged);
        connect(_document->actionPointList(), &ActionPointList::dataChanged,
                this, &SiGraphicsScene::onActionPointChanged);
        connect(_document->entityHitboxList(), &EntityHitboxList::dataChanged,
                this, &SiGraphicsScene::onEntityHitboxChanged);

        // This class uses listChanged signal as it is more efficient when multiple items change

        connect(_document->frameObjectList(), &FrameObjectList::listChanged,
                this, &SiGraphicsScene::onFrameObjectListChanged);
        connect(_document->actionPointList(), &ActionPointList::listChanged,
                this, &SiGraphicsScene::onActionPointListChanged);
        connect(_document->entityHitboxList(), &EntityHitboxList::listChanged,
                this, &SiGraphicsScene::onEntityHitboxListChanged);
    }
}

void SiGraphicsScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (_document == nullptr) {
        return;
    }

    const SI::Frame* frame = _document->frameMap()->selectedFrame();
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
                idstring frameId = _document->frameSet()->frames.getId(press->frame());
                _document->frameMap()->setSelectedId(frameId);
            }
            else {
                _document->frameMap()->unselectItem();
            }
        }
    }
}

void SiGraphicsScene::commitMovedItems()
{
    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    if (frame == nullptr) {
        return;
    }

    const SiFrameGraphicsItem* frameItem = _frameItems.value(frame);
    if (frameItem == nullptr) {
        return;
    }

    const auto& objects = frameItem->objects();
    const auto& actionPoints = frameItem->actionPoints();
    const auto& entityHitboxes = frameItem->entityHitboxes();

    QList<QUndoCommand*> commands;
    commands.reserve(4);

    if (_document->frameMap()->isTileHitboxSelected()) {
        urect hitbox = frameItem->tileHitbox()->rectUrect();
        auto* c = FrameMapUndoHelper(_document->frameMap())
                      .editSelectedFieldCommand(hitbox, QString(),
                                                [](SI::Frame& f) -> urect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectListUndoHelper(_document->frameObjectList())
            .editSelectedCommand(
                [&](SI::FrameObject& obj, size_t i) {
                    obj.location = objects.at(i)->posUpoint();
                }));
    commands.append(
        ActionPointListUndoHelper(_document->actionPointList())
            .editSelectedCommand(
                [&](SI::ActionPoint& ap, size_t i) {
                    ap.location = actionPoints.at(i)->posUpoint();
                }));
    commands.append(
        EntityHitboxListUndoHelper(_document->entityHitboxList())
            .editSelectedCommand(
                [&](SI::EntityHitbox& eh, size_t i) {
                    eh.aabb = entityHitboxes.at(i)->rectUrect();
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

void SiGraphicsScene::onSelectedFrameChanged()
{
    const SI::Frame* frame = _document->frameMap()->selectedFrame();

    for (SiFrameGraphicsItem* item : _frameItems) {
        bool s = item->frame() == frame;
        item->setFrameSelected(s);

        // Selected frame is always on top
        item->setZValue(s ? SELECTED_FRAME_ZVALUE : FRAME_ZVALUE);
    }

    update();
}

template <typename F>
void SiGraphicsScene::updateSelection(F method,
                                      const vectorset<size_t>& selectedIndexes)
{
    if (_document == nullptr || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    SiFrameGraphicsItem* frameItem = _frameItems.value(frame);

    if (frameItem != nullptr) {
        auto f = std::mem_fn(method);
        f(frameItem, selectedIndexes);
    }

    _inUpdateSelection = false;
}

void SiGraphicsScene::updateFrameObjectSelection()
{
    updateSelection(&SiFrameGraphicsItem::updateFrameObjectSelection,
                    _document->frameObjectList()->selectedIndexes());
}

void SiGraphicsScene::updateActionPointSelection()
{
    updateSelection(&SiFrameGraphicsItem::updateActionPointSelection,
                    _document->actionPointList()->selectedIndexes());
}

void SiGraphicsScene::updateEntityHitboxSelection()
{
    updateSelection(&SiFrameGraphicsItem::updateEntityHitboxSelection,
                    _document->entityHitboxList()->selectedIndexes());
}

void SiGraphicsScene::updateTileHitboxSelection()
{
    if (_inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    bool hbSelected = _document->frameMap()->isTileHitboxSelected();

    SiFrameGraphicsItem* frameItem = _frameItems.value(frame);
    if (frameItem) {
        frameItem->updateTileHitboxSelected(hbSelected);
    }

    _inUpdateSelection = false;
}

void SiGraphicsScene::onSceneSelectionChanged()
{
    if (_inUpdateSelection) {
        return;
    }

    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    const SiFrameGraphicsItem* frameItem = _frameItems.value(frame);

    if (frameItem == nullptr) {
        return;
    }

    Q_ASSERT(_inOnSceneSelectionChanged == false);
    _inOnSceneSelectionChanged = true;

    // Only the selected frame has selectable QGraphicsItems
    // No need to worry about items from different frames being selected

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

    _document->frameObjectList()->setSelectedIndexes(getSelected(frameItem->objects()));
    _document->actionPointList()->setSelectedIndexes(getSelected(frameItem->actionPoints()));
    _document->entityHitboxList()->setSelectedIndexes(getSelected(frameItem->entityHitboxes()));

    _document->frameMap()->setTileHitboxSelected(frameItem->tileHitbox()->isSelected());

    _inOnSceneSelectionChanged = false;
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

void SiGraphicsScene::updatePaletteOutline()
{
    const SI::FrameSet* frameSet = _document->frameSet();
    const auto& palette = frameSet->palette;

    if (frameSet->image && palette.usesUserSuppliedPalette()) {
        const unsigned colorSize = palette.colorSize;
        const usize paletteSize = palette.paletteSize();
        const usize imageSize = frameSet->image->size();

        QPainterPath path;
        path.addRect(0, 0, paletteSize.width, paletteSize.height);
        for (unsigned i = 0; i < palette.nPalettes; i++) {
            path.moveTo(0, i * colorSize);
            path.lineTo(paletteSize.width, i * colorSize);
        }

        _paletteOutline->setPath(path);
        _paletteOutline->setPos(0, imageSize.height - paletteSize.height);
        _paletteOutline->setVisible(true);
    }
    else {
        _paletteOutline->setPath(QPainterPath());
        _paletteOutline->setVisible(false);
    }

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::removeAllFrameItems()
{
    for (SiFrameGraphicsItem* frameItem : _frameItems) {
        removeItem(frameItem);
        delete frameItem;
    }
    _frameItems.clear();
}

void SiGraphicsScene::buildFrameItems()
{
    removeAllFrameItems();

    if (_document) {
        for (auto it : _document->frameSet()->frames) {
            SI::Frame* frame = &it.second;

            auto* frameItem = new SiFrameGraphicsItem(frame, _frameContextMenu.data(), _style);
            _frameItems.insert(frame, frameItem);
            frameItem->setVisible(FRAME_ZVALUE);
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

void SiGraphicsScene::onFrameAdded(const idstring& id)
{
    SI::Frame* frame = _document->frameSet()->frames.getPtr(id);
    if (frame) {
        auto* frameItem = new SiFrameGraphicsItem(frame, _frameContextMenu.data(), _style);
        _frameItems.insert(frame, frameItem);
        frameItem->setVisible(FRAME_ZVALUE);
        addItem(frameItem);
    }
}

void SiGraphicsScene::onFrameAboutToBeRemoved(const idstring& frameId)
{
    const SI::Frame* frame = _document->frameSet()->frames.getPtr(frameId);

    auto it = _frameItems.find(frame);
    if (it != _frameItems.end()) {
        SiFrameGraphicsItem* frameItem = it.value();
        _frameItems.erase(it);

        removeItem(frameItem);
        delete frameItem;
    }
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

void SiGraphicsScene::onFrameDataChanged(const void* framePtr)
{
    auto it = _frameItems.find(framePtr);
    if (it != _frameItems.end()) {
        it.value()->onFrameDataChanged();
    }
}

#define FRAME_CHILDREN_SLOTS(CLS)               \
    void SiGraphicsScene::on##CLS##Changed(     \
        const void* framePtr, size_t index)     \
    {                                           \
        auto it = _frameItems.find(framePtr);   \
        if (it != _frameItems.end()) {          \
            it.value()->update##CLS(index);     \
        }                                       \
    }                                           \
                                                \
    void SiGraphicsScene::on##CLS##ListChanged( \
        const void* framePtr)                   \
    {                                           \
        auto it = _frameItems.find(framePtr);   \
        if (it != _frameItems.end()) {          \
            it.value()->on##CLS##ListChanged(); \
        }                                       \
    }
FRAME_CHILDREN_SLOTS(FrameObject)
FRAME_CHILDREN_SLOTS(ActionPoint)
FRAME_CHILDREN_SLOTS(EntityHitbox)
