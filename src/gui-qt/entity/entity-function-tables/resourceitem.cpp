/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/entity/entity-rom-structs/resourceitem.h"
#include "gui-qt/project-settings/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity::EntityFunctionTables;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _functionTableList(new EntityFunctionTableList(this))
{
    setName(tr("Function Tables"));
    setRemovable(false);

    setDependencies({
        { ResourceTypeIndex::STATIC, list->entityRomStructs()->name() },
    });

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    using namespace UnTech::Entity;

    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);

    const auto oldErrorCount = err.errorCount();

    const auto& structFieldMap = project()->staticResources()->entityRomStructs()->structFieldMap();
    generateFunctionTableFieldMap(projectFile->entityRomData.functionTables, structFieldMap, *projectFile, err);

    return oldErrorCount == err.errorCount();
}