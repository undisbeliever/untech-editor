/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistview.h"
#include "abstractaccessors.h"
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

    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &NamedListView::onViewSelectionChanged);

    connect(_actions.add, &QAction::triggered,
            this, &NamedListView::onAddTriggered);
    connect(_actions.clone, &QAction::triggered,
            this, &NamedListView::onCloneTriggered);
    connect(_actions.rename, &QAction::triggered,
            this, &NamedListView::onRenameTriggered);
    connect(_actions.raise, &QAction::triggered,
            this, &NamedListView::onRaiseTriggered);
    connect(_actions.lower, &QAction::triggered,
            this, &NamedListView::onLowerTriggered);
    connect(_actions.remove, &QAction::triggered,
            this, &NamedListView::onRemoveTriggered);
}

void NamedListView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in NamedListView.");
}

void NamedListView::setAccessor(AbstractNamedListAccessor* accessor)
{
    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);
    }
    _accessor = accessor;

    _model->setAccessor(accessor);

    setEnabled(_accessor);
    updateActions();

    if (_accessor) {
        _actions.updateText(_accessor->typeName());
        onAccessorSelectedIndexChanged();

        connect(_accessor, &AbstractNamedListAccessor::selectedIndexChanged,
                this, &NamedListView::onAccessorSelectedIndexChanged);
    }
}

void NamedListView::onAccessorSelectedIndexChanged()
{
    Q_ASSERT(_accessor);

    QModelIndex index = _model->toModelIndex(_accessor->selectedIndex());
    setCurrentIndex(index);

    updateActions();
}

void NamedListView::onViewSelectionChanged()
{
    if (_accessor) {
        size_t index = _model->toIndex(currentIndex());
        _accessor->setSelectedIndex(index);
    }
}

void NamedListView::updateActions()
{
    if (_accessor == nullptr) {
        _actions.disableAll();
        return;
    }

    const size_t selectedIndex = _accessor->selectedIndex();
    const size_t listSize = _accessor->size();
    const size_t maxSize = _accessor->maxSize();

    const bool selectionValid = selectedIndex < listSize;
    const bool canAdd = listSize < maxSize;

    _actions.add->setEnabled(canAdd);
    _actions.clone->setEnabled(selectionValid && canAdd);
    _actions.rename->setEnabled(selectionValid);
    _actions.raise->setEnabled(selectionValid && selectedIndex > 0);
    _actions.lower->setEnabled(selectionValid && selectedIndex + 1 < listSize);
    _actions.remove->setEnabled(selectionValid);
}

void NamedListView::onAddTriggered()
{
    if (_accessor == nullptr) {
        return;
    }
    if (_accessor->isSelectedIndexValid() == false) {
        return;
    }

    const QString typeName = _accessor->typeName();

    idstring name = IdstringDialog::getIdstring(
        this,
        tr("Input %1 Name").arg(typeName),
        tr("Input name of the new %1:").arg(QLocale().toLower(typeName)),
        idstring(), _model->displayList());

    if (name.isValid()) {
        _accessor->addItemWithName(name);
    }
}

void NamedListView::onCloneTriggered()
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
        this,
        tr("Input %1 Name").arg(typeName),
        tr("Input name of the cloned %1:").arg(QLocale().toLower(typeName)),
        currentName, _model->displayList());

    if (newName.isValid()) {
        _accessor->cloneSelectedItemWithName(newName);
    }
}

void NamedListView::onRenameTriggered()
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
        this,
        tr("Input %1 Name").arg(typeName),
        tr("Rename %1: to").arg(currentName),
        currentName, _model->displayList());

    if (newName.isValid()) {
        _accessor->editSelected_setName(newName);
    }
}

void NamedListView::onRaiseTriggered()
{
    if (_accessor) {
        _accessor->raiseSelectedItem();
    }
}

void NamedListView::onLowerTriggered()
{
    if (_accessor) {
        _accessor->lowerSelectedItem();
    }
}

void NamedListView::onRemoveTriggered()
{
    if (_accessor) {
        _accessor->removeSelectedItem();
    }
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
