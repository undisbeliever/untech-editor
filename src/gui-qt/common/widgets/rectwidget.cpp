/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rectwidget.h"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>

using namespace UnTech;
using namespace UnTech::GuiQt;

RectWidget::RectWidget(QWidget* parent)
    : QWidget(parent)
    , _minRectSize(1, 1)
    , _maxRectSize(255, 255)
    , _range(0, 0, INT_MAX, INT_MAX)
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

    connect(_xPos, qOverload<int>(&QSpinBox::valueChanged),
            this, &RectWidget::updateHorizontalRange);
    connect(_width, qOverload<int>(&QSpinBox::valueChanged),
            this, &RectWidget::updateHorizontalRange);
    connect(_yPos, qOverload<int>(&QSpinBox::valueChanged),
            this, &RectWidget::updateVerticalRange);
    connect(_height, qOverload<int>(&QSpinBox::valueChanged),
            this, &RectWidget::updateVerticalRange);

    _xPos->installEventFilter(this);
    _yPos->installEventFilter(this);
    _width->installEventFilter(this);
    _height->installEventFilter(this);
}

void RectWidget::focusInEvent(QFocusEvent*)
{
    _xPos->setFocus();
    _xPos->selectAll();
}

bool RectWidget::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const auto& key = keyEvent->key();

        if (key == Qt::Key_Backtab && _xPos->hasFocus()) {
            QApplication::sendEvent(this, event);
            emit editingFinished();
            return true;
        }
        else if (key == Qt::Key_Tab && _height->hasFocus()) {
            QApplication::sendEvent(this, event);
            emit editingFinished();
            return true;
        }
        else if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            emit editingFinished();
            return false;
        }
    }
    else if (event->type() == QEvent::FocusOut) {
        if (_xPos->hasFocus() == false && _yPos->hasFocus() == false
            && _width->hasFocus() == false && _height->hasFocus() == false) {

            QApplication::sendEvent(this, event);
            emit editingFinished();
            return false;
        }
    }

    return QWidget::eventFilter(object, event);
}

void RectWidget::clear()
{
    _xPos->clear();
    _yPos->clear();
    _width->clear();
    _height->clear();
}

QRect RectWidget::value() const
{
    return QRect(_xPos->value(), _yPos->value(),
                 _width->value(), _height->value());
}

void RectWidget::setValue(const QRect& r)
{
    _xPos->setValue(r.x());
    _yPos->setValue(r.y());
    _width->setValue(r.width());
    _height->setValue(r.height());

    updateRanges();
}

urect RectWidget::valueUrect() const
{
    return urect(_xPos->value(), _yPos->value(),
                 _width->value(), _height->value());
}

void RectWidget::setValue(const urect& r)
{
    _xPos->setValue(r.x);
    _yPos->setValue(r.y);
    _width->setValue(r.width);
    _height->setValue(r.height);

    updateRanges();
}

void RectWidget::setRange(const QRect& range)
{
    _range = range;
    updateRanges();
}

void RectWidget::setRange(const usize& range)
{
    _range = QRect(0, 0, range.width, range.height);
    updateRanges();
}

void RectWidget::setRange(const QSize& range)
{
    _range = QRect(0, 0, range.width(), range.height());
    updateRanges();
}

void RectWidget::setMinRectSize(const QSize& minRectSize)
{
    _minRectSize = minRectSize;
    updateRanges();
}

void RectWidget::setMinRectSize(const usize& minRectSize)
{
    _minRectSize = QSize(minRectSize.width, minRectSize.height);
    updateRanges();
}

void RectWidget::setMaxRectSize(const QSize& maxRectSize)
{
    _maxRectSize = maxRectSize;
    updateRanges();
}

void RectWidget::setMaxRectSize(const usize& maxRectSize)
{
    _minRectSize = QSize(maxRectSize.width, maxRectSize.height);
    updateRanges();
}

void RectWidget::updateRanges()
{
    updateHorizontalRange();
    updateVerticalRange();
}

void RectWidget::updateHorizontalRange()
{
    _xPos->setRange(_range.left(), _range.right() - _width->value() + 1);

    _width->setRange(_minRectSize.width(),
                     std::min(_maxRectSize.width(), _range.right() - _xPos->value() + 1));
}

void RectWidget::updateVerticalRange()
{
    _yPos->setRange(_range.top(), _range.bottom() - _height->value() + 1);

    _height->setRange(_minRectSize.height(),
                      std::min(_maxRectSize.height(), _range.bottom() - _yPos->value() + 1));
}
