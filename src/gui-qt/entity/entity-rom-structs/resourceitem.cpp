/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity::EntityRomStructs;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _structList(new EntityRomStructList(this))
    , _structFieldList(new EntityRomStructFieldList(_structList))
{
    setName(tr("Entity Rom Structs"));

    setRemovable(false);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);

    // When an item is changed (ie, by the undoStack) change selected struct
    connect(_structList, &EntityRomStructList::dataChanged,
            _structList, &EntityRomStructList::setSelectedIndex);
    connect(_structList, &EntityRomStructList::itemAdded,
            _structList, &EntityRomStructList::setSelectedIndex);
    connect(_structFieldList, &EntityRomStructFieldList::dataChanged,
            _structList, &EntityRomStructList::setSelectedIndex);
    connect(_structFieldList, &EntityRomStructFieldList::listChanged,
            _structList, &EntityRomStructList::setSelectedIndex);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);
    const auto& entityRomData = projectFile->entityRomData;

    const auto oldErrorCount = err.errorCount();

    _structFieldMap = UnTech::Entity::generateStructMap(entityRomData.structs, err);

    return err.errorCount() == oldErrorCount;
}
