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

    setEditTriggers(QAbstractItemView::AllEditTriggers);
    setAlternatingRowColors(true);

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    _insertAction = new QAction(tr("Insert Item"), this);
    _insertAction->setIcon(QIcon(":/icons/add.svg"));
    _insertAction->setShortcut(Qt::Key_Insert);
    _insertAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _insertAction->setShortcutVisibleInContextMenu(true);
    addAction(_insertAction);

    _removeAction = new QAction(tr("Remove Item"), this);
    _removeAction->setIcon(QIcon(":/icons/remove.svg"));
    _removeAction->setShortcut(Qt::Key_Delete);
    _removeAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _removeAction->setShortcutVisibleInContextMenu(true);
    addAction(_removeAction);

    onSelectionChanged();

    connect(_insertAction, &QAction::triggered,
            this, &PropertyView::onInsertActionTriggered);
    connect(_removeAction, &QAction::triggered,
            this, &PropertyView::onRemoveActionTriggered);
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
            menu.addAction(_removeAction);
        }

        menu.exec(event->globalPos());
    }
}

void PropertyView::onSelectionChanged()
{
    QModelIndex index = currentIndex();
    if (_manager && _model && index.isValid()) {
        auto& property = _model->propertyForIndex(index);

        _insertAction->setEnabled(property.isList);
        _removeAction->setEnabled(property.isList && index.parent().isValid());
    }
    else {
        _insertAction->setEnabled(false);
        _removeAction->setEnabled(false);
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
