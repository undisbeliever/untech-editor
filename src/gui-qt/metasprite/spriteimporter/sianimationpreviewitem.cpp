/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sianimationpreviewitem.h"
#include "accessors.h"
#include "document.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QPainter>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiAnimationPreviewItemFactory::SiAnimationPreviewItemFactory(LayerSettings* layerSettings,
                                                             QWidget* parent)
    : QObject(parent)
    , _layerSettings(layerSettings)
    , _style(new Style(parent))
{
    Q_ASSERT(layerSettings != nullptr);
}

SiAnimationPreviewItem* SiAnimationPreviewItemFactory::createPreviewItem(
    const AbstractMsDocument* abstractDocument)
{
    const Document* document = dynamic_cast<const Document*>(abstractDocument);
    if (document) {
        return new SiAnimationPreviewItem(_layerSettings, _style, document);
    }
    else {
        return nullptr;
    }
}

SiAnimationPreviewItem::SiAnimationPreviewItem(LayerSettings* layerSettings,
                                               Style* style,
                                               const Document* document)
    : AnimationPreviewItem(document)
    , _layerSettings(layerSettings)
    , _style(style)
    , _document(document)
    , _frame(nullptr)
    , _frameObjects(IMAGE_SIZE, IMAGE_SIZE, QImage::Format_ARGB32_Premultiplied)
    , _frameObjectsDirty(true)
{
    Q_ASSERT(layerSettings != nullptr);
    Q_ASSERT(style != nullptr);
    Q_ASSERT(document != nullptr);

    connect(_document->frameObjectList(), &FrameObjectList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameObjectsChanged);
    connect(_document->frameObjectList(), &FrameObjectList::listChanged,
            this, &SiAnimationPreviewItem::onFrameObjectsChanged);

    // Required connections to AnimationPreviewItem slots

    connect(_document->frameMap(), &FrameMap::itemAdded,
            this, &SiAnimationPreviewItem::onFrameAdded);
    connect(_document->frameMap(), &FrameMap::itemAboutToBeRemoved,
            this, &SiAnimationPreviewItem::onFrameAboutToBeRemoved);

    connect(_document->frameMap(), &FrameMap::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);

    connect(_document->frameObjectList(), &FrameObjectList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document->frameObjectList(), &FrameObjectList::listChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document->actionPointList(), &ActionPointList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document->actionPointList(), &ActionPointList::listChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document->entityHitboxList(), &EntityHitboxList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_document->entityHitboxList(), &EntityHitboxList::listChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
}

void SiAnimationPreviewItem::onFrameObjectsChanged(const void* framePtr)
{
    if (_frame == framePtr) {
        _frameObjectsDirty = true;
    }
}

const void* SiAnimationPreviewItem::setFrame(const idstring& frameName)
{
    if (_document->frameSet()->image == nullptr) {
        return nullptr;
    }

    SI::Frame* frame = _document->frameSet()->frames.getPtr(frameName);
    _frameObjectsDirty = _frame != frame;
    _frame = frame;

    return _frame;
}

void SiAnimationPreviewItem::drawFrame(QPainter* painter)
{
    Q_ASSERT(_frame != nullptr);

    if (_frameObjectsDirty) {
        drawFrameObjects();
    }

    const upoint& origin = _frame->location.origin;
    painter->translate(-int(origin.x), -int(origin.y));

    if (_layerSettings->showFrameObjects()) {
        painter->drawImage(0, 0, _frameObjects);
    }

    if (_layerSettings->showEntityHitboxes()) {
        const auto& hitboxes = _frame->entityHitboxes;
        for (auto it = hitboxes.crbegin(); it != hitboxes.crend(); it++) {
            const SI::EntityHitbox& eh = *it;

            painter->setPen(_style->entityHitboxPen(eh.hitboxType));
            painter->setBrush(_style->entityHitboxBrush(eh.hitboxType));

            const auto& r = eh.aabb;
            painter->drawRect(r.x, r.y, r.width, r.height);
        }
    }

    if (_layerSettings->showTileHitbox() && _frame->solid) {
        painter->setPen(_style->tileHitboxPen());
        painter->setBrush(_style->tileHitboxBrush());

        const auto& th = _frame->tileHitbox;
        painter->drawRect(th.x, th.y, th.width, th.height);
    }

    if (_layerSettings->showActionPoints()) {
        painter->setPen(_style->actionPointPen());
        painter->setBrush(_style->actionPointBrush());

        const auto& points = _frame->actionPoints;
        for (auto it = points.crbegin(); it != points.crend(); it++) {
            const SI::ActionPoint& ap = *it;
            painter->drawRect(ap.location.x, ap.location.y, 1, 1);
        }
    }
}

void SiAnimationPreviewItem::drawFrameObjects()
{
    Q_ASSERT(_frame != nullptr);

    _frameObjects.fill(0);

    const SI::FrameSet& frameSet = *_document->frameSet();
    if (frameSet.image == nullptr) {
        return;
    }

    for (int i = _frame->objects.size() - 1; i >= 0; i--) {
        const SI::FrameObject& obj = _frame->objects.at(i);
        const upoint& oLoc = obj.location;
        unsigned tileSize = obj.sizePx();

        if (oLoc.x + tileSize >= IMAGE_SIZE || oLoc.y + tileSize >= IMAGE_SIZE) {
            continue;
        }

        const unsigned tileX = _frame->location.aabb.x + oLoc.x;
        const unsigned tileY = _frame->location.aabb.y + oLoc.y;

        for (unsigned y = 0; y < tileSize; y++) {
            QRgb* imgBits = reinterpret_cast<QRgb*>(_frameObjects.scanLine(oLoc.y + y)) + oLoc.x;
            const rgba* fsBits = frameSet.image->scanline(tileY + y) + tileX;

            for (unsigned x = 0; x < tileSize; x++) {
                const rgba& c = fsBits[x];
                if (c != frameSet.transparentColor) {
                    imgBits[x] = qRgb(c.red, c.green, c.blue);
                }
            }
        }
    }
}
