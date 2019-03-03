/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QToolBar>
#include <QUndoStack>

namespace UnTech {
namespace GuiQt {
namespace Accessor {
class AbstractListMultipleSelectionAccessor;

struct MultiListActions : public QObject {
    Q_OBJECT

private:
    QList<QAction*> add;

public:
    QAction* const clone;
    QAction* const raise;
    QAction* const lower;
    QAction* const remove;

private:
    QList<AbstractListMultipleSelectionAccessor*> _accessors;

public:
    MultiListActions(QObject* parent = nullptr);
    ~MultiListActions() = default;

    int nAccessors() const { return add.size(); }
    void setNAccessors(int nAccessors);

    QAction* addAction(int i) const { return add.at(i); }

    const QList<AbstractListMultipleSelectionAccessor*>& accessors() const { return _accessors; }
    void setAccessors(QList<AbstractListMultipleSelectionAccessor*> accessors);

    void populateMenu(QMenu* menu, bool addSeperator = true) const;
    void populateMenuWithAddActions(QMenu* menu) const;
    void populateToolbar(QToolBar* toolbar) const;

public slots:
    void disableAll();
    void updateActions();

private slots:
    void onAddTriggered();
    void onCloneTriggered();
    void onRaiseTriggered();
    void onLowerTriggered();
    void onRemoveTriggered();

private:
    AbstractListMultipleSelectionAccessor* onlyOneAccessorWithASelection();
    QUndoStack* undoStack() const;
};

}
}
}
