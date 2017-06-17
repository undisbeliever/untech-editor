/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "pointwidget.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace UnTech;
using namespace UnTech::GuiQt;

PointWidget::PointWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1, 1, 1, 1);

    this->setLayout(layout);

    _xPos = new QSpinBox(this);
    layout->addWidget(_xPos, 1);

    layout->addWidget(new QLabel(", ", this));

    _yPos = new QSpinBox(this);
    layout->addWidget(_yPos, 1);

    _xPos->setFocusPolicy(Qt::WheelFocus);
    _yPos->setFocusPolicy(Qt::WheelFocus);

    this->setTabOrder(_xPos, _yPos);

    setMinimum(0);
    setMaximum(255);

    connect(_xPos, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(_yPos, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}

void PointWidget::clear()
{
    _xPos->clear();
    _yPos->clear();
}

upoint PointWidget::valueUpoint() const
{
    return upoint(_xPos->value(), _yPos->value());
}

void PointWidget::setValue(const upoint& p)
{
    _xPos->setValue(p.x);
    _yPos->setValue(p.y);
}

void PointWidget::setMinimum(int min)
{
    _xPos->setMinimum(min);
    _yPos->setMinimum(min);
}

void PointWidget::setMaximum(int max)
{
    _xPos->setMaximum(max);
    _yPos->setMaximum(max);
}

void PointWidget::setMaximum(const usize& max)
{
    _xPos->setMaximum(max.width);
    _yPos->setMaximum(max.height);
}
