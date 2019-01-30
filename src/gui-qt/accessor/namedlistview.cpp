/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistview.h"
#include "accessor.h"
#include "gui-qt/common/idstringdialog.h"

#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QMenu>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

using QCA = QCoreApplication;

NamedListActions::NamedListActions(QWidget* parent)
    : add(new QAction(QIcon(":/icons/add.svg"), QCA::tr("Add"), parent))
    , clone(new QAction(QIcon(":/icons/clone.svg"), QCA::tr("Clone"), parent))
    , rename(new QAction(QIcon(":/icons/rename.svg"), QCA::tr("Rename"), parent))
    , raise(new QAction(QIcon(":/icons/raise.svg"), QCA::tr("Raise Selected"), parent))
    , lower(new QAction(QIcon(":/icons/lower.svg"), QCA::tr("Lower Selected"), parent))
    , remove(new QAction(QIcon(":/icons/remove.svg"), QCA::tr("Remove"), parent))
{
}

void NamedListActions::populateMenu(QMenu* menu) const
{
    menu->addAction(add);
    menu->addAction(clone);
    menu->addAction(rename);
    menu->addAction(raise);
    menu->addAction(lower);
    menu->addAction(remove);
}

void NamedListActions::populateToolbar(QToolBar* toolbar) const
{
    toolbar->addAction(add);
    toolbar->addAction(clone);
    toolbar->addAction(rename);
    toolbar->addAction(raise);
    toolbar->addAction(lower);
    toolbar->addAction(remove);
}

void NamedListActions::updateText(const QString& typeName)
{
    add->setText(QCoreApplication::tr("Add %1").arg(typeName));
    clone->setText(QCoreApplication::tr("Clone %1").arg(typeName));
    rename->setText(QCoreApplication::tr("Rename %1").arg(typeName));
    remove->setText(QCoreApplication::tr("Remove %1").arg(typeName));
}

void NamedListActions::disableAll()
{
    add->setEnabled(false);
    clone->setEnabled(false);
    rename->setEnabled(false);
    raise->setEnabled(false);
    lower->setEnabled(false);
    remove->setEnabled(false);
}

NamedListView::NamedListView(QWidget* parent)
    : QListView(parent)
    , _actions(this)
    , _model(new NamedListModel(this))
    , _selectedContextMenu(new QMenu(this))
    , _noSelectionContextMenu(new QMenu(this))
    , _accessor(nullptr)
{
    QListView::setModel(_model);

    setEditTriggers(EditTrigger::NoEditTriggers);

    setSelectionMode(SelectionMode::SingleSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);

    this->addAction(_actions.add);
    this->addAction(_actions.clone);
    this->addAction(_actions.rename);
    this->addAction(_actions.remove);

    _actions.populateMenu(_selectedContextMenu);
    _noSelectionContextMenu->addAction(_actions.add);
}

void NamedListView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in NamedListView.");
}

// Must use contextMenuEvent. Using the customContextMenuRequested signal
// results in the context menu's location being off by ~16 pixels
void NamedListView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_accessor == nullptr || _actions.add->isEnabled() == false) {
        return;
    }

    if (_actions.remove->isEnabled()) {
        _selectedContextMenu->exec(event->globalPos());
    }
    else {
        _noSelectionContextMenu->exec(event->globalPos());
    }
}

idstring NamedListView::addIdstringDialog(const QString& typeName)
{
    return IdstringDialog::getIdstring(
        this,
        tr("Input %1 Name").arg(typeName),
        tr("Input name of the new %1:").arg(QLocale().toLower(typeName)),
        idstring(), _model->displayList());
}

idstring NamedListView::cloneIdstringDialog(const idstring& id, const QString& typeName)
{
    return IdstringDialog::getIdstring(
        this,
        tr("Input %1 Name").arg(typeName),
        tr("Input name of the cloned %1:").arg(QLocale().toLower(typeName)),
        id, _model->displayList());
}

idstring NamedListView::renameIdstringDialog(const idstring& oldId, const QString& typeName)
{
    return IdstringDialog::getIdstring(
        this,
        tr("Input %1 Name").arg(typeName),
        tr("Rename %1: to").arg(QString::fromStdString(oldId)),
        oldId, _model->displayList());
}
