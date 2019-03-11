/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>

namespace UnTech {
namespace GuiQt {

inline QAction* createAction(QObject* parent, const char* icon, const char* text, QKeySequence shortcut)
{
    QAction* a = new QAction(QIcon(icon), parent->tr(text), parent);
    a->setShortcut(shortcut);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    a->setShortcutVisibleInContextMenu(true);
#endif

    return a;
}

}
}
