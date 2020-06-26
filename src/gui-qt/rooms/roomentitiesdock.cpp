/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "roomentitiesdock.h"
#include "accessors.h"
#include "resourceitem.h"
#include "roomentitiesmodel.h"
#include "gui-qt/accessor/listactionhelper.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/entity/entity-rom-entries/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"

#include "gui-qt/rooms/roomentitiesdock.ui.h"

#include <QMenu>
#include <QToolBar>
#include <QToolButton>

using namespace UnTech;
using namespace UnTech::GuiQt::Rooms;

RoomEntitiesDock::RoomEntitiesDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::RoomEntitiesDock)
    , _model(new RoomEntitiesModel(this))
    , _moveToGroupMenu(new QMenu(tr("Move Entity To Group")))
    , _resourceItem(nullptr)
{
    _ui->setupUi(this);

    setEnabled(false);

    _ui->treeView->setModel(_model);
    _ui->treeView->setItemDelegate(new PropertyDelegate(_ui->treeView));
    _ui->treeView->setEditTriggers(QTreeView::EditTriggers(QTreeView::AllEditTriggers).setFlag(QTreeView::CurrentChanged, false));
    _ui->treeView->setAlternatingRowColors(true);

    _ui->treeView->setSelectionMode(QTreeView::SelectionMode::ExtendedSelection);
    _ui->treeView->setSelectionBehavior(QTreeView::SelectionBehavior::SelectRows);

    _ui->treeView->setDragDropMode(QTreeView::InternalMove);

    populateActions(_ui->treeView);
    _ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(_ui->action_AddEntityGroup, &QAction::triggered,
            this, &RoomEntitiesDock::onAddEntityGroupTriggered);
    connect(_ui->action_AddEntityEntry, &QAction::triggered,
            this, &RoomEntitiesDock::onAddEntityEntryTriggered);

    connect(_ui->action_SelectAll, &QAction::triggered,
            this, &RoomEntitiesDock::onSelectAllTriggered);

    connect(_ui->action_CloneSelected, &QAction::triggered,
            this, &RoomEntitiesDock::onCloneSelectedTriggered);
    connect(_ui->action_RemoveSelected, &QAction::triggered,
            this, &RoomEntitiesDock::onRemoveSelectedTriggered);
    connect(_ui->action_RaiseToTop, &QAction::triggered,
            this, &RoomEntitiesDock::onRaiseToTopTriggered);
    connect(_ui->action_Raise, &QAction::triggered,
            this, &RoomEntitiesDock::onRaiseTriggered);
    connect(_ui->action_Lower, &QAction::triggered,
            this, &RoomEntitiesDock::onLowerTriggered);
    connect(_ui->action_LowerToBottom, &QAction::triggered,
            this, &RoomEntitiesDock::onLowerToBottomTriggered);

    connect(_ui->treeView, &QTreeView::customContextMenuRequested,
            this, &RoomEntitiesDock::onContextMenuRequested);

    connect(_ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &RoomEntitiesDock::onViewSelectionChanged);

    connect(_moveToGroupMenu, &QMenu::triggered,
            this, &RoomEntitiesDock::onMoveToGroupMenuTriggered);
}

RoomEntitiesDock::~RoomEntitiesDock() = default;

void RoomEntitiesDock::setResourceItem(ResourceItem* item)
{
    if (item == _resourceItem) {
        return;
    }

    if (_resourceItem) {
        _resourceItem->entityGroups()->disconnect(this);
        _resourceItem->entityEntries()->disconnect(this);
    }
    _resourceItem = item;

    _model->setResourceItem(_resourceItem);
    _ui->treeView->expandAll();

    updateMoveToGroupMenuActions();

    if (_resourceItem) {
        onAccessorSelectionChanged();

        connect(_resourceItem->entityGroups(), &EntityGroupList::listChanged,
                this, &RoomEntitiesDock::updateMoveToGroupMenuActions);
        connect(_resourceItem->entityGroups(), &EntityGroupList::nameChanged,
                this, &RoomEntitiesDock::updateMoveToGroupMenuActions);

        connect(_resourceItem->entityGroups(), &EntityGroupList::selectedIndexChanged,
                this, &RoomEntitiesDock::onAccessorSelectionChanged);
        connect(_resourceItem->entityEntries(), &EntityEntriesList::selectedIndexesChanged,
                this, &RoomEntitiesDock::onAccessorSelectionChanged);
    }

    setEnabled(_resourceItem != nullptr);
}

