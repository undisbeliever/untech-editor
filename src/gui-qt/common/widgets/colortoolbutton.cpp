/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "colortoolbutton.h"

using namespace UnTech;
using namespace UnTech::GuiQt;

ColorToolButton::ColorToolButton(QWidget* parent)
    : QToolButton(parent)
{
}

void ColorToolButton::setColor(const rgba& color)
{
    setColor(QColor(color.rgbHex()));
}

void ColorToolButton::setColor(const QColor& color)
{
    setStyleSheet(QString("QToolButton{ background: %1; }").arg(color.name()));
}

void ColorToolButton::unsetColor()
{
    setStyleSheet(QString());
}
