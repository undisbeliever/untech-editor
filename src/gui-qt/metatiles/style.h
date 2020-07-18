/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {

class Style : public QObject {
    Q_OBJECT

private:
    static const QColor GRID_PEN_COLOR;

    static const QColor GRID_SELECTION_PEN_COLOR;
    static const QColor GRID_SELECTION_BRUSH_COLOR;

    static const QColor VALID_CURSOR_PEN_COLOR;
    static const QColor VALID_CURSOR_BRUSH_COLOR;
    static const QColor ERASER_CURSOR_PEN_COLOR;
    static const QColor ERASER_CURSOR_BRUSH_COLOR;
    static const QColor INVALID_CURSOR_PEN_COLOR;
    static const QColor INVALID_CURSOR_BRUSH_COLOR;

    static const QColor TILE_COLLISIONS_COLOR;

public:
    Style(QWidget* parent);
    ~Style() = default;

    void populateActions(QWidget* widget);

    bool showGrid() const { return _showGrid; }
    QAction* showGridAction() const { return _showGridAction; }

    bool showTiles() const { return _showTiles; }
    QAction* showTilesAction() const { return _showTilesAction; }

    bool showTileCollisions() const { return _showTileCollisions; }
    QAction* showTileCollisionsAction() const { return _showTileCollisionsAction; }

    bool showInteractiveTiles() const { return _showInteractiveTiles; }
    QAction* showInteractiveTilesAction() const { return _showInteractiveTilesAction; }

    bool showSmallInteractiveTiles() const { return _showSmallInteractiveTiles; }
    QAction* showSmallInteractiveTilesAction() const { return _showSmallInteractiveTilesAction; }

    QPen gridPen() const;

    QPen gridSelectionPen() const;
    QBrush gridSelectionBrush() const;

    QPen validCursorPen() const;
    QBrush validCursorBrush() const;

    QPen eraserCursorPen() const;
    QBrush eraserCursorBrush() const;

    QPen invalidCursorPen() const;
    QBrush invalidCursorBrush() const;

    const QColor& tileCollisionsColor() const { return TILE_COLLISIONS_COLOR; }

public slots:
    void setShowGrid(bool s);
    void setShowTiles(bool s);
    void setShowTileCollisions(bool s);
    void setShowInteractiveTiles(bool s);
    void setShowSmallInteractiveTiles(bool s);

signals:
    void showLayersChanged();

private:
    QPen createCosmeticPen(const QColor& color) const;

private:
    QWidget* const _widget;
    QAction* const _showGridAction;
    QAction* const _showTilesAction;
    QAction* const _showTileCollisionsAction;
    QAction* const _showInteractiveTilesAction;
    QAction* const _showSmallInteractiveTilesAction;

    bool _showGrid;
    bool _showTiles;
    bool _showTileCollisions;
    bool _showInteractiveTiles;
    bool _showSmallInteractiveTiles;
};
}
}
}
