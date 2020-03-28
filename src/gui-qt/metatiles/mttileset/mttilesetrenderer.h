/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/optional.h"
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
namespace MtTileset {
class ResourceItem;

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

    unsigned nPalettes() const { return _nPalettes; }
    unsigned nTilesets() const { return _nTilesets; }

    QColor backgroundColor();
    QColor backgroundColor(unsigned paletteId);

    const QPixmap& pixmap();
    const QPixmap& pixmap(unsigned paletteId, unsigned tilesetId);
    const QBitmap& tileCollisionsBitmap();

signals:
    void pixmapChanged(); // also emitted when tileCollisionsBitmap() is changed.
    void paletteItemChanged();
    void tilesetItemChanged();
    void tileCollisionsChanged();

public slots:
    void resetAnimations();
    void pauseAndAdvancePaletteFrame();
    void pauseAndAdvanceTilesetFrame();

    void onAnimationDelaysChanged();
    void resetPixmaps();

    void resetTileCollisionsBitmap();

    void onResourceItemAboutToBeRemoved(AbstractResourceItem* item);

private:
    optional<const UnTech::Resources::PaletteData&> paletteData() const;
    optional<const UnTech::MetaTiles::MetaTileTilesetData&> metaTileTilesetData() const;

    QPixmap buildPixmap(unsigned paletteFrame, unsigned tilesetFrame);
    QBitmap buildTileCollisionsBitmap();

private:
    Resources::DualAnimationTimer* const _animationTimer;

    Resources::Palette::ResourceItem* _paletteItem;
    ResourceItem* _tilesetItem;

    QVector<QPixmap> _pixmaps; // [ paletteFrame * _nTilesets + tilesetFrame ]
    QBitmap _tileCollisionsPixmap;

    unsigned _nPalettes;
    unsigned _nTilesets;
};

class MtTilesetGridPainter {
public:
    MtTilesetGridPainter();

    // MUST be called when grid changed.
    // The grid<uint16_t> updateFragments will not draw any tiles with a value >= N_METATILES
    void updateFragments(const grid<uint8_t>& grid);
    void updateFragments(const grid<uint16_t>& grid);

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

private:
    template <typename T>
    void _updateFragments(const grid<T>& grid);

private:
    std::vector<QPainter::PixmapFragment> _fragments;
};
}
}
}
}
