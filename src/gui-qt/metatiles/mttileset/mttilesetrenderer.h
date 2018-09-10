/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include <QPainter>
#include <QPixmap>
#include <QVector>
#include <cstdint>
#include <vector>

class QAbstractButton;
class QComboBox;

namespace UnTech {
namespace GuiQt {
class AbstractResourceItem;

namespace Resources {
class DualAnimationTimer;
class PaletteResourceItem;
}

namespace MetaTiles {
class MtTilesetResourceItem;

class MtTilesetRenderer : public QObject {
    Q_OBJECT

public:
    static constexpr int PIXMAP_WIDTH = 16;
    static constexpr int METATILE_SIZE = 16;

public:
    MtTilesetRenderer(QObject* parent);
    ~MtTilesetRenderer() = default;

    void setPlayButton(QAbstractButton* button);
    void setRegionCombo(QComboBox* comboBox);

    Resources::PaletteResourceItem* paletteItem() const { return _paletteItem; }
    void setPaletteItem(Resources::PaletteResourceItem* item);

    MtTilesetResourceItem* tilesetItem() const { return _tilesetItem; }
    void setTilesetItem(MtTilesetResourceItem* item);

    const QPixmap& pixmap();
    const QPixmap& pixmap(unsigned paletteId, unsigned tilesetId);

    void drawGridTiles(QPainter* painter, const grid<uint16_t>& grid);

signals:
    void pixmapChanged();
    void paletteItemChanged();
    void tilesetItemChanged();

public slots:
    void resetAnimations();
    void pauseAndAdvancePaletteFrame();
    void pauseAndAdvanceTilesetFrame();

    void onAnimationDelaysChanged();
    void resetPixmaps();

    void onResourceItemAboutToBeRemoved(AbstractResourceItem* item);

private:
    QPixmap buildPixmap(unsigned paletteFrame, unsigned tilesetFrame);

private:
    Resources::DualAnimationTimer* const _animationTimer;

    Resources::PaletteResourceItem* _paletteItem;
    MtTilesetResourceItem* _tilesetItem;

    QVector<QPixmap> _pixmaps; // [ paletteFrame * _nTilesets + tilesetFrame ]
    unsigned _nPalettes;
    unsigned _nTilesets;
    unsigned _nMetaTiles;

    // This is a class member to reduce the number of memory allocation
    std::vector<QPainter::PixmapFragment> _fragments;
};
}
}
}
