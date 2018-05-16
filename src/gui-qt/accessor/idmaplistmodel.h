/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idmap.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class IdmapListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit IdmapListModel(QObject* parent = nullptr);
    ~IdmapListModel() = default;

    int indexOf(const QString& id) const;

    QModelIndex toModelIndex(const idstring& id) const;
    idstring toIdstring(int row) const;
    idstring toIdstring(const QModelIndex& index) const;

    const QStringList& displayList() const { return _displayList; }

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual QVariant data(const QModelIndex& index, int role) const override;

    template <class AccessorT>
    void setAccessor(AccessorT* accessor)
    {
        using DataT = typename AccessorT::DataT;

        if (_accessor) {
            _accessor->disconnect(this);
        }
        _accessor = accessor;

        if (accessor) {
            const idmap<DataT>* map = accessor->map();

            if (map) {
                buildLists(*map);

                connect(accessor, &AccessorT::itemAdded,
                        this, &IdmapListModel::addIdstring);
                connect(accessor, &AccessorT::itemAboutToBeRemoved,
                        this, &IdmapListModel::removeIdstring);
                connect(accessor, &AccessorT::itemRenamed,
                        this, &IdmapListModel::renameIdstring);
            }
        }
        else {
            clear();
        }
    }

private:
    void clear();

    template <class T>
    void buildLists(const idmap<T>& map)
    {
        beginResetModel();

        _displayList.clear();

        for (const auto& it : map) {
            _displayList.append(QString::fromStdString(it.first));
        }

        endResetModel();
    }

    int indexToInsert(const QString& id) const;

private slots:
    void addIdstring(const idstring& id);
    void removeIdstring(const idstring& id);
    void renameIdstring(const idstring& oldId, const idstring& newId);

private:
    QObject* _accessor;

    QStringList _displayList;
};
}
}
}
