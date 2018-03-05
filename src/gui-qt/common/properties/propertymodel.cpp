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
    resizeCache();

    connect(manager, &PropertyManager::propertyListChanged,
            this, &PropertyModel::resizeCache);
    connect(manager, &PropertyManager::dataChanged,
            this, &PropertyModel::invalidateCache);
    connect(manager, &PropertyManager::enabledChanged,
            this, &PropertyModel::updateAll);
}

void PropertyModel::resizeCache()
{
    beginResetModel();

    int nItems = _manager->propertiesList().size();

    _cacheDirty.fill(true, nItems);
    _dataCache.resize(nItems);
    _listSizeCache.resize(nItems);

    endResetModel();
}

void PropertyModel::invalidateCache()
{
    _cacheDirty.fill(true);
    updateAll();
}

void PropertyModel::updateAll()
{
    emit dataChanged(createIndex(0, 0),
                     createIndex(_manager->propertiesList().size(), N_COLUMNS - 1),
                     { Qt::DisplayRole, Qt::EditRole });
    emit layoutChanged();
}

void PropertyModel::updateCacheIfDirty(int index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < _manager->propertiesList().size());
    Q_ASSUME(index < _cacheDirty.size());
    Q_ASSUME(index < _dataCache.size());
    Q_ASSUME(index < _listSizeCache.size());

    if (_cacheDirty.at(index)) {
        const auto& settings = _manager->propertiesList().at(index);

        if (settings.id >= 0) {
            const QVariant data = _manager->data(index);
            int listSize = 0;

            if (settings.isList) {
                if (data.type() == QVariant::StringList) {
                    listSize = data.toStringList().size();
                }
                else if (data.type() == QVariant::List) {
                    listSize = data.toList().size();
                }
            }

            _dataCache.replace(index, std::move(data));
            _listSizeCache.replace(index, listSize);
        }
        else {
            _dataCache.replace(index, QVariant());
        }
        _cacheDirty.clearBit(index);
    }
}

const QVariant& PropertyModel::dataFromCache(int index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSUME(index < _dataCache.size());

    updateCacheIfDirty(index);
    return _dataCache.at(index);
}

int PropertyModel::propertyListSize(int index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSUME(index < _listSizeCache.size());

    updateCacheIfDirty(index);
    return _listSizeCache.at(index);
}

bool PropertyModel::checkIndex(const QModelIndex& index) const
{
    const auto& pl = _manager->propertiesList();

    Q_ASSUME(_manager->propertiesList().size() >= 0);

    if (pl.isEmpty()
        || index.isValid() == false
        || index.model() != this
        || index.column() >= N_COLUMNS) {

        return false;
    }

    if (index.internalId() == ROOT_INTERNAL_ID) {
        return index.row() < pl.size();
    }
    else {
        if (index.internalId() >= (unsigned)pl.size()
            || pl.at(index.internalId()).isList == false) {
            return false;
        }
        return index.row() < propertyListSize(index.internalId());
    }
}

QModelIndex PropertyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0
        || column < 0 || column >= N_COLUMNS) {

        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (row < _manager->propertiesList().size()) {
            return createIndex(row, column, ROOT_INTERNAL_ID);
        }
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID) {
        if (row < propertyListSize(parent.row())) {
            return createIndex(row, column, parent.row());
        }
    }

    return QModelIndex();
}

QModelIndex PropertyModel::parent(const QModelIndex& index) const
{
    Q_ASSUME(_manager->propertiesList().size() >= 0);

    if (index.internalId() == ROOT_INTERNAL_ID) {

        return QModelIndex();
    }
    else if (index.internalId() < (unsigned)_manager->propertiesList().size()) {
        return createIndex(index.internalId(), 0, ROOT_INTERNAL_ID);
    }
    else {
        return QModelIndex();
    }
}

