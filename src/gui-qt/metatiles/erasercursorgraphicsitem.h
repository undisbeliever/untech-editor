/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractcursorgraphicsitem.h"
#include "mtgraphicsscenes.h"
#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtEditableGraphicsScene;

class EraserCursorGraphicsItem : public AbstractCursorGraphicsItem {
    Q_OBJECT

    enum class DrawState {
        STAMP,
        DRAW_BOX,
        DRAW_BOX_FIRST_CLICK,
        DRAW_BOX_SECOND_CLICK,
        DRAW_BOX_DRAGING,
    };

public:
    using selection_grid_t = UnTech::grid<uint16_t>;
    constexpr static int METATILE_SIZE = 16;

public:
    EraserCursorGraphicsItem(MtEditableGraphicsScene* scene);
    ~EraserCursorGraphicsItem() = default;

    const point& cursorPosition() const { return _cursorPosition; }
    bool setCursorPosition(const point& cursorPosition);

    void setCursorSize(unsigned width, unsigned height);

    QRect validCellsRect() const;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

    virtual bool processMouseScenePosition(const QPointF& scenePos) final;

    virtual bool processEscape() final;

protected:
    virtual void keyPressEvent(QKeyEvent* event) final;
    virtual void keyReleaseEvent(QKeyEvent* event) final;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;

private:
    void placeTiles(bool firstClick);

    void resetDrawState();
    void updateBoundingBox();

    void updateDrawState(Qt::KeyboardModifiers modifiers);

    bool processNormalMousePosition(const QPointF& scenePos);
    bool processDrawBoxMousePosition(const QPointF& scenePos);

public slots:
    void updateAll();

    void updateTileGridFragments();

private:
    MtEditableGraphicsScene* const _scene;
    MtTileset::MtTilesetGridPainter _tileGridPainter;

    DrawState _drawState;

    point _cursorPosition;
    usize _cursorSize;

    QRectF _boundingRect;
    QPointF _mouseScenePosition;

    point _startBoxPosition;
};

class EraserCursorFactory : public AbstractCursorFactory {
    Q_OBJECT

private:
    MtEditableGraphicsScene* const _scene;

public:
    EraserCursorFactory(MtEditableGraphicsScene* scene);

    virtual EraserCursorGraphicsItem* createCursor() final;
};
}
}
}
