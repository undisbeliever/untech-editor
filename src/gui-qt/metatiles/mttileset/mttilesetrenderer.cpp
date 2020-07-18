/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetrenderer.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilecollisionpixmaps.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metatiles/interactive-tiles/resourceitem.h"
#include "gui-qt/metatiles/mtgraphicsscenes.h"
#include "gui-qt/metatiles/style.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/dualanimationtimer.h"
#include "gui-qt/resources/palette/resourceitem.h"
#include "gui-qt/snes/tile.hpp"
#include "gui-qt/staticresourcelist.h"
#include "models/metatiles/interactive-tiles.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/project/project-data.h"
#include "models/project/project.h"
#include "models/resources/animation-frames-input.h"

#include <QPainter>
#include <cassert>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles::MtTileset;

namespace RES = UnTech::Resources;

MtTilesetRenderer::MtTilesetRenderer(QObject* parent)
    : QObject(parent)
    , _animationTimer(new Resources::DualAnimationTimer(this))
    , _paletteItem(nullptr)
    , _tilesetItem(nullptr)
    , _interactiveTilesItem(nullptr)
    , _pixmaps()
    , _interactiveTileSymbolsSmall()
    , _interactiveTileSymbolsLarge()
    , _nPalettes(0)
    , _nTilesets(0)
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

void MtTilesetRenderer::setPaletteItem(Resources::Palette::ResourceItem* item)
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

void MtTilesetRenderer::setTilesetItem(ResourceItem* item)
{
    if (_tilesetItem != item) {
        if (_tilesetItem) {
            _tilesetItem->disconnect(this);
        }
        _tilesetItem = item;

        _animationTimer->resetTimer();

        setInteractiveTilesItem(_tilesetItem ? _tilesetItem->project()->staticResources()->interactiveTiles() : nullptr);

        resetPixmaps();
        resetTileCollisionsBitmap();
        onAnimationDelaysChanged();

        if (_tilesetItem) {
            connect(_tilesetItem, &AbstractResourceItem::resourceComplied,
                    this, &MtTilesetRenderer::resetPixmaps);

            connect(_tilesetItem->tileParameters(), &MtTilesetTileParameters::tileCollisionsChanged,
                    this, &MtTilesetRenderer::resetTileCollisionsBitmap);

            connect(_tilesetItem, &ResourceItem::animationDelayChanged,
                    this, &MtTilesetRenderer::onAnimationDelaysChanged);

            connect(_tilesetItem->resourceList(), &AbstractResourceList::resourceItemAboutToBeRemoved,
                    this, &MtTilesetRenderer::onResourceItemAboutToBeRemoved);
        }

        tilesetItemChanged();
    }
}

void MtTilesetRenderer::setInteractiveTilesItem(InteractiveTiles::ResourceItem* item)
{
    if (_interactiveTilesItem == item) {
        return;
    }

    if (_interactiveTilesItem) {
        _interactiveTilesItem->disconnect(this);
    }
    _interactiveTilesItem = item;

    buildInteractiveTileSymbols();

    if (_interactiveTilesItem) {
        connect(_interactiveTilesItem, &InteractiveTiles::ResourceItem::resourceComplied,
                this, &MtTilesetRenderer::buildInteractiveTileSymbols);

        connect(_interactiveTilesItem, &InteractiveTiles::ResourceItem::resourceComplied,
                this, &MtTilesetRenderer::interactiveTilesChanged);
    }
}

UnTech::optional<const RES::PaletteData&> MtTilesetRenderer::paletteData() const
{
    if (_paletteItem == nullptr) {
        return {};
    }
    return _paletteItem->project()->projectData().palettes().at(_paletteItem->index());
}

UnTech::optional<const MT::MetaTileTilesetData&> MtTilesetRenderer::metaTileTilesetData() const
{
    if (_tilesetItem == nullptr) {
        return {};
    }
    return _tilesetItem->project()->projectData().metaTileTilesets().at(_tilesetItem->index());
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
    if (_nPalettes == 0 || _nTilesets == 0) {

        static const QPixmap nullPixmap;
        return nullPixmap;
    }

    const auto& afc = _animationTimer->animationFrameCount();
    return pixmap(afc.first % _nPalettes, afc.second % _nTilesets);
}

