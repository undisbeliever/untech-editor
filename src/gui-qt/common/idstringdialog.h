/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace UnTech {
namespace GuiQt {

class IdstringDialog : public QDialog {
    Q_OBJECT

public:
    explicit IdstringDialog(const QString& title = QString(),
                            const QString& labelText = QString(),
                            QWidget* parent = nullptr);
    ~IdstringDialog() = default;

    static idstring getIdstring(QWidget* parent,
                                const QString& title,
                                const QString& labelText,
                                const idstring& value = idstring(),
                                const QStringList& invalidIdstrings = QStringList());

    void setInvalidIdstrings(const QStringList& invalidIdstrings);

    void setLabelText(const QString& text);

    void setValue(const idstring& id);
    idstring value() const;

    void setTextValue(const QString& id);
    QString textValue() const;

    bool isValid() const;

private slots:
    void onInputChanged();

private:
    QLabel* _label;
    QLineEdit* _input;
    QPushButton* _okButton;

    QStringList _invalidIdstrings;
};
}
}
