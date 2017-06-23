/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sizewidget.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

using namespace UnTech;
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

    connect(_width, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(_height, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    _width->installEventFilter(this);
    _height->installEventFilter(this);
}

void SizeWidget::focusInEvent(QFocusEvent*)
{
    _width->setFocus();
    _width->selectAll();
}

bool SizeWidget::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const auto& key = keyEvent->key();

        if (key == Qt::Key_Backtab && _width->hasFocus()) {
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
        if (_width->hasFocus() == false && _height->hasFocus() == false) {
            QApplication::sendEvent(this, event);
            emit editingFinished();
            return false;
        }
    }

    return QWidget::eventFilter(object, event);
}

void SizeWidget::clear()
{
    _width->clear();
    _height->clear();
}

usize SizeWidget::valueUsize() const
{
    return usize(_width->value(), _height->value());
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
