/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractpropertymodel.h"
#include <QAbstractItemModel>
#include <QBitArray>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class PropertyListManager;
class PropertyModelCache;

class PropertyListModel : public AbstractPropertyModel {
    Q_OBJECT

public:
    // Internal Id:
    //     For property Items: propertyList index
    //     For list items:     propertyList index | LIST_ITEM_FLAG
    static constexpr unsigned LIST_ITEM_FLAG = 0x100000;
    static constexpr unsigned PINDEX_MASK = 0x0fffff;

    enum {
        PROPERTY_COLUMN,
        VALUE_COLUMN,
    };
    static constexpr int N_COLUMNS = 2;

    static const QString ITEM_MIME_TYPE;
    struct InternalMimeData;

public:
    PropertyListModel(PropertyListManager* manager);
    ~PropertyListModel() = default;

    PropertyListManager* manager() const { return _manager; }

    const Property& propertyForIndexIgnoreColumn(const QModelIndex& index) const;
    virtual const Property& propertyForIndex(const QModelIndex& index) const final;
    virtual QPair<QVariant, QVariant> propertyParametersForIndex(const QModelIndex& index) const final;
    virtual bool isListItem(const QModelIndex& index) const final;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const final;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

    bool insertRows(int row, const QModelIndex& parent, const QStringList& values);
    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) final;
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) final;
    virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                          const QModelIndex& destinationParent, int destinationChild) final;

    virtual Qt::DropActions supportedDragActions() const final;
    virtual Qt::DropActions supportedDropActions() const final;
    virtual QStringList mimeTypes() const final;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const final;
    virtual bool canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                 int destRow, int column, const QModelIndex& parent) const final;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                              int row, int column, const QModelIndex& parent) final;

private:
    void updateCacheIfDirty(int index) const;
    const QVariant& dataFromCache(int index) const;
    const QString& displayFromCache(int index) const;
    int propertyListSize(int index) const;

    bool checkIndex(const QModelIndex& index) const;

private slots:
    void onManagerPropertyListChanged();

public slots:
    void invalidateCache();
    void updateAll();

private:
    struct PropertyLayout {
        // parentRow is not needed, but used for padding
        int rowPos;
        int nChildren;
        int parentIndex;
        int parentRow;
    };

private:
    PropertyListManager* const _manager;

    QVector<int> _rootIndexes;
    QVector<PropertyLayout> _propertyLayout;

    mutable QBitArray _cacheDirty;
    mutable QVector<QVariant> _dataCache;
    mutable QVector<int> _listSizeCache;
    mutable QVector<QString> _displayCache;
};
}
}
