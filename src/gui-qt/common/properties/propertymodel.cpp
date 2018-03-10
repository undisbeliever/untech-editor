/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertymodel.h"
#include "propertymanager.h"

#include <QMimeData>

using namespace UnTech::GuiQt;
using Type = PropertyType;

const Property PropertyModel::blankProperty;
const QString PropertyModel::ITEM_MIME_TYPE = QStringLiteral("application/x-untech-property-data");

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

const Property& PropertyModel::propertyForIndex(const QModelIndex& index) const
{
    if (index.isValid() == false
        || index.model() != this
        || index.column() >= N_COLUMNS) {

        return blankProperty;
    }

    const auto& pl = _manager->propertiesList();
    if (index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
        if (index.row() < pl.size()) {
            return pl.at(index.row());
        }
    }
    else {
        if (index.internalId() < (unsigned)pl.size()
            && pl.at(index.internalId()).isList) {

            return pl.at(index.internalId());
        }
    }

    return blankProperty;
}

void PropertyModel::resizeCache()
{
    beginResetModel();

    int nItems = _manager->propertiesList().size();

    _cacheDirty.fill(true, nItems);
    _dataCache.resize(nItems);
    _listSizeCache.resize(nItems);
    _displayCache.resize(nItems);

    endResetModel();
}

void PropertyModel::invalidateCache()
{
    _cacheDirty.fill(true);
    updateAll();
}

void PropertyModel::updateAll()
{
    emit dataChanged(createIndex(0, VALUE_COLUMN, ROOT_INTERNAL_ID),
                     createIndex(_manager->propertiesList().size() - 1, VALUE_COLUMN, ROOT_INTERNAL_ID),
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
    Q_ASSUME(index < _displayCache.size());

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
            _displayCache.replace(index, displayForProperty(settings, data, listSize));
        }
        else {
            _dataCache.replace(index, QVariant());
        }
        _cacheDirty.clearBit(index);
    }
}

QString PropertyModel::displayForProperty(const Property& settings, const QVariant& value, int listSize) const
{
    switch (settings.type) {
    case Type::BOOLEAN:
    case Type::INTEGER:
    case Type::UNSIGNED:
    case Type::STRING:
    case Type::IDSTRING:
    case Type::FILENAME:
    case Type::COLOR: {
        return value.toString();
    }

    case Type::COMBO: {
        QVariant param1 = settings.parameter1;
        QVariant param2 = settings.parameter2;
        _manager->updateParameters(settings.id, param1, param2);

        int index = -1;
        if (param2.canConvert(QVariant::List)) {
            index = param2.toList().indexOf(value);
        }

        QStringList displayList = param1.toStringList();
        if (index >= 0 && index < displayList.size()) {
            return displayList.at(index);
        }
        else {
            return value.toString();
        }
    }

    case Type::STRING_LIST:
    case Type::IDSTRING_LIST:
    case Type::FILENAME_LIST: {
        if (listSize != 1) {
            return tr("(%1 items)").arg(listSize);
        }
        else {
            return tr("(1 item)");
        }
    }
    }

    return QString();
}

const QString& PropertyModel::displayFromCache(int index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSUME(index < _displayCache.size());

    updateCacheIfDirty(index);
    return _displayCache.at(index);
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
        else {
            flags |= Qt::ItemIsDropEnabled;
        }
        if (index.column() == VALUE_COLUMN
            && settings.id >= 0) {

            flags |= Qt::ItemIsEditable;
        }
    }
    else {
        flags |= Qt::ItemNeverHasChildren;

        if (index.column() == VALUE_COLUMN) {
            flags |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
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
                return displayFromCache(index.row());
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

bool PropertyModel::insertRows(int row, const QModelIndex& parent, const QStringList& values)
{
    if (row < 0
        || parent.internalId() != ROOT_INTERNAL_ID
        || checkIndex(parent) == false) {

        return false;
    }

    const auto& settings = _manager->propertiesList().at(parent.row());
    if (settings.isList == false) {
        return false;
    }

    QVariant data = _manager->data(settings.id);

    if (data.type() == QVariant::StringList) {
        QStringList list = data.toStringList();

        if (row > list.size()) {
            row = list.size();
        }
        for (int i = 0; i < values.size(); i++) {
            list.insert(row + i, values.at(i));
        }
        return _manager->setData(settings.id, list);
    }
    else {
        return false;
    }
}

bool PropertyModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0
        || count <= 0
        || parent.internalId() != ROOT_INTERNAL_ID
        || checkIndex(parent) == false) {

        return false;
    }

    const auto& settings = _manager->propertiesList().at(parent.row());
    if (settings.isList == false) {
        return false;
    }
    if (settings.type == Type::FILENAME_LIST) {
        // Do not create empty strings in a filename list
        return false;
    }

    auto doInsert = [&](auto list, auto value) {
        if (row > list.size()) {
            row = list.size();
        }
        for (int i = 0; i < count; i++) {
            list.insert(row, value);
        }
        return _manager->setData(settings.id, list);
    };

    QVariant data = _manager->data(settings.id);

    if (data.type() == QVariant::List) {
        return doInsert(data.toList(), QVariant());
    }
    else if (data.type() == QVariant::StringList) {
        return doInsert(data.toStringList(), QString());
    }
    else {
        return false;
    }
}

