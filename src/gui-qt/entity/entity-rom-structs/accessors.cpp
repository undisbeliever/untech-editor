/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/accessor/selectedindexhelper.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity;

template <>
const NamedList<EN::EntityRomStruct>* NamedListAccessor<EN::EntityRomStruct, EntityRomStructsResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.structs;
}

template <>
NamedList<EN::EntityRomStruct>* NamedListAccessor<EN::EntityRomStruct, EntityRomStructsResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.structs;
}

EntityRomStructList::EntityRomStructList(EntityRomStructsResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString EntityRomStructList::typeName() const
{
    return tr("Entity ROM Struct");
}

EntityRomStructFieldList::EntityRomStructFieldList(EntityRomStructsResourceItem* resourceItem)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
{
    connect(resourceItem->structList(), &EntityRomStructList::selectedIndexChanged,
            this, &EntityRomStructFieldList::selectedListChanged);
    connect(resourceItem->structList(), &EntityRomStructList::selectedIndexChanged,
            this, &EntityRomStructFieldList::unselectItem);

    SelectedIndexHelper::buildAndConnectSlots(this);
}

void EntityRomStructFieldList::setSelectedIndex(size_t index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}
