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

class Ms8pointWidget : public QWidget {
    Q_OBJECT

public:
    Ms8pointWidget(QWidget* parent = nullptr);
    ~Ms8pointWidget() = default;

    void clear();

    QPoint value() const;
    void setValue(const QPoint&);

    ms8point valueMs8point() const;
    void setValue(const ms8point&);

protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual bool eventFilter(QObject* object, QEvent* event) override;

signals:
    void editingFinished();

private:
    QSpinBox* _xPos;
    QSpinBox* _yPos;
};
}
}
