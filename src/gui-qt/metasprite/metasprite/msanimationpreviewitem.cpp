/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "msanimationpreviewitem.h"
#include "document.h"
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

MsAnimationPreviewItem* MsAnimationPreviewItemFactory::createPreviewItem(
    const AbstractMsDocument* abstractDocument)
{
    const Document* document = dynamic_cast<const Document*>(abstractDocument);
    if (document) {
        Q_ASSERT(document == _tilesetPixmaps->document());
        return new MsAnimationPreviewItem(_layerSettings, _style,
                                          _tilesetPixmaps, document);
    }
    else {
        return nullptr;
    }
}

MsAnimationPreviewItem::MsAnimationPreviewItem(LayerSettings* layerSettings,
                                               Style* style,
                                               TilesetPixmaps* tilesetPixmaps,
                                               const Document* document)
    : AnimationPreviewItem(document)
    , _layerSettings(layerSettings)
    , _style(style)
    , _tilesetPixmaps(tilesetPixmaps)
    , _document(document)
    , _frame(nullptr)
{
    Q_ASSERT(layerSettings != nullptr);
    Q_ASSERT(style != nullptr);
    Q_ASSERT(tilesetPixmaps != nullptr);
    Q_ASSERT(document != nullptr);
}

const void* MsAnimationPreviewItem::setFrame(const idstring& frameName)
{
    _frame = _document->frameSet()->frames.getPtr(frameName);
    return _frame;
}

void MsAnimationPreviewItem::drawFrame(QPainter* painter)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    Q_ASSERT(_frame != nullptr);

    if (_layerSettings->showFrameObjects()) {
        for (int i = _frame->objects.size() - 1; i >= 0; i--) {
            const MS::FrameObject& obj = _frame->objects.at(i);

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
        for (int i = _frame->entityHitboxes.size() - 1; i >= 0; i--) {
            const MS::EntityHitbox& eh = _frame->entityHitboxes.at(i);

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

        for (int i = _frame->actionPoints.size() - 1; i >= 0; i--) {
            const MS::ActionPoint& ap = _frame->actionPoints.at(i);
            painter->drawRect(ap.location.x, ap.location.y, 1, 1);
        }
    }
}
