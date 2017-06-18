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

class SizeWidget : public QWidget {
    Q_OBJECT

public:
    SizeWidget(QWidget* parent = nullptr);
    ~SizeWidget() = default;

    void clear();

    usize valueUsize() const;
    void setValue(const usize&);

    void setMaximum(unsigned max);

    void setMinimum(unsigned min);

    virtual bool eventFilter(QObject* object, QEvent* event);

signals:
    void editingFinished();

private:
    QSpinBox* _width;
    QSpinBox* _height;
};
}
}
