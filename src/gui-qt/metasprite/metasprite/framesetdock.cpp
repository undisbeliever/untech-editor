/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetdock.h"
#include "accessors.h"
#include "document.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/metasprite/framesetdock.ui.h"

#include <QMenu>
#include <QMessageBox>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using TilesetType = UnTech::MetaSprite::TilesetType;

FrameSetDock::FrameSetDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _document(nullptr)
{
    _ui->setupUi(this);

    _ui->frameSetName->setValidator(new IdstringValidator(this));
    _ui->exportOrder->setValidator(new IdstringValidator(this));

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

        connect(_document, &Document::nameChanged,
                this, &FrameSetDock::updateGui);
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
}

void FrameSetDock::updateGui()
{
    const MS::FrameSet& fs = *_document->frameSet();

    _ui->frameSetName->setText(QString::fromStdString(fs.name));
    _ui->tilesetType->setCurrentEnum(fs.tilesetType);
    _ui->exportOrder->setText(QString::fromStdString(fs.exportOrder));
}

void FrameSetDock::onNameEdited()
{
    _document->editFrameSet_setName(
        _ui->frameSetName->text().toStdString());
    updateGui();
}

void FrameSetDock::onTilesetTypeEdited()
{
    _document->editFrameSet_setTilesetType(
        _ui->tilesetType->currentEnum<TilesetType>());
}

void FrameSetDock::onExportOrderEdited()
{
    _document->editFrameSet_setExportOrder(
        _ui->exportOrder->text().toStdString());
}
