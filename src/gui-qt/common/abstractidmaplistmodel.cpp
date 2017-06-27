/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractidmaplistmodel.h"

using UnTech::idstring;
using namespace UnTech::GuiQt;

AbstractIdmapListModel::AbstractIdmapListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _idstrings()
    , _displayList()
{
}

QModelIndex AbstractIdmapListModel::toModelIndex(const idstring& id) const
{
    int row = _idstrings.indexOf(id);

    return createIndex(row, 0);
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
