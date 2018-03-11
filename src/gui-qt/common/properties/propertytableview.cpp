/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertytableview.h"
#include "propertydelegate.h"
#include "propertytablemanager.h"
#include "propertytablemodel.h"

#include <QContextMenuEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>

using namespace UnTech::GuiQt;
using Type = PropertyType;

PropertyTableView::PropertyTableView(QWidget* parent)
    : QTreeView(parent)
    , _model(nullptr)
    , _delegate(new PropertyDelegate(this))
{
    setItemDelegate(_delegate);

    setDragDropMode(QTreeView::InternalMove);

    setEditTriggers(QAbstractItemView::AllEditTriggers);
    setAlternatingRowColors(true);

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto createAction = [this](const char* title, const char* icon,
                               const QKeySequence& shortcut) {
        QAction* a = new QAction(tr(title), this);
        a->setIcon(QIcon(icon));
        a->setShortcut(shortcut);
        a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        a->setShortcutVisibleInContextMenu(true);
        addAction(a);

        return a;
    };

    // NOTE: if you change the move item shortcuts, update `PropertyView::keyPressEvent` to match

    _insertAction = createAction("Insert Item", ":/icons/add.svg", Qt::Key_Insert);
    _cloneAction = createAction("Clone Item", ":/icons/clone.svg", 0);
    _removeAction = createAction("Remove Item", ":/icons/remove.svg", Qt::Key_Delete);
    _raiseAction = createAction("Raise Item", ":/icons/raise.svg", Qt::SHIFT + Qt::Key_PageUp);
    _lowerAction = createAction("Lower Item", ":/icons/lower.svg", Qt::SHIFT + Qt::Key_PageDown);
    _raiseToTopAction = createAction("Raise To Top", ":/icons/raise-to-top.svg", Qt::SHIFT + Qt::Key_Home);
    _lowerToBottomAction = createAction("Lower To Bottom", ":/icons/lower-to-bottom.svg", Qt::SHIFT + Qt::Key_End);

    onSelectionChanged();

    connect(_insertAction, &QAction::triggered,
            this, &PropertyTableView::onInsertActionTriggered);
    connect(_cloneAction, &QAction::triggered,
            this, &PropertyTableView::onCloneActionTriggered);
    connect(_removeAction, &QAction::triggered,
            this, &PropertyTableView::onRemoveActionTriggered);
    connect(_raiseAction, &QAction::triggered,
            this, &PropertyTableView::onRaiseActionTriggered);
    connect(_lowerAction, &QAction::triggered,
            this, &PropertyTableView::onLowerActionTriggered);
    connect(_raiseToTopAction, &QAction::triggered,
            this, &PropertyTableView::onRaiseToTopActionTriggered);
    connect(_lowerToBottomAction, &QAction::triggered,
            this, &PropertyTableView::onLowerToBottomActionTriggered);
}

void PropertyTableView::setPropertyManagers(const QList<PropertyTableManager*>& managers, const QStringList& columns)
{
    if (managers.isEmpty()) {
        setPropertyModel(nullptr);
    }
    else {
        setPropertyModel(new PropertyTableModel(managers, columns, this));
    }
}

void PropertyTableView::setPropertyManager(PropertyTableManager* manager)
{
    if (manager == nullptr) {
        setPropertyModel(nullptr);
    }
    else {
        setPropertyModel(new PropertyTableModel(manager, this));
    }
}

void PropertyTableView::setPropertyModel(PropertyTableModel* model)
{
    if (auto* m = selectionModel()) {
        m->deleteLater();
    }

    if (_model) {
        _model->deleteLater();
    }
    _model = model;
    QTreeView::setModel(model);

    if (model) {
        connect(_model, &PropertyTableModel::rowsMoved,
                this, &PropertyTableView::onSelectionChanged);
    }

    onSelectionChanged();

    if (selectionModel()) {
        connect(_model, &PropertyTableModel::layoutChanged,
                this, &PropertyTableView::onSelectionChanged);
        connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &PropertyTableView::onSelectionChanged);
    }

    this->expandAll();
}

