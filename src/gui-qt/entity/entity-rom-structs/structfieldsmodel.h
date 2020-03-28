/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/abstractpropertymodel.h"
#include "gui-qt/entity/structfields.h"
#include <QList>

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityRomStructs {
class ResourceItem;

class StructFieldsModel : public AbstractPropertyModel {
    Q_OBJECT

    enum ColumnId {
        NAME_COLUMN,
        TYPE_COLUMN,
        DEFAULT_VALUE_COLUMN,
        COMMENT_COLUMN
    };
    constexpr static int N_COLUMNS = 4;

public:
    explicit StructFieldsModel(QObject* parent = nullptr);
    ~StructFieldsModel() = default;

    void setResourceItem(ResourceItem* item);

    virtual const Property& propertyForIndex(const QModelIndex& index) const final;
    virtual bool isListItem(const QModelIndex&) const final;

    bool checkIndex(const QModelIndex& index) const;

    QModelIndex toModelIndex(size_t fieldIndex) const;
    size_t toFieldIndex(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const final;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

private slots:
    void rebuildData();
    void onStructFieldDataChanged(size_t structId, size_t index);

private:
    const QVector<Property> _properties;

    ResourceItem* _item;

    StructFields _data;
};

}
}
}
}
