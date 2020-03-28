/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "listactionhelper.h"
#include "listundohelper.h"
#include <QAction>
#include <QToolBar>

namespace UnTech {
namespace GuiQt {
namespace Accessor {
class AbstractListAccessor;
class AbstractListSingleSelectionAccessor;
class AbstractListMultipleSelectionAccessor;

struct ListActions : public QObject {
    Q_OBJECT

public:
    QAction* const add;
    QAction* const selectAll; // Only enabled and visible on AbstractListMultipleSelectionAccessor
    QAction* const clone;
    QAction* const raiseToTop;
    QAction* const raise;
    QAction* const lower;
    QAction* const lowerToBottom;
    QAction* const remove;

private:
    AbstractListAccessor* _accessor;

public:
    ListActions(QObject* parent);
    ~ListActions() = default;

    AbstractListAccessor* accessor() const { return _accessor; }

    void setShortcutContext(Qt::ShortcutContext context);

    void updateText(const QString& typeName);

    void disableAll();

    void populate(QWidget* widget) const;

    void setAccessor(AbstractListSingleSelectionAccessor* accessor);
    void setAccessor(AbstractListMultipleSelectionAccessor* accessor);

private slots:
    void updateActions_singleSelection();
    void updateActions_multipleSelection();
};

}
}
}
