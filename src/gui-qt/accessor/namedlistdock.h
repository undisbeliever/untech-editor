/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "namedlistview.h"
#include <QDockWidget>

class QToolBar;

namespace UnTech {
namespace GuiQt {
namespace Accessor {
class NamedListView;
class AbstractNamedListAccessor;

class NamedListDock : public QDockWidget {
    Q_OBJECT

private:
    NamedListView* const _namedListView;
    QToolBar* const _toolBar;

public:
    explicit NamedListDock(QWidget* parent = nullptr);
    ~NamedListDock() = default;

    NamedListView* namedListView() const { return _namedListView; }
    NamedListActions* namedListActions() const { return _namedListView->namedListActions(); }
    QToolBar* toolbar() const { return _toolBar; }

    void setAccessor(AbstractNamedListAccessor* accessor);
};
}
}
}
