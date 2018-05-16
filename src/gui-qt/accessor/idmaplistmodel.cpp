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
    , _displayList()
{
}

int IdmapListModel::indexOf(const QString& id) const
{
    auto it = std::lower_bound(_displayList.cbegin(), _displayList.cend(), id);
    if (it != _displayList.cend() && *it == id) {
        return std::distance(_displayList.cbegin(), it);
    }
    else {
        return -1;
    }
}

int IdmapListModel::indexToInsert(const QString& id) const
{
    auto it = std::lower_bound(_displayList.cbegin(), _displayList.cend(), id);
    return std::distance(_displayList.cbegin(), it);
}

void IdmapListModel::clear()
{
    beginResetModel();

    _displayList.clear();

    endResetModel();
}

QModelIndex IdmapListModel::toModelIndex(const idstring& id) const
{
    return createIndex(indexOf(QString::fromStdString(id)), 0);
}

idstring IdmapListModel::toIdstring(int row) const
{
    if (row < 0 || row >= _displayList.size()) {
        return idstring();
    }
    return _displayList.at(row).toStdString();
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
    QString display = QString::fromStdString(id);

    Q_ASSERT(_displayList.contains(display) == false);

    int index = indexToInsert(display);

    beginInsertRows(QModelIndex(), index, index);

    _displayList.insert(index, QString::fromStdString(id));

    endInsertRows();
}

void IdmapListModel::removeIdstring(const UnTech::idstring& id)
{
    QString display = QString::fromStdString(id);

    int index = indexOf(display);
    Q_ASSERT(index >= 0);

    beginRemoveRows(QModelIndex(), index, index);

    _displayList.removeAt(index);

    endRemoveRows();
}

void IdmapListModel::renameIdstring(const UnTech::idstring& oldId, const UnTech::idstring& newId)
{
    QString newDisplay = QString::fromStdString(newId);

    Q_ASSERT(_displayList.contains(newDisplay) == false);

    int oldIndex = indexOf(QString::fromStdString(oldId));
    Q_ASSERT(oldIndex >= 0);

    int newIndex = indexToInsert(newDisplay);

    if (oldIndex != newIndex && oldIndex != newIndex - 1) {
        beginMoveRows(QModelIndex(), oldIndex, oldIndex,
                      QModelIndex(), newIndex);

        if (oldIndex < newIndex) {
            newIndex--;
        }

        _displayList.takeAt(oldIndex);
        _displayList.insert(newIndex, newDisplay);

        endMoveRows();
    }
    else {
        _displayList.replace(oldIndex, newDisplay);

        emit dataChanged(createIndex(oldIndex, 0), createIndex(oldIndex, 0));
    }
}
