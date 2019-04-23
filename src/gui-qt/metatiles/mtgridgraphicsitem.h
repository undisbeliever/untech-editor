/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
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
    MtGridGraphicsItem(MtGraphicsScene* scene);
    ~MtGridGraphicsItem() = default;

    bool showBackgroundColor() const { return _showBackgroundColor; }
    void setShowBackgroundColor(bool showBackgroundColor);

    bool showGridSelection() const { return _showGridSelection; }
    void setShowGridSelection(bool showGridSelection);

    bool enableMouseSelection() const { return _enableMouseSelection; }
    void setEnableMouseSelection(bool e) { _enableMouseSelection = e; }

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;

    virtual bool eventFilter(QObject* watched, QEvent* event) final;

private:
    upoint positionToGridCell(const QPointF& pos);

    void processRenctangularSelection(const upoint& cell);

public slots:
    void updateAll();
    void onGridResized();
    void onGridSelectionChanged();

    void updateTileGridFragments();

private:
    MtGraphicsScene* const _scene;
    QRectF _boundingRect;

    MtTileset::MtTilesetGridPainter _tileGridPainter;

    upoint _previousMouseCell;
    upoint _firstCellOfRectangularSelection;
    upoint_vectorset _gridSelectionBeforeRectangularSelection;

    bool _showBackgroundColor;
    bool _showGridSelection;
    bool _enableMouseSelection;
    bool _inRectangularSelection;

    // If true then the user has changed the selection but
    // `_scene->gridSelectionEdited()` has not yet been emitted.
    //
    // If true then `_scene->gridSelectionEdited()` will be emitted when the
    // user releases the Control key or leaves the scene.
    bool _uncommittedSelection;
};
}
}
}
