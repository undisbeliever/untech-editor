/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rectwidget.h"

#include <QGridLayout>
#include <QLabel>

using namespace UnTech::GuiQt;

RectWidget::RectWidget(QWidget* parent)
    : QWidget(parent)
    , _range(INT_MAX, INT_MAX)
    , _minRectSize(1, 1)
    , _maxRectSize(255, 255)
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1, 1, 1, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);

    this->setLayout(layout);

    _xPos = new QSpinBox(this);
    layout->addWidget(_xPos, 0, 0);

    auto* comma = new QLabel(", ", this);
    layout->addWidget(comma, 0, 1);

    _yPos = new QSpinBox(this);
    layout->addWidget(_yPos, 0, 2);

    _width = new QSpinBox(this);
    layout->addWidget(_width, 1, 0);

    auto* cross = new QLabel("x", this);
    layout->addWidget(cross, 1, 1);

    _height = new QSpinBox(this);
    layout->addWidget(_height, 1, 2);

    this->setMinimumSize(layout->minimumSize());

    _xPos->setFocusPolicy(Qt::WheelFocus);
    _yPos->setFocusPolicy(Qt::WheelFocus);
    _width->setFocusPolicy(Qt::WheelFocus);
    _height->setFocusPolicy(Qt::WheelFocus);

    this->setTabOrder(_xPos, _yPos);
    this->setTabOrder(_yPos, _width);
    this->setTabOrder(_width, _height);

    updateRanges();

    connect(_xPos, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalRange()));
    connect(_width, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalRange()));
    connect(_yPos, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalRange()));
    connect(_height, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalRange()));
}

void RectWidget::clear()
{
    _xPos->clear();
    _yPos->clear();
    _width->clear();
    _height->clear();
}

void RectWidget::setValue(const urect& r)
{
    _xPos->setValue(r.x);
    _yPos->setValue(r.y);
    _width->setValue(r.width);
    _height->setValue(r.height);

    updateRanges();
}

void RectWidget::setRange(const usize& range)
{
    _range = range;
    updateRanges();
}

void RectWidget::setMinRectSize(const usize& minRectSize)
{
    _minRectSize = minRectSize;
    updateRanges();
}

void RectWidget::setMaxRectSize(const usize& maxRectSize)
{
    _maxRectSize = maxRectSize;
    updateRanges();
}

void RectWidget::updateRanges()
{
    updateHorizontalRange();
    updateVerticalRange();
}

void RectWidget::updateHorizontalRange()
{
    _xPos->setRange(0, _range.width - _width->value());

    _width->setRange(_minRectSize.width,
                     std::min(_maxRectSize.width, _range.width - _xPos->value()));
}

void RectWidget::updateVerticalRange()
{
    _yPos->setRange(0, _range.height - _height->value());

    _height->setRange(_minRectSize.height,
                      std::min(_maxRectSize.height, _range.height - _yPos->value()));
}
