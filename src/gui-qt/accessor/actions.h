/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QToolBar>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

// namedlistview.cpp
struct NamedListActions {
    QAction* const add;
    QAction* const clone;
    QAction* const rename;
    QAction* const raise;
    QAction* const lower;
    QAction* const remove;

    explicit NamedListActions(QWidget* parent);

    void populate(QWidget* widget) const;

private:
    friend class NamedListView;
    void disableAll();
    void updateText(const QString& typeName);
};

}
}
}
