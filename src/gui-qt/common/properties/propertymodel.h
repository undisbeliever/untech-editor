/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAbstractItemModel>
#include <QBitArray>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class PropertyManager;
class PropertyModelCache;

class PropertyModel : public QAbstractItemModel {
    Q_OBJECT

    enum {
        PROPERTY_COLUMN,
        VALUE_COLUMN,
    };

    static constexpr int N_COLUMNS = 2;

public:
    PropertyModel(PropertyManager* manager);
    ~PropertyModel() = default;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const final;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

private:
    const QVariant& dataFromCache(int index) const;

private slots:
    void resizeCache();

public slots:
    void invalidateCache();
    void updateAll();

private:
    PropertyManager* const _manager;

    mutable QBitArray _cacheDirty;
    mutable QVector<QVariant> _dataCache;
};
}
}
