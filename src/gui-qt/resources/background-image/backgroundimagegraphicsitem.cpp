/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "backgroundimagegraphicsitem.h"
#include "gui-qt/project.h"
#include "gui-qt/snes/tile.hpp"
#include "models/project/project-data.h"
#include "models/resources/background-image.h"
#include "models/resources/palette.h"

#include <QPainter>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace BackgroundImage {

namespace RES = UnTech::Resources;

template <bool showTransparent>
static inline QPixmap drawPixmapFromBackgroundImage(
    const RES::BackgroundImageData& backgroundImageData,
    const RES::PaletteData& paletteData, unsigned paletteFrame)
{
    constexpr unsigned TS = decltype(backgroundImageData.tiles)::TILE_SIZE;
    const auto& tileset = backgroundImageData.tiles;
    const auto& tileMap = backgroundImageData.tileMap;

    const auto pixelMask = tileset.pixelMask();
    const unsigned colorsPerPalette = tileset.colorsPerTile();

    paletteFrame %= paletteData.paletteFrames.size();
    const std::vector<UnTech::Snes::SnesColor>& palette = paletteData.paletteFrames.at(paletteFrame);

    QSize imageSize(tileMap.width() * TS, tileMap.height() * TS);

    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    if constexpr (showTransparent) {
        image.fill(0);
    }

    auto mapIt = tileMap.begin();
    for (int yPos = 0; yPos < imageSize.height(); yPos += TS) {
        for (int xPos = 0; xPos < imageSize.width(); xPos += TS) {
            const auto& tm = *mapIt++;

            Snes::drawTile<TS, showTransparent>(image, tileset.tile(tm.character()),
                                                palette, tm.palette() * colorsPerPalette, pixelMask,
                                                xPos, yPos, tm.hFlip(), tm.vFlip());
        }
    }
    assert(mapIt == tileMap.end());

    return QPixmap::fromImage(image);
}
}
}
}
}

using namespace UnTech::GuiQt::Resources::BackgroundImage;

BackgroundImageGraphicsItem::BackgroundImageGraphicsItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , _boundingRect()
    , _pixmaps()
    , _paletteFrame(0)
    , _transparentTiles(true)
{
}

QRectF BackgroundImageGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

void BackgroundImageGraphicsItem::paint(QPainter* painter,
                                        const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_paletteFrame >= unsigned(_pixmaps.size())) {
        // can't use `resetPaletteFrame` as it calls update().
        if (_paletteFrame != 0) {
            _paletteFrame = 0;
            emit paletteFrameChanged();
        }
    }

    if (_pixmaps.isEmpty()) {
        return;
    }

    auto& pixmap = _pixmaps.at(_paletteFrame);

    if (!pixmap.isNull()) {
        painter->drawPixmap(0, 0, pixmap);
    }
}

void BackgroundImageGraphicsItem::resetPaletteFrame()
{
    setPaletteFrame(0);
}

void BackgroundImageGraphicsItem::nextPaletteFrame()
{
    setPaletteFrame(_paletteFrame + 1);
}

void BackgroundImageGraphicsItem::setPaletteFrame(unsigned paletteFrame)
{
    if (paletteFrame >= unsigned(_pixmaps.size())) {
        paletteFrame = 0;
    }

    if (_paletteFrame != paletteFrame) {
        _paletteFrame = paletteFrame;
        emit paletteFrameChanged();
        update();
    }
}

void BackgroundImageGraphicsItem::drawPixmaps(const RES::BackgroundImageData& backgroundImageData,
                                              const RES::PaletteData& paletteData)
{
    prepareGeometryChange();

    _pixmaps.clear();
    _boundingRect = QRect();

    const unsigned nPalettes = paletteData.paletteFrames.size();

    for (unsigned pId = 0; pId < nPalettes; pId++) {
        if (_transparentTiles) {
            _pixmaps.append(drawPixmapFromBackgroundImage<false>(backgroundImageData, paletteData, pId));
        }
        else {
            _pixmaps.append(drawPixmapFromBackgroundImage<true>(backgroundImageData, paletteData, pId));
        }
    }

    if (!_pixmaps.empty()) {
        const QPixmap& firstPixmap = _pixmaps.first();
        Q_ASSERT(!firstPixmap.isNull());
        _boundingRect = QRect(0, 0, firstPixmap.width(), firstPixmap.height());

        resetPaletteFrame();
    }

    update();
}

void BackgroundImageGraphicsItem::removePixmaps()
{
    prepareGeometryChange();

    _pixmaps.clear();
    _boundingRect = QRect();
}
