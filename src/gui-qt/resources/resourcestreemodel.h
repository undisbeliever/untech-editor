/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourceitem.h"
#include "models/metasprite/metasprite.h"
#include <QAbstractItemModel>
#include <QIcon>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class Document;

class ResourcesTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    static constexpr unsigned ROOT_INTERNAL_ID = 0xff;
    static constexpr int N_COLUMNS = 1;

public:
    explicit ResourcesTreeModel(QObject* parent = nullptr);
    ~ResourcesTreeModel() = default;

    void setDocument(Document* document);

    QModelIndex toModelIndex(const AbstractResourceItem* item) const;
    AbstractResourceItem* toResourceItem(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const final;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;

private:
    QVariant stateIcon(ResourceState s) const;

    void emitResourceDataChanged(AbstractResourceItem* item, const QVector<int>& roles);

private slots:
    void connectResourceItemSignals(AbstractResourceItem* item);
    void onResourceListChanged();
    void onResourceListStateChanged();
    void onResourceItemStateChanged();
    void onResourceItemNameChanged();

private:
    Document* _document;
    QIcon _uncheckedIcon;
    QIcon _validIcon;
    QIcon _errorIcon;
};
}
}
}
