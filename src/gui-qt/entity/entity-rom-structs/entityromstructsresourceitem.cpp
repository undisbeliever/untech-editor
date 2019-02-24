/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromstructsresourceitem.h"
#include "accessors.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity;

EntityRomStructsResourceItem::EntityRomStructsResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _structList(new EntityRomStructList(this))
    , _structFieldList(new EntityRomStructFieldList(this))
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

bool EntityRomStructsResourceItem::compileResource(UnTech::ErrorList& err)
{
    using namespace UnTech::Entity;

    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);
    const auto& entityRomData = projectFile->entityRomData;

    auto oldErrorCount = err.errorCount();

    _structFieldMap = generateStructMap(entityRomData.structs, err);

    return err.errorCount() == oldErrorCount;
}
