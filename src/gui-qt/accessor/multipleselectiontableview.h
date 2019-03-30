/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "multilistactions.h"
#include <QTreeView>

namespace UnTech {
namespace GuiQt {
class PropertyDelegate;
class PropertyTableModel;

namespace Accessor {
class ListAccessorTableManager;

class MultipleSelectionTableView : public QTreeView {
    Q_OBJECT

private:
    MultiListActions* const _actions;

    PropertyDelegate* const _delegate;
    QMenu* const _selectedContextMenu;
    QMenu* const _noSelectionContextMenu;

    PropertyTableModel* _model;
    QList<AbstractListMultipleSelectionAccessor*> _accessors;

public:
    explicit MultipleSelectionTableView(QWidget* parent = nullptr);
    ~MultipleSelectionTableView() = default;

    void setPropertyManagers(const QList<ListAccessorTableManager*>& managers, const QStringList& columns);

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    MultiListActions* viewActions() const { return _actions; }
    PropertyTableModel* propertyTableModel() const { return _model; }

    QMenu* selectedContextmenu() { return _selectedContextMenu; }
    QMenu* noSelectionContextMenu() { return _noSelectionContextMenu; }

    void rebuildMenus();

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;

private slots:
    void onAccessorsChanged();
    void onAccessorSelectedIndexesChanged();
    void onViewSelectionChanged();

private:
    QList<AbstractListMultipleSelectionAccessor*> buildAccessorsList();
};
}
}
}
