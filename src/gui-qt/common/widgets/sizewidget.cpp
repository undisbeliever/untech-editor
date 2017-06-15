/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sizewidget.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace UnTech::GuiQt;

SizeWidget::SizeWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1, 1, 1, 1);

    this->setLayout(layout);

    _width = new QSpinBox(this);
    layout->addWidget(_width, 1);

    layout->addWidget(new QLabel("x", this));

    _height = new QSpinBox(this);
    layout->addWidget(_height, 1);

    _width->setFocusPolicy(Qt::WheelFocus);
    _height->setFocusPolicy(Qt::WheelFocus);

    this->setTabOrder(_width, _height);

    setMinimum(1);
    setMaximum(255);
}

void SizeWidget::clear()
{
    _width->clear();
    _height->clear();
}

void SizeWidget::setValue(const usize& s)
{
    _width->setValue(s.width);
    _height->setValue(s.height);
}

void SizeWidget::setMaximum(unsigned max)
{
    _width->setMaximum(max);
    _height->setMaximum(max);
}

void SizeWidget::setMinimum(unsigned min)
{
    _width->setMinimum(min);
    _height->setMinimum(min);
}
