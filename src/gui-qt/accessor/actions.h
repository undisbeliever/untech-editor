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

// idmaplistview.cpp
struct IdmapActions {
    QAction* const add;
    QAction* const clone;
    QAction* const rename;
    QAction* const remove;

    explicit IdmapActions(QWidget* parent);

    void populateMenu(QMenu* menu) const;
    void populateToolbar(QToolBar* toolbar) const;

private:
    friend class IdmapListView;
    void disableAll();
    void updateText(const QString& typeName);
};

// multipleselectiontableview.cpp
struct MultiTableViewActions {
    QList<QAction*> add;
    QAction* const clone;
    QAction* const raise;
    QAction* const lower;
    QAction* const remove;

    explicit MultiTableViewActions(QObject* parent);

    void populateMenu(QMenu* menu, bool addSeperator = true) const;
    void populateMenuWithAddActions(QMenu* menu) const;
    void populateToolbar(QToolBar* toolbar) const;

    void disconnectAll(QObject* o);

private:
    friend class MultipleSelectionTableView;
    void resizeAddList(int count, QObject* parent);
};

// namedlistview.cpp
struct NamedListActions {
    QAction* const add;
    QAction* const clone;
    QAction* const rename;
    QAction* const raise;
    QAction* const lower;
    QAction* const remove;

    explicit NamedListActions(QWidget* parent);

    void populateMenu(QMenu* menu) const;
    void populateToolbar(QToolBar* toolbar) const;

private:
    friend class NamedListView;
    void disableAll();
    void updateText(const QString& typeName);
};
}
}
}
