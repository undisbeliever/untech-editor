/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listaccessortableview.h"
#include "abstractaccessors.h"
#include "accessor.h"
#include "listaccessortablemanager.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"

#include <QContextMenuEvent>
#include <QIcon>
#include <QLocale>
#include <QMenu>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

ListAccessorTableView::ListAccessorTableView(QWidget* parent)
    : QTreeView(parent)
    , _actions(new ListActions(this))
    , _delegate(new PropertyDelegate(this))
    , _contextMenu(new QMenu(this))
    , _model(nullptr)
    , _manager(nullptr)
    , _accessor(nullptr)
{
    setItemDelegate(_delegate);

    setDragDropMode(QTreeView::InternalMove);

    setEditTriggers(EditTriggers(AllEditTriggers).setFlag(CurrentChanged, false));
    setAlternatingRowColors(true);

    setSelectionMode(SelectionMode::NoSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);

    _actions->populate(this);
    _actions->populate(_contextMenu);
}

void ListAccessorTableView::setPropertyManager(ListAccessorTableManager* manager)
{
    if (auto* sm = selectionModel()) {
        sm->deleteLater();
    }
    if (_model) {
        _model->deleteLater();
    }

    if (_manager) {
        _manager->disconnect(this);
    }
    _manager = manager;

    if (_accessor) {
        _accessor->disconnect(this);
    }
    _accessor = nullptr;

    if (manager) {
        _model = new PropertyTableModel(manager, this);
    }
    else {
        _model = nullptr;
    }
    QTreeView::setModel(_model);

    onAccessorChanged();

    if (_model) {
        connect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &ListAccessorTableView::onViewSelectionChanged);
    }

    if (_manager) {
        connect(_manager, &ListAccessorTableManager::accessorChanged,
                this, &ListAccessorTableView::onAccessorChanged);
    }
}

void ListAccessorTableView::onAccessorChanged()
{
    auto* accessor = _manager ? _manager->accessor() : nullptr;

    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);
    }
    _accessor = accessor;

    if (auto* sa = qobject_cast<AbstractListSingleSelectionAccessor*>(_accessor)) {
        _actions->setAccessor(sa);
        setSelectionMode(SelectionMode::SingleSelection);

        connect(sa, &AbstractListSingleSelectionAccessor::selectedIndexChanged,
                this, &ListAccessorTableView::onAccessorSelectionChanged);
    }
    else if (auto* ma = qobject_cast<AbstractListMultipleSelectionAccessor*>(_accessor)) {
        _actions->setAccessor(ma);
        setSelectionMode(SelectionMode::ExtendedSelection);

        connect(ma, &AbstractListMultipleSelectionAccessor::selectedIndexesChanged,
                this, &ListAccessorTableView::onAccessorSelectionChanged);
    }
    else {
        setSelectionMode(SelectionMode::NoSelection);
        _actions->setAccessor(static_cast<AbstractListSingleSelectionAccessor*>(nullptr));
    }

    onAccessorSelectionChanged();
}

void ListAccessorTableView::onAccessorSelectionChanged()
{
    QItemSelection sel;

    if (auto* sa = qobject_cast<AbstractListSingleSelectionAccessor*>(_accessor)) {
        QModelIndex index = _model->toModelIndex(0, sa->selectedIndex());
        if (index.isValid()) {
            sel.select(index, index);
        }
    }
    else if (auto* ma = qobject_cast<AbstractListMultipleSelectionAccessor*>(_accessor)) {
        for (auto si : ma->selectedIndexes()) {
            QModelIndex index = _model->toModelIndex(0, si);
            if (index.isValid()) {
                sel.select(index, index);
            }
        }
    }
    selectionModel()->select(
        sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    // BUGFIX: Sometimes the view will not hightlight the new selection
    viewport()->update();
}

void ListAccessorTableView::onViewSelectionChanged()
{
    const auto selectedRows = selectionModel()->selectedRows();

    if (auto* sa = qobject_cast<AbstractListSingleSelectionAccessor*>(_accessor)) {
        if (selectedRows.size() == 1) {
            auto mi = _model->toManagerIdAndIndex(selectedRows.first());
            sa->setSelectedIndex(mi.second);
        }
        else {
            sa->unselectItem();
        }
    }
    else if (auto* ma = qobject_cast<AbstractListMultipleSelectionAccessor*>(_accessor)) {
        std::vector<size_t> selected;
        selected.reserve(selectedRows.size());

        for (const auto& index : selectedRows) {
            auto mi = _model->toManagerIdAndIndex(index);
            selected.push_back(mi.second);
        }
        ma->setSelectedIndexes(std::move(selected));
    }
}

void ListAccessorTableView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in ListAccessorTableView.");
}

void ListAccessorTableView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_model == nullptr) {
        return;
    }

    _contextMenu->exec(event->globalPos());
}
