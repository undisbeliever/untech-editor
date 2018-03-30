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
#include "gui-qt/metasprite/metasprite/framesetdock.ui.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using TilesetType = UnTech::MetaSprite::TilesetType;

FrameSetDock::FrameSetDock(FrameListModel* frameListModel, Actions* actions, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _frameListModel(frameListModel)
    , _actions(actions)
    , _document(nullptr)
{
    Q_ASSERT(_frameListModel);
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->frameSetName->setValidator(new IdstringValidator(this));

    _ui->exportOrder->setDialogTitle(tr("Open Export File"));
    _ui->exportOrder->setDialogFilter(tr("FrameSet Export File (*.utfseo);;All Files (*)"));

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

    connect(_ui->exportOrder, &FilenameInputWidget::fileSelected,
            this, &FrameSetDock::onExportOrderFileSelected);

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
    _ui->frameSetType->clear();
}

void FrameSetDock::updateGui()
{
    const MS::FrameSet& fs = *_document->frameSet();

    _ui->frameSetName->setText(QString::fromStdString(fs.name));

    _ui->tilesetType->setCurrentEnum(fs.tilesetType);

    if (fs.exportOrder) {
        _ui->exportOrder->setFilename(QString::fromStdString(fs.exportOrder->filename));
        _ui->frameSetType->setText(QString::fromStdString(fs.exportOrder->name));
    }
    else {
        _ui->exportOrder->clear();
        _ui->frameSetType->clear();
    }
}

void FrameSetDock::onNameEdited()
{
    const MS::FrameSet& fs = *_document->frameSet();

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

void FrameSetDock::onExportOrderFileSelected()
{
    const MS::FrameSet& fs = *_document->frameSet();

    QString oldFilename;
    if (fs.exportOrder) {
        oldFilename = QString::fromStdString(fs.exportOrder->filename);
    }

    const QString filename = _ui->exportOrder->filename();
    Q_ASSERT(!filename.isEmpty());

    try {
        std::string fn = filename.toStdString();
        auto eo = UnTech::MetaSprite::loadFrameSetExportOrderCached(fn);

        if (eo != fs.exportOrder) {
            _document->undoStack()->push(
                new ChangeFrameSetExportOrder(_document, eo));
        }
    }
    catch (std::exception& ex) {
        QMessageBox::critical(this, tr("Error Opening File"), ex.what());
        _ui->exportOrder->setFilename(oldFilename);
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
