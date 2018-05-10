/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetdock.h"
#include "accessors.h"
#include "document.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/spriteimporter/framesetdock.ui.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;
using TilesetType = UnTech::MetaSprite::TilesetType;

FrameSetDock::FrameSetDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _document(nullptr)
{
    _ui->setupUi(this);

    _ui->frameSetName->setValidator(new IdstringValidator(this));
    _ui->exportOrder->setValidator(new IdstringValidator(this));

    _ui->imageFilename->setDialogTitle(tr("Open Image"));
    _ui->imageFilename->setDialogFilter(tr("PNG Image (*.png);;All Files (*)"));

    _ui->transparent->setDialogTitle(tr("Select Transparent Color"));

    _ui->nPalettes->setMaximum(UnTech::MetaSprite::MAX_PALETTES);
    _ui->paletteSize->setMaximum(32);

    _ui->tilesetType->populateData(TilesetType::enumMap);

    _ui->frameList->idmapActions().populateToolbar(_ui->frameListButtons);

    clearGui();
    setEnabled(false);

    connect(_ui->frameSetName, &QLineEdit::editingFinished,
            this, &FrameSetDock::onNameEdited);
    connect(_ui->tilesetType, qOverload<int>(&EnumComboBox::activated),
            this, &FrameSetDock::onTilesetTypeEdited);
    connect(_ui->exportOrder, &QLineEdit::editingFinished,
            this, &FrameSetDock::onExportOrderEdited);

    connect(_ui->imageFilename, &FilenameInputWidget::fileSelected,
            this, &FrameSetDock::onImageFilenameFileSelected);
    connect(_ui->transparent, &ColorInputWidget::colorSelected,
            this, &FrameSetDock::onTransparentColorSelected);

    connect(_ui->gridSize, &SizeWidget::editingFinished,
            this, &FrameSetDock::onGridEdited);
    connect(_ui->gridOffset, &PointWidget::editingFinished,
            this, &FrameSetDock::onGridEdited);
    connect(_ui->gridPadding, &PointWidget::editingFinished,
            this, &FrameSetDock::onGridEdited);
    connect(_ui->gridOrigin, &PointWidget::editingFinished,
            this, &FrameSetDock::onGridEdited);

    connect(_ui->userSuppliedPaletteBox, &QGroupBox::clicked,
            this, &FrameSetDock::onPaletteEdited);
    connect(_ui->nPalettes, &QSpinBox::editingFinished,
            this, &FrameSetDock::onPaletteEdited);
    connect(_ui->paletteSize, &QSpinBox::editingFinished,
            this, &FrameSetDock::onPaletteEdited);
}

FrameSetDock::~FrameSetDock() = default;

void FrameSetDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        updateGui();

        _ui->frameList->setAccessor(_document->frameMap());

        connect(_document, &Document::frameSetDataChanged,
                this, &FrameSetDock::updateGui);
    }
    else {
        clearGui();

        _ui->frameList->setAccessor<FrameMap>(nullptr);
    }
}

const Accessor::IdmapActions& FrameSetDock::frameActions() const
{
    return _ui->frameList->idmapActions();
}

Accessor::IdmapListModel* FrameSetDock::frameListModel() const
{
    return _ui->frameList->idmapListModel();
}

void FrameSetDock::populateMenu(QMenu* menu)
{
    _ui->frameList->idmapActions().populateMenu(menu);
    // :: TODO add toggle tileset hitbox here::
}

