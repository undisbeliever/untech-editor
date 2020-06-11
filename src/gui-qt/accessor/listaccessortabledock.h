/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "listaccessortableview.h"
#include <QDockWidget>
#include <QToolBar>

namespace UnTech {
namespace GuiQt {
namespace Accessor {
class ListAccessorTableView;
class ListAccessorTableManager;

class ListAccessorTableDock : public QDockWidget {
    Q_OBJECT

private:
    ListAccessorTableView* const _view;
    QToolBar* const _toolBar;

public:
    ListAccessorTableDock(const QString& windowTitle,
                          ListAccessorTableManager* manager,
                          QWidget* parent = nullptr);
    ~ListAccessorTableDock() = default;

    ListActions* viewActions() const { return _view->viewActions(); }
    QToolBar* toolbar() const { return _toolBar; }

    QMenu* selectedContextmenu() { return _view->contextMenu(); }
};

}
}
}
