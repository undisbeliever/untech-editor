/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity;

template <>
const NamedList<EN::EntityRomEntry>* NamedListAccessor<EN::EntityRomEntry, EntityRomEntriesResourceItem>::list() const
{
    const auto* erel = qobject_cast<const EntityRomEntriesList*>(this);
    Q_ASSERT(erel);
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    if (erel->isEntityList()) {
        return &projectFile->entityRomData.entities;
    }
    else {
        return &projectFile->entityRomData.projectiles;
    }
}

template <>
NamedList<EN::EntityRomEntry>* NamedListAccessor<EN::EntityRomEntry, EntityRomEntriesResourceItem>::getList()
{
    auto* erel = qobject_cast<EntityRomEntriesList*>(this);
    Q_ASSERT(erel);
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    if (erel->isEntityList()) {
        return &projectFile->entityRomData.entities;
    }
    else {
        return &projectFile->entityRomData.projectiles;
    }
}

EntityRomEntriesList::EntityRomEntriesList(EntityRomEntriesResourceItem* resourceItem,
                                           bool entityList)
    : NamedListAccessor(resourceItem, 255)
    , _entityList(entityList)
{
}

QString EntityRomEntriesList::typeName() const
{
    return tr("Entity ROM Struct");
}

template class UnTech::GuiQt::Accessor::NamedListAccessor<EN::EntityRomEntry, EntityRomEntriesResourceItem>;
