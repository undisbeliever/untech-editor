/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "entityromstructsresourceitem.h"
#include "structfieldsmodel.h"
#include "gui-qt/accessor/namedlistdock.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/entity/entity-rom-structs/editorwidget.ui.h"

using namespace UnTech::GuiQt::Entity::EntityRomStructs;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _namedListDock(new Accessor::NamedListDock(this))
    , _fieldsModel(new StructFieldsModel(this))
    , _fieldListActions(this)
    , _item(nullptr)
{
    Q_ASSERT(parent);

    _ui->setupUi(this);

    _ui->name->setValidator(new IdstringValidator(this));

    _ui->fieldsView->setModel(_fieldsModel);
    _ui->fieldsView->setItemDelegate(new PropertyDelegate(this));

    _fieldListActions.populate(_ui->fieldsViewButtons);
    _fieldListActions.populate(_ui->fieldsView);
    _ui->fieldsView->setContextMenuPolicy(Qt::ActionsContextMenu);

    setEnabled(false);

    connect(_ui->name, &QLineEdit::editingFinished,
            this, &EditorWidget::onNameEdited);
    connect(_ui->parentCombo, qOverload<int>(&QComboBox::activated),
            this, &EditorWidget::onParentActivated);
    connect(_ui->comment, &QLineEdit::editingFinished,
            this, &EditorWidget::onCommentEdited);

    connect(_ui->fieldsView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EditorWidget::onFieldViewSelectionChanged);
}

EditorWidget::~EditorWidget() = default;

QList<QDockWidget*> EditorWidget::createDockWidgets(QMainWindow*)
{
    _namedListDock->setObjectName(QStringLiteral("EntityRomEntries_Dock"));

    return {
        _namedListDock,
    };
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<EntityRomStructsResourceItem*>(abstractItem);

    if (_item == item) {
        return item != nullptr;
    }

    if (_item) {
        _item->disconnect(this);
        _item->structList()->disconnect(this);
    }
    _item = item;

    _fieldsModel->setResourceItem(item);
    _namedListDock->setAccessor(item ? item->structList() : nullptr);

    EntityRomStructFieldList* fieldList = nullptr;
    if (item) {
        EntityRomStructList* structList = item->structList();
        fieldList = item->structFieldList();

        updateParentComboList();
        onSelectedStructChanged();

        connect(structList, &EntityRomStructList::listChanged,
                this, &EditorWidget::updateParentComboList);
        connect(structList, &EntityRomStructList::nameChanged,
                this, &EditorWidget::updateParentComboList);

        connect(structList, &EntityRomStructList::selectedIndexChanged,
                this, &EditorWidget::onSelectedStructChanged);
        connect(structList, &EntityRomStructList::nameChanged,
                this, &EditorWidget::onStructNameChanged);
        connect(structList, &EntityRomStructList::parentChanged,
                this, &EditorWidget::onStructParentChanged);
        connect(structList, &EntityRomStructList::commentChanged,
                this, &EditorWidget::onStructCommentChanged);

        connect(fieldList, &EntityRomStructFieldList::selectedIndexesChanged,
                this, &EditorWidget::onSelectedFieldsChanged);
    }
    else {
        clearGui();
        setEnabled(false);
    }

    _fieldListActions.setAccessor(fieldList);

    return item != nullptr;
}

void EditorWidget::clearGui()
{
    _ui->name->clear();
    _ui->parentCombo->setCurrentIndex(-1);
    _ui->comment->clear();
}

void EditorWidget::updateParentComboList()
{
    _ui->parentCombo->clear();

    if (_item) {
        const auto* projectFile = _item->project()->projectFile();
        Q_ASSERT(projectFile);

        _ui->parentCombo->addItem(QString());
        for (const auto& s : projectFile->entityRomData.structs) {
            _ui->parentCombo->addItem(QString::fromStdString(s.name));
        }

        if (auto* s = _item->structList()->selectedItem()) {
            _ui->parentCombo->setCurrentText(QString::fromStdString(s->parent));
        }
    }
}

void EditorWidget::onSelectedStructChanged()
{
    Q_ASSERT(_item);

    if (auto* s = _item->structList()->selectedItem()) {
        _ui->name->setText(QString::fromStdString(s->name));
        _ui->parentCombo->setCurrentText(QString::fromStdString(s->parent));
        _ui->comment->setText(QString::fromStdString(s->comment));

        setEnabled(true);
    }
    else {
        clearGui();

        setEnabled(false);
    }
}

void EditorWidget::onSelectedFieldsChanged()
{
    Q_ASSERT(_item);

    auto* selectionModel = _ui->fieldsView->selectionModel();
    const auto& selectedIndexes = _item->structFieldList()->selectedIndexes();

    QItemSelection sel;
    for (auto si : selectedIndexes) {
        QModelIndex index = _fieldsModel->toModelIndex(si);
        if (index.isValid()) {
            sel.select(index, index);
        }
    }

    selectionModel->select(
        sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    // If the currentIndex is not part of the selection then make it so.
    if (not sel.empty()) {
        if (sel.contains(selectionModel->currentIndex()) == false) {
            QModelIndex index = sel.first().topLeft();
            selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
    }

    if (selectedIndexes.size() == 1) {
        auto si = selectedIndexes.back();
        QModelIndex index = _fieldsModel->toModelIndex(si);

        // If cell is empty (ie, no name) then edit the cell
        if (index.isValid() && index.data().toString().isEmpty()) {
            _ui->fieldsView->edit(index);
        }
    }
}

void EditorWidget::onFieldViewSelectionChanged()
{
    if (_item == nullptr) {
        return;
    }

    const auto selectedRows = _ui->fieldsView->selectionModel()->selectedRows();

    std::vector<size_t> selected;
    selected.reserve(selectedRows.size());

    for (const auto& index : selectedRows) {
        selected.push_back(_fieldsModel->toFieldIndex(index));
    }
    _item->structFieldList()->setSelectedIndexes(std::move(selected));
}

void EditorWidget::onStructNameChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->structList()->selectedIndex()) {
        if (auto* s = _item->structList()->selectedItem()) {
            _ui->name->setText(QString::fromStdString(s->name));
        }
    }
}

void EditorWidget::onStructParentChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->structList()->selectedIndex()) {
        if (auto* s = _item->structList()->selectedItem()) {
            _ui->parentCombo->setCurrentText(QString::fromStdString(s->parent));
        }
    }
}

void EditorWidget::onStructCommentChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->structList()->selectedIndex()) {
        if (auto* s = _item->structList()->selectedItem()) {
            _ui->comment->setText(QString::fromStdString(s->comment));
        }
    }
}

void EditorWidget::onNameEdited()
{
    Q_ASSERT(_item);
    idstring name{ _ui->name->text().toStdString() };

    if (name.isValid()) {
        _item->structList()->editSelected_setName(name);
    }
    else if (auto* s = _item->structList()->selectedItem()) {
        _ui->name->setText(QString::fromStdString(s->name));
    };
}

void EditorWidget::onParentActivated()
{
    Q_ASSERT(_item);
    _item->structList()->editSelected_setParent(
        _ui->parentCombo->currentText().toStdString());
}

void EditorWidget::onCommentEdited()
{
    Q_ASSERT(_item);
    _item->structList()->editSelected_setComment(
        _ui->comment->text().toStdString());
}
