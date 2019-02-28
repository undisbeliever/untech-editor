/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class AbstractNamedListAccessor;

// This model does not emit rows added/removed/moved signals
// as it interferes with AbstractNamedListAccessor::selectedIndexChanged
class NamedListModel : public QAbstractListModel {
    Q_OBJECT

private:
    AbstractNamedListAccessor* _accessor;

    QStringList _displayList;

public:
    explicit NamedListModel(QObject* parent = nullptr);
    ~NamedListModel() = default;

    QModelIndex toModelIndex(int i) const;
    size_t toIndex(const QModelIndex& index) const;

    const QStringList& displayList() const { return _displayList; }

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual QVariant data(const QModelIndex& index, int role) const override;

    void setAccessor(AbstractNamedListAccessor* accessor);

private:
    void resetDisplayList();
    void onNameChanged(size_t index);
    void onItemAdded(size_t index);
    void onItemAboutToBeRemoved(size_t index);
    void onItemMoved(size_t from, size_t to);
};
}
}
}
