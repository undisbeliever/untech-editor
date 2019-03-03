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
const NamedList<EN::EntityFunctionTable>* NamedListAccessor<EN::EntityFunctionTable, EntityFunctionTablesResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.functionTables;
}

template <>
NamedList<EN::EntityFunctionTable>* NamedListAccessor<EN::EntityFunctionTable, EntityFunctionTablesResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.functionTables;
}

EntityFunctionTableList::EntityFunctionTableList(EntityFunctionTablesResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString EntityFunctionTableList::typeName() const
{
    return tr("Entity Function Table");
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<EN::EntityFunctionTable, EntityFunctionTablesResourceItem>;
