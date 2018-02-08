/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "snescolordialog.h"
#include "gui-qt/snes/snescolordialog.ui.h"

#include <QColorDialog>

using namespace UnTech::GuiQt::Snes;

SnesColorDialog::SnesColorDialog(QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SnesColorDialog)
    , _color()
{
    _ui->setupUi(this);

    updateColorButton();

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SnesColorDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SnesColorDialog::reject);

    connect(_ui->redSlider, &QSlider::valueChanged,
            this, &SnesColorDialog::onRedSliderChanged);
    connect(_ui->redSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &SnesColorDialog::onRedSpinBoxChanged);

    connect(_ui->greenSlider, &QSlider::valueChanged,
            this, &SnesColorDialog::onGreenSliderChanged);
    connect(_ui->greenSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &SnesColorDialog::onGreenSpinBoxChanged);

    connect(_ui->blueSlider, &QSlider::valueChanged,
            this, &SnesColorDialog::onBlueSliderChanged);
    connect(_ui->blueSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &SnesColorDialog::onBlueSpinBoxChanged);

    connect(_ui->colorButton, &QToolButton::clicked,
            this, &SnesColorDialog::onColorButtonClicked);
}

SnesColorDialog::~SnesColorDialog() = default;

void SnesColorDialog::setColor(const SnesColor& color)
{
    if (_color != color) {
        _color = color;

        _ui->redSlider->setValue(_color.red());
        _ui->redSpinBox->setValue(_color.red());
        _ui->greenSlider->setValue(_color.green());
        _ui->greenSpinBox->setValue(_color.green());
        _ui->blueSlider->setValue(_color.blue());
        _ui->blueSpinBox->setValue(_color.blue());

        updateColorButton();

        emit colorChanged(_color);
    }
}

void SnesColorDialog::updateColorButton()
{
    QString colorHex = tr("0x%1").arg(_color.data(), 4, 16, QChar('0'));
    QColor bgColor(_color.rgb().rgbHex());
    QColor textColor = Qt::black;

    if (bgColor.lightness() <= 96) {
        textColor = Qt::white;
    }

    _ui->colorButton->setColor(bgColor);
    _ui->colorButton->setText(colorHex);

    auto p = _ui->colorButton->palette();
    p.setColor(QPalette::WindowText, textColor);
    p.setColor(QPalette::ButtonText, textColor);
    _ui->colorButton->setPalette(p);
}

void SnesColorDialog::onRedSliderChanged(int value)
{
    if (_color.red() != value) {
        _color.setRed(value);
        _ui->redSpinBox->setValue(value);

        updateColorButton();
        emit colorChanged(_color);
    }
}

void SnesColorDialog::onRedSpinBoxChanged(int value)
{
    if (_color.red() != value) {
        _color.setRed(value);
        _ui->redSlider->setValue(value);

        updateColorButton();
        emit colorChanged(_color);
    }
}

void SnesColorDialog::onGreenSliderChanged(int value)
{
    if (_color.green() != value) {
        _color.setGreen(value);
        _ui->greenSpinBox->setValue(value);

        updateColorButton();
        emit colorChanged(_color);
    }
}

void SnesColorDialog::onGreenSpinBoxChanged(int value)
{
    if (_color.green() != value) {
        _color.setGreen(value);
        _ui->greenSlider->setValue(value);

        updateColorButton();
        emit colorChanged(_color);
    }
}

void SnesColorDialog::onBlueSliderChanged(int value)
{
    if (_color.blue() != value) {
        _color.setBlue(value);
        _ui->blueSpinBox->setValue(value);

        updateColorButton();
        emit colorChanged(_color);
    }
}

void SnesColorDialog::onBlueSpinBoxChanged(int value)
{
    if (_color.blue() != value) {
        _color.setBlue(value);
        _ui->blueSlider->setValue(value);

        updateColorButton();
        emit colorChanged(_color);
    }
}

void SnesColorDialog::onColorButtonClicked()
{
    QColor color(_color.rgb().rgbHex());

    QColorDialog dialog(color, this);

    connect(&dialog, &QColorDialog::currentColorChanged,
            [this](const QColor& c) {
                setColor(SnesColor(rgba(c.red(), c.green(), c.blue())));
            });

    dialog.exec();

    if (dialog.result() == QDialog::Accepted) {
        QColor c = dialog.selectedColor();
        setColor(SnesColor(rgba(c.red(), c.green(), c.blue())));
    }
}
