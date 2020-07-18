/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/optional.h"
#include "models/metatiles/common.h"
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QVector>
#include <cstdint>
#include <vector>

class QAbstractButton;
class QComboBox;

namespace UnTech {
namespace Resources {
struct PaletteData;
}
namespace MetaTiles {
struct MetaTileTilesetData;
struct InteractiveTiles;
}

namespace GuiQt {
class AbstractResourceItem;

namespace Resources {
class DualAnimationTimer;

namespace Palette {
class ResourceItem;
}
}

namespace MetaTiles {
class MtGraphicsScene;
class Style;

namespace InteractiveTiles {
class ResourceItem;
}

namespace MtTileset {
class ResourceItem;

namespace MT = UnTech::MetaTiles;

template <unsigned TS>
struct InteractiveTileSymbols {
    static constexpr unsigned N_SYMBOLS = MT::MAX_INTERACTIVE_TILE_FUNCTION_TABLES;

    static constexpr int TILE_SIZE = TS;
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = N_SYMBOLS / WIDTH;

    QPixmap pixmap;

    InteractiveTileSymbols() = default;

    void clearPixmap();
    void buildPixmap(const MT::InteractiveTiles& interactiveTiles);
};

class MtTilesetRenderer : public QObject {
    Q_OBJECT

public:
    static constexpr int PIXMAP_TILE_WIDTH = 16;
    static constexpr int PIXMAP_TILE_HEIGHT = 16;
    static constexpr int METATILE_SIZE = 16;

public:
    MtTilesetRenderer(QObject* parent);
    ~MtTilesetRenderer() = default;

    void setPlayButton(QAbstractButton* button);
    void setRegionCombo(QComboBox* comboBox);

    Resources::Palette::ResourceItem* paletteItem() const { return _paletteItem; }
    void setPaletteItem(Resources::Palette::ResourceItem* item);

    ResourceItem* tilesetItem() const { return _tilesetItem; }
    void setTilesetItem(ResourceItem* item);

    InteractiveTiles::ResourceItem* interactiveTilesItem() const { return _interactiveTilesItem; }

    unsigned nPalettes() const { return _nPalettes; }
    unsigned nTilesets() const { return _nTilesets; }

    QColor backgroundColor();
    QColor backgroundColor(unsigned paletteId);

    const QPixmap& pixmap();
    const QPixmap& pixmap(unsigned paletteId, unsigned tilesetId);
    const QBitmap& tileCollisionsBitmap();

    const auto& interactiveTileSymbolsSmall() const { return _interactiveTileSymbolsSmall; }
    const auto& interactiveTileSymbolsLarge() const { return _interactiveTileSymbolsLarge; }

signals:
    void pixmapChanged(); // also emitted when tileCollisionsBitmap() is changed.
    void paletteItemChanged();
    void tilesetItemChanged();
    void tileCollisionsChanged();
    void interactiveTilesChanged();

public slots:
    void resetAnimations();
    void pauseAndAdvancePaletteFrame();
    void pauseAndAdvanceTilesetFrame();

    void onAnimationDelaysChanged();
    void resetPixmaps();

    void resetTileCollisionsBitmap();

    void buildInteractiveTileSymbols();

    void onResourceItemAboutToBeRemoved(AbstractResourceItem* item);

private:
    void setInteractiveTilesItem(InteractiveTiles::ResourceItem* item);

    optional<const UnTech::Resources::PaletteData&> paletteData() const;
    optional<const UnTech::MetaTiles::MetaTileTilesetData&> metaTileTilesetData() const;

    QPixmap buildPixmap(unsigned paletteFrame, unsigned tilesetFrame);
    QBitmap buildTileCollisionsBitmap();

private:
    Resources::DualAnimationTimer* const _animationTimer;

    Resources::Palette::ResourceItem* _paletteItem;
    ResourceItem* _tilesetItem;
    InteractiveTiles::ResourceItem* _interactiveTilesItem;

    QVector<QPixmap> _pixmaps; // [ paletteFrame * _nTilesets + tilesetFrame ]
    QBitmap _tileCollisionsPixmap;

    InteractiveTileSymbols<16> _interactiveTileSymbolsSmall;
    InteractiveTileSymbols<32> _interactiveTileSymbolsLarge;

    unsigned _nPalettes;
    unsigned _nTilesets;
};

class MtTilesetGridPainter {
public:
    struct InteractiveTileFragment {
        const QPointF position;
        const unsigned interactiveTileIndex;

        InteractiveTileFragment(int x, int y, unsigned interactiveTileIndex);
    };

public:
    MtTilesetGridPainter();

    // MUST be called when grid changed.
    // The grid<uint16_t> updateFragments will not draw any tiles with a value >= N_METATILES
    void updateFragments(const grid<uint8_t>& grid);
    void updateFragments(const grid<uint16_t>& grid);

    void updateInteractiveTileFragments(const grid<uint8_t>& grid, const MtGraphicsScene* scene);

    void generateEraseFragments(unsigned width, unsigned height);

    inline void paintTiles(QPainter* painter, MtTilesetRenderer* renderer)
    {
        painter->drawPixmapFragments(_fragments.data(), _fragments.size(), renderer->pixmap());
    }

    // NOTE: you must set the QPainter pen before using this function.
    inline void paintCollisions(QPainter* painter, MtTilesetRenderer* renderer)
    {
        painter->drawPixmapFragments(_fragments.data(), _fragments.size(), renderer->tileCollisionsBitmap());
    }

    void paintInteractiveTiles(QPainter* painter, const MtTilesetRenderer* renderer, const Style* style);

private:
    template <typename T>
    void _updateFragments(const grid<T>& grid);
    template <typename T>
    void _updateInteractiveTileFragments(const grid<T>& grid, const MtGraphicsScene* scene);

private:
    std::vector<QPainter::PixmapFragment> _fragments;
    std::vector<InteractiveTileFragment> _interactiveTileFragments;

    std::vector<QPainter::PixmapFragment> _interactiveTilePixmapFragments;
};
}
}
}
}
