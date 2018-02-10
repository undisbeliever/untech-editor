/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcestreemodel.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "document.h"

using namespace UnTech::GuiQt::Resources;

static_assert(N_RESOURCE_TYPES <= ResourcesTreeModel::ROOT_INTERNAL_ID,
              "ROOT_INTERNAL_ID too low");

ResourcesTreeModel::ResourcesTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , _document(nullptr)
{
}

void ResourcesTreeModel::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
    }

    beginResetModel();

    _document = document;

    endResetModel();
}

QModelIndex ResourcesTreeModel::toModelIndex(const AbstractResourceItem* item) const
{
    if (item) {
        return createIndex(item->index(), 0, (int)item->resourceTypeIndex());
    }

    return QModelIndex();
}

AbstractResourceItem* ResourcesTreeModel::toResourceItem(const QModelIndex& index) const
{
    if (_document == nullptr
        || index.row() < 0
        || index.column() < 0 || index.column() >= N_COLUMNS) {
        return nullptr;
    }

    const quintptr internalId = index.internalId();
    const int row = index.row();

    if (internalId != ROOT_INTERNAL_ID) {
        const auto& list = _document->resourceLists().at(internalId)->list();

        if (row < list.size()) {
            return list.at(row);
        }
    }

    return nullptr;
}

QModelIndex ResourcesTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (_document == nullptr
        || row < 0
        || column < 0 || column >= N_COLUMNS) {

        return QModelIndex();
    }

    const auto& rl = _document->resourceLists();

    if (!parent.isValid()) {
        if ((unsigned)row < rl.size()) {
            return createIndex(row, column, ROOT_INTERNAL_ID);
        }
    }
    else {
        if ((unsigned)parent.row() < rl.size()) {
            const auto& list = rl.at(parent.row())->list();

            if (row < list.size()) {
                return createIndex(row, column, parent.row());
            }
        }
    }

    return QModelIndex();
}

QModelIndex ResourcesTreeModel::parent(const QModelIndex& index) const
{
    if (_document == nullptr
        || !index.isValid()
        || index.internalId() == ROOT_INTERNAL_ID) {

        return QModelIndex();
    }

    if (index.internalId() <= Document::N_RESOURCE_TYPES) {
        return createIndex(index.internalId(), 0, ROOT_INTERNAL_ID);
    }
    return QModelIndex();
}

bool ResourcesTreeModel::hasChildren(const QModelIndex& parent) const
{
    if (_document == nullptr) {
        return false;
    }

    return !parent.isValid() || parent.internalId() == ROOT_INTERNAL_ID;
}

int ResourcesTreeModel::rowCount(const QModelIndex& parent) const
{
    if (_document == nullptr) {
        return 0;
    }

    const auto& rl = _document->resourceLists();
    const unsigned row = parent.row();

    if (!parent.isValid()) {
        return rl.size();
    }
    else if (row < rl.size()) {
        return rl.at(row)->list().size();
    }
    else {
        return 0;
    }
}

int ResourcesTreeModel::columnCount(const QModelIndex& parent) const
{
    if (_document == nullptr) {
        return 0;
    }

    if (!parent.isValid() || parent.internalId() == ROOT_INTERNAL_ID) {
        return N_COLUMNS;
    }

    return 0;
}

Qt::ItemFlags ResourcesTreeModel::flags(const QModelIndex& index) const
{
    if (_document == nullptr
        || !index.isValid()
        || index.column() >= N_COLUMNS) {

        return 0;
    }

    if (index.internalId() == ROOT_INTERNAL_ID) {
        return Qt::ItemIsEnabled;
    }
    else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }
}

QVariant ResourcesTreeModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole
        || _document == nullptr
        || !index.isValid()
        || index.column() != 0) {

        return QVariant();
    }

    const quintptr internalId = index.internalId();
    const int row = index.row();
    const auto& rl = _document->resourceLists();

    if (internalId == ROOT_INTERNAL_ID) {
        return rl.at(row)->resourceTypeName();
    }
    else if (internalId < rl.size()) {
        const auto& list = rl.at(internalId)->list();

        if (row < list.size()) {
            return list.at(row)->name();
        }
    }

    return QVariant();
}
