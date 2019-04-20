/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
    static const QColor INVALID_CURSOR_PEN_COLOR;
    static const QColor INVALID_CURSOR_BRUSH_COLOR;

public:
    Style(QWidget* parent);
    ~Style() = default;

    bool showGrid() const { return _showGrid; }
    QAction* showGridAction() const { return _showGridAction; }

    QPen gridPen() const;

    QPen gridSelectionPen() const;
    QBrush gridSelectionBrush() const;

    QPen validCursorPen() const;
    QBrush validCursorBrush() const;

    QPen invalidCursorPen() const;
    QBrush invalidCursorBrush() const;

public slots:
    void setShowGrid(bool showGrid);

signals:
    void showGridChanged();

private:
    QPen createCosmeticPen(const QColor& color) const;

private:
    QWidget* const _widget;
    QAction* const _showGridAction;

    bool _showGrid;
};
}
}
}
