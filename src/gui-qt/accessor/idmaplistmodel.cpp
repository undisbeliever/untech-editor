/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "idmaplistmodel.h"

#include <algorithm>
#include <iterator>

using UnTech::idstring;
using namespace UnTech::GuiQt::Accessor;

IdmapListModel::IdmapListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _accessor(nullptr)
    , _idstrings()
    , _displayList()
{
}

int IdmapListModel::indexOf(const idstring& id) const
{
    auto it = std::lower_bound(_idstrings.cbegin(), _idstrings.cend(), id);
    if (it != _idstrings.cend() && *it == id) {
        return std::distance(_idstrings.cbegin(), it);
    }
    else {
        return -1;
    }
}

int IdmapListModel::indexToInsert(const idstring& id) const
{
    auto it = std::lower_bound(_idstrings.cbegin(), _idstrings.cend(), id);
    return std::distance(_idstrings.cbegin(), it);
}

void IdmapListModel::clear()
{
    beginResetModel();

    _displayList.clear();
    _idstrings.clear();

    endResetModel();
}

QModelIndex IdmapListModel::toModelIndex(const idstring& id) const
{
    return createIndex(indexOf(id), 0);
}

idstring IdmapListModel::toIdstring(int row) const
{
    if (row < 0 || row >= _idstrings.size()) {
        return idstring();
    }
    return _idstrings.at(row);
}

idstring IdmapListModel::toIdstring(const QModelIndex& index) const
{
    return toIdstring(index.row());
}

int IdmapListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return _displayList.size();
}

QVariant IdmapListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= _displayList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _displayList.at(index.row());
    }

    return QVariant();
}

void IdmapListModel::addIdstring(const UnTech::idstring& id)
{
    Q_ASSERT(_idstrings.contains(id) == false);

    int index = indexToInsert(id);

    beginInsertRows(QModelIndex(), index, index);

    _idstrings.insert(index, id);
    _displayList.insert(index, QString::fromStdString(id));

    endInsertRows();
}

void IdmapListModel::removeIdstring(const UnTech::idstring& id)
{
    int index = indexOf(id);
    Q_ASSERT(index >= 0);

    beginRemoveRows(QModelIndex(), index, index);

    _idstrings.removeAt(index);
    _displayList.removeAt(index);

    endRemoveRows();
}

void IdmapListModel::renameIdstring(const UnTech::idstring& oldId, const UnTech::idstring& newId)
{
    Q_ASSERT(_idstrings.contains(newId) == false);

    int oldIndex = indexOf(oldId);
    Q_ASSERT(oldIndex >= 0);

    int newIndex = indexToInsert(newId);

    if (oldIndex != newIndex && oldIndex != newIndex - 1) {
        beginMoveRows(QModelIndex(), oldIndex, oldIndex,
                      QModelIndex(), newIndex);

        if (oldIndex < newIndex) {
            newIndex--;
        }

        _idstrings.takeAt(oldIndex);
        _idstrings.insert(newIndex, newId);

        _displayList.takeAt(oldIndex);
        _displayList.insert(newIndex, QString::fromStdString(newId));

        endMoveRows();
    }
    else {
        _idstrings.replace(oldIndex, newId);
        _displayList.replace(oldIndex, QString::fromStdString(newId));

        emit dataChanged(createIndex(oldIndex, 0), createIndex(oldIndex, 0));
    }
}
