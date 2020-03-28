/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sianimationpreviewitem.h"
#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"
#include "models/common/imagecache.h"

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
    const AbstractMsResourceItem* abstractResourceItem)
{
    auto* resourceItem = dynamic_cast<const ResourceItem*>(abstractResourceItem);
    if (resourceItem) {
        return new SiAnimationPreviewItem(_layerSettings, _style, resourceItem);
    }
    else {
        return nullptr;
    }
}

SiAnimationPreviewItem::SiAnimationPreviewItem(LayerSettings* layerSettings,
                                               Style* style,
                                               const ResourceItem* resourceItem)
    : AnimationPreviewItem(resourceItem)
    , _layerSettings(layerSettings)
    , _style(style)
    , _resourceItem(resourceItem)
    , _frameObjects(IMAGE_SIZE, IMAGE_SIZE, QImage::Format_ARGB32_Premultiplied)
    , _frameObjectsDirty(true)
{
    Q_ASSERT(layerSettings != nullptr);
    Q_ASSERT(style != nullptr);
    Q_ASSERT(resourceItem != nullptr);

    connect(_resourceItem->frameObjectList(), &FrameObjectList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameObjectsChanged);
    connect(_resourceItem->frameObjectList(), &FrameObjectList::listChanged,
            this, &SiAnimationPreviewItem::onFrameObjectsChanged);

    // Required connections to AnimationPreviewItem slots

    connect(_resourceItem->frameList(), &FrameList::itemAdded,
            this, &SiAnimationPreviewItem::onFrameAdded);
    connect(_resourceItem->frameList(), &FrameList::itemAboutToBeRemoved,
            this, &SiAnimationPreviewItem::onFrameAboutToBeRemoved);

    connect(_resourceItem->frameList(), &FrameList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);

    connect(_resourceItem->frameObjectList(), &FrameObjectList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->frameObjectList(), &FrameObjectList::listChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->actionPointList(), &ActionPointList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->actionPointList(), &ActionPointList::listChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->entityHitboxList(), &EntityHitboxList::dataChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->entityHitboxList(), &EntityHitboxList::listChanged,
            this, &SiAnimationPreviewItem::onFrameDataAndContentsChanged);
}

void SiAnimationPreviewItem::onFrameObjectsChanged(size_t frameIndex)
{
    if (frameIndex == this->frameIndex()) {
        _frameObjectsDirty = true;
    }
}

size_t SiAnimationPreviewItem::getFrameIndex(const idstring& frameName)
{
    return _resourceItem->frameSet()->frames.indexOf(frameName);
}

void SiAnimationPreviewItem::drawFrame(QPainter* painter)
{
    const auto& frames = _resourceItem->frameSet()->frames;
    if (frameIndex() >= frames.size()) {
        return;
    }
    const auto& frame = frames.at(frameIndex());

    if (_frameObjectsDirty) {
        drawFrameObjects(frame);
    }

    const upoint& origin = frame.location.origin;
    painter->translate(-int(origin.x), -int(origin.y));

    if (_layerSettings->showFrameObjects()) {
        painter->drawImage(0, 0, _frameObjects);
    }

    if (_layerSettings->showEntityHitboxes()) {
        const auto& hitboxes = frame.entityHitboxes;
        for (auto it = hitboxes.crbegin(); it != hitboxes.crend(); it++) {
            const SI::EntityHitbox& eh = *it;

            painter->setPen(_style->entityHitboxPen(eh.hitboxType));
            painter->setBrush(_style->entityHitboxBrush(eh.hitboxType));

            const auto& r = eh.aabb;
            painter->drawRect(r.x, r.y, r.width, r.height);
        }
    }

    if (_layerSettings->showTileHitbox() && frame.solid) {
        painter->setPen(_style->tileHitboxPen());
        painter->setBrush(_style->tileHitboxBrush());

        const auto& th = frame.tileHitbox;
        painter->drawRect(th.x, th.y, th.width, th.height);
    }

    if (_layerSettings->showActionPoints()) {
        painter->setPen(_style->actionPointPen());
        painter->setBrush(_style->actionPointBrush());

        const auto& points = frame.actionPoints;
        for (auto it = points.crbegin(); it != points.crend(); it++) {
            const SI::ActionPoint& ap = *it;
            painter->drawRect(ap.location.x, ap.location.y, 1, 1);
        }
    }
}

void SiAnimationPreviewItem::drawFrameObjects(const SI::Frame& frame)
{
    _frameObjects.fill(0);

    const SI::FrameSet& frameSet = *_resourceItem->frameSet();
    const auto fsImage = ImageCache::loadPngImage(frameSet.imageFilename);
    Q_ASSERT(fsImage);

    const auto& fsImgSize = fsImage->size();

    for (int i = frame.objects.size() - 1; i >= 0; i--) {
        const SI::FrameObject& obj = frame.objects.at(i);
        const upoint& oLoc = obj.location;
        unsigned tileSize = obj.sizePx();

        if (oLoc.x + tileSize >= IMAGE_SIZE || oLoc.y + tileSize >= IMAGE_SIZE) {
            continue;
        }

        const unsigned tileX = frame.location.aabb.x + oLoc.x;
        const unsigned tileY = frame.location.aabb.y + oLoc.y;

        bool objInsideFsImage = tileX + tileSize <= fsImgSize.width
                                && tileY + tileSize <= fsImgSize.height;

        if (objInsideFsImage) {
            for (unsigned y = 0; y < tileSize; y++) {
                QRgb* imgBits = reinterpret_cast<QRgb*>(_frameObjects.scanLine(oLoc.y + y)) + oLoc.x;

                const rgba* fsBits = fsImage->scanline(tileY + y) + tileX;

                for (unsigned x = 0; x < tileSize; x++) {
                    const rgba& c = fsBits[x];
                    if (c != frameSet.transparentColor) {
                        imgBits[x] = qRgb(c.red, c.green, c.blue);
                    }
                }
            }
        }
        else {
            // object not inside frameSet image, draw checkerpattern instead
            uint8_t p = 0;
            for (unsigned y = 0; y < tileSize; y++) {
                QRgb* imgBits = reinterpret_cast<QRgb*>(_frameObjects.scanLine(oLoc.y + y)) + oLoc.x;

                p ^= 0xff;
                for (unsigned x = 0; x < tileSize; x++) {
                    imgBits[x] = qRgb(p, p, p);
                    p ^= 0xff;
                }
            }
        }
    }
}
