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

template <>
const std::vector<EN::StructField>* ChildVectorAccessor<EN::StructField, EntityRomStructsResourceItem>::list(size_t pIndex) const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    auto& structs = projectFile->entityRomData.structs;
    if (pIndex >= structs.size()) {
        return nullptr;
    }
    return &structs.at(pIndex).fields;
}

template <>
std::vector<EN::StructField>* ChildVectorAccessor<EN::StructField, EntityRomStructsResourceItem>::getList(size_t pIndex)
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    auto& structs = projectFile->entityRomData.structs;
    if (pIndex >= structs.size()) {
        return nullptr;
    }
    return &structs.at(pIndex).fields;
}

EntityRomStructFieldList::EntityRomStructFieldList(EntityRomStructList* structList)
    : ChildVectorAccessor(structList, structList->resourceItem(), 24)
{
}

QString EntityRomStructFieldList::typeName() const
{
    return tr("Struct Field");
}
