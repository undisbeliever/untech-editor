/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "idstringdialog.h"
#include "idstringvalidator.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

using namespace UnTech;
using namespace UnTech::GuiQt;

IdstringDialog::IdstringDialog(const QString& title, const QString& labelText,
                               QWidget* parent)
    : QDialog(parent)
    , _invalidIdstrings()
{
    using DBB = QDialogButtonBox;

    setWindowTitle(title);

    QVBoxLayout* layout = new QVBoxLayout;
    this->setLayout(layout);

    _label = new QLabel(labelText, this);
    layout->addWidget(_label);

    _input = new QLineEdit(this);
    _input->setValidator(new IdstringValidator(this));
    layout->addWidget(_input);

    _label->setBuddy(_input);

    auto* buttonBox = new QDialogButtonBox(DBB::Ok | DBB::Cancel, this);
    layout->addWidget(buttonBox);

    _okButton = buttonBox->button(DBB::Ok);
    onInputChanged();

    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &IdstringDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &IdstringDialog::reject);
    connect(_input, &QLineEdit::textChanged,
            this, &IdstringDialog::onInputChanged);
}

idstring IdstringDialog::getIdstring(QWidget* parent,
                                     const QString& title, const QString& labelText,
                                     const idstring& value,
                                     const QStringList& invalidIdstrings)
{
    return getIdstring(parent, title, labelText,
                       QString::fromStdString(value),
                       invalidIdstrings);
}

idstring IdstringDialog::getIdstring(QWidget* parent,
                                     const QString& title, const QString& labelText,
                                     const QString& value,
                                     const QStringList& invalidIdstrings)
{
    IdstringDialog dialog(title, labelText, parent);
    dialog.setInvalidIdstrings(invalidIdstrings);
    dialog.setValue(value);
    int r = dialog.exec();

    if (r == QDialog::Accepted && dialog.isValid()) {
        return dialog.value();
    }
    else {
        return idstring();
    }
}

bool IdstringDialog::isValid() const
{
    if (_invalidIdstrings.contains(_input->text())) {
        return false;
    }
    return _input->text().isEmpty() == false;
}

void IdstringDialog::onInputChanged()
{
    _okButton->setEnabled(isValid());
}

void IdstringDialog::setInvalidIdstrings(const QStringList& invalidIdstrings)
{
    _invalidIdstrings = invalidIdstrings;
    onInputChanged();
}

void IdstringDialog::setLabelText(const QString& text)
{
    _label->setText(text);
}

void IdstringDialog::setValue(const idstring& id)
{
    _input->setText(QString::fromStdString(id));
    _input->selectAll();
}

void IdstringDialog::setValue(const QString& id)
{
    _input->setText(id);
    _input->selectAll();
}

idstring IdstringDialog::value() const
{
    return _input->text().toStdString();
}

void IdstringDialog::setTextValue(const QString& id)
{
    _input->setText(id);
    _input->selectAll();
}

QString IdstringDialog::textValue() const
{
    return _input->text();
}
