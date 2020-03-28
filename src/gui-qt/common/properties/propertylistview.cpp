/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertylistview.h"
#include "propertydelegate.h"
#include "propertylistmanager.h"
#include "propertylistmodel.h"

#include <QContextMenuEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>

using namespace UnTech::GuiQt;
using Type = PropertyType;

PropertyListView::PropertyListView(QWidget* parent)
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        a->setShortcutVisibleInContextMenu(true);
#endif
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
            this, &PropertyListView::onInsertActionTriggered);
    connect(_removeAction, &QAction::triggered,
            this, &PropertyListView::onRemoveActionTriggered);
    connect(_raiseAction, &QAction::triggered,
            this, &PropertyListView::onRaiseActionTriggered);
    connect(_lowerAction, &QAction::triggered,
            this, &PropertyListView::onLowerActionTriggered);
    connect(_raiseToTopAction, &QAction::triggered,
            this, &PropertyListView::onRaiseToTopActionTriggered);
    connect(_lowerToBottomAction, &QAction::triggered,
            this, &PropertyListView::onLowerToBottomActionTriggered);
}

PropertyListView::PropertyListView(PropertyListManager* manager, QWidget* parent)
    : PropertyListView(parent)
{
    setPropertyManager(manager);
}

void PropertyListView::setPropertyManager(PropertyListManager* manager)
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
        _model = new PropertyListModel(_manager);
        QTreeView::setModel(_model);

        connect(_model, &PropertyListModel::modelReset,
                this, &PropertyListView::onSelectionChanged);
        connect(_model, &PropertyListModel::rowsMoved,
                this, &PropertyListView::onSelectionChanged);
    }

    onSelectionChanged();

    if (selectionModel()) {
        connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &PropertyListView::onSelectionChanged);
    }

    this->expandAll();
}

// Must use contextMenuEvent. Using the customContextMenuRequested signal
// results in the context menu's location being off by ~16 pixels
void PropertyListView::contextMenuEvent(QContextMenuEvent* event)
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

void PropertyListView::keyPressEvent(QKeyEvent* event)
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

void PropertyListView::onSelectionChanged()
{
    QModelIndex index = currentIndex();
    if (_manager && _model && index.isValid()) {
        auto& property = _model->propertyForIndexIgnoreColumn(index);

        bool isListItem = property.isList && _model->isListItem(index);
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

void PropertyListView::onInsertActionTriggered()
{
    if (_manager == nullptr) {
        return;
    }

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        Q_ASSERT(_model);

        // Close any open editors
        setCurrentIndex(QModelIndex());

        int row;
        QModelIndex parent = index;
        if (_model->isListItem(index)) {
            row = index.row() + 1;
            parent = index.parent();
        }
        else {
            row = _model->rowCount(parent);
        }

        bool ok = false;
        auto& property = _model->propertyForIndexIgnoreColumn(parent);
        if (property.type == Type::FILENAME_LIST) {
            const QStringList filenames = showAddFilenameDialog(property);
            ok = _model->insertRows(row, parent, filenames);
        }
        else {
            ok = _model->insertRow(row, parent);
        }

        if (ok) {
            QModelIndex newItemIndex = _model->index(row, PropertyListModel::VALUE_COLUMN, parent);
            setCurrentIndex(newItemIndex);
        }
    }
}

void PropertyListView::onRemoveActionTriggered()
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

void PropertyListView::onRaiseActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, index.row() - 1);
}

void PropertyListView::onLowerActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, index.row() + 2);
}

void PropertyListView::onRaiseToTopActionTriggered()
{
    QModelIndex index = currentIndex();
    moveModelRow(index, 0);
}

void PropertyListView::onLowerToBottomActionTriggered()
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

void PropertyListView::moveModelRow(const QModelIndex& index, int destRow)
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

QStringList PropertyListView::showAddFilenameDialog(const Property& property)
{
    if (_manager == nullptr) {
        return QStringList();
    }
    Q_ASSERT(_model);

    QVariant filter = property.parameter1;
    QVariant param2 = property.parameter2;

    _manager->updateParameters(property.id, filter, param2);

    QStringList filenames = QFileDialog::getOpenFileNames(
        this, QString(), QString(), filter.toString(),
        nullptr, QFileDialog::DontUseNativeDialog);

    for (QString& fn : filenames) {
        fn = QDir::toNativeSeparators(fn);
    }

    return filenames;
}

void PropertyListView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in PropertyView.");
}
