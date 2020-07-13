/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "staticresourcelist.h"
#include "project.h"
#include "entity/entity-function-tables/resourceitem.h"
#include "entity/entity-rom-entries/resourceitem.h"
#include "entity/entity-rom-structs/resourceitem.h"
#include "metasprite/actionpoints/resourceitem.h"
#include "metatiles/interactive-tiles/resourceitem.h"
#include "project-settings/resourceitem.h"
#include "resources/scene-settings/resourceitem.h"
#include "resources/scenes/resourceitem.h"

using namespace UnTech::GuiQt;

namespace EN = UnTech::Entity;

StaticResourceList::StaticResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::STATIC)
    , _projectSettings(new ProjectSettings::ResourceItem(this, PROJECT_SETTINGS))
    , _interactiveTiles(new MetaTiles::InteractiveTiles::ResourceItem(this, INTERACTIVE_TILES))
    , _actionPoints(new MetaSprite::ActionPoints::ResourceItem(this, ACTION_POINTS))
    , _entityRomStructs(new Entity::EntityRomStructs::ResourceItem(this, ENTITY_ROM_STRUCTS))
    , _entityFunctionTables(new Entity::EntityFunctionTables::ResourceItem(this, ENTITY_FUNCTION_TABLES))
    , _entities(new Entity::EntityRomEntries::ResourceItem(this, ENTITIES, EN::EntityType::ENTITY))
    , _projectiles(new Entity::EntityRomEntries::ResourceItem(this, PROJECTILES, EN::EntityType::PROJECTILE))
    , _players(new Entity::EntityRomEntries::ResourceItem(this, PLAYERS, EN::EntityType::PLAYER))
    , _sceneSettings(new Resources::SceneSettings::ResourceItem(this, SCENE_SETTINGS))
    , _scenes(new Resources::Scenes::ResourceItem(this, SCENES))
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

const QVector<AbstractResourceList::AddResourceSettings>& StaticResourceList::addResourceSettings() const
{
    static const QVector<AbstractResourceList::AddResourceSettings> emptyList;
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
        return _projectSettings;

    case INTERACTIVE_TILES:
        return _interactiveTiles;

    case ACTION_POINTS:
        return _actionPoints;

    case ENTITY_ROM_STRUCTS:
        return _entityRomStructs;

    case ENTITY_FUNCTION_TABLES:
        return _entityFunctionTables;

    case ENTITIES:
        return _entities;

    case PROJECTILES:
        return _projectiles;

    case PLAYERS:
        return _players;

    case SCENE_SETTINGS:
        return _sceneSettings;

    case SCENES:
        return _scenes;
    }

    throw std::out_of_range("index is invalid");
}

void StaticResourceList::do_addResource(int, const std::string&)
{
}

void StaticResourceList::do_removeResource(unsigned)
{
}