bool PropertyModel::hasChildren(const QModelIndex& parent) const
{
    if (_manager->propertiesList().isEmpty()) {
        return false;
    }

    if (!parent.isValid()) {
        return true;
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID
             && parent.row() < _manager->propertiesList().size()
             && _manager->propertiesList().at(parent.row()).isList) {
        return true;
    }
    else {
        return false;
    }
}

int PropertyModel::rowCount(const QModelIndex& parent) const
{
    if (_manager->propertiesList().isEmpty()) {
        return 0;
    }

    if (!parent.isValid()) {
        return _manager->propertiesList().size();
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID
             && parent.row() >= 0
             && parent.row() < _manager->propertiesList().size()
             && _manager->propertiesList().at(parent.row()).isList) {
        return propertyListSize(parent.row());
    }
    else {
        return 0;
    }
}

int PropertyModel::columnCount(const QModelIndex& parent) const
{
    if (_manager->propertiesList().isEmpty()) {
        return 0;
    }

    if (!parent.isValid() || parent.internalId() == ROOT_INTERNAL_ID) {
        return N_COLUMNS;
    }
    else {
        return 0;
    }
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex& index) const
{
    const auto& pl = _manager->propertiesList();

    if (checkIndex(index) == false
        || _manager->isEnabled() == false) {

        return 0;
    }

    static_assert(N_COLUMNS == 2, "Invalid flags N_COLUMNS");

    QFlags<Qt::ItemFlag> flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.internalId() == ROOT_INTERNAL_ID) {
        const auto& settings = pl.at(index.row());

        if (settings.isList == false) {
            flags |= Qt::ItemNeverHasChildren;
        }
        if (index.column() == VALUE_COLUMN
            && settings.id >= 0
            && settings.isList == false) {

            flags |= Qt::ItemIsEditable;
        }
    }
    else {
        flags |= Qt::ItemNeverHasChildren;

        if (index.column() == VALUE_COLUMN) {
            flags |= Qt::ItemIsEditable;
        }
    }

    return flags;
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

    if (checkIndex(index) == false) {
        return QVariant();
    }

    if (_manager->isEnabled() == false
        && index.column() != PROPERTY_COLUMN) {

        return QVariant();
    }

    static_assert(N_COLUMNS == 2, "Invalid data N_COLUMNS");
    if (role == Qt::DisplayRole) {
        if (index.column() == PROPERTY_COLUMN) {
            if (index.internalId() == ROOT_INTERNAL_ID) {
                return pl.at(index.row()).title;
            }
        }
        else {
            // Value column

            if (index.internalId() == ROOT_INTERNAL_ID) {
                return dataFromCache(index.row());
            }
            else {
                QVariantList vl = dataFromCache(index.internalId()).toList();
                return vl.at(index.row());
            }
        }
    }
    else if (role == Qt::EditRole && index.column() == VALUE_COLUMN) {
        if (index.internalId() == ROOT_INTERNAL_ID) {
            return dataFromCache(index.row());
        }
        else {
            QVariantList vl = dataFromCache(index.internalId()).toList();
            return vl.at(index.row());
        }
    }

    return QVariant();
}

bool PropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole
        || checkIndex(index) == false
        || index.column() != VALUE_COLUMN) {

        return false;
    }

    if (index.internalId() == ROOT_INTERNAL_ID) {
        const auto& settings = _manager->propertiesList().at(index.row());

        if (settings.id < 0) {
            return false;
        }
        return _manager->setData(settings.id, value);
    }
    else {
        const auto& settings = _manager->propertiesList().at(index.internalId());
        const QVariant& data = dataFromCache(index.internalId());

        if (settings.id < 0) {
            return false;
        }

        if (data.type() == QVariant::StringList) {
            QStringList sl = data.toStringList();
            sl.replace(index.row(), value.toString());
            return _manager->setData(settings.id, sl);
        }
        else if (data.type() == QVariant::List) {
            QVariantList vl = data.toList();
            vl.replace(index.row(), value);
            return _manager->setData(settings.id, vl);
        }
        else {
            qWarning("Expected list from manager");
            return false;
        }
    }
}
