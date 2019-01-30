/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistmodel.h"

#include <algorithm>
#include <iterator>

using UnTech::idstring;
using namespace UnTech::GuiQt::Accessor;

NamedListModel::NamedListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _accessor(nullptr)
    , _displayList()
{
}

void NamedListModel::clear()
{
    beginResetModel();

    _displayList.clear();

    endResetModel();
}

QModelIndex NamedListModel::toModelIndex(int i) const
{
    if (i < 0 || i >= _displayList.size()) {
        return QModelIndex();
    }
    return createIndex(i, 0);
}

size_t NamedListModel::toIndex(const QModelIndex& index) const
{
    const int row = index.row();
    if (row < 0 || row >= _displayList.size()) {
        return INT_MAX;
    }
    return row;
}

int NamedListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return _displayList.size();
}

QVariant NamedListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= _displayList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _displayList.at(index.row());
    }

    return QVariant();
}
