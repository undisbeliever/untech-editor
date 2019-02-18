/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/namedlist.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class NamedListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit NamedListModel(QObject* parent = nullptr);
    ~NamedListModel() = default;

    QModelIndex toModelIndex(int i) const;
    size_t toIndex(const QModelIndex& index) const;

    const QStringList& displayList() const { return _displayList; }

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual QVariant data(const QModelIndex& index, int role) const override;

    template <class AccessorT>
    void setAccessor(AccessorT* accessor)
    {
        using DataT = typename AccessorT::DataT;
        using ListT = UnTech::NamedList<DataT>;

        if (_accessor) {
            _accessor->disconnect(this);
        }
        _accessor = accessor;

        if (accessor) {
            const ListT* list = accessor->list();

            if (list) {
                buildLists(*list);

                connect(accessor, &AccessorT::itemAdded,
                        this, [=](size_t index) {
                            const ListT* list = accessor->list();
                            Q_ASSERT(list);
                            beginInsertRows(QModelIndex(), index, index);
                            _displayList.insert(index, QString::fromStdString(list->at(index).name));
                            endInsertRows();
                        });
                connect(accessor, &AccessorT::itemAboutToBeRemoved,
                        this, [=](size_t index) {
                            Q_ASSERT(index >= 0 && index < size_t(_displayList.size()));

                            beginRemoveRows(QModelIndex(), index, index);
                            _displayList.removeAt(index);
                            endRemoveRows();
                        });
                connect(accessor, &AccessorT::itemMoved,
                        this, [=](size_t from, size_t to) {
                            const ListT* list = accessor->list();
                            Q_ASSERT(list);
                            Q_ASSERT(from >= 0 && from < size_t(_displayList.size()));
                            Q_ASSERT(to >= 0 && to < size_t(_displayList.size()));
                            _displayList.move(from, to);
                            emit layoutChanged();
                        });
                connect(accessor, &AccessorT::nameChanged,
                        this, [=](size_t index) {
                            const ListT* list = accessor->list();
                            Q_ASSERT(list);
                            Q_ASSERT(index >= 0 && index < size_t(_displayList.size()));
                            _displayList.replace(index, QString::fromStdString(list->at(index).name));
                            QModelIndex mIndex = createIndex(index, 0);
                            emit dataChanged(mIndex, mIndex);
                        });
            }
        }
        else {
            clear();
        }
    }

private:
    void clear();

    template <class T>
    void buildLists(const UnTech::NamedList<T>& list)
    {
        beginResetModel();

        _displayList.clear();

        for (const auto& item : list) {
            _displayList.append(QString::fromStdString(item.name));
        }

        endResetModel();
    }

private:
    QObject* _accessor;

    QStringList _displayList;
};
}
}
}
