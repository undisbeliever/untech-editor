/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/ms8aabb.h"
#include <QSpinBox>
#include <QWidget>

namespace UnTech {
namespace GuiQt {

class Ms8rectWidget : public QWidget {
    Q_OBJECT

public:
    Ms8rectWidget(QWidget* parent = nullptr);
    ~Ms8rectWidget() = default;

    void clear();

    QRect value() const;
    void setValue(const QRect&);

    ms8rect valueMs8rect() const;
    void setValue(const ms8rect&);

protected:
    virtual void focusInEvent(QFocusEvent*) override;
    virtual bool eventFilter(QObject* object, QEvent* event) override;

signals:
    void editingFinished();

private:
    QSpinBox* _xPos;
    QSpinBox* _yPos;
    QSpinBox* _width;
    QSpinBox* _height;
};
}
}
