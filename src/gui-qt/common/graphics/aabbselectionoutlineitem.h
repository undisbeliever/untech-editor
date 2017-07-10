/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QBasicTimer>
#include <QGraphicsObject>

namespace UnTech {
namespace GuiQt {

class AabbSelectionOutlineItem : public QGraphicsObject {
public:
    static constexpr qreal SELECTION_OUTLINE_ZVALUE = 9999999;

public:
    explicit AabbSelectionOutlineItem(QGraphicsItem* parent = nullptr);
    ~AabbSelectionOutlineItem() = default;

    void setRect(const QRectF& drawRect);

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

    virtual void timerEvent(QTimerEvent* event) override;

private:
    QRectF _rect;
    qreal _dashOffset;
    QBasicTimer _timer;
};
}
}
