/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/snes/snescolor.h"
#include <QDialog>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Snes {
namespace Ui {
class SnesColorDialog;
}

class SnesColorDialog : public QDialog {
    Q_OBJECT

    using SnesColor = UnTech::Snes::SnesColor;

public:
    SnesColorDialog(QWidget* parent = nullptr);
    ~SnesColorDialog();

    const SnesColor& color() const { return _color; }
    void setColor(const SnesColor& color);

signals:
    void colorChanged(const SnesColor& color);

private slots:
    void onRedSliderChanged(int value);
    void onRedSpinBoxChanged(int value);

    void onGreenSliderChanged(int value);
    void onGreenSpinBoxChanged(int value);

    void onBlueSliderChanged(int value);
    void onBlueSpinBoxChanged(int value);

    void onColorButtonClicked();

private:
    void updateColorButton();

private:
    std::unique_ptr<Ui::SnesColorDialog> const _ui;

    SnesColor _color;
};
}
}
}
