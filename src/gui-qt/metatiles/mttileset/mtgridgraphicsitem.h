/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "mttilesetrenderer.h"
#include "models/common/grid.h"
#include "models/common/vectorset-upoint.h"
#include <QGraphicsObject>
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtGraphicsScene;

class MtGridGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint16_t>;

public:
    MtGridGraphicsItem(MtGraphicsScene* scene);
    ~MtGridGraphicsItem() = default;

    bool enableMouseSelection() const { return _enableMouseSelection; }
    void setEnableMouseSelection(bool e) { _enableMouseSelection = e; }

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final;

private:
    upoint positionToGridCell(const QPointF& pos);

public slots:
    void updateAll();
    void onGridResized();

    void updateTileGridFragments();

private:
    MtGraphicsScene* const _scene;
    QRectF _boundingRect;

    MtTilesetGridPainter _tileGridPainter;

    upoint _previouslyClickedCell;
    upoint_vectorset _gridSelectionBeforeShiftClick;

    bool _enableMouseSelection;
};
}
}
}
