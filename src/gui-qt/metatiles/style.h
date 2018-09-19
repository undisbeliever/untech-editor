/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

public:
    Style(QWidget* parent);
    ~Style() = default;

    bool showGrid() const { return _showGrid; }
    void setShowGrid(bool showGrid);

    QPen gridPen() const;

    QPen gridSelectionPen() const;
    QBrush gridSelectionBrush() const;

signals:
    void showGridChanged();

private:
    QPen createCosmeticPen(const QColor& color) const;

private:
    QWidget* const _widget;

    bool _showGrid;
};
}
}
}
