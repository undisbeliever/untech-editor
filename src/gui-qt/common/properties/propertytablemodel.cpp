/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertytablemodel.h"
#include "propertytablemanager.h"

#include <QMimeData>

using namespace UnTech::GuiQt;
using Type = PropertyType;

const Property PropertyTableModel::blankProperty;
const QString PropertyTableModel::ITEM_MIME_TYPE = QStringLiteral("application/x-untech-propertytable-row");

PropertyTableModel::PropertyTableModel(PropertyTableManager* manager, QObject* parent)
    : PropertyTableModel({ manager }, manager->propertyTitles(), parent)
{
}

PropertyTableModel::PropertyTableModel(QList<PropertyTableManager*> managers, QStringList columnHeaders,
                                       QObject* parent)
    : AbstractPropertyModel(parent)
    , _managers(managers)
    , _columnHeaders(columnHeaders)
    , _nColumns(columnHeaders.size())
{
    for (int mIndex = 0; mIndex < _managers.size(); mIndex++) {
        auto* manager = _managers[mIndex];

        connect(manager, &PropertyTableManager::dataReset,
                this, &PropertyTableModel::resetModel);

        connect(manager, &PropertyTableManager::listAboutToChange,
                this, &PropertyTableModel::requestCloseEditors);

        connect(manager, &PropertyTableManager::propertyListChanged,
                this, &PropertyTableModel::updateAll);
        connect(manager, &PropertyTableManager::titleChanged,
                this, &PropertyTableModel::updateAll);
        connect(manager, &PropertyTableManager::enabledChanged,
                this, &PropertyTableModel::updateAll);
        connect(manager, &PropertyTableManager::canMoveItemsChanged,
                this, &PropertyTableModel::updateAll);

        connect(manager, &PropertyTableManager::dataChanged,
                this, &PropertyTableModel::updateAll);

        connect(manager, &PropertyTableManager::itemChanged,
                [this, mIndex](int index) { this->onManagerItemChanged(mIndex, index); });
        connect(manager, &PropertyTableManager::itemAdded,
                [this, mIndex](int index) { this->onManagerItemAdded(mIndex, index); });
        connect(manager, &PropertyTableManager::itemRemoved,
                [this, mIndex](int index) { this->onManagerItemRemoved(mIndex, index); });
        connect(manager, &PropertyTableManager::itemMoved,
                [this, mIndex](int from, int to) { this->onManagerItemMoved(mIndex, from, to); });
    }
}

void PropertyTableModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

void PropertyTableModel::updateAll()
{
    emit layoutChanged();
}

QModelIndex PropertyTableModel::createManagerParentIndex(int managerIndex)
{
    if (_managers.size() == 1) {
        return QModelIndex();
    }
    else {
        Q_ASSERT(managerIndex >= 0);
        Q_ASSERT(managerIndex < _managers.size());

        return createIndex(managerIndex, 0, ROOT_INTERNAL_ID);
    }
}

void PropertyTableModel::onManagerItemChanged(int managerIndex, int row)
{
    emit dataChanged(createIndex(row, 0, managerIndex),
                     createIndex(row, _nColumns, managerIndex),
                     { Qt::DisplayRole, Qt::EditRole });
}

void PropertyTableModel::onManagerItemAdded(int managerIndex, int row)
{
    beginInsertRows(createManagerParentIndex(managerIndex), row, row);
    endInsertRows();
}

void PropertyTableModel::onManagerItemRemoved(int managerIndex, int row)
{
    beginRemoveRows(createManagerParentIndex(managerIndex), row, row);
    endRemoveRows();
}

void PropertyTableModel::onManagerItemMoved(int managerIndex, int from, int to)
{
    if (to >= from) {
        to++;
    }

    QModelIndex parent = createManagerParentIndex(managerIndex);

    beginMoveRows(parent, from, from, parent, to);
    endMoveRows();
}

QModelIndex PropertyTableModel::toModelIndex(PropertyTableManager* manager, int index) const
{
    return toModelIndex(_managers.indexOf(manager), index);
}

QModelIndex PropertyTableModel::toModelIndex(int managerIndex, int index) const
{
    if (managerIndex >= 0 && managerIndex < _managers.size()
        && index >= 0 && index < rowCountFromManager(managerIndex)) {

        return createIndex(index, 0, managerIndex);
    }
    else {
        return QModelIndex();
    }
}

