/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetdock.h"
#include "document.h"
#include "gui-qt/metasprite/spriteimporter/framesetdock.ui.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;
using TilesetType = UnTech::MetaSprite::TilesetType;

FrameSetDock::FrameSetDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _document(nullptr)
{
    _ui->setupUi(this);

    _ui->nPalettes->setMaximum(UnTech::MetaSprite::MAX_PALETTES);
    _ui->paletteSize->setMaximum(32);

    _ui->tilesetType->populateData(TilesetType::enumMap);

    clearGui();
    setEnabled(false);
}

FrameSetDock::~FrameSetDock() = default;

void FrameSetDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        updateGui();
    }
    else {
        clearGui();
    }
}

void FrameSetDock::clearGui()
{
    _ui->frameSetName->clear();
    _ui->tilesetType->setCurrentIndex(-1);
    _ui->exportOrder->clear();
    _ui->frameSetType->clear();
    _ui->imageFilename->clear();
    _ui->transparent->clear();
    _ui->transparentButton->setStyleSheet(QString());
    _ui->gridSize->clear();
    _ui->gridOffset->clear();
    _ui->gridPadding->clear();
    _ui->gridOrigin->clear();
    _ui->userSuppliedPaletteBox->setChecked(false);
    _ui->nPalettes->clear();
    _ui->paletteSize->clear();
}

void FrameSetDock::updateGui()
{
    const SI::FrameSet& fs = *_document->frameSet();

    _ui->frameSetName->setText(QString::fromStdString(fs.name));

    _ui->tilesetType->setCurrentEnum(fs.tilesetType);

    if (fs.exportOrder) {
        _ui->exportOrder->setText(QString::fromStdString(fs.exportOrder->filename));
        _ui->frameSetType->setText(QString::fromStdString(fs.exportOrder->name));
    }
    else {
        _ui->exportOrder->clear();
        _ui->frameSetType->clear();
    }

    _ui->imageFilename->setText(QString::fromStdString(fs.imageFilename));

    if (fs.transparentColorValid()) {
        QColor c(fs.transparentColor.rgbHex());
        QString colorHex = c.name();

        _ui->transparent->setText(colorHex);
        _ui->transparentButton->setStyleSheet(
            QString("QToolButton{ background: %1; }").arg(colorHex));
    }
    else {
        _ui->transparent->clear();
        _ui->transparentButton->setStyleSheet(QString());
    }

    _ui->gridSize->setValue(fs.grid.frameSize);
    _ui->gridOffset->setValue(fs.grid.offset);
    _ui->gridPadding->setValue(fs.grid.padding);

    _ui->gridOrigin->setMaximum(fs.grid.originRange());
    _ui->gridOrigin->setValue(fs.grid.origin);

    _ui->userSuppliedPaletteBox->setChecked(fs.palette.usesUserSuppliedPalette());
    _ui->nPalettes->setValue(fs.palette.nPalettes);
    _ui->paletteSize->setValue(fs.palette.colorSize);
}
