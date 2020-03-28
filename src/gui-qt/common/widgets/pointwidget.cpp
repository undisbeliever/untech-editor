/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "pointwidget.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
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

    this->setMinimumSize(layout->minimumSize());

    _xPos->setFocusPolicy(Qt::WheelFocus);
    _yPos->setFocusPolicy(Qt::WheelFocus);

    this->setTabOrder(_xPos, _yPos);

    setMinimum(0);
    setMaximum(255);

    _xPos->installEventFilter(this);
    _yPos->installEventFilter(this);
}

void PointWidget::focusInEvent(QFocusEvent*)
{
    _xPos->setFocus();
    _xPos->selectAll();
}

bool PointWidget::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const auto& key = keyEvent->key();

        if (key == Qt::Key_Backtab && _xPos->hasFocus()) {
            QApplication::sendEvent(this, event);
            emit editingFinished();
            return true;
        }
        else if (key == Qt::Key_Tab && _yPos->hasFocus()) {
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
        if (_xPos->hasFocus() == false && _yPos->hasFocus() == false) {
            QApplication::sendEvent(this, event);
            emit editingFinished();
            return false;
        }
    }

    return QWidget::eventFilter(object, event);
}

void PointWidget::clear()
{
    _xPos->clear();
    _yPos->clear();
}

QPoint PointWidget::value() const
{
    return QPoint(_xPos->value(), _yPos->value());
}

void PointWidget::setValue(const QPoint& p)
{
    _xPos->setValue(p.x());
    _yPos->setValue(p.y());
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

void PointWidget::setMinimum(const QPoint& min)
{
    _xPos->setMinimum(min.x());
    _yPos->setMinimum(min.y());
}

void PointWidget::setMaximum(int max)
{
    _xPos->setMaximum(max);
    _yPos->setMaximum(max);
}

void PointWidget::setMaximum(const QPoint& max)
{
    _xPos->setMaximum(max.x());
    _yPos->setMaximum(max.y());
}

void PointWidget::setMaximum(const usize& max)
{
    _xPos->setMaximum(max.width);
    _yPos->setMaximum(max.height);
}

void PointWidget::setMaximum(const QSize& max)
{
    _xPos->setMaximum(max.width());
    _yPos->setMaximum(max.height());
}
