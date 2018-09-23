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

    enum class DrawState {
        STAMP,
        DRAW_BOX,
        DRAW_BOX_FIRST_CLICK,
        DRAW_BOX_SECOND_CLICK,
        DRAW_BOX_DRAGING,
    };

public:
    using grid_t = UnTech::grid<uint16_t>;
    constexpr static int METATILE_SIZE = 16;

public:
    StampGraphicsItem(MtEditableGraphicsScene* scene);
    ~StampGraphicsItem() = default;

    const point& tilePosition() const { return _tilePosition; }
    bool setTilePosition(const point& tilePosition);

    const grid_t& sourceGrid() const { return _sourceGrid; }
    void setSourceGrid(grid_t&& sourceGrid);

    const grid_t& activeGrid() const;

    QRect validCellsRect() const;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

    virtual bool processMouseScenePosition(const QPointF& scenePos) final;

    virtual void processClick() final;
    virtual bool processEscape() final;

protected:
    virtual void keyPressEvent(QKeyEvent* event) final;
    virtual void keyReleaseEvent(QKeyEvent* event) final;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;

private:
    void resetDrawState();
    void updateBoundingBox();

    void updateDrawState(Qt::KeyboardModifiers modifiers);

    bool processNormalMousePosition(const QPointF& scenePos);
    bool processDrawBoxMousePosition(const QPointF& scenePos);

    void createBoxGrid(unsigned width, unsigned height);

public slots:
    void updateAll();

    void updateTileGridFragments();

private:
    MtEditableGraphicsScene* const _scene;
    MtTilesetGridPainter _tileGridPainter;

    DrawState _drawState;

    point _tilePosition;
    QRectF _boundingRect;
    QPointF _mouseScenePosition;

    grid_t _sourceGrid;

    grid_t _boxGrid;
    point _startBoxPosition;
};
}
}
}
