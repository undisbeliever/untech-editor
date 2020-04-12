/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "roomentitiesmodel.h"
#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/properties/internalmovepairmimedata.h"
#include "gui-qt/entity/entity-rom-entries/accessors.h"
#include "gui-qt/entity/entity-rom-entries/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Rooms;

RoomEntitiesModel::RoomEntitiesModel(QObject* parent)
    : AbstractPropertyModel(parent)
    , _resourceItem(nullptr)
    , _properties({
          Property(tr("Name"), NAME, PropertyType::IDSTRING),
          Property(tr("Entity"), ENTITY_ID, PropertyType::COMBO),
          Property(tr("Position"), POSITION, PropertyType::POINT),
      })
{
}

void RoomEntitiesModel::setResourceItem(ResourceItem* item)
{
    if (_resourceItem == item) {
        return;
    }

    if (_resourceItem) {
        _resourceItem->entityGroups()->disconnect(this);
        _resourceItem->entityEntries()->disconnect(this);
    }
    _resourceItem = item;

    if (_resourceItem) {
        connect(_resourceItem->entityGroups(), &EntityGroupList::listReset,
                this, &RoomEntitiesModel::onListChanged);
        connect(_resourceItem->entityGroups(), &EntityGroupList::listChanged,
                this, &RoomEntitiesModel::onListChanged);
        connect(_resourceItem->entityGroups(), &EntityGroupList::dataChanged,
                this, &RoomEntitiesModel::onEntityGroupChanged);

        connect(_resourceItem->entityEntries(), &EntityEntriesList::listReset,
                this, &RoomEntitiesModel::onListChanged);
        connect(_resourceItem->entityEntries(), &EntityEntriesList::listChanged,
                this, &RoomEntitiesModel::onListChanged);
        connect(_resourceItem->entityEntries(), &EntityEntriesList::dataChanged,
                this, &RoomEntitiesModel::onEntityEntryChanged);
    }

    emit layoutChanged();
}

optional<const RM::RoomInput&> RoomEntitiesModel::roomInput() const
{
    return _resourceItem ? _resourceItem->roomInput() : optional<const RM::RoomInput&>{};
}

bool RoomEntitiesModel::isGroupIndex(const QModelIndex& index) const
{
    // Assumes index was created by this model
    return index.internalId() == GROUP_INTERNAL_ID;
}

unsigned RoomEntitiesModel::toEntryGroupIndex(const QModelIndex& index) const
{
    if (index.model() != this) {
        return INT_MAX;
    }
    return index.internalId() == GROUP_INTERNAL_ID ? index.row() : index.internalId();
}

std::pair<unsigned, unsigned> RoomEntitiesModel::toEntityEntryIndex(const QModelIndex& index) const
{
    if (index.isValid() == false
        || index.model() != this
        || index.internalId() == GROUP_INTERNAL_ID) {

        return { INT_MAX, INT_MAX };
    }
    return { index.internalId(), unsigned(index.row()) };
}

vectorset<std::pair<size_t, size_t>> RoomEntitiesModel::toEntityEntryIndexes(const QModelIndexList& modelIndexes) const
{
    vectorset<std::pair<size_t, size_t>> out;

    for (auto& index : modelIndexes) {
        if (index.isValid()
            && index.model() == this
            && index.internalId() != GROUP_INTERNAL_ID) {

            out.insert({ index.internalId(), unsigned(index.row()) });
        }
    }

    return out;
}

QModelIndex RoomEntitiesModel::toModelIndex(size_t groupIndex) const
{
    if (groupIndex >= INT_MAX) {
        return QModelIndex();
    }

    return createIndex(groupIndex, NAME, GROUP_INTERNAL_ID);
}

QModelIndex RoomEntitiesModel::toModelIndex(size_t groupIndex, size_t entryIndex) const
{
    if (groupIndex >= INT_MAX
        || entryIndex >= INT_MAX) {

        return QModelIndex();
    }

    // Prefer editing ENTITY_ID over name
    return createIndex(entryIndex, ENTITY_ID, groupIndex);
}

