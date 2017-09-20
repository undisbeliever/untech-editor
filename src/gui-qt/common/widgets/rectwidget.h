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

class RectWidget : public QWidget {
    Q_OBJECT

public:
    RectWidget(QWidget* parent = nullptr);
    ~RectWidget() = default;

    void clear();

    QRect value() const;
    void setValue(const QRect&);

    urect valueUrect() const;
    void setValue(const urect&);

    void setRange(const usize&);
    void setRange(const QSize&);
    void setMinRectSize(const usize&);
    void setMaxRectSize(const usize&);

protected:
    virtual void focusInEvent(QFocusEvent*) override;
    virtual bool eventFilter(QObject* object, QEvent* event) override;

signals:
    void editingFinished();

private slots:
    void updateRanges();
    void updateHorizontalRange();
    void updateVerticalRange();

private:
    usize _range, _minRectSize, _maxRectSize;

    QSpinBox* _xPos;
    QSpinBox* _yPos;
    QSpinBox* _width;
    QSpinBox* _height;
};
}
}
