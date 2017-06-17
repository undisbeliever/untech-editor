/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/aabb.h"
#include <QSpinBox>
#include <QWidget>

namespace UnTech {
namespace GuiQt {

class PointWidget : public QWidget {
    Q_OBJECT

public:
    PointWidget(QWidget* parent = nullptr);
    ~PointWidget() = default;

    void clear();

    upoint valueUpoint() const;
    void setValue(const upoint&);

    void setMinimum(int min);

    void setMaximum(int max);
    void setMaximum(const usize&);

signals:
    void editingFinished();

private:
    QSpinBox* _xPos;
    QSpinBox* _yPos;
};
}
}
