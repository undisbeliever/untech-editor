/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistview.h"
#include "abstractaccessors.h"
#include "gui-qt/common/actionhelpers.h"
#include "gui-qt/common/validatoritemdelegate.h"

#include <QContextMenuEvent>
#include <QMenu>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

NamedListView::NamedListView(QWidget* parent)
    : QListView(parent)
    , _actions(new NamedListActions(this))
    , _model(new NamedListModel(this))
    , _selectedContextMenu(new QMenu(this))
    , _noSelectionContextMenu(new QMenu(this))
    , _accessor(nullptr)
{
    QListView::setModel(_model);

    setItemDelegate(new ValidatorItemDelegate(this));

    setDragDropMode(QListView::InternalMove);

    setSelectionMode(SelectionMode::SingleSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);

    _actions->populate(this);
    _actions->populate(_selectedContextMenu);
    _noSelectionContextMenu->addAction(_actions->add);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &NamedListView::onViewSelectionChanged);
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

    _actions->setAccessor(accessor);

    _model->setAccessor(accessor);

    setEnabled(_accessor);

    if (_accessor) {
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
}

void NamedListView::onViewSelectionChanged()
{
    if (_accessor) {
        size_t index = _model->toIndex(currentIndex());
        _accessor->setSelectedIndex(index);
    }
}

// Must use contextMenuEvent. Using the customContextMenuRequested signal
// results in the context menu's location being off by ~16 pixels
void NamedListView::contextMenuEvent(QContextMenuEvent* event)
{
    if (_accessor == nullptr) {
        return;
    }

    if (_actions->remove->isEnabled()) {
        _selectedContextMenu->exec(event->globalPos());
    }
    else {
        _noSelectionContextMenu->exec(event->globalPos());
    }
}
