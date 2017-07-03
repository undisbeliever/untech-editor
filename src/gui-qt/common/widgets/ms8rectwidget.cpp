/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "ms8rectwidget.h"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>

using namespace UnTech;
using namespace UnTech::GuiQt;

Ms8rectWidget::Ms8rectWidget(QWidget* parent)
    : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1, 1, 1, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);

    this->setLayout(layout);

    _xPos = new QSpinBox(this);
    _xPos->setRange(int_ms8_t::MIN, int_ms8_t::MAX);
    layout->addWidget(_xPos, 0, 0);

    auto* comma = new QLabel(", ", this);
    layout->addWidget(comma, 0, 1);

    _yPos = new QSpinBox(this);
    _yPos->setRange(int_ms8_t::MIN, int_ms8_t::MAX);
    layout->addWidget(_yPos, 0, 2);

    _width = new QSpinBox(this);
    _width->setRange(1, UINT8_MAX);
    layout->addWidget(_width, 1, 0);

    auto* cross = new QLabel("x", this);
    layout->addWidget(cross, 1, 1);

    _height = new QSpinBox(this);
    _height->setRange(1, UINT8_MAX);
    layout->addWidget(_height, 1, 2);

    this->setMinimumSize(layout->minimumSize());

    _xPos->setFocusPolicy(Qt::WheelFocus);
    _yPos->setFocusPolicy(Qt::WheelFocus);
    _width->setFocusPolicy(Qt::WheelFocus);
    _height->setFocusPolicy(Qt::WheelFocus);

    this->setTabOrder(_xPos, _yPos);
    this->setTabOrder(_yPos, _width);
    this->setTabOrder(_width, _height);

    _xPos->installEventFilter(this);
    _yPos->installEventFilter(this);
    _width->installEventFilter(this);
    _height->installEventFilter(this);
}

void Ms8rectWidget::focusInEvent(QFocusEvent*)
{
    _xPos->setFocus();
    _xPos->selectAll();
}

bool Ms8rectWidget::eventFilter(QObject* object, QEvent* event)
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

void Ms8rectWidget::clear()
{
    _xPos->clear();
    _yPos->clear();
    _width->clear();
    _height->clear();
}

QRect Ms8rectWidget::value() const
{
    return QRect(_xPos->value(), _yPos->value(),
                 _width->value(), _height->value());
}

void Ms8rectWidget::setValue(const QRect& r)
{
    _xPos->setValue(r.x());
    _yPos->setValue(r.y());
    _width->setValue(r.width());
    _height->setValue(r.height());
}

ms8rect Ms8rectWidget::valueMs8rect() const
{
    return ms8rect(_xPos->value(), _yPos->value(),
                   _width->value(), _height->value());
}

void Ms8rectWidget::setValue(const ms8rect& r)
{
    _xPos->setValue(r.x);
    _yPos->setValue(r.y);
    _width->setValue(r.width);
    _height->setValue(r.height);
}
