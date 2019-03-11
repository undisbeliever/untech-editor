/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 201, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "multilistactions.h"
#include "abstractaccessors.h"
#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/common/actionhelpers.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

MultiListActions::MultiListActions(QObject* parent)
    : QObject(parent)
    , add()
    , selectAll(createAction(this, "", "Select All", Qt::CTRL + Qt::Key_A))
    , clone(createAction(this, ":/icons/clone.svg", "Clone Selected", Qt::CTRL + Qt::Key_D))
    , raise(createAction(this, ":/icons/raise.svg", "Raise Selected", Qt::SHIFT + Qt::Key_PageUp))
    , lower(createAction(this, ":/icons/lower.svg", "Lower Selected", Qt::SHIFT + Qt::Key_PageDown))
    , remove(createAction(this, ":/icons/remove.svg", "Remove Selected", Qt::Key_Delete))
    , _accessors()
{
    setShortcutContext(Qt::WidgetWithChildrenShortcut);

    disableAll();

    connect(selectAll, &QAction::triggered,
            this, &MultiListActions::onSelectAllTriggered);
    connect(clone, &QAction::triggered,
            this, &MultiListActions::onCloneTriggered);
    connect(raise, &QAction::triggered,
            this, &MultiListActions::onRaiseTriggered);
    connect(lower, &QAction::triggered,
            this, &MultiListActions::onLowerTriggered);
    connect(remove, &QAction::triggered,
            this, &MultiListActions::onRemoveTriggered);
}

void MultiListActions::setNAccessors(int nAccessors)
{
    if (add.size() == nAccessors) {
        return;
    }

    Q_ASSERT(nAccessors > 1);

    add.clear();
    add.reserve(nAccessors);

    _accessors.clear();
    _accessors.reserve(nAccessors);

    for (int i = 0; i < nAccessors; i++) {
        _accessors.append(nullptr);

        auto* a = createAction(this, ":/icons/add.svg", "Add", 0);
        a->setData(i);
        a->setShortcutContext(clone->shortcutContext());
        connect(a, &QAction::triggered,
                this, &MultiListActions::onAddTriggered);

        add.append(a);
    }

    disableAll();
}

void MultiListActions::populateAddActions(QWidget* widget) const
{
    for (QAction* a : add) {
        widget->addAction(a);
    }
}

void MultiListActions::populate(QWidget* widget, bool addSeperator) const
{
    for (QAction* a : add) {
        widget->addAction(a);
    }
    if (addSeperator) {
        auto* sep = new QAction(widget);
        sep->setSeparator(true);
        widget->addAction(sep);
    }
    if (qobject_cast<QToolBar*>(widget) == nullptr) {
        // do not add selectAll to a QToolBar
        widget->addAction(selectAll);
    }
    widget->addAction(clone);
    widget->addAction(raise);
    widget->addAction(lower);
    widget->addAction(remove);
}

void MultiListActions::setAccessors(QList<AbstractListMultipleSelectionAccessor*> accessors)
{
    if (accessors.size() > 0 && accessors.size() != add.size()) {
        qWarning("invalid size (missing setNAccessors() call?)");
        return;
    }

    // Ensure all accessors not nullptr
    for (auto* a : accessors) {
        if (a == nullptr) {
            accessors.clear();
            break;
        }
    }

    if (_accessors == accessors) {
        return;
    }

    // Ensure all accessors use the same resource item
    if (accessors.isEmpty() == false) {
        const auto* firstResourceItem = accessors.first()->resourceItem();
        for (int i = 0; i < accessors.size(); i++) {
            if (accessors.at(i)->resourceItem() != firstResourceItem) {
                qFatal("All accessors in MultiListActions must use the same resourceItem");
            }
        }
    }

    for (auto* a : _accessors) {
        if (a) {
            a->disconnect(this);
        }
    }
    _accessors = accessors;

    if (_accessors.isEmpty() == false) {
        for (int i = 0; i < _accessors.size(); i++) {
            auto* accessor = _accessors.at(i);
            add.at(i)->setText(tr("Add %1").arg(accessor->typeName()));

            connect(accessor, &AbstractListMultipleSelectionAccessor::listReset,
                    this, &MultiListActions::updateActions);
            connect(accessor, &AbstractListMultipleSelectionAccessor::selectedIndexesChanged,
                    this, &MultiListActions::updateActions);
            connect(accessor, &AbstractListMultipleSelectionAccessor::listChanged,
                    this, &MultiListActions::updateActions);
        }
    }

    updateActions();
}

void MultiListActions::setShortcutContext(Qt::ShortcutContext context)
{
    for (QAction* a : add) {
        a->setShortcutContext(context);
    }
    selectAll->setShortcutContext(context);
    clone->setShortcutContext(context);
    raise->setShortcutContext(context);
    lower->setShortcutContext(context);
    remove->setShortcutContext(context);
}

