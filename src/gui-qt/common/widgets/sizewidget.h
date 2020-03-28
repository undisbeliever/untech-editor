/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

    QSize value() const;
    void setValue(const QSize&);

    usize valueUsize() const;
    void setValue(const usize&);

    void setMaximum(unsigned max);
    void setMaximum(const QSize& max);

    void setMinimum(unsigned min);
    void setMinimum(const QSize& min);

protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual bool eventFilter(QObject* object, QEvent* event) override;

signals:
    void editingFinished();

private:
    QSpinBox* _width;
    QSpinBox* _height;
};
}
}
