/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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

FrameSetDock::FrameSetDock(Actions* actions, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _actions(actions)
    , _document(nullptr)
{
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->frameSetName->setValidator(new IdstringValidator(this));

    _ui->nPalettes->setMaximum(UnTech::MetaSprite::MAX_PALETTES);
    _ui->paletteSize->setMaximum(32);

    _ui->tilesetType->populateData(TilesetType::enumMap);

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

    connect(_ui->exportOrderButton, &QToolButton::clicked,
            this, &FrameSetDock::onExportOrderButtonClicked);
    connect(_ui->imageFilenameButton, &QToolButton::clicked,
            this, &FrameSetDock::onImageFilenameButtonClicked);
    connect(_ui->transparentButton, &QToolButton::clicked,
            this, &FrameSetDock::onTransparentButtonClicked);

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

    if (auto* m = _ui->frameList->selectionModel()) {
        m->deleteLater();
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        _ui->frameList->setModel(_document->frameListModel());

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
        _ui->frameList->setModel(nullptr);

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
    _ui->transparentButton->unsetColor();
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
        _ui->transparentButton->setColor(c);
    }
    else {
        _ui->transparent->clear();
        _ui->transparentButton->unsetColor();
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

void FrameSetDock::onExportOrderButtonClicked()
{
    const SI::FrameSet& fs = *_document->frameSet();

    QString oldFilename;
    if (fs.exportOrder) {
        oldFilename = QString::fromStdString(fs.exportOrder->filename);
    }

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open Export File"), oldFilename,
        tr("FrameSet Export File (*.utfseo);;All Files (*)"));

    if (!filename.isNull()) {
        std::string fn = filename.toStdString();

        try {
            auto eo = UnTech::MetaSprite::loadFrameSetExportOrderCached(fn);

            if (eo != fs.exportOrder) {
                _document->undoStack()->push(
                    new ChangeFrameSetExportOrder(_document, eo));
            }
        }
        catch (std::exception& ex) {
            QMessageBox::critical(this, tr("Error Opening File"), ex.what());
        }
    }
}

void FrameSetDock::onImageFilenameButtonClicked()
{
    const SI::FrameSet& fs = *_document->frameSet();

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open Image"),
        QString::fromStdString(fs.imageFilename),
        tr("PNG Image (*.png);;All Files (*)"));

    if (!filename.isNull()) {
        std::string fn = filename.toStdString();

        if (fn != fs.imageFilename)
            _document->undoStack()->push(
                new ChangeFrameSetImageFile(_document, fn));
    }
}

void FrameSetDock::onTransparentButtonClicked()
{
    const SI::FrameSet& fs = *_document->frameSet();

    QColor color;
    if (fs.transparentColorValid()) {
        color.setRgb(fs.transparentColor.rgbHex());
    }

    color = QColorDialog::getColor(color, this,
                                   tr("Select Transparent Colour"),
                                   QColorDialog::DontUseNativeDialog);

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
    const idstring id = _document->selection()->selectedFrameId();
    QModelIndex index = _document->frameListModel()->toModelIndex(id);

    _ui->frameList->setCurrentIndex(index);
}

void FrameSetDock::onFrameListSelectionChanged()
{
    QModelIndex index = _ui->frameList->currentIndex();
    idstring frameId = _document->frameListModel()->toIdstring(index);
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
