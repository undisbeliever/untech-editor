/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetdock.h"
#include "actions.h"
#include "document.h"
#include "framelistmodel.h"
#include "framesetcommands.h"
#include "selection.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/spriteimporter/framesetdock.ui.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;
using TilesetType = UnTech::MetaSprite::TilesetType;

FrameSetDock::FrameSetDock(FrameListModel* frameListModel, Actions* actions,
                           QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _frameListModel(frameListModel)
    , _actions(actions)
    , _document(nullptr)
{
    Q_ASSERT(frameListModel);
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->frameSetName->setValidator(new IdstringValidator(this));
    _ui->exportOrder->setValidator(new IdstringValidator(this));

    _ui->imageFilename->setDialogTitle(tr("Open Image"));
    _ui->imageFilename->setDialogFilter(tr("PNG Image (*.png);;All Files (*)"));

    _ui->transparent->setDialogTitle(tr("Select Transparent Color"));

    _ui->nPalettes->setMaximum(UnTech::MetaSprite::MAX_PALETTES);
    _ui->paletteSize->setMaximum(32);

    _ui->tilesetType->populateData(TilesetType::enumMap);

    _ui->frameList->setModel(_frameListModel);
    _ui->frameList->setContextMenuPolicy(Qt::CustomContextMenu);

    _ui->frameListButtons->addAction(_actions->addFrame());
    _ui->frameListButtons->addAction(_actions->cloneFrame());
    _ui->frameListButtons->addAction(_actions->renameFrame());
    _ui->frameListButtons->addAction(_actions->removeFrame());

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

    connect(_ui->frameList, &QListView::customContextMenuRequested,
            this, &FrameSetDock::onFrameContextMenu);
}

FrameSetDock::~FrameSetDock() = default;

void FrameSetDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        updateGui();
        updateFrameListSelection();

        connect(_document, &Document::frameSetDataChanged,
                this, &FrameSetDock::updateGui);

        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &FrameSetDock::updateFrameListSelection);

        connect(_ui->frameList->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &FrameSetDock::onFrameListSelectionChanged);
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
    const SI::FrameSet& fs = *_document->frameSet();

    idstring name = _ui->frameSetName->text().toStdString();
    if (name.isValid() && name != fs.name) {
        _document->undoStack()->push(
            new ChangeFrameSetName(_document, name));
    }
}

void FrameSetDock::onTilesetTypeEdited()
{
    TilesetType ts = _ui->tilesetType->currentEnum<TilesetType>();
    if (ts != _document->frameSet()->tilesetType) {
        _document->undoStack()->push(
            new ChangeFrameSetTilesetType(_document, ts));
    }
}

void FrameSetDock::onExportOrderEdited()
{
    idstring eo = _ui->exportOrder->text().toStdString();
    if (eo != _document->frameSet()->exportOrder) {
        _document->undoStack()->push(
            new ChangeFrameSetExportOrder(_document, eo));
    }
}

void FrameSetDock::onImageFilenameFileSelected()
{
    const SI::FrameSet& fs = *_document->frameSet();

    const QString filename = _ui->imageFilename->filename();
    Q_ASSERT(!filename.isEmpty());

    const std::string fn = filename.toStdString();
    if (fn != fs.imageFilename) {
        _document->undoStack()->push(
            new ChangeFrameSetImageFile(_document, fn));
    }
}

void FrameSetDock::onTransparentColorSelected()
{
    const SI::FrameSet& fs = *_document->frameSet();

    QColor color = _ui->transparent->color();
    rgba tc(color.red(), color.green(), color.blue());
    if (tc != fs.transparentColor) {
        _document->undoStack()->push(
            new ChangeFrameSetTransparentColor(_document, tc));
    }
}

void FrameSetDock::onGridEdited()
{
    SI::FrameSetGrid grid;

    grid.frameSize = _ui->gridSize->valueUsize();
    grid.offset = _ui->gridOffset->valueUpoint();
    grid.padding = _ui->gridPadding->valueUpoint();
    grid.origin = _ui->gridOrigin->valueUpoint();

    if (grid != _document->frameSet()->grid) {
        _document->undoStack()->push(
            new ChangeFrameSetGrid(_document, grid));
    }
}

void FrameSetDock::onPaletteEdited()
{
    SI::UserSuppliedPalette palette;
    if (_ui->userSuppliedPaletteBox->isChecked()) {
        palette.nPalettes = _ui->nPalettes->value();
        palette.colorSize = _ui->paletteSize->value();
    }

    if (palette != _document->frameSet()->palette) {
        _document->undoStack()->push(
            new ChangeFrameSetPalette(_document, palette));
    }
}

void FrameSetDock::updateFrameListSelection()
{
    if (_document) {
        const idstring id = _document->selection()->selectedFrameId();
        QModelIndex index = _frameListModel->toModelIndex(id);

        _ui->frameList->setCurrentIndex(index);
    }
}

void FrameSetDock::onFrameListSelectionChanged()
{
    QModelIndex index = _ui->frameList->currentIndex();
    idstring frameId = _frameListModel->toIdstring(index);
    _document->selection()->selectFrame(frameId);
}

void FrameSetDock::onFrameContextMenu(const QPoint& pos)
{
    if (_document && _actions) {
        bool onFrame = _ui->frameList->indexAt(pos).isValid();

        QMenu menu;
        menu.addAction(_actions->addFrame());

        if (onFrame) {
            menu.addAction(_actions->cloneFrame());
            menu.addAction(_actions->renameFrame());
            menu.addAction(_actions->removeFrame());
        }

        QPoint globalPos = _ui->frameList->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}
