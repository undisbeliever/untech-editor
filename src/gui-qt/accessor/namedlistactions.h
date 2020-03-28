/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "listactionhelper.h"
#include <QAction>

namespace UnTech {
namespace GuiQt {
namespace Accessor {
class AbstractNamedListAccessor;

struct NamedListActions : public QObject {
    Q_OBJECT

public:
    QAction* const add;
    QAction* const clone;
    QAction* const rename;
    QAction* const raiseToTop;
    QAction* const raise;
    QAction* const lower;
    QAction* const lowerToBottom;
    QAction* const remove;

private:
    QWidget* _widget;
    AbstractNamedListAccessor* _accessor;

public:
    explicit NamedListActions(QWidget* parent);

    void setShortcutContext(Qt::ShortcutContext context);

    void populate(QWidget* widget) const;

    void setAccessor(AbstractNamedListAccessor* accessor);

private:
    void disableAll();
    void updateText(const QString& typeName);

private slots:
    void onAddTriggered();
    void onCloneTriggered();
    void onRenameTriggered();
    void onRaiseToTopTriggered();
    void onRaiseTriggered();
    void onLowerTriggered();
    void onLowerToBottomTriggered();
    void onRemoveTriggered();

private:
    void updateActions();
};

}
}
}