QColor MtTilesetRenderer::backgroundColor(unsigned paletteId)
{
    const auto palData = this->paletteData();
    if (!palData || paletteId >= _nPalettes) {
        return QColor();
    }

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

const QBitmap& MtTilesetRenderer::tileCollisionsBitmap()
{
    if (_tileCollisionsPixmap.isNull() && _tilesetItem) {
        _tileCollisionsPixmap = buildTileCollisionsBitmap();
    }

    return _tileCollisionsPixmap;
}

void MtTilesetRenderer::onAnimationDelaysChanged()
{
    unsigned paletteDelay = 0;
    unsigned tilesetDelay = 0;

    if (_paletteItem) {
        const RES::PaletteInput& pal = _paletteItem->paletteInput();
        paletteDelay = pal.animationDelay;
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

    const auto palData = this->paletteData();
    const auto tilesetData = this->metaTileTilesetData();

    if (palData && tilesetData) {
        _nPalettes = palData->nAnimations();
        _nTilesets = tilesetData->animatedTileset.nAnimatedFrames();

        Q_ASSERT(_nPalettes > 0 && _nTilesets > 0);

        _pixmaps.resize(_nPalettes * _nTilesets);
    }
    else {
        _nPalettes = 0;
        _nTilesets = 0;
    }

    emit pixmapChanged();
}

void MtTilesetRenderer::resetTileCollisionsBitmap()
{
    _tileCollisionsPixmap = QPixmap();

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
    if (item == _interactiveTilesItem) {
        setInteractiveTilesItem(nullptr);
    }
}

static void drawTile(QImage& image,
                     const std::vector<UnTech::Snes::SnesColor>& palette,
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
    const auto palData = this->paletteData();
    const auto tilesetData = this->metaTileTilesetData();

    if (!palData || !tilesetData) {
        return QPixmap();
    }

    if (paletteFrame >= palData->nAnimations()
        || tilesetFrame >= tilesetData->animatedTileset.nAnimatedFrames()) {

        return QPixmap();
    }

    const auto& palette = palData->paletteFrames.at(paletteFrame);

    QImage img(PIXMAP_TILE_WIDTH * METATILE_SIZE, PIXMAP_TILE_HEIGHT * METATILE_SIZE, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    unsigned tileMapXPos = 0;
    unsigned tileMapYPos = 0;

    for (int tileY = 0; tileY < PIXMAP_TILE_HEIGHT; tileY++) {
        for (int tileX = 0; tileX < PIXMAP_TILE_WIDTH; tileX++) {
            for (int ty = 0; ty < 2; ty++) {
                for (int tx = 0; tx < 2; tx++) {
                    drawTile(img, palette, tilesetData->animatedTileset, tilesetFrame,
                             tilesetData->animatedTileset.tileMap.at(tileMapXPos + tx, tileMapYPos + ty),
                             tileX * 16 + tx * 8, tileY * 16 + ty * 8);
                }
            }

            // goto next tile
            tileMapXPos += 2;
            if (tileMapXPos >= tilesetData->animatedTileset.tileMap.width()) {
                tileMapXPos = 0;
                tileMapYPos += 2;

                if (tileMapYPos >= tilesetData->animatedTileset.tileMap.height()) {
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
    , _interactiveTileFragments()
    , _interactiveTilePixmapFragments()
{
}

static QPainter::PixmapFragment blankPixmapFragment(int size)
{
    QPainter::PixmapFragment fragment;
    fragment.x = 0;
    fragment.y = 0;
    fragment.sourceLeft = 0;
    fragment.sourceTop = 0;
    fragment.width = size;
    fragment.height = size;
    fragment.scaleX = 1;
    fragment.scaleY = 1;
    fragment.rotation = 0;
    fragment.opacity = 1;

    return fragment;
}

template <typename T>
inline void MtTilesetGridPainter::_updateFragments(const grid<T>& grid)
{
    constexpr int PIXMAP_WIDTH = MtTilesetRenderer::PIXMAP_TILE_WIDTH;
    constexpr int METATILE_SIZE = MtTilesetRenderer::METATILE_SIZE;

    static const QPainter::PixmapFragment blankFragment = blankPixmapFragment(MtTilesetRenderer::METATILE_SIZE);

    _fragments.resize(grid.cellCount(), blankFragment);
    unsigned fIndex = 0;

    auto gridIt = grid.cbegin();
    for (unsigned y = 0; y < grid.height(); y++) {
        for (unsigned x = 0; x < grid.width(); x++) {

            const auto& tile = *gridIt++;
            if (tile < MT::N_METATILES) {
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

MtTilesetGridPainter::InteractiveTileFragment::InteractiveTileFragment(int x, int y, unsigned interactiveTileIndex)
    : position(x, y)
    , interactiveTileIndex(interactiveTileIndex)
{
}

template <typename T>
void MtTilesetGridPainter::_updateInteractiveTileFragments(const grid<T>& grid, const MtGraphicsScene* scene)
{
    _interactiveTileFragments.clear();

    Q_ASSERT(scene);
    auto* tilesetItem = scene->tilesetItem();
    if (tilesetItem == nullptr) {
        return;
    }
    auto& compiledData = tilesetItem->compiledData();
    if (!compiledData) {
        return;
    }

    auto gridIt = grid.cbegin();
    for (unsigned y = 0; y < grid.height(); y++) {
        for (unsigned x = 0; x < grid.width(); x++) {
            const auto& tile = *gridIt++;

            if (tile < MT::N_METATILES) {
                unsigned interactiveIndex = compiledData->tileFunctionTables.at(tile);
                if (interactiveIndex > 0) {
                    _interactiveTileFragments.emplace_back(
                        x * MT::METATILE_SIZE_PX + 1, y * MT::METATILE_SIZE_PX + 1,
                        interactiveIndex);
                }
            }
        }
    }
    assert(gridIt == grid.cend());
}

void MtTilesetGridPainter::updateFragments(const grid<uint8_t>& grid)
{
    _updateFragments(grid);
}

void MtTilesetGridPainter::updateFragments(const grid<uint16_t>& grid)
{
    _updateFragments(grid);
}

void MtTilesetGridPainter::updateInteractiveTileFragments(const grid<uint8_t>& grid, const MetaTiles::MtGraphicsScene* scene)
{
    _updateInteractiveTileFragments(grid, scene);
}

void MtTilesetGridPainter::generateEraseFragments(unsigned width, unsigned height)
{
    constexpr int METATILE_SIZE = MtTilesetRenderer::METATILE_SIZE;

    static const QPainter::PixmapFragment blankFragment = blankPixmapFragment(METATILE_SIZE);

    _fragments.resize(width * height, blankFragment);

    auto fragmentIt = _fragments.begin();
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            fragmentIt->x = x * METATILE_SIZE + METATILE_SIZE / 2;
            fragmentIt->y = y * METATILE_SIZE + METATILE_SIZE / 2;

            *fragmentIt++;
        }
    }
    Q_ASSERT(fragmentIt == _fragments.end());

    _interactiveTileFragments.clear();
}

inline QBitmap MtTilesetRenderer::buildTileCollisionsBitmap()
{
    using Texture = TileCollisionTexture;

    QVector<QPainter::PixmapFragment> fragments;
    fragments.reserve(MT::N_METATILES);

    // This routine uses tilesetInput for the tileCollisions.
    // This allows MtTilesetRenderer to display tileCollision data in the event of a compile error in the tilesetItem.
    Q_ASSERT(_tilesetItem);
    if (auto* tilesetInput = _tilesetItem->tilesetInput()) {
        for (unsigned i = 0; i < tilesetInput->tileCollisions.size(); i++) {
            const auto& tc = tilesetInput->tileCollisions.at(i);
            QPainter::PixmapFragment f = blankPixmapFragment(METATILE_SIZE);
            f.x = unsigned(i % PIXMAP_TILE_WIDTH) * METATILE_SIZE + METATILE_SIZE / 2;
            f.y = unsigned(i / PIXMAP_TILE_HEIGHT) * METATILE_SIZE + METATILE_SIZE / 2;
            f.sourceLeft = Texture::xOffset(tc);
            f.sourceTop = 0;

            fragments.append(f);
        }
    }

    QBitmap bitmap(PIXMAP_TILE_WIDTH * METATILE_SIZE, PIXMAP_TILE_HEIGHT * METATILE_SIZE);
    bitmap.fill();

    if (not fragments.empty()) {
        QPainter painter(&bitmap);
        painter.drawPixmapFragments(fragments.data(), fragments.size(), Texture::bitmap());
    }

    return bitmap;
}

void MtTilesetRenderer::buildInteractiveTileSymbols()
{
    if (_interactiveTilesItem) {
        const auto* projectFile = _interactiveTilesItem->project()->projectFile();
        Q_ASSERT(projectFile);

        _interactiveTileSymbolsSmall.buildPixmap(projectFile->interactiveTiles);
        _interactiveTileSymbolsLarge.buildPixmap(projectFile->interactiveTiles);
    }
    else {
        _interactiveTileSymbolsSmall.clearPixmap();
        _interactiveTileSymbolsLarge.clearPixmap();
    }
}

template <unsigned TS>
void InteractiveTileSymbols<TS>::clearPixmap()
{
    pixmap = QPixmap();
}

template <unsigned TS>
void InteractiveTileSymbols<TS>::buildPixmap(const MT::InteractiveTiles& interactiveTiles)
{
    static_assert(WIDTH * HEIGHT == MT::MAX_INTERACTIVE_TILE_FUNCTION_TABLES);

    constexpr bool isSmall = TILE_SIZE <= 16;
    constexpr int FONT_SIZE = isSmall ? 8 : 16;
    constexpr int OUTLINE_WIDTH = isSmall ? 1 : 3;

    QFont font(QStringLiteral("Sans"));
    font.setStyleHint(QFont::SansSerif);
    font.setBold(true);
    font.setPixelSize(FONT_SIZE);

    const QFontMetrics fontMetrics(font);
    const int tx = 1;
    const int ty = fontMetrics.ascent() + 1;

    QPixmap pix(WIDTH * TILE_SIZE, HEIGHT * TILE_SIZE);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    int x = 0;
    int y = 0;

    auto draw = [&](const MT::InteractiveTileFunctionTable& itf) {
        if (y >= TILE_SIZE * HEIGHT) {
            return;
        }

        const QString str = QString::fromStdString(itf.symbol);

        painter.setClipRect(x, y, TILE_SIZE, TILE_SIZE, Qt::ReplaceClip);

        QPainterPath path;
        path.addText(x + tx, y + ty, font, str);

        painter.setBrush(Qt::black);
        painter.setPen(QPen(Qt::black, OUTLINE_WIDTH));
        painter.drawPath(path);

        painter.setBrush(fromRgba(itf.symbolColor));
        painter.setPen(Qt::NoPen);
        painter.drawPath(path);

        x += TILE_SIZE;
        if (x >= TILE_SIZE * WIDTH) {
            x = 0;
            y += TILE_SIZE;
        }
    };

    for (const auto& itf : interactiveTiles.FIXED_FUNCTION_TABLES) {
        draw(itf);
    }
    for (const auto& itf : interactiveTiles.functionTables) {
        draw(itf);
    }

    painter.end();

    pixmap.swap(pix);
}

template <unsigned TILE_SIZE>
static void drawInteractiveTiles(QPainter* painter, const QTransform transform,
                                 const InteractiveTileSymbols<TILE_SIZE>& symbols,
                                 const std::vector<MtTilesetGridPainter::InteractiveTileFragment>& itFragments,
                                 std::vector<QPainter::PixmapFragment>& pixmapFragments)
{
    static_assert(TILE_SIZE == InteractiveTileSymbols<TILE_SIZE>::TILE_SIZE);
    static const QPainter::PixmapFragment blankFragment = blankPixmapFragment(TILE_SIZE);

    // Transform is unknown when generating `_interactiveTileFragments`, must populate pixmapFragments when drawing to painter
    pixmapFragments.clear();
    pixmapFragments.resize(itFragments.size(), blankFragment);

    if (symbols.pixmap.isNull()) {
        return;
    }

    unsigned fIndex = 0;
    for (const auto& itfragment : itFragments) {
        const unsigned i = itfragment.interactiveTileIndex;
        if (i > 0 && i < symbols.N_SYMBOLS) {
            const QPointF pos = transform.map(itfragment.position);

            auto& pf = pixmapFragments[fIndex++];
            pf.x = pos.x() + TILE_SIZE / 2;
            pf.y = pos.y() + TILE_SIZE / 2;
            pf.sourceLeft = (i % symbols.WIDTH) * TILE_SIZE;
            pf.sourceTop = (i / symbols.WIDTH) * TILE_SIZE;
            ;
        }
    }
    assert(fIndex <= pixmapFragments.size());

    if (fIndex > 0) {
        painter->save();
        painter->resetTransform();

        painter->drawPixmapFragments(pixmapFragments.data(), fIndex, symbols.pixmap);

        painter->restore();
    }
}

void MtTilesetGridPainter::paintInteractiveTiles(QPainter* painter, const MtTilesetRenderer* renderer, const Style* style)
{
    if (_interactiveTileFragments.empty()) {
        return;
    }

    const QTransform transform = painter->worldTransform();
    const int scaledTileHeight = transform.map(QLine(0, 0, 0, MT::METATILE_SIZE_PX)).dy();

    if (scaledTileHeight < renderer->interactiveTileSymbolsSmall().TILE_SIZE) {
        return;
    }

    if (scaledTileHeight < renderer->interactiveTileSymbolsLarge().TILE_SIZE) {
        if (style->showSmallInteractiveTiles()) {
            drawInteractiveTiles(painter, transform, renderer->interactiveTileSymbolsSmall(), _interactiveTileFragments, _interactiveTilePixmapFragments);
        }
    }
    else {
        drawInteractiveTiles(painter, transform, renderer->interactiveTileSymbolsLarge(), _interactiveTileFragments, _interactiveTilePixmapFragments);
    }
}
