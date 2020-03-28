/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QPen>
#include <QWidget>
#include <QWindow>

namespace UnTech {
namespace GuiQt {

inline QPen createCosmeticPen(const QColor& color, const QWidget* widget)
{
#if QT_VERSION >= 0x050600
    qreal pixelRatio = widget->devicePixelRatioF();
#else
    int pixelRatio = widget->devicePixelRatio();
#endif

    QPen pen(color, pixelRatio);
    pen.setCosmetic(true);
    return pen;
}

inline QPen createCosmeticPen(const QColor& color, const QWindow* window)
{
    qreal pixelRatio = window->devicePixelRatio();

    QPen pen(color, pixelRatio);
    pen.setCosmetic(true);
    return pen;
}
}
}