void RoomEntitiesDock::updateActions()
{
    if (_resourceItem == nullptr) {
        return;
    }

    const auto* entityGroups = _resourceItem->entityGroups();
    const auto* entityEntries = _resourceItem->entityEntries();

    const auto selectedGroupIndex = entityGroups->selectedIndex();

    const auto groupActionStatus = entityGroups->listActionStatus();
    const auto entryActionStatus = entityEntries->listActionStatus(selectedGroupIndex);

    _ui->action_AddEntityGroup->setEnabled(groupActionStatus.canAdd);
    _ui->action_AddEntityEntry->setEnabled(entryActionStatus.canAdd);

    if (entityEntries->selectedIndexes().empty()) {
        _moveToGroupMenu->setEnabled(false);

        _ui->action_CloneSelected->setEnabled(groupActionStatus.canClone);
        _ui->action_RemoveSelected->setEnabled(groupActionStatus.canRemove);
        _ui->action_RaiseToTop->setEnabled(groupActionStatus.canRaise);
        _ui->action_Raise->setEnabled(groupActionStatus.canRaise);
        _ui->action_Lower->setEnabled(groupActionStatus.canLower);
        _ui->action_LowerToBottom->setEnabled(groupActionStatus.canLower);
    }
    else {
        _moveToGroupMenu->setEnabled(entryActionStatus.selectionValid);

        _ui->action_CloneSelected->setEnabled(entryActionStatus.canClone);
        _ui->action_RemoveSelected->setEnabled(entryActionStatus.canRemove);
        _ui->action_RaiseToTop->setEnabled(entryActionStatus.canRaise);
        _ui->action_Raise->setEnabled(entryActionStatus.canRaise);
        _ui->action_Lower->setEnabled(entryActionStatus.canLower);
        _ui->action_LowerToBottom->setEnabled(entryActionStatus.canLower);
    }
}

void RoomEntitiesDock::onAccessorSelectionChanged()
{
    Q_ASSERT(_resourceItem);

    const auto& selectedEntries = _resourceItem->entityEntries()->selectedIndexes();

    if (!selectedEntries.empty()) {
        QItemSelection sel;
        sel.reserve(selectedEntries.size());

        unsigned previousParent = INT_MAX;

        for (auto& si : selectedEntries) {
            QModelIndex index = _model->toModelIndex(si.first, si.second);
            sel.select(index, index);

            if (previousParent != si.first) {
                _ui->treeView->expand(_model->toModelIndex(si.first));
                previousParent = si.first;
            }
        }
        _ui->treeView->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
    }
    else {
        // Only an EntitiesGroup is selected
        auto index = _model->toModelIndex(_resourceItem->entityGroups()->selectedIndex());
        if (index.isValid()) {
            _ui->treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
        }
        else {
            _ui->treeView->clearSelection();
        }
    }
    // BUGFIX: Sometimes the view will not hightlight the new selection
    _ui->treeView->viewport()->update();

    updateActions();
}

void RoomEntitiesDock::onViewSelectionChanged()
{
    if (_resourceItem == nullptr) {
        return;
    }

    const auto selectedRows = _ui->treeView->selectionModel()->selectedRows();

    if (selectedRows.empty()) {
        _resourceItem->entityGroups()->unselectItem();
        _resourceItem->entityEntries()->clearSelection();
        return;
    }

    if (selectedRows.size() == 1) {
        auto& index = selectedRows.front();
        if (index.isValid()) {
            if (_model->isGroupIndex(index)) {
                _resourceItem->entityEntries()->clearSelection();
                _resourceItem->entityGroups()->setSelectedIndex(_model->toEntryGroupIndex(index));
                return;
            }
        }
    }
    // either multiple items or an EntityEntry if selected

    _resourceItem->entityEntries()->setSelectedIndexes(_model->toEntityEntryIndexes(selectedRows));

    // Prevents multiple groups from being selected
    // Prevents groups from being selected if entity entries are selected
    bool isGroupSelected = std::any_of(selectedRows.begin(), selectedRows.end(),
                                       [&](auto& index) { return _model->isGroupIndex(index); });
    if (isGroupSelected) {
        onAccessorSelectionChanged();
    }
}

