/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "pixmapgridwidget.h"

namespace UnTech {
namespace GuiQt {

class DrawingPixmapGridWidget : public PixmapGridWidget {
    Q_OBJECT

public:
    DrawingPixmapGridWidget(QWidget* parent = nullptr);
    ~DrawingPixmapGridWidget() = default;

    bool canDraw() const { return _canDraw; }
    void setCanDraw(bool canDraw);

protected:
    QPoint pixmapPosition(int index, const QPoint& point) const;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

    virtual void drawPixel(int index, const QPoint& point, bool press) = 0;

private:
    int _lastIndex;
    QPoint _lastPixmapPos;
    bool _canDraw;
    bool _drawing;
};
}
}
