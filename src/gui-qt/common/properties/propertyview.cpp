/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertyview.h"
#include "propertydelegate.h"
#include "propertymanager.h"
#include "propertymodel.h"

#include <QHeaderView>

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

    this->expandAll();
}

void PropertyView::setModel(QAbstractItemModel*)
{
    qCritical("Must not call setModel in PropertyView.");
}