QPair<const PropertyTableManager*, int> PropertyTableModel::toManagerAndIndex(const QModelIndex& index) const
{
    if (checkIndex(index) == false
        || index.internalId() == ROOT_INTERNAL_ID) {

        return { nullptr, -1 };
    }

    return { _managers.at(index.internalId()), index.row() };
}

QPair<int, int> PropertyTableModel::toManagerIdAndIndex(const QModelIndex& index) const
{
    if (checkIndex(index) == false
        || index.internalId() == ROOT_INTERNAL_ID) {

        return { -1, -1 };
    }

    return { int(index.internalId()), index.row() };
}

int PropertyTableModel::rowCountFromManager(int index) const
{
    if (index < 0 || index >= _managers.size()) {
        return 0;
    }

    return _managers.at(index)->rowCount();
}

int PropertyTableModel::columnCountFromManager(int index) const
{
    if (index < 0 || index >= _managers.size()) {
        return 0;
    }

    return _managers.at(index)->propertiesList().size();
}

QVariant PropertyTableModel::dataFromManager(const QModelIndex& index) const
{
    if (checkIndex(index) == false
        || index.internalId() == ROOT_INTERNAL_ID) {

        return QVariant();
    }

    auto* m = _managers.at(index.internalId());
    auto& p = m->propertiesList().at(index.column());

    return _managers.at(index.internalId())->data(index.row(), p.id);
}

QString PropertyTableModel::displayFromManager(const QModelIndex& index) const
{
    if (checkIndex(index) == false) {
        return QString();
    }

    if (index.internalId() == ROOT_INTERNAL_ID) {
        if (index.column() == 0) {
            return _managers.at(index.row())->title();
        }
        else {
            return QString();
        }
    }
    else {
        return displayForProperty(index, dataFromManager(index));
    }
}

const Property& PropertyTableModel::propertyForIndex(const QModelIndex& index) const
{
    Q_ASSUME(_managers.size() >= 0);

    if (index.isValid() == false
        || index.model() != this
        || index.internalId() >= (unsigned)_managers.size()
        || index.column() >= _managers.at(index.internalId())->propertiesList().size()) {

        return blankProperty;
    }

    return _managers.at(index.internalId())->propertiesList().at(index.column());
}

QPair<QVariant, QVariant> PropertyTableModel::propertyParametersForIndex(const QModelIndex& index) const
{
    auto& settings = propertyForIndex(index);

    auto param = qMakePair(settings.parameter1, settings.parameter2);
    if (settings.id >= 0) {
        _managers.at(index.internalId())->updateParameters(index.row(), settings.id, param.first, param.second);
    }

    return param;
}

bool PropertyTableModel::isListItem(const QModelIndex&) const
{
    return false;
}

PropertyTableManager* PropertyTableModel::managerForParent(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        if (_managers.size() == 1) {
            return _managers.first();
        }
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID
             && parent.row() >= 0 && parent.row() < _managers.size()) {
        return _managers.at(parent.row());
    }

    return nullptr;
}

bool PropertyTableModel::checkIndex(const QModelIndex& index) const
{
    if (!index.isValid()
        || index.model() != this
        || index.column() >= _nColumns) {

        return false;
    }

    if (index.internalId() == ROOT_INTERNAL_ID) {
        return index.column() == 0 && index.row() < _managers.size();
    }
    else if (index.internalId() < (unsigned)_managers.size()) {
        return index.row() < rowCountFromManager(index.internalId())
               && index.column() < columnCountFromManager(index.internalId());
    }
    else {
        return false;
    }
}

QModelIndex PropertyTableModel::index(int row, int column, const QModelIndex& parent) const
{
    static_assert(ROOT_INTERNAL_ID >= INT_MAX, "ROOT_INTERNAL_ID cannot match QList index");

    if (row < 0
        || column < 0 || column >= _nColumns) {

        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (_managers.size() == 1) {
            if (row < rowCountFromManager(0)) {
                return createIndex(row, column, (quintptr)0);
            }
        }
        else if (row < _managers.size()) {
            return createIndex(row, column, ROOT_INTERNAL_ID);
        }
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID
             && row < rowCountFromManager(parent.row())) {
        return createIndex(row, column, parent.row());
    }

    return QModelIndex();
}

QModelIndex PropertyTableModel::parent(const QModelIndex& index) const
{
    if (index.internalId() == ROOT_INTERNAL_ID) {
        return QModelIndex();
    }
    else if (index.internalId() < (unsigned)_managers.size()
             && _managers.size() > 1) {
        return createIndex(index.internalId(), 0, ROOT_INTERNAL_ID);
    }
    else {
        return QModelIndex();
    }
}

