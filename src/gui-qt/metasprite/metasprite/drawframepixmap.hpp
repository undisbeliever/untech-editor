/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/snes/tile.hpp"
#include "models/metasprite/metasprite.h"
#include <QPixmap>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

inline QPair<QPixmap, QPoint> drawFramePixmap(const MS::FrameSet& fs, const MS::Frame& frame, unsigned paletteIndex, const QSize minSize)
{
    using ObjectSize = UnTech::MetaSprite::ObjectSize;
    static const UnTech::Snes::Palette4bpp BLANK_PALETTE;

    const UnTech::Snes::Palette4bpp& palette = paletteIndex < fs.palettes.size() ? fs.palettes.at(paletteIndex) : BLANK_PALETTE;

    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = INT_MIN;
    int maxY = INT_MIN;

    if (frame.objects.empty()) {
        return qMakePair(QPixmap(), QPoint());
    }

    for (auto& fo : frame.objects) {
        minX = qMin(minX, int(fo.location.x));
        maxX = qMax(maxX, int(fo.location.x) + int(fo.sizePx()));
        minY = qMin(minY, int(fo.location.y));
        maxY = qMax(maxY, int(fo.location.y) + int(fo.sizePx()));
    }

    Q_ASSERT(minX >= INT8_MIN && maxX < UINT8_MAX);
    Q_ASSERT(minY >= INT8_MIN && maxY < UINT8_MAX);

    const int frameWidth = maxX - minX + 1;
    const int frameHeight = maxY - minY + 1;
    Q_ASSERT(frameWidth >= 8);
    Q_ASSERT(frameHeight >= 8);

    const QSize imgSize{ qMax(frameWidth, minSize.width()), qMax(frameHeight, minSize.height()) };

    auto originAxisPoint = [](const int min, const int max, const int frameSize, const int imgSize) {
        int v = -min;
        if (imgSize != frameSize) {
            v = imgSize / 2;
            if (-min > v || max >= imgSize) {
                v = -min + (imgSize - frameSize) / 2;
            }
        }
        return v;
    };
    const QPoint origin{ originAxisPoint(minX, maxX, frameWidth, imgSize.width()),
                         originAxisPoint(minY, maxY, frameHeight, imgSize.height()) };

    QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    for (auto& fo : frame.objects) {
        if (fo.size == ObjectSize::SMALL) {
            if (fo.tileId < fs.smallTileset.size()) {
                Snes::drawTransparentTile(img, fs.smallTileset.tile(fo.tileId), palette, fo.location.x + origin.x(), fo.location.y + origin.y(), fo.hFlip, fo.vFlip);
            }
        }
        else {
            if (fo.tileId < fs.largeTileset.size()) {
                Snes::drawTransparentTile(img, fs.largeTileset.tile(fo.tileId), palette, fo.location.x + origin.x(), fo.location.y + origin.y(), fo.hFlip, fo.vFlip);
            }
        }
    }

    return { QPixmap::fromImage(img), origin };
}

}
}
}
}
