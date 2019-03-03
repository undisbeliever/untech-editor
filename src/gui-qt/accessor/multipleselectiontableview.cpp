/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "multipleselectiontableview.h"
#include "abstractaccessors.h"
#include "accessor.h"
#include "gui-qt/common/properties/propertydelegate.h"

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
    , _accessors()
    , _delegate(new PropertyDelegate(this))
    , _selectedContextMenu(new QMenu(this))
    , _noSelectionContextMenu(new QMenu(this))
    , _model(nullptr)
{
    setItemDelegate(_delegate);

    setEditTriggers(EditTrigger::AllEditTriggers);

    setSelectionMode(SelectionMode::ExtendedSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);

    rebuildMenus();
}

void MultipleSelectionTableView::disconnectAll()
{
    if (auto* sm = selectionModel()) {
        sm->disconnect(this);
    }

    for (int i = 0; i < _accessors.size(); i++) {
        if (AbstractListMultipleSelectionAccessor* a = _accessors.at(i)) {
            a->disconnect(this);
        }
        _accessors.replace(i, nullptr);
    }
}

void MultipleSelectionTableView::setPropertyManagers(const QList<GuiQt::PropertyTableManager*>& managers, const QStringList& columns)
{
    disconnectAll();

    _actions->setNAccessors(managers.size());

    if (auto* sm = selectionModel()) {
        sm->deleteLater();
    }

    if (_model) {
        _model->deleteLater();
    }

    if (!managers.isEmpty()) {
        _model = new PropertyTableModel(managers, columns, this);
    }
    else {
        _model = nullptr;
    }
    QTreeView::setModel(_model);

    _accessors.clear();
    for (int i = 0; i < managers.size(); i++) {
        _accessors.append(nullptr);
    }

    rebuildMenus();
}

void MultipleSelectionTableView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in MultipleSelectionTableView.");
}

void MultipleSelectionTableView::rebuildMenus()
{
    _selectedContextMenu->clear();
    _noSelectionContextMenu->clear();

    _actions->populateMenu(_selectedContextMenu, true);
    _actions->populateMenuWithAddActions(_noSelectionContextMenu);
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