bool PropertyTableModel::hasChildren(const QModelIndex& parent) const
{
    if (_managers.isEmpty()) {
        return false;
    }

    if (!parent.isValid()) {
        return true;
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID
             && _managers.size() > 1) {
        return true;
    }
    else {
        return false;
    }
}

int PropertyTableModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        if (_managers.size() == 1) {
            return rowCountFromManager(0);
        }
        else {
            return _managers.size();
        }
    }
    else if (parent.internalId() == ROOT_INTERNAL_ID) {
        return rowCountFromManager(parent.row());
    }
    else {
        return 0;
    }
}

int PropertyTableModel::columnCount(const QModelIndex& parent) const
{
    if (_managers.isEmpty()) {
        return 0;
    }

    if (!parent.isValid() || parent.internalId() == ROOT_INTERNAL_ID) {
        return _nColumns;
    }
    else {
        return 0;
    }
}

Qt::ItemFlags PropertyTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() && _managers.size() == 1) {
        if (_managers.first()->canMoveItems()) {
            return Qt::ItemIsDropEnabled;
        }
        else {
            return 0;
        }
    }

    if (checkIndex(index) == false) {
        return 0;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled;

    if (index.internalId() == ROOT_INTERNAL_ID) {
        PropertyTableManager* manager = _managers.at(index.row());

        if (manager->canMoveItems()) {
            flags |= Qt::ItemIsDropEnabled;
        }
    }
    else {
        PropertyTableManager* manager = _managers.at(index.internalId());

        flags |= Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

        if (manager->canMoveItems()) {
            flags |= Qt::ItemIsDragEnabled;
        }
        if (propertyForIndex(index).id >= 0) {
            flags |= Qt::ItemIsEditable;
        }
    }

    return flags;
}

QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= _nColumns
        || orientation != Qt::Horizontal
        || role != Qt::DisplayRole) {

        return QVariant();
    }

    return _columnHeaders.at(section);
}

QVariant PropertyTableModel::data(const QModelIndex& index, int role) const
{
    if (checkIndex(index) == false) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return displayFromManager(index);
    }
    else if (role == Qt::EditRole
             && index.internalId() != ROOT_INTERNAL_ID) {

        return dataFromManager(index);
    }

    return QVariant();
}

bool PropertyTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole
        || checkIndex(index) == false
        || index.internalId() == ROOT_INTERNAL_ID) {

        return false;
    }

    const auto& settings = propertyForIndex(index);
    if (settings.id >= 0) {
        auto* m = _managers.at(index.internalId());
        return m->setData(index.row(), settings.id, value);
    }
    else {
        return false;
    }
}

bool PropertyTableModel::canInsert(const QModelIndex& parent)
{
    if (PropertyTableManager* m = managerForParent(parent)) {
        return m->canInsertItem();
    }
    else {
        return false;
    }
}

bool PropertyTableModel::canClone(int row, const QModelIndex& parent)
{
    if (PropertyTableManager* m = managerForParent(parent)) {
        return row >= 0 && row < m->rowCount() && m->canCloneItem(row);
    }
    else {
        return false;
    }
}

bool PropertyTableModel::canMoveItems(const QModelIndex& parent)
{
    if (PropertyTableManager* m = managerForParent(parent)) {
        return m->canMoveItems();
    }
    else {
        return false;
    }
}

// Can only insert one row at a time with this model
bool PropertyTableModel::insertRows(int row, int count, const QModelIndex& parent)
{
    PropertyTableManager* manager = managerForParent(parent);

    if (row < 0
        || count != 1
        || manager == nullptr) {

        return false;
    }

    if (manager->canInsertItem() && row <= manager->rowCount()) {
        return manager->insertItem(row);
    }
    else {
        return false;
    }
}

bool PropertyTableModel::cloneRow(int row, const QModelIndex& parent)
{
    PropertyTableManager* manager = managerForParent(parent);

    if (row < 0
        || manager == nullptr) {

        return false;
    }

    if (row < manager->rowCount() && manager->canCloneItem(row)) {
        return manager->cloneItem(row);
    }
    else {
        return false;
    }
}

// Can only remove one row with this model
bool PropertyTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    PropertyTableManager* manager = managerForParent(parent);

    if (row < 0
        || count != 1
        || manager == nullptr) {

        return false;
    }

    if (row < manager->rowCount()) {
        return manager->removeItem(row);
    }
    else {
        return false;
    }
}

