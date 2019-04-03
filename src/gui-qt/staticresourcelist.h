/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcelist.h"

namespace UnTech {
namespace GuiQt {

namespace ProjectSettings {
class ResourceItem;
}
namespace Entity {
namespace EntityRomStructs {
class ResourceItem;
}
namespace EntityFunctionTables {
class ResourceItem;
}
namespace EntityRomEntries {
class ResourceItem;
}
}
namespace MetaSprite {
namespace ActionPoints {
class ResourceItem;
}
}

class Project;
class AbstractResourceItem;

class StaticResourceList : public AbstractResourceList {
    Q_OBJECT

    enum Indexes {
        PROJECT_SETTINGS,
        ACTION_POINTS,
        ENTITY_ROM_STRUCTS,
        ENTITY_FUNCTION_TABLES,
        ENTITIES,
        PROJECTILES,
    };
    constexpr static size_t N_ITEMS = 6;

public:
    StaticResourceList(Project* project);
    ~StaticResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QList<AddResourceSettings>& addResourceSettings() const final;

    auto* projectSettingsResourceItem() const { return _projectSettingsResourceItem; }
    auto* actionPointsResourceItem() const { return _actionPointsResourceItem; }
    auto* entityRomStructsResourceItem() const { return _entityRomStructsResourceItem; }
    auto* entityFunctionTablesResourceItem() const { return _entityFunctionTablesResourceItem; }
    auto* entitiesResourceItem() const { return _entitiesResourceItem; }
    auto* projectilesResourceItem() const { return _projectilesResourceItem; }

protected:
    virtual size_t nItems() const final;
    virtual AbstractResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& filename) final;
    virtual void do_removeResource(unsigned index) final;

private:
    ProjectSettings::ResourceItem* const _projectSettingsResourceItem;
    MetaSprite::ActionPoints::ResourceItem* const _actionPointsResourceItem;
    Entity::EntityRomStructs::ResourceItem* const _entityRomStructsResourceItem;
    Entity::EntityFunctionTables::ResourceItem* const _entityFunctionTablesResourceItem;
    Entity::EntityRomEntries::ResourceItem* const _entitiesResourceItem;
    Entity::EntityRomEntries::ResourceItem* const _projectilesResourceItem;
};

}
}
