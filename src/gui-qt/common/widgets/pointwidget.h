/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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

    QPoint value() const;
    void setValue(const QPoint&);

    upoint valueUpoint() const;
    void setValue(const upoint&);

    void setMinimum(int min);
    void setMinimum(const QPoint&);

    void setMaximum(int max);
    void setMaximum(const QPoint&);
    void setMaximum(const usize&);
    void setMaximum(const QSize&);

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