// Must use contextMenuEvent. Using the customContextMenuRequested signal
// results in the context menu's location being off by ~16 pixels
void PropertyTableView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_model == nullptr) {
        return;
    }

    // Only show menu if mouse cursor is on a list type
    if (_insertAction->isEnabled() || _removeAction->isEnabled()) {
        QMenu menu;
        menu.addAction(_insertAction);

        if (_removeAction->isEnabled()) {
            menu.addAction(_cloneAction);
            menu.addSeparator();
            menu.addAction(_raiseAction);
            menu.addAction(_lowerAction);
            menu.addAction(_raiseToTopAction);
            menu.addAction(_lowerToBottomAction);
            menu.addSeparator();
            menu.addAction(_removeAction);
        }

        menu.exec(event->globalPos());
    }
}

void PropertyTableView::keyPressEvent(QKeyEvent* event)
{
    // Do not process the "move item" shortcuts when the action has been disabled
    if ((event->modifiers() == Qt::SHIFT && event->key() == Qt::Key_PageUp)
        || (event->modifiers() == Qt::SHIFT && event->key() == Qt::Key_PageDown)
        || (event->modifiers() == Qt::SHIFT && event->key() == Qt::Key_Home)
        || (event->modifiers() == Qt::SHIFT && event->key() == Qt::Key_End)) {

        return;
    }
    else {
        QTreeView::keyPressEvent(event);
    }
}

void PropertyTableView::onSelectionChanged()
{
    QModelIndex index = currentIndex();
    if (_model) {
        bool isListItem = index.isValid() && index.internalId() != PropertyTableModel::ROOT_INTERNAL_ID;
        bool canInsert = _model->canInsert(index.parent());
        bool canClone = isListItem && _model->canClone(index.row(), index.parent());
        bool canRaise = isListItem && index.sibling(index.row() - 1, 0).isValid();
        bool canLower = isListItem && index.sibling(index.row() + 1, 0).isValid();

        _insertAction->setEnabled(canInsert);
        _cloneAction->setEnabled(canClone);
        _removeAction->setEnabled(isListItem);
        _raiseAction->setEnabled(canRaise);
        _lowerAction->setEnabled(canLower);
        _raiseToTopAction->setEnabled(canRaise);
        _lowerToBottomAction->setEnabled(canLower);
    }
    else {
        _insertAction->setEnabled(false);
        _cloneAction->setEnabled(false);
        _removeAction->setEnabled(false);
        _raiseAction->setEnabled(false);
        _lowerAction->setEnabled(false);
        _raiseToTopAction->setEnabled(false);
        _lowerToBottomAction->setEnabled(false);
    }
}

void PropertyTableView::onInsertActionTriggered()
{
    if (_model == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        int row = index.row() + 1;
        QModelIndex parent = index.parent();

        bool ok = _model->insertRow(row, parent);
        if (ok) {
            QModelIndex newItemIndex = _model->index(row, 0, parent);
            setCurrentIndex(newItemIndex);
        }
    }
}

void PropertyTableView::onCloneActionTriggered()
{
    if (_model == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        QModelIndex parent = index.parent();
        bool ok = _model->cloneRow(index.row(), parent);
        if (ok) {
            int nRows = _model->rowCount(parent);
            QModelIndex newItemIndex = _model->index(nRows - 1, 0, parent);
            setCurrentIndex(newItemIndex);
        }
    }
}

void PropertyTableView::onRemoveActionTriggered()
{
    if (_model == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        _model->removeRow(index.row(), index.parent());
    }
}

void PropertyTableView::onRaiseActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, index.row() - 1);
}

void PropertyTableView::onLowerActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, index.row() + 2);
}

void PropertyTableView::onRaiseToTopActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, 0);
}

void PropertyTableView::onLowerToBottomActionTriggered()
{
    if (_model == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    QModelIndex parent = index.parent();

    int nItems = _model->rowCount(parent);
    _model->moveRow(parent, index.row(), parent, nItems);
}

void PropertyTableView::moveModelRow(const QModelIndex& index, int destRow)
{
    if (_model == nullptr) {
        return;
    }

    if (index.isValid()) {
        QModelIndex parent = index.parent();
        _model->moveRow(parent, index.row(), parent, destRow);
    }
}

void PropertyTableView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in PropertyView.");
}
