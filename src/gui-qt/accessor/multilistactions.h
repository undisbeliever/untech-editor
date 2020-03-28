/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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
    QAction* const selectAll;
    QAction* const clone;
    QAction* const raiseToTop;
    QAction* const raise;
    QAction* const lower;
    QAction* const lowerToBottom;
    QAction* const remove;

private:
    QList<AbstractListMultipleSelectionAccessor*> _accessors;

public:
    MultiListActions(QObject* parent = nullptr);
    ~MultiListActions() = default;

    int nAccessors() const { return add.size(); }
    void setNAccessors(int nAccessors);

    const QList<QAction*>& addActions() const { return add; }
    QAction* addAction(int i) const { return add.at(i); }

    const QList<AbstractListMultipleSelectionAccessor*>& accessors() const { return _accessors; }
    void setAccessors(QList<AbstractListMultipleSelectionAccessor*> accessors);

    void setShortcutContext(Qt::ShortcutContext context);

    void populateAddActions(QWidget* widget) const;
    void populateNotAddActions(QWidget* widget) const;
    void populate(QWidget* widget, bool addSeperator = false) const;

public slots:
    void disableAll();
    void updateActions();

private slots:
    void onAddTriggered();
    void onSelectAllTriggered();
    void onCloneTriggered();
    void onRaiseToTopTriggered();
    void onRaiseTriggered();
    void onLowerTriggered();
    void onLowerToBottomTriggered();
    void onRemoveTriggered();

private:
    AbstractListMultipleSelectionAccessor* onlyOneAccessorWithASelection();
    QUndoStack* undoStack() const;
};

}
}
}
