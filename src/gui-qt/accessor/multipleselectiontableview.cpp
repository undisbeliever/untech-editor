/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "multipleselectiontableview.h"
#include "accessor.h"
#include "gui-qt/common/properties/propertydelegate.h"

#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QMenu>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

using QCA = QCoreApplication;

MultiTableViewActions::MultiTableViewActions(QObject* parent)
    : add()
    , clone(new QAction(QIcon(":/icons/clone.svg"), QCA::tr("Clone Selected"), parent))
    , raise(new QAction(QIcon(":/icons/raise.svg"), QCA::tr("Raise Selected"), parent))
    , lower(new QAction(QIcon(":/icons/lower.svg"), QCA::tr("Lower Selected"), parent))
    , remove(new QAction(QIcon(":/icons/remove.svg"), QCA::tr("Remove Selected"), parent))
{
}

void MultiTableViewActions::resizeAddList(int count, QObject* parent)
{
    Q_ASSERT(count >= 0);

    add.reserve(count);
    while (add.size() > count) {
        add.takeLast()->deleteLater();
    }
    while (add.size() < count) {
        add.append(new QAction(QIcon(":/icons/add.svg"), QCA::tr("Add"), parent));
    }
}

void MultiTableViewActions::disconnectAll(QObject* o)
{
    for (QAction* a : add) {
        a->disconnect(o);
    }
    clone->disconnect(o);
    raise->disconnect(o);
    lower->disconnect(o);
    remove->disconnect(o);
}

void MultiTableViewActions::populateMenu(QMenu* menu, bool addSeperator) const
{
    populateMenuWithAddActions(menu);
    if (addSeperator) {
        menu->addSeparator();
    }
    menu->addAction(clone);
    menu->addAction(raise);
    menu->addAction(lower);
    menu->addAction(remove);
}

void MultiTableViewActions::populateMenuWithAddActions(QMenu* menu) const
{
    for (QAction* a : add) {
        menu->addAction(a);
    }
}

void MultiTableViewActions::populateToolbar(QToolBar* toolbar) const
{
    for (QAction* a : add) {
        toolbar->addAction(a);
    }
    toolbar->addAction(clone);
    toolbar->addAction(raise);
    toolbar->addAction(lower);
    toolbar->addAction(remove);
}

MultipleSelectionTableView::MultipleSelectionTableView(QWidget* parent)
    : _actions(parent)
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
    _actions.disconnectAll(this);

    if (auto* sm = selectionModel()) {
        sm->disconnect(this);
    }

    for (int i = 0; i < _accessors.size(); i++) {
        if (QObject* a = _accessors.at(i)) {
            a->disconnect(this);
        }
        _accessors.replace(i, nullptr);
    }
}

void MultipleSelectionTableView::setPropertyManagers(const QList<GuiQt::PropertyTableManager*>& managers, const QStringList& columns)
{
    disconnectAll();

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

    _actions.resizeAddList(managers.size(), this);

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

    _actions.populateMenu(_selectedContextMenu, true);
    _actions.populateMenuWithAddActions(_noSelectionContextMenu);
}

void MultipleSelectionTableView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_model == nullptr) {
        return;
    }

    if (_actions.remove->isEnabled()) {
        _selectedContextMenu->exec(event->globalPos());
    }
    else {
        _noSelectionContextMenu->exec(event->globalPos());
    }
}
