/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistactions.h"
#include "abstractaccessors.h"
#include "gui-qt/common/actionhelpers.h"
#include "gui-qt/common/idstringdialog.h"

#include <QLocale>

using namespace UnTech::GuiQt::Accessor;

NamedListActions::NamedListActions(QWidget* parent)
    : QObject(parent)
    , add(createAction(parent, ":/icons/add.svg", "Add", Qt::Key_Insert))
    , clone(createAction(parent, ":/icons/clone.svg", "Clone Selected", Qt::CTRL + Qt::Key_D))
    , rename(createAction(parent, ":/icons/rename.svg", "Rename Selected", 0))
    , raiseToTop(createAction(this, ":/icons/raise-to-top.svg", "Raise Selected To Top", Qt::SHIFT + Qt::Key_Home))
    , raise(createAction(parent, ":/icons/raise.svg", "Raise Selected", Qt::SHIFT + Qt::Key_PageUp))
    , lower(createAction(parent, ":/icons/lower.svg", "Lower Selected", Qt::SHIFT + Qt::Key_PageDown))
    , lowerToBottom(createAction(this, ":/icons/lower-to-bottom.svg", "Lower Selected To Bottom", Qt::SHIFT + Qt::Key_End))
    , remove(createAction(parent, ":/icons/remove.svg", "Remove Selected", Qt::Key_Delete))
    , _widget(parent)
    , _accessor(nullptr)
{
    setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(add, &QAction::triggered,
            this, &NamedListActions::onAddTriggered);
    connect(clone, &QAction::triggered,
            this, &NamedListActions::onCloneTriggered);
    connect(rename, &QAction::triggered,
            this, &NamedListActions::onRenameTriggered);
    connect(raiseToTop, &QAction::triggered,
            this, &NamedListActions::onRaiseToTopTriggered);
    connect(raise, &QAction::triggered,
            this, &NamedListActions::onRaiseTriggered);
    connect(lower, &QAction::triggered,
            this, &NamedListActions::onLowerTriggered);
    connect(lowerToBottom, &QAction::triggered,
            this, &NamedListActions::onLowerToBottomTriggered);
    connect(remove, &QAction::triggered,
            this, &NamedListActions::onRemoveTriggered);
}

void NamedListActions::setShortcutContext(Qt::ShortcutContext context)
{
    add->setShortcutContext(context);
    clone->setShortcutContext(context);
    raiseToTop->setShortcutContext(context);
    raise->setShortcutContext(context);
    lower->setShortcutContext(context);
    lowerToBottom->setShortcutContext(context);
    remove->setShortcutContext(context);
}

void NamedListActions::populate(QWidget* widget) const
{
    widget->addAction(add);
    widget->addAction(clone);
    widget->addAction(rename);
    widget->addAction(raiseToTop);
    widget->addAction(raise);
    widget->addAction(lower);
    widget->addAction(lowerToBottom);
    widget->addAction(remove);
}

void NamedListActions::setAccessor(AbstractNamedListAccessor* accessor)
{
    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);
    }
    _accessor = accessor;

    updateActions();

    if (_accessor) {
        updateText(_accessor->typeName());

        connect(_accessor, &AbstractNamedListAccessor::selectedIndexChanged,
                this, &NamedListActions::updateActions);
    }
}

void NamedListActions::updateText(const QString& typeName)
{
    add->setText(tr("Add %1").arg(typeName));
    clone->setText(tr("Clone %1").arg(typeName));
    rename->setText(tr("Rename %1").arg(typeName));
    raiseToTop->setText(tr("Raise %1 To Top").arg(typeName));
    raise->setText(tr("Raise %1").arg(typeName));
    lower->setText(tr("Lower %1").arg(typeName));
    lowerToBottom->setText(tr("Lower %1 To Bottom").arg(typeName));
    remove->setText(tr("Remove %1").arg(typeName));
}

void NamedListActions::disableAll()
{
    add->setEnabled(false);
    clone->setEnabled(false);
    rename->setEnabled(false);
    raiseToTop->setEnabled(false);
    raise->setEnabled(false);
    lower->setEnabled(false);
    lowerToBottom->setEnabled(false);
    remove->setEnabled(false);
}

void NamedListActions::updateActions()
{
    if (_accessor == nullptr) {
        disableAll();
        return;
    }

    const size_t selectedIndex = _accessor->selectedIndex();
    const bool listExists = _accessor->listExists();
    const size_t listSize = _accessor->size();
    const size_t maxSize = _accessor->maxSize();

    const bool selectionValid = listExists && selectedIndex < listSize;
    const bool canAdd = listExists && listSize < maxSize;
    const bool canRaise = selectionValid && selectedIndex > 0;
    const bool canLower = selectionValid && selectedIndex + 1 < listSize;

    add->setEnabled(canAdd);
    clone->setEnabled(selectionValid && canAdd);
    rename->setEnabled(selectionValid);
    raiseToTop->setEnabled(canRaise);
    raise->setEnabled(canRaise);
    lower->setEnabled(canLower);
    lowerToBottom->setEnabled(canLower);
    remove->setEnabled(selectionValid);
}

void NamedListActions::onAddTriggered()
{
    if (_accessor == nullptr) {
        return;
    }
    if (_accessor->listExists() == false) {
        return;
    }

    const QString typeName = _accessor->typeName();

    idstring name = IdstringDialog::getIdstring(
        _widget,
        tr("Input %1 Name").arg(typeName),
        tr("Input name of the new %1:").arg(QLocale().toLower(typeName)),
        idstring(), _accessor->itemNames());

    if (name.isValid()) {
        _accessor->addItemWithName(name);
    }
}

void NamedListActions::onCloneTriggered()
{
    if (_accessor == nullptr) {
        return;
    }
    if (_accessor->isSelectedIndexValid() == false) {
        return;
    }

    const QString typeName = _accessor->typeName();
    const QString currentName = _accessor->itemName(_accessor->selectedIndex());

    const idstring newName = IdstringDialog::getIdstring(
        _widget,
        tr("Input %1 Name").arg(typeName),
        tr("Input name of the cloned %1:").arg(QLocale().toLower(typeName)),
        currentName, _accessor->itemNames());

    if (newName.isValid()) {
        _accessor->cloneSelectedItemWithName(newName);
    }
}

void NamedListActions::onRenameTriggered()
{
    if (_accessor == nullptr) {
        return;
    }
    if (_accessor->isSelectedIndexValid() == false) {
        return;
    }

    const QString typeName = _accessor->typeName();
    const QString currentName = _accessor->itemName(_accessor->selectedIndex());

    const idstring newName = IdstringDialog::getIdstring(
        _widget,
        tr("Input %1 Name").arg(typeName),
        tr("Rename %1: to").arg(currentName),
        currentName, _accessor->itemNames());

    if (newName.isValid()) {
        _accessor->editSelected_setName(newName);
    }
}

void NamedListActions::onRaiseToTopTriggered()
{
    if (_accessor) {
        _accessor->raiseSelectedItemToTop();
    }
}

void NamedListActions::onRaiseTriggered()
{
    if (_accessor) {
        _accessor->raiseSelectedItem();
    }
}

void NamedListActions::onLowerTriggered()
{
    if (_accessor) {
        _accessor->lowerSelectedItem();
    }
}

void NamedListActions::onLowerToBottomTriggered()
{
    if (_accessor) {
        _accessor->lowerSelectedItemToBottom();
    }
}

void NamedListActions::onRemoveTriggered()
{
    if (_accessor) {
        _accessor->removeSelectedItem();
    }
}
