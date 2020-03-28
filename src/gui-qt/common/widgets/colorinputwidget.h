/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QLineEdit>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
class ColorToolButton;

class ColorInputWidget : public QWidget {
    Q_OBJECT

public:
    ColorInputWidget(QWidget* parent = nullptr);
    ~ColorInputWidget() = default;

    void setFrame(bool f);

    QColor color() const { return _color; }
    void setColor(const QColor& c);
    void clear();

    const QString& dialogTitle() const { return _dialogTitle; }
    void setDialogTitle(const QString& title);

public slots:
    void showDialog();

signals:
    void colorChanged(const QColor& c);
    void colorSelected(const QColor& c);

private:
    QLineEdit* _lineEdit;
    ColorToolButton* _button;
    QColor _color;
    QString _dialogTitle;
};
}
}
