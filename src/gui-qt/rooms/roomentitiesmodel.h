/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/abstractpropertymodel.h"
#include "models/common/optional.h"
#include "models/common/vectorset.h"
#include "models/rooms/rooms.h"

namespace UnTech {
namespace GuiQt {
namespace Rooms {
class ResourceItem;

namespace RM = UnTech::Rooms;

class RoomEntitiesModel final : public AbstractPropertyModel {
    Q_OBJECT

    constexpr static quintptr GROUP_INTERNAL_ID = std::numeric_limits<quintptr>::max();

    enum ColumnId {
        NAME,
        ENTITY_ID,
        POSITION
    };
    constexpr static int N_COLUMNS = 3;

public:
    explicit RoomEntitiesModel(QObject* parent = nullptr);
    ~RoomEntitiesModel() = default;

    void setResourceItem(ResourceItem* item);

    bool isGroupIndex(const QModelIndex& index) const;
    unsigned toEntryGroupIndex(const QModelIndex& index) const;
    std::pair<unsigned, unsigned> toEntityEntryIndex(const QModelIndex& index) const;
    vectorset<std::pair<size_t, size_t>> toEntityEntryIndexes(const QModelIndexList& indexes) const;

    QModelIndex toModelIndex(size_t groupIndex) const;
    QModelIndex toModelIndex(size_t groupIndex, size_t entryIndex) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const final;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual const Property& propertyForIndex(const QModelIndex& index) const final;
    virtual bool isListItem(const QModelIndex& index) const final;
    virtual QPair<QVariant, QVariant> propertyParametersForIndex(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const final;
    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

    virtual Qt::DropActions supportedDragActions() const final;
    virtual Qt::DropActions supportedDropActions() const final;
    virtual QStringList mimeTypes() const final;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const final;
    virtual bool canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                 int destRow, int column, const QModelIndex& parent) const final;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                              int row, int column, const QModelIndex& parent) final;

private:
    optional<const RM::RoomInput&> roomInput() const;

    bool isIndexValid(const QModelIndex& index) const;

    QVariant entityGroupData(unsigned groupIndex, int column, int role) const;
    QVariant entityEntryData(unsigned groupIndex, unsigned entryIndex, int column, int role) const;
    bool setEntityGroupData(unsigned groupIndex, int column, const QVariant& value) const;
    bool setEntityEntryData(unsigned groupIndex, unsigned entryIndex, int column, const QVariant& value) const;

private slots:
    void onListChanged();
    void onEntityGroupChanged(size_t groupIndex);
    void onEntityEntryChanged(size_t groupIndex, size_t entryIndex);

private:
    ResourceItem* _resourceItem;

    const QVector<Property> _properties;
};

}
}
}