void MultiListActions::disableAll()
{
    for (auto* a : add) {
        a->setEnabled(false);
    }
    selectAll->setEnabled(false);
    clone->setEnabled(false);
    raise->setEnabled(false);
    lower->setEnabled(false);
    remove->setEnabled(false);
}

void MultiListActions::updateActions()
{
    if (add.size() != _accessors.size()) {
        disableAll();
        return;
    }

    bool canSelectAll = false;
    bool tooManySelectedToClone = false;
    bool canClone = false;
    bool canRaise = false;
    bool canLower = false;
    bool canRemove = false;

    for (int i = 0; i < _accessors.size(); i++) {
        auto* accessor = _accessors.at(i);

        const vectorset<size_t>& indexes = accessor->selectedIndexes();
        bool listExists = accessor->listExists();
        const size_t listSize = accessor->size();
        const size_t maxSize = accessor->maxSize();

        const bool selectionValid = listExists && !indexes.empty() && indexes.back() < listSize;

        add.at(i)->setEnabled(listExists && listSize < maxSize);

        canSelectAll |= listExists;
        tooManySelectedToClone |= listSize + indexes.size() > maxSize;
        canClone |= selectionValid && listSize + indexes.size() <= maxSize;
        canRaise |= selectionValid && indexes.front() > 0;
        canLower |= selectionValid && indexes.back() + 1 < listSize;
        canRemove |= selectionValid;
    }

    selectAll->setEnabled(canSelectAll);
    clone->setEnabled(canClone & !tooManySelectedToClone);
    raise->setEnabled(canRaise);
    lower->setEnabled(canLower);
    remove->setEnabled(canRemove);
}

void MultiListActions::onAddTriggered()
{
    if (auto* action = qobject_cast<QAction*>(sender())) {
        bool ok = false;
        int index = action->data().toInt(&ok);
        if (auto* accessor = _accessors.value(index, nullptr)) {
            accessor->addItem(accessor->size());

            for (auto* a : _accessors) {
                if (a != accessor) {
                    a->clearSelection();
                }
            }
        }
    }
}

void MultiListActions::onSelectAllTriggered()
{
    for (auto* a : _accessors) {
        a->selectAll();
    }
}

void MultiListActions::onCloneTriggered()
{
    if (auto* singleAccessor = onlyOneAccessorWithASelection()) {
        singleAccessor->cloneSelectedItems();
    }
    else {
        if (auto* us = undoStack()) {
            us->beginMacro(tr("Clone Selected Items"));
            for (auto* a : _accessors) {
                a->cloneSelectedItems();
            }
            us->endMacro();
        }
    }
}

void MultiListActions::onRaiseTriggered()
{
    if (auto* singleAccessor = onlyOneAccessorWithASelection()) {
        singleAccessor->raiseSelectedItems();
    }
    else {
        if (auto* us = undoStack()) {
            us->beginMacro(tr("Raise Selected Items"));
            for (auto* a : _accessors) {
                a->raiseSelectedItems();
            }
            us->endMacro();
        }
    }
}

void MultiListActions::onLowerTriggered()
{
    if (auto* singleAccessor = onlyOneAccessorWithASelection()) {
        singleAccessor->lowerSelectedItems();
    }
    else {
        if (auto* us = undoStack()) {
            us->beginMacro(tr("Lower Selected Items"));
            for (auto* a : _accessors) {
                a->lowerSelectedItems();
            }
            us->endMacro();
        }
    }
}

void MultiListActions::onRemoveTriggered()
{
    if (auto* singleAccessor = onlyOneAccessorWithASelection()) {
        singleAccessor->removeSelectedItems();
    }
    else {
        if (auto* us = undoStack()) {
            us->beginMacro(tr("Remove Selected Items"));
            for (auto* a : _accessors) {
                a->removeSelectedItems();
            }
            us->endMacro();
        }
    }
}

// If there is only one accessor with a selection then return that selection
// If no accessors have a selection return nullptr
// If more than one accessors have a selection return nullptr
AbstractListMultipleSelectionAccessor* MultiListActions::onlyOneAccessorWithASelection()
{
    AbstractListMultipleSelectionAccessor* ret = nullptr;

    for (auto* a : _accessors) {
        if (a->selectedIndexes().empty() == false) {
            if (ret) {
                return nullptr;
            }
            ret = a;
        }
    }
    return ret;
}

QUndoStack* MultiListActions::undoStack() const
{
    if (_accessors.empty()) {
        return nullptr;
    }
    return _accessors.first()->resourceItem()->undoStack();
}
