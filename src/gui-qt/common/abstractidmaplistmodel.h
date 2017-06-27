/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idmap.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {

class AbstractIdmapListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit AbstractIdmapListModel(QObject* parent = nullptr);
    ~AbstractIdmapListModel() = default;

    QModelIndex toModelIndex(const idstring& id) const;
    idstring toIdstring(int row) const;
    idstring toIdstring(const QModelIndex& index) const;

    const QVector<idstring>& idstrings() const { return _idstrings; }
    const QStringList& displayList() const { return _displayList; }

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual QVariant data(const QModelIndex& index, int role) const override;

protected:
    template <class T>
    void buildLists(const idmap<T>& map)
    {
        beginResetModel();

        _displayList.clear();
        _idstrings.clear();

        for (const auto& it : map) {
            _idstrings.append(it.first);
            _displayList.append(QString::fromStdString(it.first));
        }

        endResetModel();
    }

    template <class T>
    void insertMapItem(idmap<T>& map, const idstring& id, std::unique_ptr<T> item)
    {
        Q_ASSERT(map.contains(id) == false);
        Q_ASSERT(item != nullptr);

        int index = 0;
        for (index = 0; index < _idstrings.size(); index++) {
            if (_idstrings.at(index).str() > id.str()) {
                break;
            }
        }

        beginInsertRows(QModelIndex(), index, index);

        map.insertInto(id, std::move(item));
        _idstrings.insert(index, id);
        _displayList.insert(index, QString::fromStdString(id));

        endInsertRows();
    }

    template <class T>
    std::unique_ptr<T> removeMapItem(idmap<T>& map, const idstring& id)
    {
        Q_ASSERT(map.contains(id));

        int index = _idstrings.indexOf(id);

        beginRemoveRows(QModelIndex(), index, index);

        auto item = map.extractFrom(id);
        _idstrings.removeAt(index);
        _displayList.removeAt(index);

        endRemoveRows();

        return item;
    }

    template <class T>
    void renameMapItem(idmap<T>& map,
                       const idstring& oldId, const idstring& newId)
    {
        Q_ASSERT(map.contains(oldId));
        Q_ASSERT(map.contains(newId) == false);

        int oldIndex = _idstrings.indexOf(oldId);

        int newIndex = 0;
        for (newIndex = 0; newIndex < _idstrings.size(); newIndex++) {
            if (_idstrings.at(newIndex).str() > newId.str()) {
                break;
            }
        }

        if (oldIndex != newIndex && oldIndex != newIndex - 1) {
            beginMoveRows(QModelIndex(), oldIndex, oldIndex,
                          QModelIndex(), newIndex);
            if (oldIndex < newIndex) {
                newIndex--;
            }
            map.rename(oldId, newId);

            _idstrings.takeAt(oldIndex);
            _idstrings.insert(newIndex, newId);

            _displayList.takeAt(oldIndex);
            _displayList.insert(newIndex, QString::fromStdString(newId));

            endMoveRows();
        }
        else {
            map.rename(oldId, newId);

            _idstrings.replace(oldIndex, newId);
            _displayList.replace(oldIndex, QString::fromStdString(newId));

            emit dataChanged(createIndex(oldIndex, 0), createIndex(oldIndex, 0));
        }
    }

private:
    QVector<idstring> _idstrings;
    QStringList _displayList;
};
}
}
