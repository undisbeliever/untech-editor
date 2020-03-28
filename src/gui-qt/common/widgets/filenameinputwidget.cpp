/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "filenameinputwidget.h"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>

using namespace UnTech::GuiQt;

FilenameInputWidget::FilenameInputWidget(QWidget* parent)
    : QWidget(parent)
    , _dialogTitle()
    , _dialogFilter()
    , _blankFilenameAccepted(false)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(0);

    _lineEdit = new QLineEdit(this);
    _lineEdit->setReadOnly(true);
    layout->addWidget(_lineEdit);

    _button = new QToolButton(this);
    _button->setText(QStringLiteral("..."));
    _button->setToolTip(tr("Select File"));
    layout->addWidget(_button);

    connect(_button, &QToolButton::clicked,
            this, &FilenameInputWidget::showDialog);
    connect(_lineEdit, &QLineEdit::textChanged,
            this, &FilenameInputWidget::filenameChanged);
}

void FilenameInputWidget::setFrame(bool b)
{
    _lineEdit->setFrame(b);
}

void FilenameInputWidget::setFilename(const QString& filename)
{
    _lineEdit->setText(filename);
}

void FilenameInputWidget::clear()
{
    _lineEdit->clear();
}

void FilenameInputWidget::setDialogTitle(const QString& title)
{
    _dialogTitle = title;
}

void FilenameInputWidget::setDialogFilter(const QString& filter)
{
    _dialogFilter = filter;
}

void FilenameInputWidget::setBlankFilenameAccepted(bool allowBlankFilename)
{
    _blankFilenameAccepted = allowBlankFilename;
}

void FilenameInputWidget::showDialog()
{
    // DontUseNativeDialog is required to prevent a segfault on my Win7 VM
    const QString fn = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(
            this, _dialogTitle, filename(), _dialogFilter,
            nullptr, QFileDialog::DontUseNativeDialog));

    if (_blankFilenameAccepted || !fn.isNull()) {
        setFilename(fn);
        emit fileSelected(fn);
    }
}
