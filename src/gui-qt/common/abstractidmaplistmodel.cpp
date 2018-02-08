/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractidmaplistmodel.h"

#include <algorithm>
#include <iterator>

using UnTech::idstring;
using namespace UnTech::GuiQt;

AbstractIdmapListModel::AbstractIdmapListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _idstrings()
    , _displayList()
{
}

bool AbstractIdmapListModel::contains(const idstring& id) const
{
    return std::binary_search(_idstrings.cbegin(), _idstrings.cend(), id);
}

bool AbstractIdmapListModel::contains(const QString& id) const
{
    return std::binary_search(_displayList.cbegin(), _displayList.cend(), id);
}

int AbstractIdmapListModel::indexOf(const idstring& id) const
{
    auto it = std::lower_bound(_idstrings.cbegin(), _idstrings.cend(), id);
    if (it != _idstrings.cend() && *it == id) {
        return std::distance(_idstrings.cbegin(), it);
    }
    else {
        return -1;
    }
}

int AbstractIdmapListModel::indexToInsert(const idstring& id) const
{
    auto it = std::lower_bound(_idstrings.cbegin(), _idstrings.cend(), id);
    return std::distance(_idstrings.cbegin(), it);
}

QModelIndex AbstractIdmapListModel::toModelIndex(const idstring& id) const
{
    return createIndex(indexOf(id), 0);
}

idstring AbstractIdmapListModel::toIdstring(int row) const
{
    if (row < 0 || row >= _idstrings.size()) {
        return idstring();
    }
    return _idstrings.at(row);
}

idstring AbstractIdmapListModel::toIdstring(const QModelIndex& index) const
{
    return toIdstring(index.row());
}

int AbstractIdmapListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return _displayList.size();
}

QVariant AbstractIdmapListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= _displayList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _displayList.at(index.row());
    }

    return QVariant();
}