bool RoomEntitiesModel::isIndexValid(const QModelIndex& index) const
{
    auto ri = this->roomInput();
    if (ri.exists() == false
        || index.isValid() == false
        || index.column() >= N_COLUMNS
        || index.model() != this) {

        return false;
    }

    if (index.internalId() == GROUP_INTERNAL_ID) {
        const unsigned groupIndex = index.row();

        return groupIndex < ri->entityGroups.size();
    }
    else {
        const unsigned groupIndex = index.internalId();
        const unsigned entryIndex = index.row();

        if (groupIndex >= ri->entityGroups.size()) {
            return false;
        }
        return entryIndex < ri->entityGroups.at(groupIndex).entities.size();
    }
}

QModelIndex RoomEntitiesModel::index(int row, int column, const QModelIndex& parent) const
{
    auto ri = this->roomInput();
    if (ri.exists() == false
        || row < 0
        || column < 0 || column >= N_COLUMNS) {

        return QModelIndex();
    }

    if (!parent.isValid()) {
        // parent = root node

        if (unsigned(row) < ri->entityGroups.size()) {
            return createIndex(row, column, GROUP_INTERNAL_ID);
        }
    }
    else if (isGroupIndex(parent)) {
        const size_t groupIndex = parent.row();
        if (groupIndex < ri->entityGroups.size()) {
            if (unsigned(row) < ri->entityGroups.at(groupIndex).entities.size()) {
                return createIndex(row, column, groupIndex);
            }
        }
    }

    return QModelIndex();
}

QModelIndex RoomEntitiesModel::parent(const QModelIndex& index) const
{
    auto ri = this->roomInput();
    if (ri.exists() == false
        || index.isValid() == false
        || index.model() != this) {

        return QModelIndex();
    }

    if (isGroupIndex(index)) {
        return QModelIndex();
    }
    else {
        const size_t gi = toEntryGroupIndex(index);
        if (gi < ri->entityGroups.size()) {
            return toModelIndex(gi);
        }
        else {
            return QModelIndex();
        }
    }
}

bool RoomEntitiesModel::hasChildren(const QModelIndex& parent) const
{
    auto ri = this->roomInput();
    if (ri.exists() == false) {
        return false;
    }

    if (!parent.isValid()) {
        return true;
    }

    return isGroupIndex(parent);
}

