/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractpropertymodel.h"
#include <QAbstractItemModel>
#include <QBitArray>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class PropertyTableManager;
class PropertyModelCache;

class PropertyTableModel : public AbstractPropertyModel {
    Q_OBJECT

    static constexpr quintptr ROOT_INTERNAL_ID = UINT_MAX;

public:
    static const Property blankProperty;

    static const QString ITEM_MIME_TYPE;
    struct InternalMimeData;

public:
    PropertyTableModel(PropertyTableManager* manager, QObject* parent = nullptr);
    PropertyTableModel(QList<PropertyTableManager*> managers,
                       QStringList columnHeaders, QObject* parent = nullptr);
    ~PropertyTableModel() = default;

    const QList<PropertyTableManager*>& managers() const { return _managers; }

    QModelIndex toModelIndex(PropertyTableManager* manager, int index) const;
    QModelIndex toModelIndex(int managerIndex, int index) const;
    QPair<const PropertyTableManager*, int> toManagerAndIndex(const QModelIndex& index) const;

    virtual const Property& propertyForIndex(const QModelIndex& index) const final;
    virtual QPair<QVariant, QVariant> propertyParametersForIndex(const QModelIndex& index) const;
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

    bool canInsert(const QModelIndex& parent);
    bool canClone(int row, const QModelIndex& parent);
    bool canMoveItems(const QModelIndex& parent);

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) final;
    bool cloneRow(int row, const QModelIndex& parent = QModelIndex());
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
    int rowCountFromManager(int index) const;
    int columnCountFromManager(int index) const;
    QVariant dataFromManager(const QModelIndex& index) const;
    QString displayFromManager(const QModelIndex& index) const;

    bool checkIndex(const QModelIndex& index) const;

    PropertyTableManager* managerForParent(const QModelIndex& parent) const;
    QModelIndex createManagerParentIndex(int managerIndex);

private slots:
    void resetModel();
    void updateAll();
    void onManagerItemChanged(int managerIndex, int row);
    void onManagerItemAdded(int managerIndex, int row);
    void onManagerItemRemoved(int managerIndex, int row);
    void onManagerItemMoved(int managerIndex, int from, int to);

private:
    const QList<PropertyTableManager*> _managers;
    const QStringList _columnHeaders;
    const int _nColumns;
};
}
}
