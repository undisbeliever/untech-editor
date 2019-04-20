/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "ms8pointwidget.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

using namespace UnTech;
using namespace UnTech::GuiQt;

Ms8pointWidget::Ms8pointWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1, 1, 1, 1);

    this->setLayout(layout);

    _xPos = new QSpinBox(this);
    _xPos->setRange(int_ms8_t::MIN, int_ms8_t::MAX);
    layout->addWidget(_xPos, 1);

    layout->addWidget(new QLabel(", ", this));

    _yPos = new QSpinBox(this);
    _yPos->setRange(int_ms8_t::MIN, int_ms8_t::MAX);
    layout->addWidget(_yPos, 1);

    this->setMinimumSize(layout->minimumSize());

    _xPos->setFocusPolicy(Qt::WheelFocus);
    _yPos->setFocusPolicy(Qt::WheelFocus);

    this->setTabOrder(_xPos, _yPos);

    _xPos->installEventFilter(this);
    _yPos->installEventFilter(this);
}

void Ms8pointWidget::focusInEvent(QFocusEvent*)
{
    _xPos->setFocus();
    _xPos->selectAll();
}

bool Ms8pointWidget::eventFilter(QObject* object, QEvent* event)
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

void Ms8pointWidget::clear()
{
    _xPos->clear();
    _yPos->clear();
}

QPoint Ms8pointWidget::value() const
{
    return QPoint(_xPos->value(), _yPos->value());
}

void Ms8pointWidget::setValue(const QPoint& p)
{
    _xPos->setValue(p.x());
    _yPos->setValue(p.y());
}

ms8point Ms8pointWidget::valueMs8point() const
{
    return ms8point(_xPos->value(), _yPos->value());
}

void Ms8pointWidget::setValue(const ms8point& p)
{
    _xPos->setValue(p.x);
    _yPos->setValue(p.y);
}
