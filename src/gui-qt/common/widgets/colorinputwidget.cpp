/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "colorinputwidget.h"
#include "colortoolbutton.h"

#include <QColorDialog>
#include <QHBoxLayout>

using namespace UnTech::GuiQt;

ColorInputWidget::ColorInputWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(0);

    _lineEdit = new QLineEdit(this);
    _lineEdit->setReadOnly(true);
    layout->addWidget(_lineEdit);

    _button = new ColorToolButton(this);
    _button->setText(QStringLiteral("..."));
    _button->setToolTip(tr("Open Color Dialog"));
    layout->addWidget(_button);

    connect(_button, &QToolButton::clicked,
            this, &ColorInputWidget::showDialog);
    connect(_lineEdit, &QLineEdit::textChanged,
            this, &ColorInputWidget::colorChanged);
}

void ColorInputWidget::setFrame(bool f)
{
    _lineEdit->setFrame(f);
}

void ColorInputWidget::setColor(const QColor& c)
{
    if (_color != c) {
        _color = c;
        if (c.isValid()) {
            _lineEdit->setText(c.name());
            _button->setColor(c);
        }
        else {
            _lineEdit->clear();
            _button->unsetColor();
        }

        emit colorChanged(_color);
    }
}

void ColorInputWidget::clear()
{
    bool emitSignal = _color.isValid();

    _color = QColor();
    _lineEdit->clear();
    _button->unsetColor();

    if (emitSignal) {
        emit colorChanged(_color);
    }
}

void ColorInputWidget::setDialogTitle(const QString& title)
{
    _dialogTitle = title;
}

void ColorInputWidget::showDialog()
{
    QColor c = QColorDialog::getColor(
        color(), this, _dialogTitle);

    if (c.isValid()) {
        setColor(c);
        emit colorSelected(c);
    }
}
