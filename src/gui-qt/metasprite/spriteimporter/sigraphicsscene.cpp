/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sigraphicsscene.h"
#include "accessors.h"
#include "resourceitem.h"
#include "siframegraphicsitem.h"
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
    , _resourceItem(nullptr)
    , _inUpdateSelection(false)
    , _inOnSceneSelectionChanged(false)
{
    Q_ASSERT(_frameContextMenu != nullptr);
    Q_ASSERT(_layerSettings != nullptr);

    _frameSetPixmap->setTransformationMode(Qt::FastTransformation);
    _frameSetPixmap->setZValue(PIXMAP_ZVALUE);
    addItem(_frameSetPixmap);

    _paletteOutline->setToolTip(tr("User Supplied Palette"));
    _paletteOutline->setPen(_style->paletteOutlinePen());
    _paletteOutline->setVisible(false);
    _paletteOutline->setZValue(PALETTE_ZVALUE);
    addItem(_paletteOutline);

    connect(this, &SiGraphicsScene::selectionChanged,
            this, &SiGraphicsScene::onSceneSelectionChanged);

    connect(_layerSettings, &LayerSettings::layerSettingsChanged,
            this, &SiGraphicsScene::onLayerSettingsChanged);
}

void SiGraphicsScene::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->disconnect(this);
        _resourceItem->frameList()->disconnect(this);
        _resourceItem->frameObjectList()->disconnect(this);
        _resourceItem->actionPointList()->disconnect(this);
        _resourceItem->entityHitboxList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    // reset scene
    removeAllFrameItems();
    _frameSetPixmap->setPixmap(QPixmap());
    _paletteOutline->setPath(QPainterPath());
    _paletteOutline->setVisible(false);
    setSceneRect(QRect(0, 0, 0, 0));

    if (_resourceItem) {
        updateFrameSetPixmap();
        updatePaletteOutline();
        buildFrameItems();
        onSelectedFrameChanged();

        updateFrameObjectSelection();
        updateActionPointSelection();
        updateEntityHitboxSelection();
        updateTileHitboxSelection();

        connect(_resourceItem->frameList(), &FrameList::selectedIndexChanged,
                this, &SiGraphicsScene::onSelectedFrameChanged);

        connect(_resourceItem, &ResourceItem::externalFilesModified,
                this, &SiGraphicsScene::updateFrameSetPixmap);
        connect(_resourceItem, &ResourceItem::frameSetPaletteChanged,
                this, &SiGraphicsScene::updatePaletteOutline);
        connect(_resourceItem, &ResourceItem::frameSetGridChanged,
                this, &SiGraphicsScene::onFrameSetGridChanged);

        connect(_resourceItem->frameList(), &FrameList::itemAdded,
                this, &SiGraphicsScene::onFrameAdded);
        connect(_resourceItem->frameList(), &FrameList::itemAboutToBeRemoved,
                this, &SiGraphicsScene::onFrameAboutToBeRemoved);

        connect(_resourceItem->frameList(), &FrameList::frameLocationChanged,
                this, &SiGraphicsScene::onFrameLocationChanged);
        connect(_resourceItem->frameList(), &FrameList::dataChanged,
                this, &SiGraphicsScene::onFrameDataChanged);

        connect(_resourceItem->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &SiGraphicsScene::updateFrameObjectSelection);
        connect(_resourceItem->actionPointList(), &ActionPointList::selectedIndexesChanged,
                this, &SiGraphicsScene::updateActionPointSelection);
        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &SiGraphicsScene::updateEntityHitboxSelection);

        connect(_resourceItem->frameList(), &FrameList::tileHitboxSelectedChanged,
                this, &SiGraphicsScene::updateTileHitboxSelection);

        connect(_resourceItem->frameObjectList(), &FrameObjectList::dataChanged,
                this, &SiGraphicsScene::onFrameObjectChanged);
        connect(_resourceItem->actionPointList(), &ActionPointList::dataChanged,
                this, &SiGraphicsScene::onActionPointChanged);
        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::dataChanged,
                this, &SiGraphicsScene::onEntityHitboxChanged);

        // This class uses listChanged signal as it is more efficient when multiple items change

        connect(_resourceItem->frameObjectList(), &FrameObjectList::listChanged,
                this, &SiGraphicsScene::onFrameObjectListChanged);
        connect(_resourceItem->actionPointList(), &ActionPointList::listChanged,
                this, &SiGraphicsScene::onActionPointListChanged);
        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::listChanged,
                this, &SiGraphicsScene::onEntityHitboxListChanged);
    }
}

void SiGraphicsScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (_resourceItem == nullptr) {
        return;
    }

    const SI::Frame* frame = _resourceItem->frameList()->selectedItem();
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

    if (_resourceItem && event->button() == Qt::LeftButton) {
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
                size_t frameIndex = _frameItems.indexOf(press);
                _resourceItem->frameList()->setSelectedIndex(frameIndex);
            }
            else {
                _resourceItem->frameList()->unselectItem();
            }
        }
    }
}
void SiGraphicsScene::onSelectedFrameChanged()
{
    size_t frameIndex = _resourceItem->frameList()->selectedIndex();

    for (int i = 0; i < _frameItems.size(); i++) {
        SiFrameGraphicsItem* item = _frameItems.at(i);

        bool s = frameIndex == unsigned(i);
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
    if (_resourceItem == nullptr || _inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    const size_t frameIndex = _resourceItem->frameList()->selectedIndex();
    SiFrameGraphicsItem* frameItem = _frameItems.value(frameIndex);

    if (frameItem) {
        auto f = std::mem_fn(method);
        f(frameItem, selectedIndexes);
    }

    _inUpdateSelection = false;
}

void SiGraphicsScene::updateFrameObjectSelection()
{
    updateSelection(&SiFrameGraphicsItem::updateFrameObjectSelection,
                    _resourceItem->frameObjectList()->selectedIndexes());
}

void SiGraphicsScene::updateActionPointSelection()
{
    updateSelection(&SiFrameGraphicsItem::updateActionPointSelection,
                    _resourceItem->actionPointList()->selectedIndexes());
}

void SiGraphicsScene::updateEntityHitboxSelection()
{
    updateSelection(&SiFrameGraphicsItem::updateEntityHitboxSelection,
                    _resourceItem->entityHitboxList()->selectedIndexes());
}

void SiGraphicsScene::updateTileHitboxSelection()
{
    if (_inOnSceneSelectionChanged) {
        return;
    }

    Q_ASSERT(_inUpdateSelection == false);
    _inUpdateSelection = true;

    const size_t frameIndex = _resourceItem->frameList()->selectedIndex();
    bool hbSelected = _resourceItem->frameList()->isTileHitboxSelected();

    SiFrameGraphicsItem* frameItem = _frameItems.value(frameIndex);
    if (frameItem) {
        frameItem->updateTileHitboxSelected(hbSelected);
    }

    _inUpdateSelection = false;
}

void SiGraphicsScene::onSceneSelectionChanged()
{
    if (_inUpdateSelection || _resourceItem == nullptr) {
        return;
    }

    const size_t frameIndex = _resourceItem->frameList()->selectedIndex();
    const SiFrameGraphicsItem* frameItem = _frameItems.value(frameIndex);

    if (frameItem == nullptr) {
        return;
    }

    Q_ASSERT(_inOnSceneSelectionChanged == false);
    _inOnSceneSelectionChanged = true;

    // Only the selected frame has selectable QGraphicsItems
    // No need to worry about items from different frames being selected

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

    _resourceItem->frameObjectList()->setSelectedIndexes(getSelected(frameItem->objects()));
    _resourceItem->actionPointList()->setSelectedIndexes(getSelected(frameItem->actionPoints()));
    _resourceItem->entityHitboxList()->setSelectedIndexes(getSelected(frameItem->entityHitboxes()));

    _resourceItem->frameList()->setTileHitboxSelected(frameItem->tileHitbox()->isSelected());

    if (contentSelected) {
        emit frameContentSelected();
    }

    _inOnSceneSelectionChanged = false;
}

void SiGraphicsScene::updateFrameSetPixmap()
{
    if (_resourceItem && _resourceItem->frameSet()->imageFilename.empty() == false) {
        const std::string& fn = _resourceItem->frameSet()->imageFilename;

        QPixmap p(QString::fromStdString(fn), "PNG");
        _frameSetPixmap->setPixmap(p);
    }
    else {
        _frameSetPixmap->setPixmap(QPixmap());
    }

    updatePaletteOutline();

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::updatePaletteOutline()
{
    using Position = SI::UserSuppliedPalette::Position;

    const SI::FrameSet* frameSet = _resourceItem->frameSet();
    const auto& palette = frameSet->palette;

    if (palette.usesUserSuppliedPalette()) {
        const unsigned colorSize = palette.colorSize;
        const usize paletteSize = palette.paletteSize();
        const QSize imageSize = _frameSetPixmap->pixmap().size();

        QPainterPath path;
        path.addRect(0, 0, paletteSize.width, paletteSize.height);
        for (unsigned i = 0; i < palette.nPalettes; i++) {
            path.moveTo(0, i * colorSize);
            path.lineTo(paletteSize.width, i * colorSize);
        }

        _paletteOutline->setPath(path);

        switch (palette.position) {
        case Position::TOP_LEFT:
            _paletteOutline->setPos(0, 0);
            break;

        case Position::TOP_RIGHT:
            _paletteOutline->setPos(imageSize.width() - paletteSize.width, 0);
            break;

        case Position::BOTTOM_LEFT:
            _paletteOutline->setPos(0, imageSize.height() - paletteSize.height);
            break;

        case Position::BOTTOM_RIGHT:
            _paletteOutline->setPos(imageSize.width() - paletteSize.width, imageSize.height() - paletteSize.height);
            break;
        }

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
        // cannot call `this->removeItem(frameItem);` here (despite the Qt docs
        // recomending it) as it causes a use after free segfault.
        delete frameItem;
    }
    _frameItems.clear();
}

void SiGraphicsScene::buildFrameItems()
{
    removeAllFrameItems();

    if (_resourceItem) {
        for (const SI::Frame& frame : _resourceItem->frameSet()->frames) {
            auto* frameItem = new SiFrameGraphicsItem(frame, _frameContextMenu.data(), _style);
            _frameItems.append(frameItem);
            frameItem->setVisible(FRAME_ZVALUE);
            addItem(frameItem);
        }
    }

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::onLayerSettingsChanged()
{
    Q_ASSERT(_resourceItem);
    Q_ASSERT(_resourceItem->frameSet());
    const auto& frames = _resourceItem->frameSet()->frames;

    for (int i = 0; i < _frameItems.size(); i++) {
        auto* frameItem = _frameItems.at(i);
        auto& frame = frames.at(i);
        frameItem->updateLayerSettings(frame, _layerSettings);
    }
}

void SiGraphicsScene::onFrameSetGridChanged()
{
    Q_ASSERT(_resourceItem);
    Q_ASSERT(_resourceItem->frameSet());
    const auto& frames = _resourceItem->frameSet()->frames;

    for (int i = 0; i < _frameItems.size(); i++) {
        auto* frameItem = _frameItems.at(i);
        auto& frame = frames.at(i);
        frameItem->updateFrameLocation(frame);
    }

    setSceneRect(itemsBoundingRect());
}

void SiGraphicsScene::onFrameAdded(size_t frameIndex)
{
    Q_ASSERT(_resourceItem);
    Q_ASSERT(_resourceItem->frameSet());
    SI::Frame& frame = _resourceItem->frameSet()->frames.at(frameIndex);

    auto* frameItem = new SiFrameGraphicsItem(frame, _frameContextMenu.data(), _style);
    _frameItems.insert(frameIndex, frameItem);
    frameItem->setVisible(FRAME_ZVALUE);
    addItem(frameItem);
}

void SiGraphicsScene::onFrameAboutToBeRemoved(size_t frameIndex)
{
    Q_ASSERT(frameIndex < unsigned(_frameItems.size()));

    SiFrameGraphicsItem* frameItem = _frameItems.takeAt(frameIndex);

    // cannot call `this->removeItem(frameItem);` here (despite the Qt docs
    // recomending it) as it causes a use after free segfault.
    delete frameItem;
}

void SiGraphicsScene::onFrameLocationChanged(size_t frameIndex)
{
    Q_ASSERT(_resourceItem);
    Q_ASSERT(_resourceItem->frameSet());
    SI::Frame& frame = _resourceItem->frameSet()->frames.at(frameIndex);

    _frameItems.at(frameIndex)->updateFrameLocation(frame);

    setSceneRect(itemsBoundingRect());
    update();
}

void SiGraphicsScene::onFrameDataChanged(size_t frameIndex)
{
    Q_ASSERT(_resourceItem);
    Q_ASSERT(_resourceItem->frameSet());
    SI::Frame& frame = _resourceItem->frameSet()->frames.at(frameIndex);

    _frameItems.at(frameIndex)->onFrameDataChanged(frame);
}

#define FRAME_CHILDREN_SLOTS(CLS, FRAME_LIST)                                       \
    void SiGraphicsScene::on##CLS##Changed(                                         \
        size_t frameIndex, size_t index)                                            \
    {                                                                               \
        auto* frames = _resourceItem->frameList()->list();                          \
        Q_ASSERT(frames);                                                           \
        const SI::Frame& frame = frames->at(frameIndex);                            \
        _frameItems.at(frameIndex)->update##CLS(index, frame.FRAME_LIST.at(index)); \
    }                                                                               \
                                                                                    \
    void SiGraphicsScene::on##CLS##ListChanged(                                     \
        size_t frameIndex)                                                          \
    {                                                                               \
        auto* frames = _resourceItem->frameList()->list();                          \
        Q_ASSERT(frames);                                                           \
        const SI::Frame& frame = frames->at(frameIndex);                            \
        _frameItems.at(frameIndex)->on##CLS##ListChanged(frame);                    \
    }
FRAME_CHILDREN_SLOTS(FrameObject, objects)
FRAME_CHILDREN_SLOTS(ActionPoint, actionPoints)
FRAME_CHILDREN_SLOTS(EntityHitbox, entityHitboxes)
