/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entitieswithiconsmodel.h"
#include "resourceitem.h"
#include <QMimeData>

using namespace UnTech;
using namespace UnTech::GuiQt::Entity::EntityRomEntries;

const QString EntitiesWithIconsModel::ENTITY_MIME_TYPE = QStringLiteral("application/x-untech-entity-row");

EntitiesWithIconsModel::EntitiesWithIconsModel(QObject* parent)
    : QAbstractListModel(parent)
    , _resourceItem(nullptr)
{
}

void EntitiesWithIconsModel::setResourceItem(ResourceItem* item)
{
    if (_resourceItem == item) {
        return;
    }

    if (_resourceItem) {
        _resourceItem->disconnect(this);
    }
    _resourceItem = item;

    emit layoutChanged();

    if (_resourceItem) {
        connect(_resourceItem, &ResourceItem::entityPixmapsChanged,
                this, &EntitiesWithIconsModel::onEntityPixmapsChanged);
    }
}

void EntitiesWithIconsModel::onEntityPixmapsChanged()
{
    emit layoutChanged();
}

inline int EntitiesWithIconsModel::nEntityPixmaps() const
{
    return _resourceItem ? _resourceItem->entityPixmaps().size() : 0;
}

QModelIndex EntitiesWithIconsModel::toModelIndex(int i) const
{
    if (i < 0 || i > nEntityPixmaps()) {
        return QModelIndex();
    }
    return createIndex(i, 0);
}

bool EntitiesWithIconsModel::isIndexValid(const QModelIndex& index) const
{
    return index.model() == this
           && index.row() >= 0
           && index.row() < nEntityPixmaps();
}

int EntitiesWithIconsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return nEntityPixmaps();
}

Qt::ItemFlags EntitiesWithIconsModel::flags(const QModelIndex& index) const
{
    if (!isIndexValid(index)) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled;
}

QVariant EntitiesWithIconsModel::data(const QModelIndex& index, int role) const
{
    if (_resourceItem == nullptr) {
        return QVariant();
    }

    const auto& entityPixmaps = _resourceItem->entityPixmaps();

    if (index.row() < 0 || index.row() >= entityPixmaps.size()) {
        return QVariant();
    }

    const auto& ep = entityPixmaps.at(index.row());

    if (role == Qt::DisplayRole) {
        return ep.name;
    }
    else if (role == Qt::DecorationRole) {
        return ep.pixmap;
    }

    return QVariant();
}

Qt::DropActions EntitiesWithIconsModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

QMimeData* EntitiesWithIconsModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.size() != 1) {
        return nullptr;
    }

    const auto& index = indexes.first();
    if (!isIndexValid(index)) {
        return nullptr;
    }

    const auto& ep = _resourceItem->entityPixmaps().at(index.row());

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(ENTITY_MIME_TYPE, ep.name.toUtf8());

    return mimeData;
}

std::optional<idstring> EntitiesWithIconsModel::toEntityName(const QMimeData* mimeData)
{
    QByteArray encodedData = mimeData->data(ENTITY_MIME_TYPE);

    if (!mimeData->hasFormat(ENTITY_MIME_TYPE)) {
        return std::nullopt;
    }

    idstring name{ mimeData->data(ENTITY_MIME_TYPE).toStdString() };

    if (name.isValid()) {
        return name;
    }
    else {
        return std::nullopt;
    }
}
