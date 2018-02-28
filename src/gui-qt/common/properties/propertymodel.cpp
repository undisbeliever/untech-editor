/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertymodel.h"
#include "propertymanager.h"

using namespace UnTech::GuiQt;

PropertyModel::PropertyModel(PropertyManager* manager)
    : QAbstractItemModel(manager)
    , _manager(manager)
{
    connect(manager, &PropertyManager::propertyListChanged,
            this, &PropertyModel::updateAll);
    connect(manager, &PropertyManager::enabledChanged,
            this, &PropertyModel::updateAll);
    connect(manager, &PropertyManager::dataChanged,
            this, &PropertyModel::updateAll);
}

void PropertyModel::updateAll()
{
    emit dataChanged(createIndex(0, VALUE_COLUMN),
                     createIndex(_manager->propertiesList().size(), VALUE_COLUMN),
                     { Qt::DisplayRole, Qt::EditRole });
}

QModelIndex PropertyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || row >= _manager->propertiesList().size()
        || column < 0 || column >= N_COLUMNS
        || parent.isValid()) {

        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex PropertyModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

bool PropertyModel::hasChildren(const QModelIndex& index) const
{
    if (_manager->propertiesList().isEmpty()) {
        return false;
    }

    return index.isValid() == false;
}

int PropertyModel::rowCount(const QModelIndex& parent) const
{
    if (_manager->propertiesList().isEmpty()) {
        return false;
    }

    if (!parent.isValid()) {
        return _manager->propertiesList().size();
    }
    else {
        return 0;
    }
}

int PropertyModel::columnCount(const QModelIndex& parent) const
{
    if (_manager->propertiesList().isEmpty()) {
        return false;
    }

    if (!parent.isValid()) {
        return N_COLUMNS;
    }
    else {
        return 0;
    }
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex& index) const
{
    const auto& pl = _manager->propertiesList();

    if (pl.isEmpty()
        || _manager->isEnabled() == false
        || !index.isValid()
        || index.column() >= N_COLUMNS
        || index.row() >= pl.size()
        || pl.at(index.row()).id < 0) {

        return 0;
    }

    static_assert(N_COLUMNS == 2, "Invalid flags N_COLUMNS");
    if (index.column() == PROPERTY_COLUMN) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }
    else {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }
}

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= N_COLUMNS
        || orientation != Qt::Horizontal
        || role != Qt::DisplayRole) {

        return QVariant();
    }

    static_assert(N_COLUMNS == 2, "Invalid headerData N_COLUMNS");
    if (section == PROPERTY_COLUMN) {
        return tr("Property");
    }
    else {
        return tr("Value");
    }
}

QVariant PropertyModel::data(const QModelIndex& index, int role) const
{
    const auto& pl = _manager->propertiesList();

    if (pl.isEmpty()
        || !index.isValid()
        || index.column() >= N_COLUMNS
        || index.row() >= pl.size()
        || pl.at(index.row()).id < 0) {

        return QVariant();
    }

    static_assert(N_COLUMNS == 2, "Invalid data N_COLUMNS");
    if (role == Qt::DisplayRole) {
        if (index.column() == PROPERTY_COLUMN) {
            return pl.at(index.row()).title;
        }
        else if (_manager->isEnabled()) {
            return _manager->data(pl.at(index.row()).id);
        }
    }
    else if (role == Qt::EditRole) {
        if (_manager->isEnabled()) {
            return _manager->data(pl.at(index.row()).id);
        }
    }

    return QVariant();
}

bool PropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole
        || _manager->isEnabled() == false
        || !index.isValid()
        || index.column() != VALUE_COLUMN
        || index.row() >= _manager->propertiesList().size()) {

        return false;
    }

    const auto& settings = _manager->propertiesList().at(index.row());
    if (settings.id >= 0) {
        return _manager->setData(settings.id, value);
    }
    else {
        return false;
    }
}
