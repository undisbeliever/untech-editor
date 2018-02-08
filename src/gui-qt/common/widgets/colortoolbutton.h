/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/rgba.h"
#include <QToolButton>

namespace UnTech {
namespace GuiQt {

class ColorToolButton : public QToolButton {
    Q_OBJECT

public:
    ColorToolButton(QWidget* parent = nullptr);
    ~ColorToolButton() = default;

    void setColor(const rgba& color);
    void setColor(const QColor& color);

    void unsetColor();
};
}
}
