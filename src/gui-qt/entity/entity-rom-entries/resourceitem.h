/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/entity/entityromdata.h"

#include <QPixmap>

namespace UnTech {
namespace GuiQt {
class StaticResourceList;

namespace Entity {
namespace EntityRomEntries {
class EntityRomEntriesList;

struct EntityPixmap {
    // The pixmaps have a minimum size to ensure they are centered within the
    // `EntitiesWithIconsModel` view.
    // It appears you cannot center icons in a View without subclassing QStyledItemDelegate,
    // so I have decided to center it in the drawFrame function instead.
    constexpr static int MIN_PIXMAP_SIZE = 32;

    QPixmap pixmap;
    QString name;
    QPoint origin;
    QRectF boundingBox;
};

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

    using EntityRomData = UnTech::Entity::EntityRomData;
    using StructFieldMap = UnTech::Entity::StructFieldMap;

public:
    ResourceItem(StaticResourceList* list, unsigned index, bool entityList);
    ~ResourceItem() = default;

    StaticResourceList* resourceList() const { return static_cast<StaticResourceList*>(_list); }

    EntityRomEntriesList* entriesList() const { return _entriesList; }

    // entityPixmaps exists in ResourceItem so it can be reused across rooms/windows
    // without rebuilding it every time the resource item changes.
    //
    // This is fine, as there are only two EntityRomEntries ResourceItems per project
    const QVector<EntityPixmap>& entityPixmaps() const { return _entityPixmaps; }

    optional<const EntityPixmap&> findEntityPixmap(const idstring& id) const
    {
        auto it = _entityNameMap.find(id);
        return it != _entityNameMap.end() ? _entityPixmaps.at(it->second) : optional<const EntityPixmap&>{};
    }

protected:
    virtual bool compileResource(ErrorList& err) final;

signals:
    void entityPixmapsChanged();

private slots:
    void updateEntityPixmaps();

private:
    EntityRomEntriesList* _entriesList;

    QVector<EntityPixmap> _entityPixmaps;
    std::unordered_map<idstring, unsigned> _entityNameMap;
};

}
}
}
}
