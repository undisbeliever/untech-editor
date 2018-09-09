/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include <QGraphicsObject>
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtTilesetRenderer;
class MtTilesetResourceItem;

class MtGridGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint16_t>;

public:
    MtGridGraphicsItem(MtTilesetRenderer* renderer);
    ~MtGridGraphicsItem() = default;

    MtTilesetRenderer* renderer() const { return _renderer; }
    MtTilesetResourceItem* tilesetItem() const { return _tilesetItem; }

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

protected:
    virtual void tilesetItemChanged() = 0;

    const grid_t& grid() const { return _grid; }
    void setGrid(grid_t&& grid);

public slots:
    void onPixmapChanged();

private slots:
    void onRendererTilesetItemChanged();

private:
    MtTilesetRenderer* const _renderer;
    MtTilesetResourceItem* _tilesetItem;
    QRectF _boundingRect;
    grid_t _grid;
};

class MtTilesetGraphicsItem : public MtGridGraphicsItem {
    Q_OBJECT

public:
    MtTilesetGraphicsItem(MtTilesetRenderer* renderer);
    ~MtTilesetGraphicsItem() = default;

protected:
    virtual void tilesetItemChanged() final;

private slots:
    void onTilesetCompiled();
};

class MtTilesetScratchpadGraphicsItem : public MtGridGraphicsItem {
    Q_OBJECT

public:
    MtTilesetScratchpadGraphicsItem(MtTilesetRenderer* renderer);
    ~MtTilesetScratchpadGraphicsItem() = default;

protected:
    virtual void tilesetItemChanged() final;
};
}
}
}