int RoomEntitiesModel::rowCount(const QModelIndex& parent) const
{
    auto ri = this->roomInput();
    if (ri.exists() == false) {
        return 0;
    }

    if (!parent.isValid()) {
        return ri->entityGroups.size();
    }
    else if (isGroupIndex(parent)) {
        const size_t groupIndex = toEntryGroupIndex(parent);
        if (groupIndex < ri->entityGroups.size()) {
            return ri->entityGroups.at(groupIndex).entities.size();
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

int RoomEntitiesModel::columnCount(const QModelIndex&) const
{
    return N_COLUMNS;
}

Qt::ItemFlags RoomEntitiesModel::flags(const QModelIndex& index) const
{
    auto ri = this->roomInput();
    if (ri.exists() == false
        || isIndexValid(index) == false) {

        return 0;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (isGroupIndex(index)) {
        if (index.column() == NAME) {
            flags |= Qt::ItemIsEditable;
        }
        flags |= Qt::ItemIsDropEnabled;
    }
    else {
        // entity entry index
        flags |= Qt::ItemIsEditable | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled;
    }

    return flags;
}

const GuiQt::Property& RoomEntitiesModel::propertyForIndex(const QModelIndex& index) const
{
    if (isIndexValid(index) == false) {
        return blankProperty;
    }

    if (isGroupIndex(index)) {
        if (index.column() == NAME) {
            return _properties.at(0);
        }
        else {
            return blankProperty;
        }
    }
    else {
        return _properties.at(index.column());
    }
}

bool RoomEntitiesModel::isListItem(const QModelIndex&) const
{
    return false;
}

QPair<QVariant, QVariant> RoomEntitiesModel::propertyParametersForIndex(const QModelIndex& index) const
{
    static const QPair<QVariant, QVariant> BLANK_PAIR{ QVariant(), QVariant() };

    if (isIndexValid(index) == false) {
        return BLANK_PAIR;
    }

    if (isGroupIndex(index)) {
        // EntityGroup
        return BLANK_PAIR;
    }
    else {
        // EntityEntry
        switch (ColumnId(index.column())) {
        case NAME:
            return BLANK_PAIR;

        case ENTITY_ID:
            Q_ASSERT((_resourceItem));
            return qMakePair(_resourceItem->project()->staticResources()->entities()->entriesList()->itemNames(), QVariant());

        case POSITION:
            if (auto ri = roomInput()) {
                return qMakePair(QPoint(0, -int(RM::ENTITY_VERTICAL_SPACING)),
                                 QPoint(ri->mapRight(), ri->mapBottom() + RM::ENTITY_VERTICAL_SPACING - 1));
            }
        }
        return BLANK_PAIR;
    }
}

QVariant RoomEntitiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= N_COLUMNS
        || orientation != Qt::Horizontal
        || role != Qt::DisplayRole) {

        return QVariant();
    }

    return _properties.at(section).title;
}

QVariant RoomEntitiesModel::entityGroupData(unsigned groupIndex, int column, int role) const
{
    const RM::EntityGroup& group = roomInput()->entityGroups.at(groupIndex);

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (ColumnId(column)) {
        case NAME:
            return QString::fromStdString(group.name);

        case ENTITY_ID:
        case POSITION:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant RoomEntitiesModel::entityEntryData(unsigned groupIndex, unsigned entryIndex, int column, int role) const
{
    const RM::EntityEntry& entry = roomInput()->entityGroups.at(groupIndex).entities.at(entryIndex);

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (ColumnId(column)) {
        case NAME:
            return QString::fromStdString(entry.name);

        case ENTITY_ID:
            return QString::fromStdString(entry.entityId);

        case POSITION:
            if (role == Qt::DisplayRole) {
                return QStringLiteral("%1, %2")
                    .arg(entry.position.x)
                    .arg(entry.position.y);
            }
            else {
                return fromPoint(entry.position);
            }
        }
    }

    return QVariant();
}

QVariant RoomEntitiesModel::data(const QModelIndex& index, int role) const
{
    if (isIndexValid(index) == false) {
        return QVariant();
    }

    if (isGroupIndex(index)) {
        const size_t groupIndex = toEntryGroupIndex(index);
        return entityGroupData(groupIndex, index.column(), role);
    }
    else {
        const auto ei = toEntityEntryIndex(index);
        return entityEntryData(ei.first, ei.second, index.column(), role);
    }
}

bool RoomEntitiesModel::setEntityGroupData(unsigned groupIndex, int column, const QVariant& value) const
{
    Q_ASSERT(_resourceItem);

    switch (ColumnId(column)) {
    case NAME:
        return _resourceItem->entityGroups()->edit_setName(groupIndex, value.toString().toStdString());

    case ENTITY_ID:
    case POSITION:
        return false;
    }

    return false;
}

bool RoomEntitiesModel::setEntityEntryData(unsigned groupIndex, unsigned entryIndex, int column, const QVariant& value) const
{
    Q_ASSERT(_resourceItem);

    switch (ColumnId(column)) {
    case NAME:
        return _resourceItem->entityEntries()->edit_setName(groupIndex, entryIndex, value.toString().toStdString());

    case ENTITY_ID:
        return _resourceItem->entityEntries()->edit_setEntityId(groupIndex, entryIndex, value.toString().toStdString());

    case POSITION:
        return _resourceItem->entityEntries()->edit_setPosition(groupIndex, entryIndex, toPoint(value.toPoint()));
    }

    return false;
}

bool RoomEntitiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole
        || isIndexValid(index) == false) {

        return false;
    }

    if (isGroupIndex(index)) {
        const size_t groupIndex = toEntryGroupIndex(index);
        return setEntityGroupData(groupIndex, index.column(), value);
    }
    else {
        const auto ei = toEntityEntryIndex(index);
        return setEntityEntryData(ei.first, ei.second, index.column(), value);
    }
}

void RoomEntitiesModel::onListChanged()
{
    emit layoutChanged();
}

void RoomEntitiesModel::onEntityGroupChanged(size_t groupIndex)
{
    if (auto ri = roomInput()) {
        if (groupIndex < ri->entityGroups.size()) {
            const QModelIndex mIndex = toModelIndex(groupIndex);
            auto mIndex2 = createIndex(mIndex.row(), N_COLUMNS, mIndex.internalId());

            emit dataChanged(mIndex, mIndex2,
                             { Qt::DisplayRole, Qt::EditRole });
        }
    }
}

void RoomEntitiesModel::onEntityEntryChanged(size_t groupIndex, size_t entryIndex)
{
    if (auto ri = roomInput()) {
        if (groupIndex < ri->entityGroups.size()) {
            if (entryIndex < ri->entityGroups.at(groupIndex).entities.size()) {
                const QModelIndex mIndex = toModelIndex(groupIndex, entryIndex);
                auto mIndex2 = createIndex(mIndex.row(), N_COLUMNS, mIndex.internalId());

                emit dataChanged(mIndex, mIndex2,
                                 { Qt::DisplayRole, Qt::EditRole });
            }
        }
    }
}

Qt::DropActions RoomEntitiesModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions RoomEntitiesModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList RoomEntitiesModel::mimeTypes() const
{
    static const QStringList types = {
        InternalMovePairMimeData::MIME_TYPE
    };

    return types;
}

// This code is a LOT simpler if only one item can be moved at a time
QMimeData* RoomEntitiesModel::mimeData(const QModelIndexList& indexes) const
{
    // indexes contains the entire row

    if (indexes.size() < 1
        || indexes.front().internalId() == GROUP_INTERNAL_ID
        || checkIndex(indexes.front()) == false) {

        return nullptr;
    }

    const QModelIndex& index = indexes.front();

    for (auto& i : indexes) {
        // Only allow drag+drop on a single row
        if (i.model() != this
            || i.row() != index.row()
            || i.internalId() != index.internalId()) {

            return nullptr;
        }
    }

    const auto source = toEntityEntryIndex(index);

    return InternalMovePairMimeData::toMimeData(source.first, source.second, this);
}

bool RoomEntitiesModel::canDropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                        int destRow, int column, const QModelIndex& parent) const
{
    Q_UNUSED(column)

    const auto ri = this->roomInput();

    if (mimeData == nullptr
        || action != Qt::MoveAction
        || ri.exists() == false
        || isGroupIndex(parent) == false) {

        return false;
    }

    const unsigned destGroupIndex = toEntryGroupIndex(parent);
    if (destGroupIndex >= ri->entityGroups.size()) {
        return false;
    }

    auto [sourceGroupIndex, sourceChildIndex] = InternalMovePairMimeData::fromMimeData(mimeData, this);
    if (sourceGroupIndex < 0 || sourceChildIndex < 0) {
        return false;
    }

    if (destRow < 0 && unsigned(sourceGroupIndex) == destGroupIndex) {
        // Disallow moving a child row into its own parent.
        return false;
    }

    return unsigned(sourceGroupIndex) < ri->entityGroups.size()
           && unsigned(sourceChildIndex) < ri->entityGroups.at(sourceGroupIndex).entities.size();
}

bool RoomEntitiesModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                     int destRow, int column, const QModelIndex& parent)
{
    Q_UNUSED(column)

    const auto ri = this->roomInput();

    if (mimeData == nullptr
        || action != Qt::MoveAction
        || ri.exists() == false
        || !(parent.isValid() == false || isGroupIndex(parent))) {

        return false;
    }

    const unsigned destGroupIndex = toEntryGroupIndex(parent);
    if (destGroupIndex >= ri->entityGroups.size()) {
        return false;
    }

    if (destRow < 0) {
        destRow = ri->entityGroups.at(destGroupIndex).entities.size();
    }

    auto [sourceGroupIndex, sourceChildIndex] = InternalMovePairMimeData::fromMimeData(mimeData, this);
    if (sourceGroupIndex < 0 || sourceChildIndex < 0) {
        return false;
    }

    if (unsigned(sourceGroupIndex) == destGroupIndex) {
        if (destRow > sourceChildIndex) {
            destRow--;
        }
    }

    // There is no need to validate the indexes, the UndoHandler will take care of it.
    _resourceItem->entityEntries()->moveItem(sourceGroupIndex, sourceChildIndex,
                                             destGroupIndex, destRow);

    // Return false so the View does not delete the source
    return false;
}
