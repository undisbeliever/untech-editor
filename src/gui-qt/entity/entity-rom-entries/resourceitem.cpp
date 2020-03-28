/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/entity/entity-function-tables/resourceitem.h"
#include "gui-qt/entity/entity-rom-structs/resourceitem.h"
#include "gui-qt/project-settings/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project-data.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity::EntityRomEntries;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index,
                           bool entityList)
    : AbstractInternalResourceItem(list, index)
    , _entriesList(new EntityRomEntriesList(this, entityList))
{
    if (entityList) {
        setName(tr("Entities"));
    }
    else {
        setName(tr("Projectiles"));
    }
    setRemovable(false);

    if (entityList) {
        setDependencies({
            { ResourceTypeIndex::STATIC, resourceList()->projectSettings()->name() },
            { ResourceTypeIndex::STATIC, resourceList()->entityFunctionTables()->name() },
            { ResourceTypeIndex::STATIC, tr("Projectiles") },
            { ResourceTypeIndex::MS_FRAMESET, QString() },
        });
    }
    else {
        setDependencies({
            { ResourceTypeIndex::STATIC, resourceList()->projectSettings()->name() },
            { ResourceTypeIndex::STATIC, resourceList()->entityFunctionTables()->name() },
            { ResourceTypeIndex::MS_FRAMESET, QString() },
        });
    }

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);

    // When an item is changed (ie, by the undoStack) change selected entry
    connect(_entriesList, &EntityRomEntriesList::dataChanged,
            _entriesList, &EntityRomEntriesList::setSelectedIndex);
    connect(_entriesList, &EntityRomEntriesList::itemAdded,
            _entriesList, &EntityRomEntriesList::setSelectedIndex);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    if (_entriesList) {
        // EntityRomData is required by the Room Subsystem
        return project()->projectData().compileEntityRomData(err);
    }

    using namespace UnTech::Entity;

    auto* entries = _entriesList->list();
    Q_ASSERT(entries);

    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);

    const auto& structFieldMap = project()->staticResources()->entityRomStructs()->structFieldMap();
    const auto ftFieldMap = generateFunctionTableFieldMap(projectFile->entityRomData.functionTables, structFieldMap, *projectFile, err);

    bool valid = true;

    for (const auto& e : *entries) {
        valid &= e.validate(*projectFile, ftFieldMap, err);
    }

    return valid;
}
