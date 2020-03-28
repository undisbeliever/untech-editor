/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class IdstringValidator;

namespace Accessor {

class AbstractNamedListAccessor;

// This model does not emit rows added/removed/moved signals
// as it interferes with AbstractNamedListAccessor::selectedIndexChanged
class NamedListModel : public QAbstractListModel {
    Q_OBJECT

public:
    struct InternalMimeData;

private:
    static IdstringValidator* const ID_STRING_VALIDATOR;
    static const QString ITEM_MIME_TYPE;

    AbstractNamedListAccessor* _accessor;

    QStringList _displayList;

public:
    explicit NamedListModel(QObject* parent = nullptr);
    ~NamedListModel() = default;

    void setAccessor(AbstractNamedListAccessor* accessor);

    QModelIndex toModelIndex(int i) const;
    size_t toIndex(const QModelIndex& index) const;

    const QStringList& displayList() const { return _displayList; }

    bool checkIndex(const QModelIndex& index) const;

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;
    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

    virtual Qt::DropActions supportedDragActions() const final;
    virtual Qt::DropActions supportedDropActions() const final;
    virtual QStringList mimeTypes() const final;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const final;
    virtual bool canDropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                                 int destRow, int column, const QModelIndex& parent) const final;
    virtual bool dropMimeData(const QMimeData* mimeData, Qt::DropAction action,
                              int row, int column, const QModelIndex& parent) final;

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
