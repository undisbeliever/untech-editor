/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "models/project/project.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaTiles::InteractiveTiles;

template <>
const NamedList<MT::InteractiveTileFunctionTable>* NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->interactiveTiles.functionTables;
}

template <>
NamedList<MT::InteractiveTileFunctionTable>* NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->interactiveTiles.functionTables;
}

FunctionTableList::FunctionTableList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, MT::MAX_INTERACTIVE_TILE_FUNCTION_TABLES - MT::InteractiveTiles::FIXED_FUNCTION_TABLES.size())
{
}

QString FunctionTableList::typeName() const
{
    return tr("Interactive Tile Function Table");
}

QString FunctionTableList::typeNamePlural() const
{
    return tr("Interactive Tile Function Table");
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem>;
