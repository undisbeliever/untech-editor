/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "multipleselectiontableview.h"
#include "abstractaccessors.h"
#include "accessor.h"
#include "listaccessortablemanager.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"

#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QMenu>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

MultipleSelectionTableView::MultipleSelectionTableView(QWidget* parent)
    : QTreeView(parent)
    , _actions(new MultiListActions(this))
    , _delegate(new PropertyDelegate(this))
    , _selectedContextMenu(new QMenu(this))
    , _noSelectionContextMenu(new QMenu(this))
    , _model(nullptr)
    , _accessors()
{
    setItemDelegate(_delegate);

    setDragDropMode(QTreeView::InternalMove);

    setEditTriggers(EditTriggers(AllEditTriggers).setFlag(CurrentChanged, false));
    setAlternatingRowColors(true);

    setSelectionMode(SelectionMode::ExtendedSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);

    rebuildMenus();
}

void MultipleSelectionTableView::setPropertyManagers(const QList<ListAccessorTableManager*>& managers,
                                                     const QStringList& columns)
{
    if (auto* sm = selectionModel()) {
        sm->deleteLater();
    }
    if (_model) {
        _model->deleteLater();

        for (auto* m : _model->managers()) {
            m->disconnect(this);
        }
    }

    if (!managers.isEmpty()) {
        QList<GuiQt::PropertyTableManager*> pManagers;
        pManagers.reserve(managers.size());
        for (auto* m : managers) {
            pManagers.append(m);
        }
        _model = new PropertyTableModel(pManagers, columns, this);
    }
    else {
        _model = nullptr;
    }
    QTreeView::setModel(_model);

    _actions->setNAccessors(managers.size());

    onAccessorsChanged();

    rebuildMenus();

    if (_model) {
        connect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MultipleSelectionTableView::onViewSelectionChanged);
    }
    for (auto* manager : managers) {
        connect(manager, &ListAccessorTableManager::accessorChanged,
                this, &MultipleSelectionTableView::onAccessorsChanged);
    }
}

void MultipleSelectionTableView::onAccessorsChanged()
{
    auto newAccessors = buildAccessorsList();

    if (_accessors.empty() && newAccessors.empty()) {
        return;
    }

    for (auto* a : _accessors) {
        a->disconnect(this);
    }
    _accessors = newAccessors;

    _actions->setAccessors(_accessors);
    onAccessorSelectedIndexesChanged();

    for (auto* a : newAccessors) {
        connect(a, &AbstractListMultipleSelectionAccessor::selectedIndexesChanged,
                this, &MultipleSelectionTableView::onAccessorSelectedIndexesChanged);
    }
}

QList<AbstractListMultipleSelectionAccessor*> MultipleSelectionTableView::buildAccessorsList()
{
    QList<AbstractListMultipleSelectionAccessor*> ret;

    if (_model == nullptr) {
        return ret;
    }

    const auto& managers = _model->managers();
    ret.reserve(managers.size());

    for (auto* manager : managers) {
        auto* laManager = qobject_cast<ListAccessorTableManager*>(manager);
        auto* accessor = laManager ? laManager->accessor() : nullptr;
        auto* multiSelectionAccessor = qobject_cast<AbstractListMultipleSelectionAccessor*>(accessor);

        if (multiSelectionAccessor == nullptr) {
            ret.clear();
            break;
        }
        ret.append(multiSelectionAccessor);
    }

    return ret;
}

void MultipleSelectionTableView::onAccessorSelectedIndexesChanged()
{
    auto* selectionModel = this->selectionModel();

    QItemSelection sel;

    for (int aId = 0; aId < _accessors.size(); aId++) {
        if (auto* accessor = _accessors.at(aId)) {
            for (auto si : accessor->selectedIndexes()) {
                QModelIndex index = _model->toModelIndex(aId, si);
                if (index.isValid()) {
                    sel.select(index, index);
                }
            }
        }
    }

    selectionModel->select(
        sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    // If the currentIndex is not a part of the selection then make it so.
    // This will allow the user to Clone an item (Ctrl+D) and then edit it by pressing F2.
    if (not sel.empty()) {
        if (selectionModel->isSelected(selectionModel->currentIndex()) == false) {
            QModelIndex index = sel.first().topLeft();
            selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
    }

    // BUGFIX: Sometimes the view will not hightlight the new selection
    viewport()->update();
}

void MultipleSelectionTableView::onViewSelectionChanged()
{
    const auto selectedRows = selectionModel()->selectedRows();

    for (int aId = 0; aId < _accessors.size(); aId++) {
        if (auto* accessor = _accessors.at(aId)) {
            std::vector<size_t> selected;
            selected.reserve(selectedRows.size());

            for (const auto& index : selectedRows) {
                auto mi = _model->toManagerIdAndIndex(index);
                if (mi.first == aId) {
                    selected.push_back(mi.second);
                }
            }
            accessor->setSelectedIndexes(std::move(selected));
        }
    }
}

void MultipleSelectionTableView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in MultipleSelectionTableView.");
}

void MultipleSelectionTableView::rebuildMenus()
{
    _selectedContextMenu->clear();
    _noSelectionContextMenu->clear();

    _actions->populate(_selectedContextMenu, true);
    _actions->populateAddActions(_noSelectionContextMenu);
    _noSelectionContextMenu->addAction(_actions->selectAll);
}

void MultipleSelectionTableView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_model == nullptr) {
        return;
    }

    if (_actions->remove->isEnabled()) {
        _selectedContextMenu->exec(event->globalPos());
    }
    else {
        _noSelectionContextMenu->exec(event->globalPos());
    }
}
