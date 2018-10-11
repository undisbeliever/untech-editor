/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/resources/dualanimationtimer.h"
#include "gui-qt/resources/palette/paletteresourceitem.h"
#include "gui-qt/snes/tile.hpp"
#include "models/metatiles/metatile-tileset.h"
#include "models/resources/animation-frames-input.h"

#include <QPainter>
#include <cassert>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtTilesetRenderer::MtTilesetRenderer(QObject* parent)
    : QObject(parent)
    , _animationTimer(new Resources::DualAnimationTimer(this))
    , _paletteItem(nullptr)
    , _tilesetItem(nullptr)
    , _pixmaps()
    , _nPalettes(0)
    , _nTilesets(0)
    , _nMetaTiles(0)
{
    connect(_animationTimer, &Resources::DualAnimationTimer::animationFrameCountChanged,
            this, &MtTilesetRenderer::pixmapChanged);
}

void MtTilesetRenderer::setPlayButton(QAbstractButton* button)
{
    _animationTimer->setPlayButton(button);
}

void MtTilesetRenderer::setRegionCombo(QComboBox* comboBox)
{
    _animationTimer->setRegionCombo(comboBox);
}

void MtTilesetRenderer::resetAnimations()
{
    _animationTimer->resetTimer();
}

void MtTilesetRenderer::pauseAndAdvancePaletteFrame()
{
    _animationTimer->pauseAndIncrementFirstFrameCount();
}

void MtTilesetRenderer::pauseAndAdvanceTilesetFrame()
{
    _animationTimer->pauseAndIncrementSecondFrameCount();
}

void MtTilesetRenderer::setPaletteItem(Resources::PaletteResourceItem* item)
{
    if (_paletteItem != item) {
        if (_paletteItem) {
            _paletteItem->disconnect(this);
        }

        _paletteItem = item;

        resetPixmaps();
        onAnimationDelaysChanged();

        if (_paletteItem) {
            connect(_paletteItem, &AbstractResourceItem::resourceComplied,
                    this, &MtTilesetRenderer::resetPixmaps);

            connect(_paletteItem, &AbstractResourceItem::dataChanged,
                    this, &MtTilesetRenderer::onAnimationDelaysChanged);

            connect(_paletteItem->resourceList(), &AbstractResourceList::resourceItemAboutToBeRemoved,
                    this, &MtTilesetRenderer::onResourceItemAboutToBeRemoved);
        }

        paletteItemChanged();
    }
}

void MtTilesetRenderer::setTilesetItem(MtTilesetResourceItem* item)
{
    if (_tilesetItem != item) {
        if (_tilesetItem) {
            _tilesetItem->disconnect(this);
        }

        _tilesetItem = item;

        _animationTimer->resetTimer();

        resetPixmaps();
        onAnimationDelaysChanged();

        if (_tilesetItem) {
            connect(_tilesetItem, &AbstractResourceItem::resourceComplied,
                    this, &MtTilesetRenderer::resetPixmaps);

            connect(_tilesetItem, &MtTilesetResourceItem::animationDelayChanged,
                    this, &MtTilesetRenderer::onAnimationDelaysChanged);

            connect(_tilesetItem->resourceList(), &AbstractResourceList::resourceItemAboutToBeRemoved,
                    this, &MtTilesetRenderer::onResourceItemAboutToBeRemoved);
        }

        tilesetItemChanged();
    }
}

QColor MtTilesetRenderer::backgroundColor()
{
    if (_nPalettes == 0) {
        return QColor();
    }

    const auto& afc = _animationTimer->animationFrameCount();
    return backgroundColor(afc.first % _nPalettes);
}

const QPixmap& MtTilesetRenderer::pixmap()
{
    if (_nPalettes == 0 || _nTilesets == 0 || _nMetaTiles == 0) {

        static const QPixmap nullPixmap;
        return nullPixmap;
    }

    const auto& afc = _animationTimer->animationFrameCount();
    return pixmap(afc.first % _nPalettes, afc.second % _nTilesets);
}

QColor MtTilesetRenderer::backgroundColor(unsigned paletteId)
{
    if (paletteId >= _nPalettes) {
        return QColor();
    }

    const RES::PaletteData* palData = _paletteItem ? _paletteItem->compiledData() : nullptr;
    Q_ASSERT(palData);

    const auto& bgColor = palData->paletteFrames.at(paletteId).at(0);
    return QColor(bgColor.rgb().red, bgColor.rgb().green, bgColor.rgb().blue);
}

