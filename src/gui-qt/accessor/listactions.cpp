/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listactions.h"
#include "abstractaccessors.h"
#include "accessor.h"
#include "gui-qt/common/idstringdialog.h"

#include <QMenu>

using namespace UnTech::GuiQt::Accessor;

ListActions::ListActions(QObject* parent)
    : QObject(parent)
    , add(new QAction(QIcon(":/icons/add.svg"), tr("Add"), this))
    , clone(new QAction(QIcon(":/icons/clone.svg"), tr("Clone Selected"), this))
    , raise(new QAction(QIcon(":/icons/raise.svg"), tr("Raise Selected"), this))
    , lower(new QAction(QIcon(":/icons/lower.svg"), tr("Lower Selected"), this))
    , remove(new QAction(QIcon(":/icons/remove.svg"), tr("Remove Selected"), this))
    , _accessor(nullptr)
{
    disableAll();
}

void ListActions::updateText(const QString& typeName)
{
    add->setText(tr("Add %1").arg(typeName));
    clone->setText(tr("Clone %1").arg(typeName));
    raise->setText(tr("Raise %1").arg(typeName));
    lower->setText(tr("Lower %1").arg(typeName));
    remove->setText(tr("Remove %1").arg(typeName));
}

void ListActions::setAccessor(AbstractListSingleSelectionAccessor* accessor)
{
    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);

        add->disconnect(_accessor);
        clone->disconnect(_accessor);
        raise->disconnect(_accessor);
        lower->disconnect(_accessor);
        remove->disconnect(_accessor);
    }
    _accessor = accessor;

    updateActions_singleSelection();

    if (accessor) {
        updateText(_accessor->typeName());

        connect(accessor, &AbstractListSingleSelectionAccessor::listReset,
                this, &ListActions::updateActions_singleSelection);
        connect(accessor, &AbstractListSingleSelectionAccessor::selectedIndexChanged,
                this, &ListActions::updateActions_singleSelection);
        connect(accessor, &AbstractListSingleSelectionAccessor::listChanged,
                this, &ListActions::updateActions_singleSelection);

        connect(add, &QAction::triggered,
                accessor, qOverload<>(&AbstractListAccessor::addItem));
        connect(clone, &QAction::triggered,
                accessor, &AbstractListSingleSelectionAccessor::cloneSelectedItem);
        connect(raise, &QAction::triggered,
                accessor, &AbstractListSingleSelectionAccessor::raiseSelectedItem);
        connect(lower, &QAction::triggered,
                accessor, &AbstractListSingleSelectionAccessor::lowerSelectedItem);
        connect(remove, &QAction::triggered,
                accessor, &AbstractListSingleSelectionAccessor::removeSelectedItem);
    }
}

void ListActions::setAccessor(AbstractListMultipleSelectionAccessor* accessor)
{
    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);

        add->disconnect(_accessor);
        clone->disconnect(_accessor);
        raise->disconnect(_accessor);
        lower->disconnect(_accessor);
        remove->disconnect(_accessor);
    }
    _accessor = accessor;

    updateActions_multipleSelection();

    if (accessor) {
        updateText(_accessor->typeName());

        connect(accessor, &AbstractListMultipleSelectionAccessor::listReset,
                this, &ListActions::updateActions_multipleSelection);
        connect(accessor, &AbstractListMultipleSelectionAccessor::selectedIndexesChanged,
                this, &ListActions::updateActions_multipleSelection);
        connect(accessor, &AbstractListMultipleSelectionAccessor::listChanged,
                this, &ListActions::updateActions_multipleSelection);

        connect(add, &QAction::triggered,
                accessor, qOverload<>(&AbstractListAccessor::addItem));
        connect(clone, &QAction::triggered,
                accessor, &AbstractListMultipleSelectionAccessor::cloneSelectedItems);
        connect(raise, &QAction::triggered,
                accessor, &AbstractListMultipleSelectionAccessor::raiseSelectedItems);
        connect(lower, &QAction::triggered,
                accessor, &AbstractListMultipleSelectionAccessor::lowerSelectedItems);
        connect(remove, &QAction::triggered,
                accessor, &AbstractListMultipleSelectionAccessor::removeSelectedItems);
    }
}

void ListActions::disableAll()
{
    add->setEnabled(false);
    clone->setEnabled(false);
    raise->setEnabled(false);
    lower->setEnabled(false);
    remove->setEnabled(false);
}

void ListActions::updateActions_singleSelection()
{
    auto* accessor = qobject_cast<AbstractListSingleSelectionAccessor*>(_accessor);
    if (accessor == nullptr) {
        disableAll();
        return;
    }

    if (accessor->listExists() == false) {
        disableAll();
        return;
    }

    const size_t selectedIndex = accessor->selectedIndex();
    const size_t listSize = accessor->size();
    const size_t maxSize = accessor->maxSize();

    const bool selectionValid = selectedIndex < listSize;
    const bool canAdd = listSize < maxSize;

    add->setEnabled(canAdd);
    clone->setEnabled(selectionValid && canAdd);
    raise->setEnabled(selectionValid && selectedIndex > 0);
    lower->setEnabled(selectionValid && selectedIndex + 1 < listSize);
    remove->setEnabled(selectionValid);
}

void ListActions::updateActions_multipleSelection()
{
    auto* accessor = qobject_cast<AbstractListMultipleSelectionAccessor*>(_accessor);
    if (accessor == nullptr) {
        disableAll();
        return;
    }

    if (accessor->listExists() == false) {
        disableAll();
        return;
    }

    const vectorset<size_t>& indexes = accessor->selectedIndexes();
    const bool listExists = accessor->listExists();
    const size_t listSize = accessor->size();
    const size_t maxSize = accessor->maxSize();

    const bool selectionValid = listExists && !indexes.empty() && indexes.back() < listSize;

    add->setEnabled(listExists && listSize < maxSize);
    clone->setEnabled(selectionValid && listSize + indexes.size() <= maxSize);
    raise->setEnabled(selectionValid && indexes.front() > 0);
    lower->setEnabled(selectionValid && indexes.back() + 1 < listSize);
    remove->setEnabled(selectionValid);
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
