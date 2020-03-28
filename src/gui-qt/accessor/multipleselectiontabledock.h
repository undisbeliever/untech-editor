/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "multipleselectiontableview.h"
#include <QDockWidget>
#include <QToolBar>

namespace UnTech {
namespace GuiQt {
namespace Accessor {
class ListAccessorTableManager;

class MultipleSelectionTableDock : public QDockWidget {
    Q_OBJECT

private:
    MultipleSelectionTableView* const _view;
    QToolBar* const _toolBar;

public:
    explicit MultipleSelectionTableDock(const QString& windowTitle,
                                        const QList<ListAccessorTableManager*>& managers,
                                        const QStringList& columns,
                                        QWidget* parent = nullptr);
    ~MultipleSelectionTableDock() = default;

    MultiListActions* viewActions() const { return _view->viewActions(); }
    QToolBar* toolbar() const { return _toolBar; }

    QMenu* selectedContextmenu() { return _view->selectedContextmenu(); }
    QMenu* noSelectionContextMenu() { return _view->noSelectionContextMenu(); }
    void rebuildMenus() { _view->rebuildMenus(); }

    void expandAll() { _view->expandAll(); }
};
}
}
}