const QPixmap& MtTilesetRenderer::pixmap(unsigned paletteId, unsigned tilesetId)
{
    if (paletteId >= _nPalettes
        || tilesetId >= _nTilesets) {

        static const QPixmap nullPixmap;
        return nullPixmap;
    }

    int i = paletteId * _nTilesets + tilesetId;
    Q_ASSERT(i < _pixmaps.size());

    if (_pixmaps.at(i).isNull()) {
        _pixmaps.replace(i, buildPixmap(paletteId, tilesetId));
    }
    return _pixmaps.at(i);
}

void MtTilesetRenderer::onAnimationDelaysChanged()
{
    unsigned paletteDelay = 0;
    unsigned tilesetDelay = 0;

    if (_paletteItem) {
        if (auto* p = _paletteItem->paletteInput()) {
            paletteDelay = p->animationDelay;
        }
    }

    if (_tilesetItem) {
        if (auto* t = _tilesetItem->data()) {
            tilesetDelay = t->animationFrames.animationDelay;
        }
    }

    _animationTimer->setAnimationDelays(paletteDelay, tilesetDelay);
}

void MtTilesetRenderer::resetPixmaps()
{
    // On Qt5.7+ This will not release the QVector memory
    _pixmaps.clear();

    const RES::PaletteData* palData = _paletteItem ? _paletteItem->compiledData() : nullptr;
    const MT::MetaTileTilesetData* tilesetData = _tilesetItem ? _tilesetItem->compiledData() : nullptr;

    const unsigned old_nMetaTiles = _nMetaTiles;

    if (palData && tilesetData && tilesetData->animatedTileset) {
        _nPalettes = palData->nAnimations();
        _nTilesets = tilesetData->animatedTileset->nAnimatedFrames();
        _nMetaTiles = tilesetData->nMetaTiles();

        Q_ASSERT(_nPalettes > 0 && _nTilesets > 0 && _nMetaTiles > 0);

        _pixmaps.resize(_nPalettes * _nTilesets);
    }
    else {
        _nPalettes = 0;
        _nTilesets = 0;
        _nMetaTiles = 0;
    }

    if (old_nMetaTiles != _nMetaTiles) {
        emit nMetaTilesChanged();
    }

    emit pixmapChanged();
}

void MtTilesetRenderer::onResourceItemAboutToBeRemoved(AbstractResourceItem* item)
{
    if (item == _paletteItem) {
        setPaletteItem(nullptr);
    }
    if (item == _tilesetItem) {
        setTilesetItem(nullptr);
    }
}

static void drawTile(QImage& image,
                     const std::vector<UnTech::Snes::SnesColor> palette,
                     const RES::AnimatedTilesetData& aniTilesetData, unsigned tilesetFrame, const UnTech::Snes::TilemapEntry& tm,
                     int xPos, int yPos)
{
    static const UnTech::Snes::Tile8px BLANK_TILE;
    static const std::vector<UnTech::Snes::SnesColor> BLANK_PALETTE(256);

    auto getTile = [&](unsigned c) {
        const unsigned nStaticTiles = aniTilesetData.staticTiles.size();

        if (c < nStaticTiles) {
            return aniTilesetData.staticTiles.at(c);
        }
        else if (tilesetFrame < aniTilesetData.animatedTiles.size()) {
            const auto& animatedTiles = aniTilesetData.animatedTiles.at(tilesetFrame);

            if (c < nStaticTiles + animatedTiles.size()) {
                return animatedTiles.at(c - nStaticTiles);
            }
        }

        return BLANK_TILE;
    };
    const auto& tile = getTile(tm.character());
    const uint8_t pixelMask = aniTilesetData.staticTiles.pixelMask();
    const unsigned nPaletteColors = 1 << aniTilesetData.staticTiles.bitDepthInt();

    if ((tm.palette() + 1) * nPaletteColors <= palette.size()) {
        const unsigned paletteOffset = tm.palette() * nPaletteColors;
        Snes::drawTransparentTile(image, tile, palette, paletteOffset, pixelMask,
                                  xPos, yPos, tm.hFlip(), tm.vFlip());
    }
    else {
        Snes::drawTransparentTile(image, tile, BLANK_PALETTE, 0, pixelMask,
                                  xPos, yPos, tm.hFlip(), tm.vFlip());
    }
}

