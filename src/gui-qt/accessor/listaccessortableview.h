/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "listactions.h"
#include <QTreeView>

namespace UnTech {
namespace GuiQt {
class PropertyDelegate;
class PropertyTableModel;

namespace Accessor {
class ListAccessorTableManager;
class AbstractListAccessor;

class ListAccessorTableView : public QTreeView {
    Q_OBJECT

private:
    ListActions* const _actions;

    PropertyDelegate* const _delegate;
    QMenu* const _contextMenu;

    PropertyTableModel* _model;
    ListAccessorTableManager* _manager;
    AbstractListAccessor* _accessor;

public:
    explicit ListAccessorTableView(QWidget* parent = nullptr);
    ~ListAccessorTableView() = default;

    void setPropertyManager(ListAccessorTableManager* manager);

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    ListActions* viewActions() const { return _actions; }
    PropertyTableModel* propertyTableModel() const { return _model; }

    QMenu* contextmenu() { return _contextMenu; }

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;

private slots:
    void onAccessorChanged();

    void onAccessorSelectionChanged();
    void onViewSelectionChanged();
};
}
}
}
