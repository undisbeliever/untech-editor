/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listactions.h"
#include "accessor.h"
#include "gui-qt/common/idstringdialog.h"

#include <QCoreApplication>
#include <QMenu>

using namespace UnTech::GuiQt::Accessor;

using QCA = QCoreApplication;

ListActions::ListActions(QObject* parent)
    : _accessor(nullptr)
    , add(new QAction(QIcon(":/icons/add.svg"), QCA::tr("Add"), parent))
    , clone(new QAction(QIcon(":/icons/clone.svg"), QCA::tr("Clone Selected"), parent))
    , raise(new QAction(QIcon(":/icons/raise.svg"), QCA::tr("Raise Selected"), parent))
    , lower(new QAction(QIcon(":/icons/lower.svg"), QCA::tr("Lower Selected"), parent))
    , remove(new QAction(QIcon(":/icons/remove.svg"), QCA::tr("Remove Selected"), parent))
{
    disableAll();
}

void ListActions::updateText(const QString& typeName)
{
    add->setText(QCoreApplication::tr("Add %1").arg(typeName));
    clone->setText(QCoreApplication::tr("Clone %1").arg(typeName));
    raise->setText(QCoreApplication::tr("Raise %1").arg(typeName));
    lower->setText(QCoreApplication::tr("Lower %1").arg(typeName));
    remove->setText(QCoreApplication::tr("Remove %1").arg(typeName));
}

void ListActions::disableAll()
{
    add->setEnabled(false);
    clone->setEnabled(false);
    raise->setEnabled(false);
    lower->setEnabled(false);
    remove->setEnabled(false);
}

void ListActions::populateMenu(QMenu* menu) const
{
    menu->addAction(add);
    menu->addAction(clone);
    menu->addAction(raise);
    menu->addAction(lower);
    menu->addAction(remove);
}

void ListActions::populateToolbar(QToolBar* toolbar) const
{
    toolbar->addAction(add);
    toolbar->addAction(clone);
    toolbar->addAction(raise);
    toolbar->addAction(lower);
    toolbar->addAction(remove);
}