QPixmap MtTilesetRenderer::buildPixmap(unsigned paletteFrame, unsigned tilesetFrame)
{
    const RES::PaletteData* palData = _paletteItem ? _paletteItem->compiledData() : nullptr;
    const MT::MetaTileTilesetData* tilesetData = _tilesetItem ? _tilesetItem->compiledData() : nullptr;
    const RES::AnimatedTilesetData* aniTilesetData = tilesetData ? tilesetData->animatedTileset.get() : nullptr;

    if (_nMetaTiles <= 0
        || palData == nullptr || tilesetData == nullptr || aniTilesetData == nullptr
        || paletteFrame >= palData->nAnimations()
        || tilesetFrame >= aniTilesetData->nAnimatedFrames()) {

        return QPixmap();
    }

    const auto& palette = palData->paletteFrames.at(paletteFrame);

    const int height = (_nMetaTiles + PIXMAP_WIDTH - 1) / PIXMAP_WIDTH;
    QImage img(PIXMAP_WIDTH * METATILE_SIZE, height * METATILE_SIZE, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    unsigned tileMapXPos = 0;
    unsigned tileMapYPos = 0;

    for (int tileY = 0; tileY < height; tileY++) {
        for (int tileX = 0; tileX < PIXMAP_WIDTH; tileX++) {
            for (int ty = 0; ty < 2; ty++) {
                for (int tx = 0; tx < 2; tx++) {
                    drawTile(img, palette, *aniTilesetData, tilesetFrame,
                             aniTilesetData->tileMap.at(tileMapXPos + tx, tileMapYPos + ty),
                             tileX * 16 + tx * 8, tileY * 16 + ty * 8);
                }
            }

            // goto next tile
            tileMapXPos += 2;
            if (tileMapXPos >= aniTilesetData->tileMap.width()) {
                tileMapXPos = 0;
                tileMapYPos += 2;

                if (tileMapYPos >= aniTilesetData->tileMap.height()) {
                    // no more tiles left.
                    goto EndLoop;
                }
            }
        }
    }

EndLoop:
    return QPixmap::fromImage(img);
}

MtTilesetGridPainter::MtTilesetGridPainter()
    : _fragments()
    , _nMetaTiles(0)
{
}

static QPainter::PixmapFragment blankMetaTileFragment()
{
    QPainter::PixmapFragment fragment;
    fragment.x = 0;
    fragment.y = 0;
    fragment.sourceLeft = 0;
    fragment.sourceTop = 0;
    fragment.width = MtTilesetRenderer::METATILE_SIZE;
    fragment.height = MtTilesetRenderer::METATILE_SIZE;
    fragment.scaleX = 1;
    fragment.scaleY = 1;
    fragment.rotation = 0;
    fragment.opacity = 1;

    return fragment;
}

void MtTilesetGridPainter::updateFragments(MtTilesetRenderer* renderer, const grid<uint16_t>& grid)
{
    constexpr int PIXMAP_WIDTH = MtTilesetRenderer::PIXMAP_WIDTH;
    constexpr int METATILE_SIZE = MtTilesetRenderer::METATILE_SIZE;

    static const QPainter::PixmapFragment blankFragment = blankMetaTileFragment();

    _nMetaTiles = renderer->nMetaTiles();
    _fragments.resize(grid.cellCount(), blankFragment);
    unsigned fIndex = 0;

    auto gridIt = grid.cbegin();
    for (unsigned y = 0; y < grid.height(); y++) {
        for (unsigned x = 0; x < grid.width(); x++) {

            const auto& tile = *gridIt++;
            if (tile < _nMetaTiles) {
                auto& f = _fragments[fIndex++];
                f.x = x * METATILE_SIZE + METATILE_SIZE / 2;
                f.y = y * METATILE_SIZE + METATILE_SIZE / 2;
                f.sourceLeft = (tile % PIXMAP_WIDTH) * METATILE_SIZE;
                f.sourceTop = (tile / PIXMAP_WIDTH) * METATILE_SIZE;
            }
        }
    }
    assert(gridIt == grid.cend());
    assert(fIndex <= _fragments.size());

    _fragments.resize(fIndex);
}

void MtTilesetGridPainter::paint(QPainter* painter, MtTilesetRenderer* renderer)
{
    if (_nMetaTiles <= renderer->nMetaTiles()) {
        painter->drawPixmapFragments(_fragments.data(), _fragments.size(), renderer->pixmap());
    }
}
