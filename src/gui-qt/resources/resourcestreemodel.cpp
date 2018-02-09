/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcestreemodel.h"
#include "document.h"

using namespace UnTech::GuiQt::Resources;

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

QModelIndex ResourcesTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (_document == nullptr
        || row < 0
        || column < 0 || column > N_COLUMNS) {

        return QModelIndex();
    }

    const auto& res = *_document->resourcesFile();

    if (!parent.isValid()) {
        if (row < N_ROOT_NODES) {
            return createIndex(row, column, ROOT);
        }
        return QModelIndex();
    }
    else {
        switch ((InternalId)parent.row() + 1) {
        case PALETTES:
            if ((unsigned)row < res.palettes.size()) {
                return createIndex(row, column, PALETTES);
            }
            return QModelIndex();

        case METATILE_TILESETS:
            if ((unsigned)row < res.metaTileTilesetFilenames.size()) {
                return createIndex(row, column, METATILE_TILESETS);
            }
            return QModelIndex();

        default:
            return QModelIndex();
        }
    }
}

QModelIndex ResourcesTreeModel::parent(const QModelIndex& index) const
{
    if (_document == nullptr
        || !index.isValid()
        || index.internalId() == ROOT) {

        return QModelIndex();
    }

    if (index.internalId() <= N_ROOT_NODES) {
        return createIndex(index.internalId() - 1, 0, ROOT);
    }
    return QModelIndex();
}

bool ResourcesTreeModel::hasChildren(const QModelIndex& parent) const
{
    if (_document == nullptr) {
        return false;
    }

    return !parent.isValid() || parent.internalId() == ROOT;
}

int ResourcesTreeModel::rowCount(const QModelIndex& parent) const
{
    if (_document == nullptr) {
        return 0;
    }

    const auto& res = *_document->resourcesFile();

    if (!parent.isValid()) {
        return N_ROOT_NODES;
    }
    else {
        switch ((InternalId)parent.row() + 1) {
        case InternalId::PALETTES:
            return res.palettes.size();

        case InternalId::METATILE_TILESETS:
            return res.metaTileTilesetFilenames.size();

        default:
            return 0;
        }
    }
}

int ResourcesTreeModel::columnCount(const QModelIndex& parent) const
{
    if (_document == nullptr) {
        return 0;
    }

    if (!parent.isValid() || parent.internalId() == ROOT) {
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

    if (index.internalId() == ROOT) {
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

    int row = index.row();
    const auto& res = *_document->resourcesFile();

    switch (index.internalId()) {
    case InternalId::ROOT: {
        switch ((InternalId)index.row() + 1) {
        case PALETTES:
            return tr("Palettes");

        case METATILE_TILESETS:
            return tr("MetaTile Tilesets");
        }
        break;
    }

    case InternalId::PALETTES:
        return QString::fromStdString(res.palettes.at(row)->name);

    case InternalId::METATILE_TILESETS:
        return QString::fromStdString(res.metaTileTilesetFilenames.at(row));
    }

    return QVariant();
}