bool PropertyModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0
        || count <= 0
        || parent.internalId() != ROOT_INTERNAL_ID
        || checkIndex(parent) == false) {

        return false;
    }

    const auto& settings = _manager->propertiesList().at(parent.row());
    if (settings.isList == false) {
        return false;
    }

    auto doRemove = [&](auto list) {
        if (row + count <= list.size()) {
            for (int i = 0; i < count; i++) {
                list.removeAt(row);
            }
            return _manager->setData(settings.id, list);
        }
        else {
            return false;
        }
    };

    QVariant data = _manager->data(settings.id);

    if (data.type() == QVariant::List) {
        return doRemove(data.toList());
    }
    else if (data.type() == QVariant::StringList) {
        return doRemove(data.toStringList());
    }
    else {
        return false;
    }
}

// This code is a LOT simpler if only one item can be moved at a time
bool PropertyModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                             const QModelIndex& destParent, int destRow)
{
    if (sourceRow < 0
        || destRow < 0
        || count != 1
        || sourceRow == destRow
        || sourceRow == destRow - 1
        || sourceParent != destParent
        || sourceParent.internalId() != ROOT_INTERNAL_ID
        || checkIndex(sourceParent) == false) {

        return false;
    }

    const auto& settings = _manager->propertiesList().at(sourceParent.row());
    if (settings.isList == false) {
        return false;
    }

    auto doMove = [&](auto list) {
        if (sourceRow < list.size() && destRow <= list.size()) {
            beginMoveRows(sourceParent, sourceRow, sourceRow, destParent, destRow);
            endMoveRows();

            if (destRow > sourceRow) {
                destRow--;
            }

            list.move(sourceRow, destRow);
            return _manager->setData(settings.id, list);
        }
        else {
            return false;
        }
    };

    QVariant data = _manager->data(settings.id);

    if (data.type() == QVariant::List) {
        return doMove(data.toList());
    }
    else if (data.type() == QVariant::StringList) {
        return doMove(data.toStringList());
    }
    else {
        return false;
    }
}

Qt::DropActions PropertyModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions PropertyModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList PropertyModel::mimeTypes() const
{
    static const QStringList types = {
        ITEM_MIME_TYPE
    };

    return types;
}

struct PropertyModel::InternalMimeData {
    int id;
    int row;
    const void* model;
};
QDataStream& operator<<(QDataStream& stream, const PropertyModel::InternalMimeData& data)
{
    static_assert(sizeof(quintptr) == sizeof(data.model), "Bad quintptr size");

    quintptr ptr = (quintptr)data.model;
    stream << data.id << data.row << ptr;

    return stream;
}
QDataStream& operator>>(QDataStream& stream, PropertyModel::InternalMimeData& data)
{
    static_assert(sizeof(quintptr) == sizeof(data.model), "Bad quintptr size");

    quintptr ptr;
    stream >> data.id >> data.row >> ptr;
    data.model = (const void*)ptr;

    return stream;
}

// This code is a LOT simpler if only one item can be moved at a time
QMimeData* PropertyModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.count() != 1
        || indexes.front().internalId() == ROOT_INTERNAL_ID
        || checkIndex(indexes.front()) == false) {

        return nullptr;
    }
    const QModelIndex& index = indexes.front();
    const auto& settings = _manager->propertiesList().at(index.internalId());

    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << InternalMimeData{ settings.id, index.row(), this };

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(ITEM_MIME_TYPE, encodedData);

    return mimeData;
}

bool PropertyModel::canDropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                    int destRow, int column, const QModelIndex& parent) const
{
    Q_UNUSED(column)

    if (mimeData == nullptr
        || action != Qt::MoveAction
        || destRow < 0
        || parent.internalId() != ROOT_INTERNAL_ID
        || checkIndex(parent) == false) {
        return false;
    }

    const auto& settings = _manager->propertiesList().at(parent.row());

    if (settings.isList) {
        if (mimeData->hasFormat(ITEM_MIME_TYPE) == false) {
            return false;
        }

        QByteArray encodedData = mimeData->data(ITEM_MIME_TYPE);
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        InternalMimeData data;
        stream >> data;

        return stream.atEnd()
               && data.model == this
               && data.id == settings.id
               && data.row >= 0 && data.row < propertyListSize(parent.row());
    }

    return false;
}

bool PropertyModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                 int destRow, int column, const QModelIndex& parent)
{
    Q_UNUSED(column)

    if (mimeData == nullptr
        || action != Qt::MoveAction
        || destRow < 0
        || parent.internalId() != ROOT_INTERNAL_ID
        || checkIndex(parent) == false) {
        return false;
    }

    const auto& settings = _manager->propertiesList().at(parent.row());

    if (settings.isList) {
        if (mimeData->hasFormat(ITEM_MIME_TYPE) == false) {
            return false;
        }

        QByteArray encodedData = mimeData->data(ITEM_MIME_TYPE);
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        InternalMimeData data;
        stream >> data;

        if (stream.atEnd() && data.model == this && data.id == settings.id) {

            // uses moveRow so the the manager only sees one setData call
            moveRow(parent, data.row, parent, destRow);

            // Return false so the View does not delete the source
            return false;
        }
    }

    return false;
}
