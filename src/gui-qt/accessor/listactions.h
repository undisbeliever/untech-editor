/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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

struct ListActions {
private:
    QObject* _accessor;

public:
    QAction* const add;
    QAction* const clone;
    QAction* const raise;
    QAction* const lower;
    QAction* const remove;

    explicit ListActions(QObject* parent);

    void updateText(const QString& typeName);

    void disableAll();

    void populateMenu(QMenu* menu) const;
    void populateToolbar(QToolBar* toolbar) const;

    template <class AccessorT>
    void setAccessor(AccessorT* accessor)
    {
        if (_accessor == accessor) {
            return;
        }

        // The object that stores the updateActions signal handlers
        const QObject* reciever = add;

        if (_accessor) {
            _accessor->disconnect(reciever);

            add->disconnect(_accessor);
            clone->disconnect(_accessor);
            raise->disconnect(_accessor);
            lower->disconnect(_accessor);
            remove->disconnect(_accessor);
        }
        _accessor = accessor;

        if (accessor == nullptr) {
            disableAll();
            return;
        }

        updateText(accessor->typeName());

        auto updateActions = [=]() {
            ListActionStatus sel = ListActionHelper::status(accessor);

            add->setEnabled(sel.canAdd);
            clone->setEnabled(sel.canClone);
            raise->setEnabled(sel.canRaise);
            lower->setEnabled(sel.canLower);
            remove->setEnabled(sel.canRemove);
        };
        updateActions();

        using UndoHelper = ListAndSelectionUndoHelper<AccessorT>;

        QObject::connect(accessor, &AccessorT::listReset,
                         reciever, updateActions);
        QObject::connect(accessor, &AccessorT::selectedIndexChanged,
                         reciever, updateActions);
        QObject::connect(accessor, &AccessorT::listChanged,
                         reciever, updateActions);

        QObject::connect(add, &QAction::triggered,
                         accessor, [=]() { UndoHelper(accessor).addItemToSelectedList(); });
        QObject::connect(clone, &QAction::triggered,
                         accessor, [=]() { UndoHelper(accessor).cloneSelectedItem(); });
        QObject::connect(raise, &QAction::triggered,
                         accessor, [=]() { UndoHelper(accessor).raiseSelectedItem(); });
        QObject::connect(lower, &QAction::triggered,
                         accessor, [=]() { UndoHelper(accessor).lowerSelectedItem(); });
        QObject::connect(remove, &QAction::triggered,
                         accessor, [=]() { UndoHelper(accessor).removeSelectedItem(); });
    }
};

}
}
}
