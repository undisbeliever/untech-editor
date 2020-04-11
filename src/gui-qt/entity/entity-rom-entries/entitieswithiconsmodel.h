/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <QAbstractListModel>

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityRomEntries {
class ResourceItem;

class EntitiesWithIconsModel : public QAbstractListModel {
    Q_OBJECT

public:
    static const QString ENTITY_MIME_TYPE;

private:
    ResourceItem* _resourceItem;

public:
    explicit EntitiesWithIconsModel(QObject* parent = nullptr);
    ~EntitiesWithIconsModel() = default;

    void setResourceItem(ResourceItem* item);

    QModelIndex toModelIndex(int i) const;

    bool isIndexValid(const QModelIndex& index) const;

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;
    virtual QVariant data(const QModelIndex& index, int role) const final;

    virtual Qt::DropActions supportedDragActions() const final;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const final;

    static std::optional<idstring> toEntityName(const QMimeData* mimeData);

private slots:
    void onEntityPixmapsChanged();

private:
    int nEntityPixmaps() const;
};

}
}
}
}
