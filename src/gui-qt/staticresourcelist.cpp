/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "staticresourcelist.h"
#include "project.h"
#include "entity/entity-function-tables/resourceitem.h"
#include "entity/entity-rom-entries/resourceitem.h"
#include "entity/entity-rom-structs/resourceitem.h"
#include "metasprite/actionpoints/resourceitem.h"
#include "project-settings/resourceitem.h"

using namespace UnTech::GuiQt;

StaticResourceList::StaticResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::STATIC)
    , _projectSettingsResourceItem(new ProjectSettings::ResourceItem(this, PROJECT_SETTINGS))
    , _actionPointsResourceItem(new MetaSprite::ActionPoints::ResourceItem(this, ACTION_POINTS))
    , _entityRomStructsResourceItem(new Entity::EntityRomStructs::ResourceItem(this, ENTITY_ROM_STRUCTS))
    , _entityFunctionTablesResourceItem(new Entity::EntityFunctionTables::ResourceItem(this, ENTITY_FUNCTION_TABLES))
    , _entitiesResourceItem(new Entity::EntityRomEntries::ResourceItem(this, ENTITIES, true))
    , _projectilesResourceItem(new Entity::EntityRomEntries::ResourceItem(this, PROJECTILES, false))
{
}

const QString StaticResourceList::resourceTypeNameSingle() const
{
    return tr("Project");
}

const QString StaticResourceList::resourceTypeNamePlural() const
{
    return tr("Project");
}

const QList<AbstractResourceList::AddResourceSettings>& StaticResourceList::addResourceSettings() const
{
    static const QList<AbstractResourceList::AddResourceSettings> emptyList;
    return emptyList;
}

size_t StaticResourceList::nItems() const
{
    return N_ITEMS;
}

AbstractResourceItem* StaticResourceList::buildResourceItem(size_t index)
{
    assert(index < N_ITEMS);
    switch (static_cast<Indexes>(index)) {
    case PROJECT_SETTINGS:
        return _projectSettingsResourceItem;

    case ACTION_POINTS:
        return _actionPointsResourceItem;

    case ENTITY_ROM_STRUCTS:
        return _entityRomStructsResourceItem;

    case ENTITY_FUNCTION_TABLES:
        return _entityFunctionTablesResourceItem;

    case ENTITIES:
        return _entitiesResourceItem;

    case PROJECTILES:
        return _projectilesResourceItem;
    }

    throw std::out_of_range("index is invalid");
}

void StaticResourceList::do_addResource(int, const std::string&)
{
}

void StaticResourceList::do_removeResource(unsigned)
{
}
