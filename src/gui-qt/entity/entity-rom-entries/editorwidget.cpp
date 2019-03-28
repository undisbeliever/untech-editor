/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "entityromentriesresourceitem.h"
#include "managers.h"
#include "gui-qt/accessor/namedlistdock.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/entity/entity-rom-entries/editorwidget.ui.h"

using namespace UnTech::GuiQt::Entity::EntityRomEntries;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _namedListDock(new Accessor::NamedListDock(this))
    , _manager(new EntityRomEntryManager(this))
    , _item(nullptr)
{
    Q_ASSERT(parent);

    _ui->setupUi(this);

    _ui->name->setValidator(new IdstringValidator(this));

    _ui->fieldsView->setPropertyManager(_manager);

    setEnabled(false);

    connect(_ui->name, &QLineEdit::editingFinished,
            this, &EditorWidget::onNameEdited);
    connect(_ui->functionTableCombo, qOverload<int>(&QComboBox::activated),
            this, &EditorWidget::onFunctionTableActivated);
    connect(_ui->comment, &QLineEdit::editingFinished,
            this, &EditorWidget::onCommentEdited);
}

EditorWidget::~EditorWidget() = default;

QList<QDockWidget*> EditorWidget::createDockWidgets(QMainWindow*)
{
    _namedListDock->setObjectName(QStringLiteral("EntityRomStructs_Dock"));

    return {
        _namedListDock,
    };
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<EntityRomEntriesResourceItem*>(abstractItem);
    if (_item == item) {
        return item != nullptr;
    }

    if (_item) {
        _item->disconnect(this);
        _item->entriesList()->disconnect(this);
    }
    _item = item;

    _manager->setResourceItem(item);
    _namedListDock->setAccessor(item ? item->entriesList() : nullptr);

    if (item) {
        EntityRomEntriesList* entriesList = item->entriesList();

        updateFunctionTableComboList();
        onSelectedEntryChanged();

        connect(entriesList, &EntityRomEntriesList::selectedIndexChanged,
                this, &EditorWidget::onSelectedEntryChanged);
        connect(entriesList, &EntityRomEntriesList::nameChanged,
                this, &EditorWidget::onEntryNameChanged);
        connect(entriesList, &EntityRomEntriesList::implementsChanged,
                this, &EditorWidget::onEntryFunctionTableChanged);
        connect(entriesList, &EntityRomEntriesList::commentChanged,
                this, &EditorWidget::onEntryCommentChanged);
    }
    else {
        clearGui();
        setEnabled(false);
    }

    return item != nullptr;
}

void EditorWidget::clearGui()
{
    _ui->name->clear();
    _ui->functionTableCombo->setCurrentIndex(-1);
    _ui->comment->clear();
}

void EditorWidget::updateFunctionTableComboList()
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

void EditorWidget::onSelectedEntryChanged()
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

void EditorWidget::onEntryNameChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->name->setText(QString::fromStdString(e->name));
        }
    }
}

void EditorWidget::onEntryFunctionTableChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->functionTableCombo->setCurrentText(QString::fromStdString(e->functionTable));
        }
    }
}

void EditorWidget::onEntryCommentChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            _ui->comment->setText(QString::fromStdString(e->comment));
        }
    }
}

void EditorWidget::onNameEdited()
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

void EditorWidget::onFunctionTableActivated()
{
    Q_ASSERT(_item);
    _item->entriesList()->editSelected_setFunctionTable(
        _ui->functionTableCombo->currentText().toStdString());
}

void EditorWidget::onCommentEdited()
{
    Q_ASSERT(_item);
    _item->entriesList()->editSelected_setComment(
        _ui->comment->text().toStdString());
}
