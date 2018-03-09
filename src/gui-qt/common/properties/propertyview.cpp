/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertyview.h"
#include "propertydelegate.h"
#include "propertymanager.h"
#include "propertymodel.h"

#include <QContextMenuEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>

using namespace UnTech::GuiQt;

PropertyView::PropertyView(QWidget* parent)
    : QTreeView(parent)
    , _model(nullptr)
    , _manager(nullptr)
    , _delegate(new PropertyDelegate(this))
{
    setItemDelegate(_delegate);

    setMinimumWidth(250);

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
    _removeAction = createAction("Remove Item", ":/icons/remove.svg", Qt::Key_Delete);
    _raiseAction = createAction("Raise Item", ":/icons/raise.svg", Qt::SHIFT + Qt::Key_PageUp);
    _lowerAction = createAction("Lower Item", ":/icons/lower.svg", Qt::SHIFT + Qt::Key_PageDown);
    _raiseToTopAction = createAction("Raise To Top", ":/icons/raise-to-top.svg", Qt::SHIFT + Qt::Key_Home);
    _lowerToBottomAction = createAction("Lower To Bottom", ":/icons/lower-to-bottom.svg", Qt::SHIFT + Qt::Key_End);

    onSelectionChanged();

    connect(_insertAction, &QAction::triggered,
            this, &PropertyView::onInsertActionTriggered);
    connect(_removeAction, &QAction::triggered,
            this, &PropertyView::onRemoveActionTriggered);
    connect(_raiseAction, &QAction::triggered,
            this, &PropertyView::onRaiseActionTriggered);
    connect(_lowerAction, &QAction::triggered,
            this, &PropertyView::onLowerActionTriggered);
    connect(_raiseToTopAction, &QAction::triggered,
            this, &PropertyView::onRaiseToTopActionTriggered);
    connect(_lowerToBottomAction, &QAction::triggered,
            this, &PropertyView::onLowerToBottomActionTriggered);
}

void PropertyView::setPropertyManager(PropertyManager* manager)
{
    if (_manager == manager) {
        return;
    }
    _manager = manager;

    if (auto* m = selectionModel()) {
        m->deleteLater();
    }

    if (_model) {
        _model->deleteLater();
    }
    _model = nullptr;
    QTreeView::setModel(nullptr);

    if (_manager) {
        _model = new PropertyModel(_manager);
        QTreeView::setModel(_model);

        connect(_model, &PropertyModel::rowsMoved,
                this, &PropertyView::onSelectionChanged);
    }

    onSelectionChanged();

    if (selectionModel()) {
        connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &PropertyView::onSelectionChanged);
    }

    this->expandAll();
}

// Must use contextMenuEvent. Using the customContextMenuRequested signal
// results in the context menu's location being off by ~16 pixels
void PropertyView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_manager == nullptr) {
        return;
    }

    // Only show menu if mouse cursor is on a list type
    if (_insertAction->isEnabled()) {
        QMenu menu;
        menu.addAction(_insertAction);

        if (_removeAction->isEnabled()) {
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

void PropertyView::keyPressEvent(QKeyEvent* event)
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

void PropertyView::onSelectionChanged()
{
    QModelIndex index = currentIndex();
    if (_manager && _model && index.isValid()) {
        auto& property = _model->propertyForIndex(index);

        bool isListItem = property.isList && index.parent().isValid();
        bool canRaise = isListItem && index.sibling(index.row() - 1, 0).isValid();
        bool canLower = isListItem && index.sibling(index.row() + 1, 0).isValid();

        _insertAction->setEnabled(property.isList);
        _removeAction->setEnabled(isListItem);
        _raiseAction->setEnabled(canRaise);
        _lowerAction->setEnabled(canLower);
        _raiseToTopAction->setEnabled(canRaise);
        _lowerToBottomAction->setEnabled(canLower);
    }
    else {
        _insertAction->setEnabled(false);
        _removeAction->setEnabled(false);
        _raiseAction->setEnabled(false);
        _lowerAction->setEnabled(false);
        _raiseToTopAction->setEnabled(false);
        _lowerToBottomAction->setEnabled(false);
    }
}

void PropertyView::onInsertActionTriggered()
{
    if (_manager == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        Q_ASSERT(_model);

        int row = index.row() + 1;
        QModelIndex parent = index.parent();
        if (!parent.isValid()) {
            row = 0;
            parent = index;
        }

        bool ok = false;
        auto& property = _model->propertyForIndex(parent);
        if (property.type == PropertyManager::Type::FILENAME_LIST) {
            const QStringList filenames = showAddFilenameDialog(property);
            ok = _model->insertRows(row, parent, filenames);
        }
        else {
            ok = _model->insertRow(row, parent);
        }

        if (ok) {
            QModelIndex newItemIndex = parent.child(row, PropertyModel::VALUE_COLUMN);
            setCurrentIndex(newItemIndex);
        }
    }
}

void PropertyView::onRemoveActionTriggered()
{
    if (_manager == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        Q_ASSERT(_model);
        _model->removeRow(index.row(), index.parent());
    }
}

void PropertyView::onRaiseActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, index.row() - 1);
}

void PropertyView::onLowerActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, index.row() + 2);
}

void PropertyView::onRaiseToTopActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, 0);
}

void PropertyView::onLowerToBottomActionTriggered()
{
    if (_manager == nullptr) {
        return;
    }
    Q_ASSERT(_model);

    QModelIndex index = currentIndex();
    QModelIndex parent = index.parent();
    if (parent.isValid()) {
        int nItems = _model->rowCount(parent);
        _model->moveRow(parent, index.row(), parent, nItems);
    }
}

void PropertyView::moveModelRow(const QModelIndex& index, int destRow)
{
    if (_manager == nullptr) {
        return;
    }
    Q_ASSERT(_model);

    if (index.isValid()) {
        QModelIndex parent = index.parent();
        _model->moveRow(parent, index.row(), parent, destRow);
    }
}

QStringList PropertyView::showAddFilenameDialog(const PropertyManager::Property& property)
{
    if (_manager == nullptr) {
        return QStringList();
    }
    Q_ASSERT(_model);

    QVariant filter = property.parameter1;
    QVariant param2 = property.parameter2;

    _manager->updateParameters(property.id, filter, param2);

    return QFileDialog::getOpenFileNames(
        this, QString(), QString(), filter.toString());
}

void PropertyView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in PropertyView.");
}