void RoomEntitiesDock::updateMoveToGroupMenuActions()
{
    _moveToGroupMenu->clear();

    if (_resourceItem == nullptr) {
        return;
    }
    const auto room = _resourceItem->roomInput();
    if (!room) {
        return;
    }

    for (size_t i = 0; i < room->entityGroups.size(); i++) {
        const auto& eg = room->entityGroups.at(i);

        QString text = eg.name.isValid() ? QString::fromStdString(eg.name) : tr("Entity Group %1").arg(i);
        QAction* a = _moveToGroupMenu->addAction(text);
        a->setData(unsigned(i));
    }
}

void RoomEntitiesDock::onSelectAllTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    _resourceItem->entityGroups()->unselectItem();
    _resourceItem->entityEntries()->selectAll();
}

void RoomEntitiesDock::onAddEntityGroupTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    _resourceItem->entityGroups()->appendItem();
}

void RoomEntitiesDock::onAddEntityEntryTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    _resourceItem->entityEntries()->appendItemToList(
        _resourceItem->entityGroups()->selectedIndex());
}

void RoomEntitiesDock::onCloneSelectedTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    if (_resourceItem->entityEntries()->selectedIndexes().empty()) {
        _resourceItem->entityGroups()->cloneSelectedItem();
    }
    else {
        _resourceItem->entityEntries()->cloneSelectedItems();
    }
}

void RoomEntitiesDock::onRemoveSelectedTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    if (_resourceItem->entityEntries()->selectedIndexes().empty()) {
        _resourceItem->entityGroups()->removeSelectedItem();
    }
    else {
        _resourceItem->entityEntries()->removeSelectedItems();
    }
}

void RoomEntitiesDock::onRaiseToTopTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    if (_resourceItem->entityEntries()->selectedIndexes().empty()) {
        _resourceItem->entityGroups()->raiseSelectedItemToTop();
    }
    else {
        _resourceItem->entityEntries()->raiseSelectedItemsToTop();
    }
}

void RoomEntitiesDock::onRaiseTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    if (_resourceItem->entityEntries()->selectedIndexes().empty()) {
        _resourceItem->entityGroups()->raiseSelectedItem();
    }
    else {
        _resourceItem->entityEntries()->raiseSelectedItems();
    }
}

void RoomEntitiesDock::onLowerTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    if (_resourceItem->entityEntries()->selectedIndexes().empty()) {
        _resourceItem->entityGroups()->lowerSelectedItem();
    }
    else {
        _resourceItem->entityEntries()->lowerSelectedItems();
    }
}

void RoomEntitiesDock::onLowerToBottomTriggered()
{
    if (_resourceItem == nullptr) {
        return;
    }
    if (_resourceItem->entityEntries()->selectedIndexes().empty()) {
        _resourceItem->entityGroups()->lowerSelectedItem();
    }
    else {
        _resourceItem->entityEntries()->lowerSelectedItemsToBottom();
    }
}

void RoomEntitiesDock::onMoveToGroupMenuTriggered(QAction* a)
{
    if (_resourceItem == nullptr
        || a == nullptr) {

        return;
    }
    _resourceItem->entityEntries()->moveSelectedItemsToChildList(a->data().toUInt());
}

void RoomEntitiesDock::onContextMenuRequested(const QPoint& pos)
{
    if (_resourceItem == nullptr) {
        return;
    }

    QMenu menu;

    populateActions(&menu);

    QPoint globalPos = _ui->treeView->mapToGlobal(pos);
    menu.exec(globalPos);
}

void RoomEntitiesDock::populateActions(QWidget* widget) const
{
    widget->addAction(_ui->action_AddEntityGroup);
    widget->addAction(_ui->action_AddEntityEntry);

    if (qobject_cast<QToolBar*>(widget) == nullptr) {
        // do not add selectAll to a QToolBar
        widget->addAction(_ui->action_SelectAll);
    }
    if (auto* menu = qobject_cast<QMenu*>(widget)) {
        menu->addMenu(_moveToGroupMenu);
    }

    widget->addAction(_ui->action_CloneSelected);
    widget->addAction(_ui->action_RemoveSelected);
    widget->addAction(_ui->action_RaiseToTop);
    widget->addAction(_ui->action_Raise);
    widget->addAction(_ui->action_Lower);
    widget->addAction(_ui->action_LowerToBottom);
}
