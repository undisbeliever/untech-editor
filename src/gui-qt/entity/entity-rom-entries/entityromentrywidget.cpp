/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromentrywidget.h"
#include "accessors.h"
#include "entityromentriesresourceitem.h"
#include "managers.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/entity/entity-rom-entries/entityromentrywidget.ui.h"

using namespace UnTech::GuiQt::Entity::EntityRomEntries;

EntityRomEntryWidget::EntityRomEntryWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::EntityRomEntryWidget>())
    , _manager(new EntityRomEntryManager(this))
    , _item(nullptr)
{
    Q_ASSERT(parent);

    _ui->setupUi(this);

    _ui->name->setValidator(new IdstringValidator(this));

    _ui->fieldsView->setPropertyManager(_manager);

    setEnabled(false);

    connect(_ui->name, &QLineEdit::editingFinished,
            this, &EntityRomEntryWidget::onNameEdited);
    connect(_ui->functionTableCombo, qOverload<int>(&QComboBox::activated),
            this, &EntityRomEntryWidget::onFunctionTableActivated);
    connect(_ui->comment, &QLineEdit::editingFinished,
            this, &EntityRomEntryWidget::onCommentEdited);
}

EntityRomEntryWidget::~EntityRomEntryWidget() = default;

void EntityRomEntryWidget::setResourceItem(EntityRomEntriesResourceItem* item)
{
    if (_item == item) {
        return;
    }

    if (_item) {
        _item->disconnect(this);
        _item->entriesList()->disconnect(this);
    }
    _item = item;

    _manager->setResourceItem(item);

    if (item) {
        EntityRomEntriesList* entriesList = item->entriesList();

        updateFunctionTableComboList();
        onSelectedEntryChanged();

        connect(entriesList, &EntityRomEntriesList::selectedIndexChanged,
                this, &EntityRomEntryWidget::onSelectedEntryChanged);
        connect(entriesList, &EntityRomEntriesList::nameChanged,
                this, &EntityRomEntryWidget::onEntryNameChanged);
        connect(entriesList, &EntityRomEntriesList::implementsChanged,
                this, &EntityRomEntryWidget::onEntryFunctionTableChanged);
        connect(entriesList, &EntityRomEntriesList::commentChanged,
                this, &EntityRomEntryWidget::onEntryCommentChanged);
    }
    else {
        clearGui();
        setEnabled(false);
    }
}

void EntityRomEntryWidget::clearGui()
{
    _ui->name->clear();
    _ui->functionTableCombo->setCurrentIndex(-1);
    _ui->comment->clear();
}

void EntityRomEntryWidget::updateFunctionTableComboList()
{
    _ui->functionTableCombo->clear();

    if (_item) {
        const auto* projectFile = _item->project()->projectFile();
        Q_ASSERT(projectFile);

        _ui->functionTableCombo->addItem(QString());
        for (const auto& ft : projectFile->entityRomData.functionTables) {
            _ui->functionTableCombo->addItem(QString::fromStdString(ft.name));
        }

        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->functionTableCombo->setCurrentText(QString::fromStdString(e->functionTable));
        }
    }
}

void EntityRomEntryWidget::onSelectedEntryChanged()
{
    Q_ASSERT(_item);
    if (auto* e = _item->entriesList()->selectedItem()) {
        _ui->name->setText(QString::fromStdString(e->name));
        _ui->functionTableCombo->setCurrentText(QString::fromStdString(e->functionTable));
        _ui->comment->setText(QString::fromStdString(e->comment));

        setEnabled(true);
    }
    else {
        clearGui();

        setEnabled(false);
    }
}

void EntityRomEntryWidget::onEntryNameChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->name->setText(QString::fromStdString(e->name));
        }
    }
}

void EntityRomEntryWidget::onEntryFunctionTableChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->functionTableCombo->setCurrentText(QString::fromStdString(e->functionTable));
        }
    }
}

void EntityRomEntryWidget::onEntryCommentChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->comment->setText(QString::fromStdString(e->comment));
        }
    }
}

void EntityRomEntryWidget::onNameEdited()
{
    Q_ASSERT(_item);
    idstring name{ _ui->name->text().toStdString() };

    if (name.isValid()) {
        _item->entriesList()->editSelected_setName(name);
    }
    else if (auto* e = _item->entriesList()->selectedItem()) {
        _ui->name->setText(QString::fromStdString(e->name));
    };
}

void EntityRomEntryWidget::onFunctionTableActivated()
{
    Q_ASSERT(_item);
    _item->entriesList()->editSelected_setFunctionTable(
        _ui->functionTableCombo->currentText().toStdString());
}

void EntityRomEntryWidget::onCommentEdited()
{
    Q_ASSERT(_item);
    _item->entriesList()->editSelected_setComment(
        _ui->comment->text().toStdString());
}
