/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "msanimationpreviewitem.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilesetpixmaps.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/style.h"

#include <QPainter>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

MsAnimationPreviewItemFactory::MsAnimationPreviewItemFactory(LayerSettings* layerSettings,
                                                             TilesetPixmaps* tilesetPixmaps,
                                                             QWidget* parent)
    : QObject(parent)
    , _layerSettings(layerSettings)
    , _tilesetPixmaps(tilesetPixmaps)
    , _style(new Style(parent))
{
    Q_ASSERT(layerSettings != nullptr);
    Q_ASSERT(tilesetPixmaps != nullptr);
}

MsAnimationPreviewItem* MsAnimationPreviewItemFactory::createPreviewItem(const AbstractMsResourceItem* abstractItem)
{
    const auto* resourceItem = dynamic_cast<const ResourceItem*>(abstractItem);
    if (resourceItem) {
        Q_ASSERT(resourceItem == _tilesetPixmaps->resourceItem());
        return new MsAnimationPreviewItem(_layerSettings, _style,
                                          _tilesetPixmaps, resourceItem);
    }
    else {
        return nullptr;
    }
}

MsAnimationPreviewItem::MsAnimationPreviewItem(LayerSettings* layerSettings,
                                               Style* style,
                                               TilesetPixmaps* tilesetPixmaps,
                                               const ResourceItem* resourceItem)
    : AnimationPreviewItem(resourceItem)
    , _layerSettings(layerSettings)
    , _style(style)
    , _tilesetPixmaps(tilesetPixmaps)
    , _resourceItem(resourceItem)
{
    Q_ASSERT(layerSettings != nullptr);
    Q_ASSERT(style != nullptr);
    Q_ASSERT(tilesetPixmaps != nullptr);
    Q_ASSERT(resourceItem != nullptr);

    // Required connections to AnimationPreviewItem slots

    connect(_resourceItem->frameList(), &FrameList::itemAdded,
            this, &MsAnimationPreviewItem::onFrameAdded);
    connect(_resourceItem->frameList(), &FrameList::itemAboutToBeRemoved,
            this, &MsAnimationPreviewItem::onFrameAboutToBeRemoved);

    connect(_resourceItem->frameList(), &FrameList::dataChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);

    connect(_resourceItem->frameObjectList(), &FrameObjectList::dataChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->frameObjectList(), &FrameObjectList::listChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->actionPointList(), &ActionPointList::dataChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->actionPointList(), &ActionPointList::listChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->entityHitboxList(), &EntityHitboxList::dataChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);
    connect(_resourceItem->entityHitboxList(), &EntityHitboxList::listChanged,
            this, &MsAnimationPreviewItem::onFrameDataAndContentsChanged);
}

size_t MsAnimationPreviewItem::getFrameIndex(const idstring& frameName)
{
    return _resourceItem->frameSet()->frames.indexOf(frameName);
}

void MsAnimationPreviewItem::drawFrame(QPainter* painter)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const auto& frames = _resourceItem->frameSet()->frames;
    if (frameIndex() >= frames.size()) {
        return;
    }
    const auto& frame = frames.at(frameIndex());

    if (_layerSettings->showFrameObjects()) {
        for (int i = frame.objects.size() - 1; i >= 0; i--) {
            const MS::FrameObject& obj = frame.objects.at(i);

            painter->save();

            const int halfSize = obj.sizePx() / 2;
            painter->translate(obj.location.x + halfSize, obj.location.y + halfSize);
            painter->scale(obj.hFlip ? -1 : 1,
                           obj.vFlip ? -1 : 1);

            if (obj.size == ObjSize::SMALL) {
                painter->drawPixmap(-halfSize, -halfSize,
                                    _tilesetPixmaps->smallTile(obj.tileId));
            }
            else {
                painter->drawPixmap(-halfSize, -halfSize,
                                    _tilesetPixmaps->largeTile(obj.tileId));
            }

            painter->restore();
        }
    }

    if (_layerSettings->showEntityHitboxes()) {
        const auto& hitboxes = frame.entityHitboxes;
        for (auto it = hitboxes.crbegin(); it != hitboxes.crend(); it++) {
            const MS::EntityHitbox& eh = *it;

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
            const MS::ActionPoint& ap = *it;
            painter->drawRect(ap.location.x, ap.location.y, 1, 1);
        }
    }
}
