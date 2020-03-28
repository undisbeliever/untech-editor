/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "namedlistactions.h"
#include "namedlistmodel.h"
#include <QListView>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class NamedListView : public QListView {
    Q_OBJECT

public:
    explicit NamedListView(QWidget* parent = nullptr);
    ~NamedListView() = default;

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    NamedListActions* namedListActions() const { return _actions; }
    NamedListModel* namedListModel() const { return _model; }

    QMenu* selectedContextMenu() const { return _selectedContextMenu; }
    QMenu* noSelectionContextMenu() const { return _noSelectionContextMenu; }

    void setAccessor(AbstractNamedListAccessor* accessor);

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;

private slots:
    void onAccessorSelectedIndexChanged();
    void onViewSelectionChanged();

private:
    NamedListActions* const _actions;
    NamedListModel* const _model;
    QMenu* const _selectedContextMenu;
    QMenu* const _noSelectionContextMenu;

    AbstractNamedListAccessor* _accessor;
};
}
}
}
