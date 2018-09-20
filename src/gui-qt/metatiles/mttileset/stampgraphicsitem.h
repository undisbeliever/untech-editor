/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractcursorgraphicsitem.h"
#include "mtgraphicsscenes.h"
#include "mttilesetrenderer.h"
#include "models/common/grid.h"
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtEditableGraphicsScene;

class StampGraphicsItem : public AbstractCursorGraphicsItem {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint16_t>;
    constexpr static int METATILE_SIZE = 16;

public:
    StampGraphicsItem(MtEditableGraphicsScene* scene);
    ~StampGraphicsItem() = default;

    const point& tilePosition() const { return _tilePosition; }
    void setTilePosition(const point& tilePosition);

    const grid_t& grid() const { return _grid; }
    void setGrid(grid_t&& grid);

    virtual QRectF boundingRect() const override;

    virtual bool processMouseScenePosition(const QPointF& scenePos) final;

    virtual void processClick();

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

private:
    point scenePositionToTilePosition(const QPointF& scenePos);

public slots:
    void updateAll();

    void updateTileGridFragments();

private:
    MtEditableGraphicsScene* const _scene;

    point _tilePosition;
    QRectF _boundingRect;

    grid_t _grid;
    MtTilesetGridPainter _tileGridPainter;
};
}
}
}