// This code is a LOT simpler if only one item can be moved at a time
bool PropertyTableModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                                  const QModelIndex& destParent, int destRow)
{
    PropertyTableManager* manager = managerForParent(sourceParent);

    if (sourceRow < 0
        || destRow < 0
        || count != 1
        || sourceRow == destRow
        || sourceRow == destRow - 1
        || sourceParent != destParent
        || manager == nullptr
        || manager->canMoveItems() == false) {

        return false;
    }

    int rowCount = manager->rowCount();

    if (sourceRow < rowCount && destRow <= rowCount) {
        if (destRow > sourceRow) {
            destRow--;
        }

        return manager->moveItem(sourceRow, destRow);
    }
    else {
        return false;
    }
}

Qt::DropActions PropertyTableModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions PropertyTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList PropertyTableModel::mimeTypes() const
{
    static const QStringList types = {
        ITEM_MIME_TYPE
    };

    return types;
}

struct PropertyTableModel::InternalMimeData {
    int managerIndex;
    int row;
    const void* model;
};
QDataStream& operator<<(QDataStream& stream, const PropertyTableModel::InternalMimeData& data)
{
    static_assert(sizeof(quintptr) == sizeof(data.model), "Bad quintptr size");

    quintptr ptr = (quintptr)data.model;
    stream << data.managerIndex << data.row << ptr;

    return stream;
}
QDataStream& operator>>(QDataStream& stream, PropertyTableModel::InternalMimeData& data)
{
    static_assert(sizeof(quintptr) == sizeof(data.model), "Bad quintptr size");

    quintptr ptr;
    stream >> data.managerIndex >> data.row >> ptr;
    data.model = (const void*)ptr;

    return stream;
}

// This code is a LOT simpler if only one item can be moved at a time
QMimeData* PropertyTableModel::mimeData(const QModelIndexList& indexes) const
{
    // indexes contains the entire row

    if (indexes.size() < 1
        || indexes.front().internalId() == ROOT_INTERNAL_ID
        || checkIndex(indexes.front()) == false) {

        return nullptr;
    }

    const QModelIndex& index = indexes.front();

    PropertyTableManager* manager = _managers.at(index.internalId());
    if (manager->canMoveItems() == false) {
        return nullptr;
    }

    for (auto& i : indexes) {
        // Only allow drag+drop on a single row
        if (i.model() != this
            || i.row() != index.row()
            || i.internalId() != index.internalId()) {

            return nullptr;
        }
    }

    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << InternalMimeData{ (int)index.internalId(), index.row(), this };

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(ITEM_MIME_TYPE, encodedData);

    return mimeData;
}

bool PropertyTableModel::canDropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                         int destRow, int column, const QModelIndex& parent) const
{
    Q_UNUSED(column)

    PropertyTableManager* manager = managerForParent(parent);

    if (mimeData == nullptr
        || action != Qt::MoveAction
        || destRow < 0
        || manager == nullptr
        || manager->canMoveItems() == false) {

        return false;
    }

    QByteArray encodedData = mimeData->data(ITEM_MIME_TYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    InternalMimeData data;
    stream >> data;

    int parentManagerIndex = _managers.size() > 1 ? parent.row() : 0;

    return stream.atEnd()
           && data.model == this
           && data.managerIndex == parentManagerIndex
           && data.row >= 0 && data.row < rowCountFromManager(data.managerIndex);
}

bool PropertyTableModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                      int destRow, int column, const QModelIndex& parent)
{
    Q_UNUSED(column)

    PropertyTableManager* manager = managerForParent(parent);

    if (mimeData == nullptr
        || action != Qt::MoveAction
        || destRow < 0
        || manager == nullptr
        || manager->canMoveItems() == false) {

        return false;
    }

    QByteArray encodedData = mimeData->data(ITEM_MIME_TYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    InternalMimeData data;
    stream >> data;

    int parentManagerIndex = _managers.size() > 1 ? parent.row() : 0;

    if (stream.atEnd()
        && data.model == this
        && data.managerIndex == parentManagerIndex
        && data.row >= 0 && data.row < rowCountFromManager(data.managerIndex)) {

        // uses moveRow so the the manager only sees one setData call
        moveRow(parent, data.row, parent, destRow);

        // Return false so the View does not delete the source
        return false;
    }

    return false;
}
