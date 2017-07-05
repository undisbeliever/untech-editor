/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sigraphicsscene.h"
#include "document.h"
#include "siframegraphicsitem.h"
#include "gui-qt/metasprite/style.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiGraphicsScene::SiGraphicsScene(QWidget* parent)
    : QGraphicsScene(parent)
    , _document(nullptr)
{
    _style = new Style(parent);

    _frameSetPixmap = new QGraphicsPixmapItem();
    _frameSetPixmap->setTransformationMode(Qt::FastTransformation);
    addItem(_frameSetPixmap);
}

void SiGraphicsScene::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    buildFrameItems();

    if (_document) {
        updateFrameSetPixmap();

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

            auto* frameItem = new SiFrameGraphicsItem(frame, _style);
            _frameItems.insert(frame, frameItem);
            addItem(frameItem);
        }
    }

    setSceneRect(itemsBoundingRect());
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