void FrameSetDock::clearGui()
{
    _ui->frameSetName->clear();
    _ui->tilesetType->setCurrentIndex(-1);
    _ui->exportOrder->clear();
    _ui->imageFilename->clear();
    _ui->transparent->clear();
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
    _ui->exportOrder->setText(QString::fromStdString(fs.exportOrder));

    _ui->imageFilename->setFilename(QString::fromStdString(fs.imageFilename));

    if (fs.transparentColorValid()) {
        QColor c(fs.transparentColor.rgbHex());
        _ui->transparent->setColor(c);
    }
    else {
        _ui->transparent->clear();
    }

    _ui->gridSize->setValue(fs.grid.frameSize);
    _ui->gridOffset->setValue(fs.grid.offset);
    _ui->gridPadding->setValue(fs.grid.padding);

    _ui->gridOrigin->setMaximum(fs.grid.originRange());
    _ui->gridOrigin->setValue(fs.grid.origin);

    _ui->userSuppliedPaletteBox->setChecked(fs.palette.usesUserSuppliedPalette());
    if (fs.palette.usesUserSuppliedPalette()) {
        _ui->nPalettes->setValue(fs.palette.nPalettes);
        _ui->paletteSize->setValue(fs.palette.colorSize);
    }
    else {
        _ui->nPalettes->clear();
        _ui->paletteSize->clear();
    }
}

void FrameSetDock::onNameEdited()
{
    idstring name = _ui->frameSetName->text().toStdString();
    if (name.isValid()) {
        FrameSetUndoHelper(_document)
            .editField(name, tr("Edit FrameSet Name"),
                       [](SI::FrameSet& fs) -> idstring& { return fs.name; },
                       [](Document& d) { emit d.frameSetNameChanged();
                                         emit d.frameSetDataChanged(); });
    }
    else {
        updateGui();
    }
}

void FrameSetDock::onTilesetTypeEdited()
{
    TilesetType ts = _ui->tilesetType->currentEnum<TilesetType>();
    FrameSetUndoHelper(_document)
        .editField(ts, tr("Edit Tileset Type"),
                   [](SI::FrameSet& fs) -> TilesetType& { return fs.tilesetType; },
                   [](Document& d) { emit d.frameSetDataChanged(); });
}

void FrameSetDock::onExportOrderEdited()
{
    idstring eo = _ui->exportOrder->text().toStdString();
    FrameSetUndoHelper(_document)
        .editField(eo, tr("Edit Export Order"),
                   [](SI::FrameSet& fs) -> idstring& { return fs.exportOrder; },
                   [](Document& d) { emit d.frameSetDataChanged(); });
}

void FrameSetDock::onImageFilenameFileSelected()
{
    const std::string filename = _ui->imageFilename->filename().toStdString();

    if (filename.empty()) {
        return;
    }

    FrameSetUndoHelper(_document)
        .editField(filename, tr("Change Image"),
                   [](SI::FrameSet& fs) -> std::string& { return fs.imageFilename; },
                   [](Document& d) {
                       emit d.frameSetImageFilenameChanged();
                   });
}

void FrameSetDock::onTransparentColorSelected()
{
    QColor color = _ui->transparent->color();
    rgba tc(color.red(), color.green(), color.blue());

    FrameSetUndoHelper(_document)
        .editField(tc, tr("Edit Transparent Color"),
                   [](SI::FrameSet& fs) -> rgba& { return fs.transparentColor; },
                   [](Document& d) { emit d.frameSetDataChanged(); });
}

void FrameSetDock::onGridEdited()
{
    SI::FrameSetGrid grid;

    grid.frameSize = _ui->gridSize->valueUsize();
    grid.offset = _ui->gridOffset->valueUpoint();
    grid.padding = _ui->gridPadding->valueUpoint();
    grid.origin = _ui->gridOrigin->valueUpoint();

    FrameSetUndoHelper(_document)
        .editField(grid, tr("Edit FrameSet Grid"),
                   [](SI::FrameSet& fs) -> SI::FrameSetGrid& { return fs.grid; },
                   [](Document& d) { emit d.frameSetGridChanged();
                                     emit d.frameSetDataChanged(); });
}

void FrameSetDock::onPaletteEdited()
{
    SI::UserSuppliedPalette palette;
    if (_ui->userSuppliedPaletteBox->isChecked()) {
        palette.nPalettes = _ui->nPalettes->value();
        palette.colorSize = _ui->paletteSize->value();
    }

    FrameSetUndoHelper(_document)
        .editField(palette, tr("Edit FrameSet Palette"),
                   [](SI::FrameSet& fs) -> SI::UserSuppliedPalette& { return fs.palette; },
                   [](Document& d) { emit d.frameSetPaletteChanged();
                                     emit d.frameSetDataChanged(); });
}
